#ifndef KSCOPEMANAGER_H
#define KSCOPEMANAGER_H

#include <QString>
#include <QSql>
#include <QSqlQuery>
#include <QSqlDriver>
#include <QSqlRecord>
#include <QMessageBox>
#include <QFileInfo>
#include <QVariant>

class KScopeManager
{
public:
    enum ActionType {
        Create, Check
    };
    enum DataType {
        Id, Int, Text
    };

    KScopeManager();
    ~KScopeManager();
    bool createFile(QString path);
    bool openFile(QString path);
    bool openLocalhostDB();
    int checkDBStructure(QSqlDatabase* db);
    int tablesStructures(QSqlDatabase* db, QString nameTable, ActionType action);

private:
    QStringList tables;
};

#endif // KSCOPEMANAGER_H
