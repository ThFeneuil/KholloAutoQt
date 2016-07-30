#ifndef INTROTIMESLOTS_H
#define INTROTIMESLOTS_H

#include <QDialog>
#include <QtSql>
#include "managers/timeslotsmanager.h"

namespace Ui {
class IntroTimeslots;
}

class IntroTimeslots : public QDialog
{
    Q_OBJECT

public:
    explicit IntroTimeslots(QSqlDatabase *db, QWidget *parent = 0);
    ~IntroTimeslots();

public slots:
    void openTimeslotsManager();

private:
    Ui::IntroTimeslots *ui;
    QSqlDatabase *m_db;
};

#endif // INTROTIMESLOTS_H
