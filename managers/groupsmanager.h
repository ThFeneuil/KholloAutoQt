#ifndef GROUPSMANAGER_H
#define GROUPSMANAGER_H

#include <QDialog>
#include <QtSql>
#include <QMessageBox>
#include <QQueue>
#include "storedData/group.h"

namespace Ui {
class GroupsManager;
}

class GroupsManager : public QDialog
{
    Q_OBJECT

public:
    explicit GroupsManager(QSqlDatabase *db, QWidget *parent = 0);
    ~GroupsManager();
    bool free_groups();

public slots:
    bool update_list();
    bool add_group();
    bool delete_group();

private:
    Ui::GroupsManager *ui;
    QSqlDatabase *m_db;
    QQueue<Group*> queue_displayedGroups;
};

#endif // GROUPSMANAGER_H
