#include "onlinedatabase.h"

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

ODBRequest* sendQueryODB(QString query, const QObject *receiver, const char *method) {
    const QNetworkRequest request(url_db); //On crée notre requête

    QNetworkAccessManager *m = new QNetworkAccessManager;
    QString test("password="+password_db+"&query="+query+"");
    QNetworkReply *r = m->post(request, test.toLatin1());

    ODBRequest* req = new ODBRequest(r, receiver, method);
    return req;
}
