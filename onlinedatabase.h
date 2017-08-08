#ifndef ONLINEDATABASE_H
#define ONLINEDATABASE_H

#include <QNetworkReply>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QUrl>
#include <QString>
#include <QMessageBox>
#include <QRegExp>
#include <QMap>
#include <QDebug>

//#define sendQuery2(q, r, s) connect(fnc(q), SIGNAL(executed(QList<QMap<QString, QVariant>*>*)), r, SLOT(s))
#define askODB(q, r, s) sendQueryODB(q, r, SLOT(s(ODBRequest*)))
#define progression connect(
#define with(r, s) , SIGNAL(downloadProgress(qint64, qint64)), r, SLOT(s(qint64, qint64)))

const QUrl url_db = QUrl("http://tfeneuilprojects.fr/projects/33_auto_fdb/index.php");
const QString password_db = "123456";

class ODBRequest : public QObject
{
    Q_OBJECT

    public:
        ODBRequest(QNetworkReply* r, const QObject *receiver, const char *method);
        ~ODBRequest();
        QList<QMap<QString, QVariant>*>* result();
        QString lastError();

    signals:
        void executed(ODBRequest* req);
        void downloadProgress(qint64 received, qint64 total);

    public slots:
        void getResult();
        void downloading(qint64 received, qint64 total);

    private:
        QNetworkReply* m_reply;
        QString m_lastError;
        QList<QMap<QString, QVariant>*>* m_result;
};

ODBRequest* sendQueryODB(QString query, const QObject *receiver, const char *method);

#endif // ONLINEDATABASE_H
