/*
 *	File:			(Source) introinterface.cpp
 *  Comment:        INTERFACE FILE
 *	Description:    Class of the dialog which asks necessary information for the interface execution
 *      The information is returned thanks to 'id_week' and 'monday' pointors
 */

#include "introinterface.h"
#include "ui_introinterface.h"

IntroInterface::IntroInterface(int *id_week, QDate *monday, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::IntroInterface)
{   /** CONSTRUCTOR **/

    // Initialisation
    ui->setupUi(this); // GUI
    m_id_week = id_week;
    m_monday = monday;

    // Fill the combobox of the week parity
    ui->week->addItem("Semaine paire", 1);
    ui->week->addItem("Semaine impaire", 2);

    // Set the field of the monday of the selected week with the next monday
    QDate nextMonday = QDate::currentDate();
    while(nextMonday.dayOfWeek() != 1)
        nextMonday = nextMonday.addDays(1);
    ui->monday->setDate(nextMonday);

    // Connect to detect when user valids
    connect(this, SIGNAL(accepted()), this, SLOT(save()));
}

IntroInterface::~IntroInterface() {
    /** DESTRUCTOR **/
    delete ui;
}

void IntroInterface::save() {
    /** METHOD TO SAVE INFORMATION AND UPDATE THE POINTORS **/
    // Save the week parity
    *m_id_week = ui->week->currentData().toInt();

    // Check if the entered date is a monday
    QDate monday = ui->monday->date();
    if(monday.dayOfWeek() != 1) {
        while(monday.dayOfWeek() != 1)
            monday = monday.addDays(-1);
        QMessageBox::information(this, "Information", "La date n'étant pas à un lundi, la date enregistrée sera le lundi de la même semaine.");
    }
    // Save the monday of the selected week
    *m_monday = monday;
}
