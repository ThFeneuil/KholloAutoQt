#ifndef DBINTERFACE_H
#define DBINTERFACE_H

/** DBInterface :
 *  Class template for an interface between SQLite Database and code
 *  Each interface (corresponding to a table in DB) will inherit a specific DBInterface
 *  The interface adds the attributes of the table in m_attributes, BUT NOT id !!!!
 *  It must also add itself to DependencySolver's proper_delete() method, for proper deleting **/

#include <functional>
#include <QtSql>
#include "storedData/storeddata.h"

template<class T>
class DBInterface
{
    static_assert(std::is_base_of<StoredData, T>::value, "DBInterface : T not descending from StoredData");

public:
    class DBAttribute {
    public:
        DBAttribute(QString name, QString table, std::function<QVariant(T*)> g, void(T::*s)(QVariant)) {
            attribute_name = name;
            dependency_table = table;
            getter = g;
            setter = s;
        }
        QString attribute_name;
        QString dependency_table;
        std::function<QVariant(T*)> getter;
        std::function<void(T*, QVariant)> setter;
    };

public:
    DBInterface(QSqlDatabase *db) { m_db = db; }
    virtual QString table_name() = 0;
    bool insert(T* elem);
    QQueue<T*> load(QString condition = "");
    void update(T* elem);
    bool exists(int id);
    void remove(int id);
    void remove_dependencies(QString other_table, int id);

protected:
    QList<DBAttribute> m_attributes;

private:
    QSqlDatabase *m_db;
};

#include "dependencysolver.h"
#include "dbinterface_impl.h"

#endif // DBINTERFACE_H
