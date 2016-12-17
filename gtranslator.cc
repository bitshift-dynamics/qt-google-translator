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
