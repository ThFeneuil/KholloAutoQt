#ifndef REVIEWDIALOG_H
#define REVIEWDIALOG_H

#include <QDialog>
#include <QSql>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QQueue>
#include "storedData/student.h"

namespace Ui {
class ReviewDialog;
}

class ReviewDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ReviewDialog(QSqlDatabase* db, QWidget *parent = 0);
    ~ReviewDialog();
    bool free_students();

private:
    Ui::ReviewDialog *ui;
    QSqlDatabase* m_db;
    QQueue<Student*> queue_displayedStudents;

};

#endif // REVIEWDIALOG_H
