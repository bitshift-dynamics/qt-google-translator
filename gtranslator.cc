#include "GTranslator.h"

#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>

#include <QtCore/QCoreApplication>
#include <QtCore/QJsonDocument>
#include <QtCore/QMutexLocker>
#include <QtCore/QVariantList>
#include <QtCore/QJsonObject>
#include <QtCore/QEventLoop>
#include <QtCore/QJsonArray>
#include <QtCore/QThread>
#include <QtCore/QFile>


GTranslator::GTranslator(QObject* parent)
    : QTranslator(parent)
{
    langCode = "en";
}

GTranslator::~GTranslator()
{
    for (auto qnam : qnams)
        qnam->deleteLater();
    qnams.clear();
}

void GTranslator::setApiToken(const QString& token)
{
    apiToken = token;
}

void GTranslator::setLanguage(const QString& lang)
{
    langCode = lang;

    qApp->removeTranslator(this);
    qApp->installTranslator(this);
}

void GTranslator::requestLanguageList()
{
    auto qnam = new(std::nothrow) QNetworkAccessManager(this);
    Q_CHECK_PTR(qnam);

    QObject::connect(qnam,
                     &QNetworkAccessManager::finished,
                     [=](QNetworkReply* reply) {
        QHash<QString, QString> languages;

        QVariantList langObjects = QJsonDocument::fromJson(reply->readAll())
                .object().value("data")
                .toObject().value("languages")
                .toArray().toVariantList();

        for (QVariant langObj : langObjects) {
            auto lang = langObj.toMap()["language"].toString().trimmed();
            auto name = QLocale(lang).nativeLanguageName().trimmed();

            if (name.isEmpty() == true)
                continue;

            languages[lang] = name;
        }

        emit languageListChanged(languages);

        reply->deleteLater();
        qnam->deleteLater();
    });

    QString request("https://www.googleapis.com/language/"
                    "translate/v2/languages?key={API_KEY}");
    request.replace("{API_KEY}", apiToken);
    qnam->get(QNetworkRequest(QUrl(request)));
}

QString GTranslator::translate(const char* context,
                                    const char* sourceText,
                                    const char* disambiguation,
                                    int n) const
{
    QMutexLocker locky(&qnamMutex);
    if (cache.contains(langCode) == true
            && cache[langCode].contains(sourceText) == true)
        return cache[langCode][sourceText];
    else
        cache[langCode][sourceText] = sourceText;


    if (qnams.contains(QThread::currentThreadId()) == false) {
        auto qnam = new QNetworkAccessManager;
        QObject::connect(qnam,
                         &QNetworkAccessManager::finished,
                         [=](QNetworkReply* reply) {
            auto data = reply->readAll();
            auto translation = QJsonDocument::fromJson(data)
                    .object().value("data")
                    .toObject().value("translations")
                    .toArray().first()
                    .toObject().value("translatedText")
                    .toString();

            cache[langCode][reply->property("sourceText").toString()]
                    = translation;
            qApp->removeTranslator(const_cast<GTranslator*>(this));
            qApp->installTranslator(const_cast<GTranslator*>(this));

            reply->deleteLater();
        });

        qnams[QThread::currentThreadId()] = qnam;
    }

    QString request("https://www.googleapis.com/language/translate/v2"
                    "?format=text"
                    "&q={SOURCE_TEXT}"
                    "&source={SOURCE_LANG}"
                    "&target={LANGUAGE}"
                    "&key={API_KEY}");
    request.replace("{SOURCE_TEXT}", sourceText);
    request.replace("{SOURCE_LANG}", "de");
    request.replace("{LANGUAGE}", langCode);
    request.replace("{API_KEY}", apiToken);

    auto reply = qnams[QThread::currentThreadId()]
            ->get(QNetworkRequest(QUrl(request)));
    reply->setProperty("sourceText", sourceText);

    return sourceText;
}

void GTranslator::save(const QString& path)
{
    QMutexLocker locky(&qnamMutex);

    // We have to remap cache to a QVariantMap because
    // QHash<QString, QHash<QString, QString>> is not implicitly convertable
    // to QHash<QString, QVariant>.
    QVariantMap data;
    auto languages = cache.keys();
    for (auto language : languages) {
        QVariantMap translations;
        auto sourceTexts = cache[language].keys();
        for (auto sourceText : sourceTexts)
            translations[sourceText] = cache[language][sourceText];

        data[language] = translations;
    }

    // Write the mapped data to the given path.
    QFile output(path);
    if (output.open(QIODevice::WriteOnly) == false) {
        qCritical() << "Failed to open" << path << "for writing.";
        return;
    }

    output.write(QJsonDocument::fromVariant(data).toJson());
    output.close();
}

void GTranslator::load(const QString& path)
{
    QMutexLocker locky(&qnamMutex);

    // Read the translation from the given path.
    QFile input(path);
    if (input.open(QIODevice::ReadOnly) == false) {
        qCritical() << "Failed to open" << path << "for reading.";
        return;
    }

    QByteArray raw = input.readAll();
    input.close();

    // Parse the JSON document.
    QJsonParseError parseError;
    auto document = QJsonDocument::fromJson(raw, &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        qCritical() << "The given file at" << path << "is no valid JSON.";
        return;
    }

    QHash<QString, QHash<QString, QString>> tmpCache;
    auto languages = document.object().keys();
    for (auto language : languages) {
        QHash<QString, QString> translations;
        auto sourceTexts = document.object().value(language).toObject().keys();
        for (auto sourceText : sourceTexts)
            translations[sourceText] = document.object().value(language)
                    .toObject().value(sourceText).toString();

        tmpCache[language] = translations;
    }

    cache = tmpCache;
}
