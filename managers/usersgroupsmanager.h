#ifndef USERSGROUPSMANAGER_H
#define USERSGROUPSMANAGER_H

#include <QDialog>
#include <QtSql>
#include <QMessageBox>
#include <QQueue>
#include <QListWidget>
#include <QListWidgetItem>
#include "storedData/student.h"
#include "storedData/group.h"

namespace Ui {
class usersgroupsmanager;
}

class usersgroupsmanager : public QDialog
{
    Q_OBJECT

public:
    explicit usersgroupsmanager(QSqlDatabase *db, QWidget *parent = 0);
    ~usersgroupsmanager();
    bool update_list_students();
    bool free_students();
    bool free_groups();

public slots:
    bool update_list_groups();
    bool add_group();
    bool remove_group();

private:
    Ui::usersgroupsmanager *ui;
    QSqlDatabase *m_db;
    QQueue<Student*> queue_displayedStudents;
    QQueue<Group*> queue_displayedGroups;
};

#endif // USERSGROUPSMANAGER_H
