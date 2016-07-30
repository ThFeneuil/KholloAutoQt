#ifndef STOREDDATUM_H
#define STOREDDATUM_H

#include <QSqlQuery>
#include <QSqlRecord>
#include <QMap>

class StDatum
{
    enum Type {INT, DATE, TIME, DATETIME, STRING};

    public:
        StDatum();
        ~StDatum();

        //int hydrate(QSqlQuery query);

    private:
        //QMap<Type, void> links;
};

#endif // STOREDDATUM_H
