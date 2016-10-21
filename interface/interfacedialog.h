/*
 *	File:			(Header) interfaceDialog.h
 *  Comment:        INTERFACE FILE
 *	Description:    Class of the dialog which contains the interface
 *
 */

#ifndef INTERFACEDIALOG_H
#define INTERFACEDIALOG_H

#include <QDialog>
#include <QList>
#include <QSql>
#include <QSqlQuery>
#include <QMessageBox>
#include <QListWidget>
#include "storedData/student.h"
#include "storedData/subject.h"
#include "interface/interfacetab.h"
#include "interface/introinterface.h"
#include "database.h"
#include "printpdf.h"
#include "reviewdialog.h"

class InterfaceTab;

namespace Ui {
class InterfaceDialog;
}

class InterfaceDialog : public QDialog
{
    Q_OBJECT

public:
    explicit InterfaceDialog(QSqlDatabase *db, int id_week, QDate monday, QWidget *parent = 0);
    ~InterfaceDialog();
    bool commuteKholle(Subject* subj, Student* stud1, Student* stud2); // To commute kholles (of a subject) between 2 students

public slots:
    bool selectStudent(Student* stud = NULL); // To select a student, to update the tabs and the khollotable
    void printKholloscope(); // To print the kholloscope of the selected week in a PDF file
    void openReviewDialog(); // To open the review dialog
    bool detectChangeTab(int index); // To update interface after a tab change
    bool update_list(Subject* subj = NULL); // To update students list according a subject
    bool doubleSelectStudent(QListWidgetItem* item); // To detect a double selection (to commute kholles between students)

private:
    Ui::InterfaceDialog *ui; // GUI
    QSqlDatabase *m_db; // SQL database
    DataBase* m_dbase; // Local database
    int m_id_week; // Parity of the week (peer: 1, odd: 2)
    QDate m_monday; // Monday of the selected week
    QList<Student*> *m_students; // Students List
    QListWidgetItem* m_doubleSelectedItem; // Last double-selected item
};

#endif // INTERFACEDIALOG_H
