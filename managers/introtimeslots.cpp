#include "introtimeslots.h"
#include "ui_introtimeslots.h"

IntroTimeslots::IntroTimeslots(QSqlDatabase *db, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::IntroTimeslots)
{
    ui->setupUi(this);
    m_db = db;

    //Get next monday
    QDate nextMonday = QDate::currentDate();
    while(nextMonday.dayOfWeek() != 1)
        nextMonday = nextMonday.addDays(1);
    ui->dateEdit->setDate(nextMonday);

    connect(ui->OKButton, SIGNAL(clicked()), this, SLOT(openTimeslotsManager()));
}

IntroTimeslots::~IntroTimeslots()
{
    delete ui;
}

void IntroTimeslots::openTimeslotsManager() {
    //Get monday date
    QDate m_date = ui->dateEdit->date();

    //Change date if not a monday
    if(m_date.dayOfWeek() != 1) {
        while(m_date.dayOfWeek() != 1)
            m_date = m_date.addDays(-1);
        QMessageBox::information(this, "Information", "La date n'étant pas à un lundi, la date utilisée sera le lundi de la même semaine.");
    }

    //Open TimeslotsManager with this date and DB connection
    TimeslotsManager manager(m_db, m_date);
    manager.exec();
}
