#include "timeslotsmanager.h"
#include "ui_timeslotsmanager.h"

TimeslotsManager::TimeslotsManager(QSqlDatabase *db, QDate date, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TimeslotsManager)
{
    //UI
    ui->setupUi(this);

    //DB and date
    m_db = db;
    m_date = date;

    //Initialise days
    days << "" << "Lundi" << "Mardi" << "Mercredi" << "Jeudi" << "Vendredi" << "Samedi";

    getKholleurs();

    connect(ui->listKholleurs, SIGNAL(itemSelectionChanged()), this, SLOT(onSelection_change()));
    connect(ui->addButton, SIGNAL(clicked()), this, SLOT(addTimeslot()));
    connect(ui->deleteButton, SIGNAL(clicked()), this, SLOT(deleteTimeslot()));
    connect(ui->copyButton, SIGNAL(clicked()), this, SLOT(copyTimeslots()));
    connect(ui->copyAllButton, SIGNAL(clicked()), this, SLOT(copyAllTimeslots()));
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
    query.prepare("SELECT id, time_start, time, time_end, id_kholleurs, date, pupils FROM tau_timeslots "
                  "WHERE id_kholleurs=:id_kholleurs AND date>=:monday_date AND date<=:sunday_date "
                  "ORDER BY date, time");
    query.bindValue(":id_kholleurs", id_kholleur);
    query.bindValue(":monday_date", m_date.toString("yyyy-MM-dd"));
    query.bindValue(":sunday_date", m_date.addDays(6).toString("yyyy-MM-dd"));
    query.exec();

    //Treat
    while(query.next()) {
        Timeslot* ts = new Timeslot();
        ts->setId(query.value(0).toInt());
        ts->setTime_start(QTime::fromString(query.value(1).toString(), "h:mm:ss"));
        ts->setTime(QTime::fromString(query.value(2).toString(), "h:mm:ss"));
        ts->setTime_end(QTime::fromString(query.value(3).toString(), "h:mm:ss"));
        ts->setId_kholleurs(query.value(4).toInt());
        ts->setDate(QDate::fromString(query.value(5).toString(), "yyyy-M-d"));
        ts->setPupils(query.value(6).toInt());
        queue_displayedTimeslots.enqueue(ts);

        QListWidgetItem *item = new QListWidgetItem(days[ts->getDate().dayOfWeek()] + " : "
                + ts->getTime_start().toString("HH:mm") + " >> " + ts->getTime().toString("HH:mm") + " >> " + ts->getTime_end().toString("HH:mm")
                + ", " + QString::number(ts->getPupils()) + (ts->getPupils() <= 1 ? " élève" : " élèves"), ui->listTimeslots);
        item->setData(Qt::UserRole, (qulonglong) ts);
    }
}

void TimeslotsManager::free_timeslots() {
    while(!queue_displayedTimeslots.isEmpty()) {
        delete queue_displayedTimeslots.dequeue();
    }
}

void TimeslotsManager::onSelection_change() {
    ui->listTimeslots->clear();
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
        ui->copyButton->setEnabled(false);
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
    ui->copyButton->setEnabled(true);

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
    ts->setDate(m_date.addDays(ui->comboBox->currentIndex()));
    ts->setId_kholleurs(k->getId());
    ts->setPupils(k->getPupils());

    QTime time = ui->timeEdit->time();
    ts->setTime(time);
    ts->setTime_start(time.addSecs(-60*k->getPreparation()));
    ts->setTime_end(time.addSecs(60*k->getDuration()));

    //Add to DB
    QSqlQuery query(*m_db);
    query.prepare("INSERT INTO tau_timeslots(time_start, time, time_end, id_kholleurs, date, pupils) VALUES(:time_start, :time, :time_end, :id_kholleurs, :date, :pupils)");
    query.bindValue(":time_start", ts->getTime_start().toString("HH:mm:ss"));
    query.bindValue(":time", ts->getTime().toString("HH:mm:ss"));
    query.bindValue(":time_end", ts->getTime_end().toString("HH:mm:ss"));
    query.bindValue(":id_kholleurs", ts->getId_kholleurs());
    query.bindValue(":date", ts->getDate().toString("yyyy-MM-dd"));
    query.bindValue(":pupils", ts->getPupils());
    query.exec();

    delete ts;
    update_list_timeslots(k->getId());
}

void TimeslotsManager::deleteTimeslot() {
    QList<QListWidgetItem*> selection = ui->listTimeslots->selectedItems();

    if(selection.length() <= 0) {
        QMessageBox::critical(this, "Erreur", "Veuillez sélectionner un horaire de kholle.");
        return;
    }

    Timeslot* ts = (Timeslot*) selection[0]->data(Qt::UserRole).toULongLong();

    QList<QListWidgetItem*> kholleurs = ui->listKholleurs->selectedItems();
    if(kholleurs.length() <= 0) {
        QMessageBox::critical(this, "Erreur", "Aucun kholleur n'est sélectionné. Comment est-ce possible ?");
        return;
    }

    //Confirmation
    int res = QMessageBox::warning(this, "Suppression en cours", "Vous êtes sur le point de supprimer un horaire de kholle "
                                                                 "du kholleur <strong>" + ((Kholleur*)kholleurs[0]->data(Qt::UserRole).toULongLong())->getName() + "</strong> "
                                                                 "ainsi que les <strong>kholles</strong> associées. <br /> Voulez-vous continuer ?", QMessageBox::Yes | QMessageBox::No);

    if(res == QMessageBox::Yes) {
        //Query
        QSqlQuery query(*m_db);
        query.prepare("DELETE FROM tau_kholles WHERE id_timeslots=:id_timeslots");
        query.bindValue(":id_timeslots", ts->getId());
        query.exec();
        query.prepare("DELETE FROM tau_timeslots WHERE id=:id");
        query.bindValue(":id", ts->getId());
        query.exec();

        //Update
        update_list_timeslots(((Kholleur*)kholleurs[0]->data(Qt::UserRole).toULongLong())->getId());

    }

}

void TimeslotsManager::copyTimeslots() {
    //Get selection
    QList<QListWidgetItem*> selection = ui->listKholleurs->selectedItems();

    if(selection.length() <= 0) {
        QMessageBox::critical(this, "Erreur", "Veuillez sélectionner un kholleur.");
        return;
    }
    Kholleur* k = (Kholleur*) selection[0]->data(Qt::UserRole).toULongLong();

    //Open dialog
    CopyTimeslots dialog(m_db, m_date, false, k->getId(), this);
    dialog.exec();

    //Update
    update_list_timeslots(k->getId());
}

void TimeslotsManager::copyAllTimeslots() {
    //Open dialog with all = true
    CopyTimeslots dialog(m_db, m_date, true, 0, this);
    dialog.exec();

    //Get selection
    QList<QListWidgetItem*> selection = ui->listKholleurs->selectedItems();

    //Update if necessary
    if(selection.length() > 0) {
        update_list_timeslots(((Kholleur*) selection[0]->data(Qt::UserRole).toULongLong())->getId());
    }
}
