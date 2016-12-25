#ifndef STUDENTSMANAGER_H
#define STUDENTSMANAGER_H

#include <QDialog>
#include <QtSql>
#include <QMessageBox>
#include <QQueue>
#include "storedData/student.h"
#include "managers/updatestudentdialog.h"


namespace Ui {
class StudentsManager;
}

class StudentsManager : public QDialog
{
    Q_OBJECT

public:
    explicit StudentsManager(QSqlDatabase *db, QWidget *parent = 0);
    ~StudentsManager();
    bool free_students();

public slots:
    bool update_list();
    bool add_student();
    bool update_student();
    bool delete_student();

private:
    Ui::StudentsManager *ui;
    QSqlDatabase *m_db;
    QQueue<Student*> queue_displayedStudents;
};

#endif // STUDENTSMANAGER_H
