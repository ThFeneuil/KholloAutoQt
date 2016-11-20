#ifndef UPDATESUBJECTDIALOG_H
#define UPDATESUBJECTDIALOG_H

#include <QDialog>
#include <QtSql>
#include <QMessageBox>
#include "storedData/subject.h"

namespace Ui {
class UpdateSubjectDialog;
}

class UpdateSubjectDialog : public QDialog
{
    Q_OBJECT

public:
    explicit UpdateSubjectDialog(QSqlDatabase *db, Subject *subj, QWidget *parent = 0);
    ~UpdateSubjectDialog();

public slots:
    bool update_subject();

private:
    Ui::UpdateSubjectDialog *ui;
    QSqlDatabase *m_db;
    Subject *m_subject;
};

#endif // UPDATESUBJECTDIALOG_H
