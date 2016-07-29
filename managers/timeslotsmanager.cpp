#include "timeslotsmanager.h"
#include "ui_timeslotsmanager.h"

TimeslotsManager::TimeslotsManager(QSqlDatabase *db, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TimeslotsManager)
{
    //UI
    ui->setupUi(this);

    //DB
    m_db = db;

    //Initialise days
    days << "" << "Lundi" << "Mardi" << "Mercredi" << "Jeudi" << "Vendredi" << "Samedi";

    getKholleurs();

    connect(ui->listKholleurs, SIGNAL(itemSelectionChanged()), this, SLOT(onSelection_change()));
    connect(ui->addButton, SIGNAL(clicked()), this, SLOT(addTimeslot()));
    connect(ui->deleteButton, SIGNAL(clicked()), this, SLOT(deleteTimeslot()));
}

TimeslotsManager::~TimeslotsManager()
{
    delete ui;
    freeKholleurs();
    free_timeslots();
}

void TimeslotsManager::getKholleurs() {
    //Free
    freeKholleurs();

    //Prepare query
    QSqlQuery query(*m_db);
    query.exec("SELECT id, name, id_subjects, duration, preparation, pupils FROM tau_kholleurs ORDER BY name");


    //Treat query
    while(query.next()) {
        Kholleur* k = new Kholleur();
        k->setId(query.value(0).toInt());
        k->setName(query.value(1).toString());
        k->setId_subjects(query.value(2).toInt());
        k->setDuration(query.value(3).toInt());
        k->setPreparation(query.value(4).toInt());
        k->setPupils(query.value(5).toInt());
        queue_displayedKholleurs.enqueue(k);

        QListWidgetItem *item = new QListWidgetItem(k->getName(), ui->listKholleurs);
        item->setData(Qt::UserRole, (qulonglong) k);
    }
}

void TimeslotsManager::freeKholleurs() {
    while(!queue_displayedKholleurs.isEmpty()) {
        delete queue_displayedKholleurs.dequeue();
    }
}

void TimeslotsManager::update_list_timeslots(int id_kholleur) {
    //Free
    free_timeslots();
    ui->listTimeslots->clear();

    //Prepare query
    QSqlQuery query(*m_db);
    query.prepare("SELECT id, time_start, time, time_end, id_kholleurs, id_day FROM tau_timeslots WHERE id_kholleurs=:id_kholleurs ORDER BY id_day, time");
    query.bindValue(":id_kholleurs", id_kholleur);
    query.exec();

    //Treat
    while(query.next()) {
        Timeslot* ts = new Timeslot();
        ts->setId(query.value(0).toInt());
        ts->setTime_start(QTime::fromString(query.value(1).toString(), "h:mm:ss"));
        ts->setTime(QTime::fromString(query.value(2).toString(), "h:mm:ss"));
        ts->setTime_end(QTime::fromString(query.value(3).toString(), "h:mm:ss"));
        ts->setId_kholleurs(query.value(4).toInt());
        ts->setId_day(query.value(5).toInt());
        queue_displayedTimeslots.enqueue(ts);

        QListWidgetItem *item = new QListWidgetItem(days[ts->getId_day()] + " : " + ts->getTime_start().toString("HH:mm") + " >> " + ts->getTime().toString("HH:mm") + " >> " + ts->getTime_end().toString("HH:mm"), ui->listTimeslots);
        item->setData(Qt::UserRole, (qulonglong) ts);
    }
}

void TimeslotsManager::free_timeslots() {
    while(!queue_displayedTimeslots.isEmpty()) {
        delete queue_displayedTimeslots.dequeue();
    }
}

void TimeslotsManager::onSelection_change() {
    QList<QListWidgetItem*> selection = ui->listKholleurs->selectedItems();

    if(selection.length() <= 0) {
        //Disable all
        ui->label_jour->setEnabled(false);
        ui->comboBox->setEnabled(false);
        ui->label_debut->setEnabled(false);
        ui->timeEdit->setEnabled(false);
        ui->listTimeslots->setEnabled(false);
        ui->addButton->setEnabled(false);
        ui->deleteButton->setEnabled(false);
        return;
    }

    //Enable all
    ui->label_jour->setEnabled(true);
    ui->comboBox->setEnabled(true);
    ui->label_debut->setEnabled(true);
    ui->timeEdit->setEnabled(true);
    ui->listTimeslots->setEnabled(true);
    ui->addButton->setEnabled(true);
    ui->deleteButton->setEnabled(true);

    update_list_timeslots(((Kholleur*)selection[0]->data(Qt::UserRole).toULongLong())->getId());
}

void TimeslotsManager::addTimeslot() {
    QList<QListWidgetItem*> selection = ui->listKholleurs->selectedItems();

    if(selection.length() <= 0) {
        QMessageBox::critical(this, "Erreur", "Veuillez sélectionner un kholleur.");
        return;
    }
    Kholleur* k = (Kholleur*) selection[0]->data(Qt::UserRole).toULongLong();

    //Create timeslot
    Timeslot* ts = new Timeslot();
    ts->setId_day(ui->comboBox->currentIndex() + 1);
    ts->setId_kholleurs(k->getId());

    QTime time = ui->timeEdit->time();
    ts->setTime(time);
    ts->setTime_start(time.addSecs(-60*k->getPreparation()));
    ts->setTime_end(time.addSecs(60*k->getDuration()));

    //Add to DB
    QSqlQuery query(*m_db);
    query.prepare("INSERT INTO tau_timeslots(time_start, time, time_end, id_kholleurs, id_day) VALUES(:time_start, :time, :time_end, :id_kholleurs, :id_day)");
    query.bindValue(":time_start", ts->getTime_start().toString("HH:mm:ss"));
    query.bindValue(":time", ts->getTime().toString("HH:mm:ss"));
    query.bindValue(":time_end", ts->getTime_end().toString("HH:mm:ss"));
    query.bindValue(":id_kholleurs", ts->getId_kholleurs());
    query.bindValue(":id_day", ts->getId_day());
    query.exec();

    update_list_timeslots(k->getId());
}

void TimeslotsManager::deleteTimeslot() {
    QList<QListWidgetItem*> selection = ui->listTimeslots->selectedItems();

    if(selection.length() <= 0) {
        QMessageBox::critical(this, "Erreur", "Veuillez sélectionner un horaire de kholle.");
        return;
    }

    Timeslot* ts = (Timeslot*) selection[0]->data(Qt::UserRole).toULongLong();

    //No confirmation needed, I think

    //Query
    QSqlQuery query(*m_db);
    query.prepare("DELETE FROM tau_timeslots WHERE id=:id");
    query.bindValue(":id", ts->getId());
    query.exec();

    //Update
    QList<QListWidgetItem*> kholleurs = ui->listKholleurs->selectedItems();
    if(kholleurs.length() > 0) {
        update_list_timeslots(((Kholleur*)kholleurs[0]->data(Qt::UserRole).toULongLong())->getId());
    }

}
