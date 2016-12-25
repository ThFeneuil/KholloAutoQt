#include "introtimeslots.h"
#include "ui_introtimeslots.h"

IntroTimeslots::IntroTimeslots(QDate *date, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::IntroTimeslots)
{
    ui->setupUi(this);
    m_date = date;

    //Get next monday
    QDate nextMonday = QDate::currentDate();
    while(nextMonday.dayOfWeek() != 1)
        nextMonday = nextMonday.addDays(1);
    ui->dateEdit->setDate(nextMonday);

    connect(this, SIGNAL(accepted()), this, SLOT(save()));
}

IntroTimeslots::~IntroTimeslots()
{
    delete ui;
}

void IntroTimeslots::save() {
    //Get monday date
    QDate date = ui->dateEdit->date();

    //Change date if not a monday
    if(date.dayOfWeek() != 1) {
        while(date.dayOfWeek() != 1)
            date = date.addDays(-1);
        QMessageBox::information(this, "Information", "La date n'étant pas à un lundi, la date utilisée sera le lundi de la même semaine.");
    }

    //Set contents of date pointer to this date
    *m_date = date;
}
