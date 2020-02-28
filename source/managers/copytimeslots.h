#ifndef COPYTIMESLOTS_H
#define COPYTIMESLOTS_H

#include <QDialog>
#include <QtSql>
#include <QMessageBox>

namespace Ui {
class CopyTimeslots;
}

class CopyTimeslots : public QDialog
{
    Q_OBJECT

public:
    explicit CopyTimeslots(QSqlDatabase *db, QDate date, bool all = true, int id_kholleur = 0, QWidget *parent = 0);
    ~CopyTimeslots();

public slots:
    void copy();

private:
    Ui::CopyTimeslots *ui;
    QSqlDatabase *m_db;
    QDate m_date;
    bool m_all;
    int m_id_kholleur;
};

#endif // COPYTIMESLOTS_H
