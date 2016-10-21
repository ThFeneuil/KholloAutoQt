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
#include <QSqlError>
#include <QMessageBox>
#include <QTableView>
#include <QTableWidget>
#include <QPainter>
#include <QGraphicsView>
#include <QListWidget>
#include "storedData/student.h"
#include "storedData/subject.h"
#include "storedData/kholleur.h"
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

public slots:
    bool selectStudent(Student* stud = NULL);
    void printKholloscope();
    void openReviewDialog();
    bool detectChangeTab(int index);
    bool update_list(Subject* subj = NULL);
    bool doubleSelectStudent(QListWidgetItem* item);

private:
    Ui::InterfaceDialog *ui;
    QSqlDatabase *m_db;
    QList<Student*> *m_students;
    int m_id_week;
    QDate m_monday;
    DataBase* m_dbase;
    QListWidgetItem* m_doubleSelectedItem;
};

#endif // INTERFACEDIALOG_H
