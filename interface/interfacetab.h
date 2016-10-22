/*
 *	File:			(Header) interfacetab.h
 *  Comment:        INTERFACE FILE
 *	Description:    Class of a tab which displays the khollotable for a subject
 */

#ifndef INTERFACETAB_H
#define INTERFACETAB_H

#include <QWidget>
#include <QSql>
#include <QSqlQuery>
#include <QListWidget>
#include "storedData/subject.h"
#include "storedData/kholleur.h"
#include "storedData/student.h"
#include "interface/khollotable.h"
#include "interface/interfacedialog.h"
#include "database.h"

class InterfaceDialog;

namespace Ui {
class InterfaceTab;
}

class InterfaceTab : public QWidget
{
    Q_OBJECT

public:
    explicit InterfaceTab(Subject* subj, int id_week, QDate monday, QSqlDatabase* db, DataBase* dbase, QWidget *parent = 0, InterfaceDialog* interface = NULL);
    // subj: the khollotable of the tab will be displayed with the kholles of this subject
    // id_week and monday : Parity and first day of the selected week
    // db and dbase : SQL database and local database to make the modifications of the kholloscope
    // interface : Interface which call this object
    ~InterfaceTab();
    Subject* getSubject() const; // Getter returning the subject property

public slots:
    bool displayKholleur(); // To update the khollotable according the selected kholleur
    bool selectKholleur(Kholleur* khll); // To select manually a kholleur in the list
    bool selectStudent(Student* stud); // To update the kholleurs list and the khollotable according the selected student

private:
    Ui::InterfaceTab *ui; // GUI
    QSqlDatabase* m_db; // SQL database
    DataBase* m_dbase; // Local database
    Subject* m_subject; // Subject of the tab
    int m_id_week; // Parity of the week (peer: 1, odd: 2)
    QDate m_monday; // Monday of the selected week
    InterfaceDialog* m_interface; // Pointer of the interface which calls the tab
};

#endif // INTERFACETAB_H
