#ifndef UPDATETIMESLOTDIALOG_H
#define UPDATETIMESLOTDIALOG_H

#include <QDialog>
#include <QDate>
#include "storedData/timeslot.h"

namespace Ui {
class UpdateTimeslotDialog;
}

class UpdateTimeslotDialog : public QDialog
{
    Q_OBJECT

public:
    explicit UpdateTimeslotDialog(Timeslot* slot, QWidget *parent = 0);
    ~UpdateTimeslotDialog();

public slots:
    bool updateInputs();
    bool update_timeslot();

private:
    Ui::UpdateTimeslotDialog *ui;
    Timeslot* m_slot;
    Timeslot* m_temp_slot;
    QDate m_monday;
};

#endif // UPDATETIMESLOTDIALOG_H
