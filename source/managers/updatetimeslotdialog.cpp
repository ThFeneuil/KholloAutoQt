#include "updatetimeslotdialog.h"
#include "ui_updatetimeslotdialog.h"

UpdateTimeslotDialog::UpdateTimeslotDialog(Timeslot *slot, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::UpdateTimeslotDialog)
{
    ui->setupUi(this);
    m_slot = slot;
    m_monday = m_slot->getDate().addDays(1-m_slot->getDate().dayOfWeek());
    m_temp_slot = new Timeslot();
    m_temp_slot->setDate(m_slot->getDate());
    m_temp_slot->setTime_start(m_slot->getTime_start());
    m_temp_slot->setIsDeleted(m_slot->isDeleted());

    if(m_temp_slot->isDeleted()) {
        ui->days->setCurrentIndex(0);
    } else {
        ui->hours->setTime(m_temp_slot->getTime_start());
        if(m_temp_slot->getDate().dayOfWeek() < 7)
            ui->days->setCurrentIndex(m_temp_slot->getDate().dayOfWeek());
    }

    connect(ui->days, SIGNAL(currentIndexChanged(int)), this, SLOT(updateInputs()));
    connect(ui->pushButton_update, SIGNAL(clicked(bool)), this, SLOT(update_timeslot()));
}

UpdateTimeslotDialog::~UpdateTimeslotDialog()
{
    delete ui;
    delete m_temp_slot;
}

bool UpdateTimeslotDialog::updateInputs() {
    int idx = ui->days->currentIndex();

    if(idx) {
        m_temp_slot->setIsDeleted(false);
        m_temp_slot->setDate(m_monday.addDays(idx-1));
        m_temp_slot->setTime_start(ui->hours->time());

        ui->hours->setEnabled(true);
    } else {
        m_temp_slot->setIsDeleted(true);
        ui->hours->setEnabled(false);
    }

    return true;
}

bool UpdateTimeslotDialog::update_timeslot() {
    int preparation = m_slot->getTime().msecsSinceStartOfDay() - m_slot->getTime_start().msecsSinceStartOfDay();
    int durationKholle = m_slot->getTime_end().msecsSinceStartOfDay() - m_slot->getTime().msecsSinceStartOfDay();

    m_slot->setDate(m_temp_slot->getDate());
    m_slot->setTime_start(ui->hours->time());
    m_slot->setTime(m_slot->getTime_start().addMSecs(preparation));
    m_slot->setTime_end(m_slot->getTime_start().addMSecs(preparation + durationKholle));

    m_slot->setIsDeleted(m_temp_slot->isDeleted());

    accept();
    return true;
}

