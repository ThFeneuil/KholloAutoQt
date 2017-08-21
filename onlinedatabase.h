/************************************************************************
 *          EXAMPLES FOR USING ONLINE DATABASE
 *
 *  obj : object which is call at the end of request download
 *  slt : slot (of obj) which is call at the end of request download
 *          the argument of slt is necessary "ODBRequest*"
 *
 *  INITIALISATION REQUEST :
 *      - On the default server :
 *          ODBSqlQuery query(DEFAUT INTO(obj, slt));
 *      - On an other server :
 *          ODBSqlQuery query(FROM("url script PHP", "password") INTO(obj, slt));
 *
 *   EXECUTE REQUEST
 *      - Simply request:
 *          query.exec("THE QUERY...");
 *      - Prepared request:
 *          query.prepare("SELECT id FROM spark_timeslots WHERE class=:class AND date>=:start;");
 *          query.bindValue(":class", "VALUE OF class");
 *          query.bindValue(":start", "VALUE OF start");
 *          query.exec();
 *
 *   Possible shortcut for default server:
 *      askODB("THE QUERY", obj, slt);
 *
 ************************************************************************/


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
#include <QHttpPart>

#define askODB(q, r, s) sendQueryODB(q, r, SLOT(s(ODBRequest*)))
#define progression connect(
#define with(r, s) , SIGNAL(downloadProgress(qint64, qint64)), r, SLOT(s(qint64, qint64)))
#define INTO(r,s) r, SLOT(s(ODBRequest*))
#define DEFAULT default_url_db, default_password_db,
#define FROM(s, p) QUrl(s), p,

const QUrl default_url_db = QUrl("http://tfeneuilprojects.fr/odb");
const QString default_password_db = "LE MOT DE PASSE !";

/// TO GET THE RESULT OF A QUERY

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

/// TO MAKE A QUERY

class ODBSqlQuery
{
    public:
        ODBSqlQuery(QUrl url_db, QString password_db, const QObject *receiver, const char *method);
        ~ODBSqlQuery();
        bool prepare(QString query);
        bool bindValue(QString key, QVariant value);
        ODBRequest* exec();
        ODBRequest* exec(QString query);

    private:
        const QObject *m_receiver;
        const char *m_method;
        QString m_query;
        QMap<QString, QVariant> m_data;
        QUrl m_url_db;
        QString m_password_db;
};

// Shortcut...
ODBRequest* sendQueryODB(QString query, const QObject *receiver, const char *method);

#endif // ONLINEDATABASE_H
