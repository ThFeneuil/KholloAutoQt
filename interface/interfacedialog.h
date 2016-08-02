#ifndef INTERFACEDIALOG_H
#define INTERFACEDIALOG_H

#include <QDialog>
#include <QList>
#include "storedData/student.h"
#include "storedData/subject.h"
#include "storedData/kholleur.h"
#include <QSql>
#include <QSqlQuery>
#include <QMessageBox>
#include <QTableView>
#include <QTableWidget>
#include <QPainter>
#include "interface/interfacetab.h"
#include "interface/introinterface.h"
#include "database.h"
#include <QGraphicsView>
#include <QListWidget>

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

public slots:
    bool selectStudent(Student* stud = NULL);

private:
    Ui::InterfaceDialog *ui;
    QSqlDatabase *m_db;
    QList<Student*> *m_students;
    int m_id_week;
    QDate m_monday;
    DataBase* m_dbase;
};

#endif // INTERFACEDIALOG_H
