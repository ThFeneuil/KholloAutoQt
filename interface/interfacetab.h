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
#include "database.h"
#include "interface/interfacedialog.h"

class InterfaceDialog;

namespace Ui {
class InterfaceTab;
}

class InterfaceTab : public QWidget
{
    Q_OBJECT

public:
    explicit InterfaceTab(Subject* subj, int id_week, QDate monday, QSqlDatabase* db, DataBase* dbase, QWidget *parent = 0, InterfaceDialog* interface = NULL);
    ~InterfaceTab();

public slots:
    bool displayTeacher();
    bool selectStudent(Student* stud);

private:
    Ui::InterfaceTab *ui;
    QSqlDatabase* m_db;
    Subject* m_subject;
    int m_id_week;
    QDate m_monday;
    DataBase* m_dbase;
    InterfaceDialog* m_interface;
};

#endif // INTERFACETAB_H
