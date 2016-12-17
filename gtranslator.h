#ifndef GTranslator_H
#define GTranslator_H

#include <QtCore/QTranslator>
#include <QtCore/QHash>

class QNetworkAccessManager;
class QNetworkReply;


class GTranslator : public QTranslator
{
    Q_OBJECT

public:
    explicit GTranslator(QObject* parent = nullptr);
    virtual ~GTranslator();

    void setApiToken(const QString& token);

    QString translate(const char* context,
                      const char* sourceText,
                      const char* disambiguation,
                      int n) const;

public slots:
    void setLanguage(const QString& lang);
    void requestLanguageList();

signals:
    void languageListChanged(const QHash<QString, QString>& languages);

private:
    mutable QHash<QString, QHash<QString, QString>> cache;
    mutable QHash<Qt::HANDLE, QNetworkAccessManager*> qnams;
    mutable QMutex qnamMutex;
    QString langCode;
    QString apiToken;
};

#endif // GTranslator_H
