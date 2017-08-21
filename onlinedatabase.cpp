#include "onlinedatabase.h"

/// TO PREPARE A QUERY

ODBSqlQuery::ODBSqlQuery(QUrl url_db, QString password_db, const QObject *receiver, const char *method) {
    m_receiver = receiver;
    m_method = method;
    m_url_db = url_db;
    m_password_db = password_db;
}

ODBSqlQuery::~ODBSqlQuery() {

}

bool ODBSqlQuery::prepare(QString query) {
    m_query = query;
    return true;
}

bool ODBSqlQuery::bindValue(QString key, QVariant value) {
    if(key.length() > 0 && key[0] == ':')
        key = key.right(key.length()-1);
    m_data.insert(key, value);
    return true;
}

ODBRequest *ODBSqlQuery::exec() {
    const QNetworkRequest request(m_url_db); //On crée notre requête

    QNetworkAccessManager *m = new QNetworkAccessManager;
    QHttpMultiPart *multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);
    QMapIterator<QString, QVariant> iData(m_data);
    while(iData.hasNext()) {
        iData.next();

        QHttpPart textPart;
        textPart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"$"+iData.key()+"\""));
        textPart.setBody(iData.value().toString().toLatin1());
        multiPart->append(textPart);
    }
    QHttpPart passwordPart;
    passwordPart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"#password\""));
    passwordPart.setBody(m_password_db.toLatin1());
    multiPart->append(passwordPart);
    QHttpPart queryPart;
    queryPart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"#query\""));
    queryPart.setBody(m_query.toLatin1());
    multiPart->append(queryPart);

    QNetworkReply *r = m->post(request, multiPart);

    m_data.clear();
    m_query.clear();

    ODBRequest* req = new ODBRequest(r, m_receiver, m_method);
    return req;
}

ODBRequest* ODBSqlQuery::exec(QString query) {
    prepare(query);
    return exec();
}

ODBRequest* sendQueryODB(QString query, const QObject *receiver, const char *method) {
    ODBSqlQuery q(DEFAULT INTO(receiver, method));
    return q.exec(query);
}

/// TO GET THE RESULT OF A QUERY

ODBRequest::ODBRequest(QNetworkReply* r, const QObject *receiver, const char *method) :
    QObject()
{
    m_reply = r;
    m_lastError = "";
    m_result = NULL;

    connect(this, SIGNAL(executed(ODBRequest*)), receiver, method);
    connect(r, SIGNAL(finished()), this, SLOT(getResult()));
    connect(r, SIGNAL(downloadProgress(qint64, qint64)), this, SLOT(downloading(qint64, qint64)));
}

ODBRequest::~ODBRequest() {
    m_lastError = "";
    delete m_reply;
    if(m_result != NULL)
        for(int i=0; i<m_result->count(); i++)
            delete m_result->at(i);
    delete m_result;
}

void ODBRequest::downloading(qint64 received, qint64 total) {
    downloadProgress(received, total);
}

void ODBRequest::getResult() {
    QNetworkReply* r = m_reply;
    QString str(r->readAll());
    r->deleteLater();

    if(r->error()) {
        m_lastError = "Error during the loading. Please check your connection.<br />Error : " + r->errorString();
        executed(this);
        return;
    }

    int pos = 0;
    m_lastError = "";

    QRegExp regHeaders("<h>(.*)</h>");
    regHeaders.setMinimal(true);
    QStringList headers;

    while (regHeaders.indexIn(str, pos) != -1) {
        headers << regHeaders.cap(1);
        pos += regHeaders.matchedLength();
    }
    int nbHeaders = headers.count();
    if(nbHeaders == 0) {
        m_lastError = str;
        m_result = new QList<QMap<QString, QVariant>*>;
        executed(this);
        return;
    }

    QRegExp regData("<d>(.*)</d>");
    regData.setMinimal(true);
    QList<QMap<QString, QVariant>*>* rows = new QList<QMap<QString, QVariant>*>;
    int numData = 0;

    while (regData.indexIn(str, pos) != -1) {
        QMap<QString, QVariant>* row = new QMap<QString, QVariant>;
        row->insert(headers[0], QVariant(regData.cap(1)));
        pos += regData.matchedLength();
        numData = 1;
        while(numData<nbHeaders && (pos = regData.indexIn(str, pos)) != -1) {
            row->insert(headers[numData], QVariant(regData.cap(1)));
            pos += regData.matchedLength();
            numData++;
        }
        rows->append(row);
    }

    m_result = rows;
    executed(this);
}

QList<QMap<QString, QVariant>*>* ODBRequest::result() {
    return m_result;
}

QString ODBRequest::lastError() {
    return m_lastError;
}
