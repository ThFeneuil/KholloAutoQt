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
    explicit InterfaceTab(Subject* subj, QSqlDatabase* db, QWidget *parent = 0);
    ~InterfaceTab();

public slots:
    bool displayTeacher(QListWidgetItem *item);
    bool selectStudent(Student* stud);

private:
    Ui::InterfaceTab *ui;
    QSqlDatabase* m_db;
    Subject* m_subject;
};

#endif // INTERFACETAB_H
