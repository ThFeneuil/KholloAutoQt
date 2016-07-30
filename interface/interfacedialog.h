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
#include "interface/table.h"
//#include "interface/khollotable.h"
#include "interface/interfacetab.h"
#include "interface/introinterface.h"
#include "interface/database.h"
#include <QGraphicsView>
#include <QListWidget>

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
    bool selectStudent();

private:
    Ui::InterfaceDialog *ui;
    QSqlDatabase *m_db;
    QList<Student*> *m_students;
    int m_id_week;
    QDate m_monday;
};

#endif // INTERFACEDIALOG_H
