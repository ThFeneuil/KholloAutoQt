#ifndef DBINTERFACE_IMPL_H
#define DBINTERFACE_IMPL_H

#include "dbinterface.h"

template<class T>
bool DBInterface<T>::insert(T *elem) {
    /** Inserts object elem into the DB and sets the id of elem **/
    //Test dependencies ??

    QString req = "INSERT INTO " + this->table_name() + "(";
    QString vals;
    for(DBAttribute attr : m_attributes) {
        req += attr.attribute_name + ", ";
        vals += ":" + attr.attribute_name + ", ";
    }
    req.chop(2);
    vals.chop(2);

    req += ") VALUES(" + vals + ")";

    QSqlQuery query(*m_db);
    query.prepare(req);

    for(DBAttribute attr : m_attributes)
        query.bindValue(":" + attr.attribute_name, attr.getter(elem));

    query.exec();
    ((StoredData*) elem)->setId(query.lastInsertId().toInt());
    return true;
}

template<class T>
QQueue<T*> DBInterface<T>::load(QString condition) {
    /** Loads all objects from table which respect the condition **/
    QQueue<T*> queue;

    QString req = "SELECT id, ";

    for(DBAttribute attr : m_attributes)
        req += attr.attribute_name + ", ";
    req.chop(2);

    req += " FROM " + this->table_name() + " " + condition;

    QSqlQuery query(*m_db);
    query.exec(req);

    while(query.next()) {
        T* t = new T();
        ((StoredData*) t)->setId(query.value(0).toInt());

        int c = 1; //id is in position 0
        for(DBAttribute attr : m_attributes) {
            attr.setter(t, query.value(c));
            c++;
        }

        queue.enqueue(t);
    }

    return queue;
}

template<class T>
void DBInterface<T>::update(T *elem) {
    /** Updates this elem in DB **/

    QString req;
    for(DBAttribute attr : m_attributes)
        req += attr.attribute_name + "=:" + attr.attribute_name + ", ";
    req.chop(2);

    QSqlQuery query(*m_db);
    query.prepare("UPDATE " + this->table_name() + " SET " + req + " WHERE id=:id");
    query.bindValue(":id", ((StoredData*) elem)->getId());

    for(DBAttribute attr : m_attributes)
        query.bindValue(":" + attr.attribute_name, attr.getter(elem));

    query.exec();
}

template<class T>
bool DBInterface<T>::exists(int id) {
    /** Checks if this id is contained in this table **/
    QSqlQuery query(*m_db);
    query.prepare("SELECT * FROM " + this->table_name() + " WHERE id=:id");
    query.bindValue(":id", id);
    query.exec();

    return query.next();
}

template<class T>
void DBInterface<T>::remove(int id) {
    /** PROPERLY removes the element id from this table **/
    QSqlQuery query(*m_db);
    query.prepare("DELETE FROM " + this->table_name() + " WHERE id=:id");
    query.bindValue(":id", id);
    query.exec();

    DependencySolver::proper_delete(m_db, this->table_name(), id);
}

template<class T>
void DBInterface<T>::remove_dependencies(QString other_table, int id) {
    /** Removes all rows that reference row id from other_table **/
    for(DBAttribute attr : m_attributes) {
        if(attr.dependency_table == other_table) {
            QQueue<T*> to_remove = load("WHERE " + attr.attribute_name + "=" + QString::number(id));
            while(!to_remove.isEmpty()) {
                T* t = to_remove.dequeue();
                remove(((StoredData*) t)->getId());
                delete t;
            }
        }
    }
}

#endif // DBINTERFACE_IMPL_H
