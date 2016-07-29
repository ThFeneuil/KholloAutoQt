#ifndef UPDATETEACHERDIALOG_H
#define UPDATETEACHERDIALOG_H

#include <QDialog>
#include <QtSql>
#include <QMessageBox>
#include <QListWidgetItem>
#include <QQueue>
#include "storedData/teacher.h"
#include "storedData/subject.h"

namespace Ui {
class UpdateTeacherDialog;
}

class UpdateTeacherDialog : public QDialog
{
    Q_OBJECT

public:
    explicit UpdateTeacherDialog(QSqlDatabase *db, Teacher *tcher, QWidget *parent = 0);
    ~UpdateTeacherDialog();

public slots:
    bool update_teacher();

private:
    Ui::UpdateTeacherDialog *ui;
    QSqlDatabase *m_db;
    Teacher *m_teacher;
    QQueue<Subject*> queue_subjects; // Displayed subjects in the comboBox
};

#endif // UPDATETEACHERDIALOG_H
