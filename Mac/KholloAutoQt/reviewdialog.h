#ifndef REVIEWDIALOG_H
#define REVIEWDIALOG_H

#include <QDialog>
#include <QSql>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QQueue>
#include <QListWidget>
#include <QComboBox>
#include <QDebug>
#include "storedData/student.h"
#include "storedData/kholleur.h"
#include "notepad.h"

namespace Ui {
class ReviewDialog;
}

class ReviewDialog : public QDialog
{
    Q_OBJECT

public:
    enum SelectionMethod { FROMSTUDENTS, FROMKHOLLEURS };

    explicit ReviewDialog(QSqlDatabase* db, QWidget *parent = NULL, SelectionMethod method=FROMSTUDENTS, Student* stdnt=NULL, Kholleur* khll=NULL);
    ~ReviewDialog();
    QListWidget* listStudents();
    QListWidget* listKholleurs();
    void updateStudents(QListWidget* list);
    void updateKholeurs(QListWidget* list);
    int nbKholles(Student* stdnt, Kholleur* khll);

public slots:
    void updateCol1();
    void updateCol2();
    void updateInfoArea();
    bool selectStudent(Student* stdnt);
    bool selectKholleur(Kholleur* khll);

private:
    Ui::ReviewDialog *ui;
    QSqlDatabase* m_db;
    QList<Student*>* m_students;
    QList<Kholleur*>* m_kholleurs;
    QAction* m_shortcutNotepad;
};

#endif // REVIEWDIALOG_H
