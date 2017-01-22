#ifndef INTROTIMESLOTS_H
#define INTROTIMESLOTS_H

#include <QDialog>
#include "managers/timeslotsmanager.h"

namespace Ui {
class IntroTimeslots;
}

class IntroTimeslots : public QDialog
{
    Q_OBJECT

public:
    explicit IntroTimeslots(QDate *date, QWidget *parent = 0);
    ~IntroTimeslots();

public slots:
    void save();

private:
    Ui::IntroTimeslots *ui;
    QDate *m_date;
};

#endif // INTROTIMESLOTS_H
