#ifndef UPDATESTUDENTDIALOG_H
#define UPDATESTUDENTDIALOG_H

#include <QDialog>
#include <QtSql>
#include <QMessageBox>
#include "storedData/student.h"
#include "DB/studentsdbinterface.h"

namespace Ui {
class UpdateStudentDialog;
}

class UpdateStudentDialog : public QDialog
{
    Q_OBJECT

public:
    explicit UpdateStudentDialog(QSqlDatabase *db, Student *stud, QWidget *parent = 0);
    ~UpdateStudentDialog();

public slots:
    bool update_student();

private:
    Ui::UpdateStudentDialog *ui;
    QSqlDatabase *m_db;
    Student *m_student;
};

#endif // UPDATESTUDENTDIALOG_H
