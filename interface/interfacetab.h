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

namespace Ui {
class InterfaceTab;
}

class InterfaceTab : public QWidget
{
    Q_OBJECT

public:
    explicit InterfaceTab(Subject* subj, int id_week, QDate monday, QSqlDatabase* db, QWidget *parent = 0);
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
};

#endif // INTERFACETAB_H
