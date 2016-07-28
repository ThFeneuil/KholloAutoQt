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

namespace Ui {
class InterfaceDialog;
}

class InterfaceDialog : public QDialog
{
    Q_OBJECT

public:
    explicit InterfaceDialog(QSqlDatabase *db, QWidget *parent = 0);
    ~InterfaceDialog();

public slots:


private:
    Ui::InterfaceDialog *ui;
    QSqlDatabase *m_db;
    QList<Student*> *m_students;
};

#endif // INTERFACEDIALOG_H
