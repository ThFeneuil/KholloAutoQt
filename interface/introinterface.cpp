#include "introinterface.h"
#include "ui_introinterface.h"

IntroInterface::IntroInterface(int *id_week, QDate *monday, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::IntroInterface)
{
    ui->setupUi(this);
    m_id_week = id_week;
    m_monday = monday;

    QDate nextMonday = QDate::currentDate();
    while(nextMonday.dayOfWeek() != 1)
        nextMonday = nextMonday.addDays(1);
    ui->monday->setDate(nextMonday);
}

IntroInterface::~IntroInterface()
{
    if(result() == IntroInterface::Accepted) {
        *m_id_week = ui->week->currentData().toInt();

        QDate monday = ui->monday->date();
        if(monday.dayOfWeek() != 1) {
            while(monday.dayOfWeek() != 1)
                monday.addDays(-1);
            QMessageBox::information(this, "Information", "La date ne correspondant pas à un Lundi, la date sélectionnée correspond au lundi de la même semaine.");
        }
        *m_monday = monday;
    }

    delete ui;


}
