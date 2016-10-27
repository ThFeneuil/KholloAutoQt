#include "reviewdialog.h"
#include "ui_reviewdialog.h"

ReviewDialog::ReviewDialog(QSqlDatabase *db, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ReviewDialog)
{
    // Initialisation
    ui->setupUi(this);
    m_db = db;

    // Initialisation ComboBox
    ui->comboBox_select->addItem("Étudiant", 0);
    ui->comboBox_select->addItem("Kholleur", 1);
    connect(ui->comboBox_select, SIGNAL(currentIndexChanged(int)), this, SLOT(updateCol1()));

    // Initialisation Lists
    ui->listCol1->clear();
    connect(ui->listCol1, SIGNAL(itemSelectionChanged()), this, SLOT(updateCol2()));
    ui->listCol2->clear();
    connect(ui->listCol2, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(updateInfoArea()));
    updateCol1();
}

ReviewDialog::~ReviewDialog() {
    delete ui;
    free_students();
}

bool ReviewDialog::free_students() {
    /** To free memories with students **/
    while (!queue_displayedStudents.isEmpty())
        delete queue_displayedStudents.dequeue();
    return true;
}

void ReviewDialog::updateCol1() {
    // Clear the widgets
    ui->listCol1->clear();
    ui->listCol2->clear();
    ui->infoKholles->setText("");

    // Fill the first column
    if(ui->comboBox_select->currentData().toInt())
        updateKholeurs(ui->listCol1);
    else
        updateStudents(ui->listCol1);
}

void ReviewDialog::updateCol2() {
    // Clear the widgets
    ui->listCol2->clear();
    ui->infoKholles->setText("");

    // Fill the second column
    if(ui->comboBox_select->currentData().toInt())
        updateStudents(ui->listCol2);
    else
        updateKholeurs(ui->listCol2);
}

void ReviewDialog::updateStudents(QListWidget* list) {
    free_students();

    // Make the request
    QSqlQuery query(*m_db);
    QSqlQuery qKholles(*m_db);
    query.exec("SELECT id, name, first_name, email FROM tau_users ORDER BY name, first_name");

    // Treat the request
    while (query.next()) {
        // Get the student
        Student* stdnt = new Student();
        stdnt->setId(query.value(0).toInt());
        stdnt->setName(query.value(1).toString());
        stdnt->setFirst_name(query.value(2).toString());
        stdnt->setEmail(query.value(3).toString());

        if(ui->comboBox_select->currentData().toInt() == 1) {
            // If it is to fill the 2nd column
            // Get the number of kholles
            qKholles.prepare("SELECT COUNT(*) FROM tau_kholles "
                             "WHERE id_users = :id_students AND id_timeslots IN "
                                "(SELECT id FROM tau_timeslots WHERE id_kholleurs = :id_kholleurs)");
            qKholles.bindValue(":id_students", stdnt->getId());
            qKholles.bindValue(":id_kholleurs", ((Kholleur*) ui->listCol1->currentItem()->data(Qt::UserRole).toULongLong())->getId());
            qKholles.exec();

            // Display the number of kholles
            if(qKholles.next()) {
                QListWidgetItem *item = new QListWidgetItem("["+qKholles.value(0).toString()+"] " + stdnt->getName() + " " + stdnt->getFirst_name(), list);
                item->setData(Qt::UserRole, (qulonglong) stdnt);
            } else {
                QListWidgetItem *item = new QListWidgetItem("[?] "+stdnt->getName() + " " + stdnt->getFirst_name(), list);
                item->setData(Qt::UserRole, (qulonglong) stdnt);
            }
        } else {
            // If it is to fill the 1st column
            QListWidgetItem *item = new QListWidgetItem(stdnt->getName() + " " + stdnt->getFirst_name(), list);
            item->setData(Qt::UserRole, (qulonglong) stdnt);
        }
        queue_displayedStudents.enqueue(stdnt);
    }
}

bool ReviewDialog::free_kholleurs() {
    /** To free memories with kholleurs **/
    while (!queue_displayedKholleurs.isEmpty())
        delete queue_displayedKholleurs.dequeue();
    return true;
}

void ReviewDialog::updateKholeurs(QListWidget* list) {
    free_kholleurs();

    // Make the request
    QSqlQuery query(*m_db);
    QSqlQuery qKholles(*m_db);
    query.exec("SELECT id, name, id_subjects, duration, preparation, pupils FROM tau_kholleurs ORDER BY name");

    // Treat the request & Display the kholleurs
    while (query.next()) {
        // Get the kholleur
        Kholleur* khll = new Kholleur();
        khll->setId(query.value(0).toInt());
        khll->setName(query.value(1).toString());
        khll->setId_subjects(query.value(2).toInt());
        khll->setDuration(query.value(3).toInt());
        khll->setPreparation(query.value(4).toInt());
        khll->setPupils(query.value(5).toInt());

        if(ui->comboBox_select->currentData().toInt() == 0) {
            // If it is to fill the 2nd column
            // Get the number of kholles
            qKholles.prepare("SELECT COUNT(*) FROM tau_kholles "
                             "WHERE id_users = :id_students AND id_timeslots IN "
                                "(SELECT id FROM tau_timeslots WHERE id_kholleurs = :id_kholleurs)");
            qKholles.bindValue(":id_students", ((Student*) ui->listCol1->currentItem()->data(Qt::UserRole).toULongLong())->getId());
            qKholles.bindValue(":id_kholleurs", khll->getId());
            qKholles.exec();

            // Display the number of kholles
            if(qKholles.next()) {
                QListWidgetItem *item = new QListWidgetItem("["+qKholles.value(0).toString()+"] "+khll->getName(), list);
                item->setData(Qt::UserRole, (qulonglong) khll);
            } else {
                QListWidgetItem *item = new QListWidgetItem("[?] "+khll->getName(), list);
                item->setData(Qt::UserRole, (qulonglong) khll);
            }
        } else {
            // If it is to fill the 1st column
            QListWidgetItem *item = new QListWidgetItem(khll->getName(), list);
            item->setData(Qt::UserRole, (qulonglong) khll);
        }
        queue_displayedKholleurs.enqueue(khll);
    }
}

void ReviewDialog::updateInfoArea() {
    // Get the student and the kholleur
    Kholleur* khll = NULL;
    Student* stud = NULL;
    if(ui->comboBox_select->currentData().toInt() == 0) {
        stud = (Student*) ui->listCol1->currentItem()->data(Qt::UserRole).toULongLong();
        khll = (Kholleur*) ui->listCol2->currentItem()->data(Qt::UserRole).toULongLong();
    } else {
        stud = (Student*) ui->listCol2->currentItem()->data(Qt::UserRole).toULongLong();
        khll = (Kholleur*) ui->listCol1->currentItem()->data(Qt::UserRole).toULongLong();
    }

    QString text = "";
    text += "<strong>Étudiant :</strong> " + stud->getName() + " " + stud->getFirst_name() + "<br />";
    text += "<strong>Kholleur :</strong> " + khll->getName() + "<br />";

    // Get the list of kholles
    QSqlQuery qKholles(*m_db);
    QSqlQuery qTimeslots(*m_db);
    qKholles.prepare("SELECT id, id_users, id_timeslots FROM tau_kholles "
                     "WHERE id_users = :id_students AND id_timeslots IN "
                        "(SELECT id FROM tau_timeslots WHERE id_kholleurs = :id_kholleurs)");
    qKholles.bindValue(":id_students", stud->getId());
    qKholles.bindValue(":id_kholleurs", khll->getId());
    qKholles.exec();

    QString textKholles = "";
    int numKholle = 0;
    while(qKholles.next()) {
        numKholle++;
        // Get the kholle
        Kholle* klle = new Kholle();
        klle->setId(qKholles.value(0).toInt());
        klle->setId_students(qKholles.value(1).toInt());
        klle->setId_timeslots(qKholles.value(2).toInt());

        // Get the timeslot of this kholle
        qTimeslots.prepare("SELECT id, time_start, time, time_end, id_kholleurs, date, pupils FROM tau_timeslots WHERE id=:id_timeslots");
        qTimeslots.bindValue(":id_timeslots", klle->getId_timeslots());
        qTimeslots.exec();
        if(qTimeslots.next()) {
            Timeslot* ts = new Timeslot();
            ts->setId(qTimeslots.value(0).toInt());
            ts->setTime_start(qTimeslots.value(1).toTime());
            ts->setTime(qTimeslots.value(2).toTime());
            ts->setTime_end(qTimeslots.value(3).toTime());
            ts->setId_kholleurs(qTimeslots.value(4).toInt());
            ts->setDate(qTimeslots.value(5).toDate());
            ts->setPupils(qTimeslots.value(6).toInt());
            klle->setTimeslot(ts);
        } else
            continue;

        // Display the information
        textKholles += "<br />";
        textKholles += "<strong>Kholle " + QString::number(numKholle) + " :</strong><br />";
        textKholles += "Le " + klle->timeslot()->getDate().toString("dd/MM/yyyy") + " à " + klle->timeslot()->getTime().toString("hh:mm") + "<br />";
    }

    text += "<br />Nombre de kholles : " + QString::number(numKholle) + "<br />" + textKholles;

    // Update the widgets
    ui->infoKholles->setText(text);
}
