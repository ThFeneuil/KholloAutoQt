#ifndef REVIEWDIALOG_H
#define REVIEWDIALOG_H

#include <QDialog>
#include <QSql>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QQueue>
#include <QListWidget>
#include <QComboBox>
#include "storedData/student.h"
#include "storedData/kholleur.h"

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
    bool free_kholleurs();
    void updateStudents(QListWidget* list);
    void updateKholeurs(QListWidget* list);

public slots:
    void updateCol1();
    void updateCol2();
    void updateInfoArea();

private:
    Ui::ReviewDialog *ui;
    QSqlDatabase* m_db;
    QQueue<Student*> queue_displayedStudents;
    QQueue<Kholleur*> queue_displayedKholleurs;

};

#endif // REVIEWDIALOG_H
