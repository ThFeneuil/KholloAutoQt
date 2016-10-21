#include "lastchanges.h"
#include "ui_lastchanges.h"

LastChanges::LastChanges(QSqlDatabase *db, QDate *monday, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LastChanges)
{
    ui->setupUi(this);

    //QMessageBox::information(NULL, "INFO", monday->toString("yyyy-MM-dd"));

    // Initialisation
    m_db = db;
    m_monday = monday;
    days << "" << "Lundi" << "Mardi" << "Mercredi" << "Jeudi" << "Vendredi" << "Samedi";

    QSqlQuery query(*m_db);
    query.exec("SELECT K.id, K.name, K.id_subjects, K.duration, K.preparation, K.pupils, S.shortName "
               "FROM tau_kholleurs AS K "
               "LEFT OUTER JOIN tau_subjects AS S "
                  "ON K.id_subjects = S.id "
               "ORDER BY K.name");

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
        // Display the kholleur
        QString subject = query.value(6).toString();
        if(subject != "")
            subject = " (" + subject + ") ";
        ui->comboBox_kholleurs->addItem(khll->getName() + subject, (qulonglong) khll);
        queue_displayedKholleurs.enqueue(khll);
    }

    connect(ui->comboBox_kholleurs, SIGNAL(currentIndexChanged(int)), this, SLOT(update_timeslots()));
    connect(ui->list_timeslots, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(change_timeslots(QListWidgetItem*)));
    update_timeslots();
}

LastChanges::~LastChanges()
{
    delete ui;
    free_kholleurs();
}

bool LastChanges::free_kholleurs() {
    /** To free memories with kholleurs **/
    while (!queue_displayedKholleurs.isEmpty())
        delete queue_displayedKholleurs.dequeue();
    return true;
}

bool LastChanges::free_timeslots() {
    /** To free memories with kholleurs **/
    while (!queue_displayedTimeslots.isEmpty())
        delete queue_displayedTimeslots.dequeue();
    return true;
}

bool LastChanges::free_students() {
    /** To free memories with kholleurs **/
    while (!queue_displayedStudents.isEmpty())
        delete queue_displayedStudents.dequeue();
    return true;
}

bool LastChanges::change_timeslots(QListWidgetItem *item) {
    Timeslot* ts = (Timeslot*) item->data(Qt::UserRole).toLongLong();
    UpdateTimeslotDialog dialog(ts);
    dialog.exec();

    return true;
}

bool LastChanges::update_timeslots() {
    free_timeslots();
    ui->list_timeslots->clear();
    Kholleur* khll = (Kholleur*) ui->comboBox_kholleurs->currentData().toLongLong();

    //QMessageBox::information(NULL, "INFO", m_monday->toString("yyyy-MM-dd"));

    // Get the list of timeslots
    QSqlQuery query(*m_db);
    query.prepare("SELECT id, time_start, time, time_end, id_kholleurs, date, pupils FROM tau_timeslots "
                  "WHERE id_kholleurs=:id_kholleurs AND date>=:monday_date AND date<=:sunday_date "
                  "ORDER BY date, time");
    query.bindValue(":id_kholleurs", khll->getId());
    query.bindValue(":monday_date", m_monday->toString("yyyy-MM-dd"));
    query.bindValue(":sunday_date", m_monday->addDays(6).toString("yyyy-MM-dd"));
    query.exec();

    //Treat
    while(query.next()) {
        Timeslot* ts = new Timeslot();
        ts->setId(query.value(0).toInt());
        ts->setTime_start(query.value(1).toTime());
        ts->setTime(query.value(2).toTime());
        ts->setTime_end(query.value(3).toTime());
        ts->setId_kholleurs(query.value(4).toInt());
        ts->setDate(query.value(5).toDate());
        ts->setPupils(query.value(6).toInt());
        queue_displayedTimeslots.enqueue(ts);

        QListWidgetItem *item = new QListWidgetItem(days[ts->getDate().dayOfWeek()] + " : "
                + ts->getTime_start().toString("HH:mm") + " >> " + ts->getTime().toString("HH:mm") + " >> " + ts->getTime_end().toString("HH:mm")
                + ", " + QString::number(ts->getPupils()) + (ts->getPupils() <= 1 ? " élève" : " élèves"), ui->list_timeslots);
        item->setData(Qt::UserRole, (qulonglong) ts);
    }

    return true;
}

bool LastChanges::update_students() {
    free_students();
    ui->list_students->clear();
//    Kholleur* khll = (Kholleur*) ui->comboBox_kholleurs->currentData().toLongLong();
/*
    // Get the list of timeslots
    QSqlQuery query(*m_db);
    query.prepare("SELECT id, time_start, time, time_end, id_kholleurs, date, pupils FROM tau_timeslots "
                  "WHERE id_kholleurs=:id_kholleurs AND date>=:monday_date AND date<=:sunday_date "
                  "ORDER BY date, time");
    query.bindValue(":id_kholleurs", khll->getId());
    query.bindValue(":monday_date", m_monday->toString("yyyy-MM-dd"));
    query.bindValue(":sunday_date", m_monday->addDays(6).toString("yyyy-MM-dd"));
    query.exec();

    QSqlQuery query(*m_db);
    query.exec("SELECT id, name, first_name, email FROM tau_users AS S"
               "JOIN tau_kholles ON "
               "ORDER BY name, first_name");

    // Treat the request
    while (query.next()) {
        Student* stdnt = new Student();
        stdnt->setId(query.value(0).toInt());
        stdnt->setName(query.value(1).toString());
        stdnt->setFirst_name(query.value(2).toString());
        stdnt->setEmail(query.value(3).toString());
        QListWidgetItem *item = new QListWidgetItem(stdnt->getName() + ", " + stdnt->getFirst_name() + ", " + stdnt->getEmail(), ui->list_students);
        item->setData(Qt::UserRole, (qulonglong) stdnt);
        queue_displayedStudents.enqueue(stdnt);
    }

    //Treat
    while(query.next()) {
        Timeslot* ts = new Timeslot();
        ts->setId(query.value(0).toInt());
        ts->setTime_start(query.value(1).toTime());
        ts->setTime(query.value(2).toTime());
        ts->setTime_end(query.value(3).toTime());
        ts->setId_kholleurs(query.value(4).toInt());
        ts->setDate(query.value(5).toDate());
        ts->setPupils(query.value(6).toInt());
        queue_displayedTimeslots.enqueue(ts);

        QListWidgetItem *item = new QListWidgetItem(days[ts->getDate().dayOfWeek()] + " : "
                + ts->getTime_start().toString("HH:mm") + " >> " + ts->getTime().toString("HH:mm") + " >> " + ts->getTime_end().toString("HH:mm")
                + ", " + QString::number(ts->getPupils()) + (ts->getPupils() <= 1 ? " élève" : " élèves"), ui->list_timeslots);
        item->setData(Qt::UserRole, (qulonglong) ts);
    }
*/
    return true;
}
