#include "updatetimeslotdialog.h"
#include "ui_updatetimeslotdialog.h"

UpdateTimeslotDialog::UpdateTimeslotDialog(Timeslot *slot, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::UpdateTimeslotDialog)
{
    ui->setupUi(this);
    m_slot = slot;
    m_monday = m_slot->getDate().addDays(1-m_temp_slot->getDate().dayOfWeek());
    if(m_slot) {
        m_temp_slot = new Timeslot();
        m_temp_slot->setDate(m_slot->getDate());
        m_temp_slot->setTime_start(m_slot->getTime_start());
    } else {
        m_temp_slot = NULL;
    }

    if(! m_temp_slot) {
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

    if(m_temp_slot)
        delete m_temp_slot;
}

bool UpdateTimeslotDialog::updateInputs() {
    int idx = ui->days->currentIndex();

    if(idx) {
        if(! m_temp_slot)
            m_temp_slot = new Timeslot();
        m_temp_slot->setDate(m_monday.addDays(idx-1));
        m_temp_slot->setTime_start(ui->hours->time());

        ui->hours->setEnabled(true);
    } else {
        if(m_temp_slot)
            delete m_temp_slot;

        ui->hours->setEnabled(false);
    }

    return true;
}

bool UpdateTimeslotDialog::update_timeslot() {
    int preparation = m_temp_slot->getTime().msecsSinceStartOfDay() - m_temp_slot->getTime_start().msecsSinceStartOfDay();
    int durationKholle = m_temp_slot->getTime_end().msecsSinceStartOfDay() - m_temp_slot->getTime().msecsSinceStartOfDay();

    m_slot->setDate(m_temp_slot->getDate());
    m_slot->setTime_start(m_temp_slot->getTime_start());
    m_slot->setTime(m_slot->getTime().addMSecs(preparation));
    m_slot->setTime_end(m_slot->getTime().addMSecs(preparation + durationKholle));

    accept();

    return true;
}

