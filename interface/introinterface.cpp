#include "introinterface.h"
#include "ui_introinterface.h"

IntroInterface::IntroInterface(int *id_week, QDate *monday, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::IntroInterface)
{
    ui->setupUi(this);
    m_id_week = id_week;
    m_monday = monday;

    ui->week->addItem("Semaine paire", 1);
    ui->week->addItem("Semaine impaire", 2);

    QDate nextMonday = QDate::currentDate();
    while(nextMonday.dayOfWeek() != 1)
        nextMonday = nextMonday.addDays(1);
    ui->monday->setDate(nextMonday);

    connect(this, SIGNAL(accepted()), this, SLOT(save()));
}

IntroInterface::~IntroInterface() {
    delete ui;
}

void IntroInterface::save() {
    *m_id_week = ui->week->currentData().toInt();

    QDate monday = ui->monday->date();
    if(monday.dayOfWeek() != 1) {
        while(monday.dayOfWeek() != 1)
            monday = monday.addDays(-1);
        QMessageBox::information(this, "Information", "La date n'étant pas à un lundi, la date enregistrée sera le lundi de la même semaine.");
    }
    *m_monday = monday;
}
