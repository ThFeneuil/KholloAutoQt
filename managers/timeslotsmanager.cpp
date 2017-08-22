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
    displayNameClass();

    connect(ui->listKholleurs, SIGNAL(itemSelectionChanged()), this, SLOT(onSelection_change()));
    connect(ui->listTimeslots, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(deleteTimeslot(QListWidgetItem*)));
    connect(ui->listOftenTimeslots, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(addOftenTimeslot(QListWidgetItem*)));
    connect(ui->addButton, SIGNAL(clicked()), this, SLOT(addTimeslot()));
    connect(ui->deleteButton, SIGNAL(clicked()), this, SLOT(deleteTimeslot()));
    connect(ui->copyButton, SIGNAL(clicked()), this, SLOT(copyTimeslots()));
    connect(ui->copyAllButton, SIGNAL(clicked()), this, SLOT(copyAllTimeslots()));
    connect(ui->dowloadButton, SIGNAL(clicked(bool)), this, SLOT(downloadTimeslots()));
    connect(ui->edit_name_class, SIGNAL(editingFinished()), this, SLOT(saveNameClass()));

    m_shortcutNotepad = Notepad::shortcut();
    this->addAction(m_shortcutNotepad);
}

TimeslotsManager::~TimeslotsManager()
{
    delete ui;
    freeKholleurs();
    free_timeslots();
    this->removeAction(m_shortcutNotepad);
    delete m_shortcutNotepad;
}

void TimeslotsManager::getKholleurs() {
    //Free
    freeKholleurs();
    ui->listKholleurs->clear();

    //Prepare query
    QSqlQuery query(*m_db);
    query.exec("SELECT id, name, id_subjects, duration, preparation, pupils FROM tau_kholleurs ORDER BY UPPER(name)");


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

    update_list_oftenTimeslots(id_kholleur);
}

void TimeslotsManager::update_list_oftenTimeslots(int id_kholleur) {
    ui->listOftenTimeslots->clear();

    //Prepare query
    QSqlQuery query(*m_db);
    query.prepare("SELECT strftime('%w', `date`) AS DOW, `time` FROM tau_timeslots "
                    "WHERE `id_kholleurs` = :id_kholleurs AND DOW || '#' || `time` NOT IN "
                      "(SELECT strftime('%w', `date`) || '#' || `time` FROM tau_timeslots "
                          "WHERE `date` >= :monday_date AND `date` <= :sunday_date  AND `id_kholleurs` = :id_kholleurs) "
                  "GROUP BY DOW, `time` "
                  "ORDER BY COUNT(`id`) DESC, DOW, `time`");
    query.bindValue(":id_kholleurs", id_kholleur);
    query.bindValue(":monday_date", m_date.toString("yyyy-MM-dd"));
    query.bindValue(":sunday_date", m_date.addDays(6).toString("yyyy-MM-dd"));
    query.exec();

    //Treat
    while(query.next()) {
        Timeslot* ts = new Timeslot();
        ts->setDate(m_date.addDays(query.value(0).toInt()-1));
        ts->setTime(QTime::fromString(query.value(1).toString(), "h:mm:ss"));
        queue_displayedTimeslots.enqueue(ts);

        QListWidgetItem *item = new QListWidgetItem(days[query.value(0).toInt()] + " : "
                + query.value(1).toString(), ui->listOftenTimeslots);
        item->setData(Qt::UserRole, (qulonglong) ts);
    }
}

void TimeslotsManager::addOftenTimeslot(QListWidgetItem* item) {
    QList<QListWidgetItem*> selection = ui->listKholleurs->selectedItems();

    if(selection.length() <= 0) {
        QMessageBox::critical(this, "Erreur", "Veuillez sélectionner un kholleur.");
        return;
    }
    Kholleur* k = (Kholleur*) selection[0]->data(Qt::UserRole).toULongLong();

    //Create timeslot
    Timeslot* ts = (Timeslot*) item->data(Qt::UserRole).toULongLong();
    ts->setId_kholleurs(k->getId());
    ts->setPupils(k->getPupils());

    QTime time = ts->getTime();
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
        ui->listOftenTimeslots->setEnabled(false);
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
    ui->listOftenTimeslots->setEnabled(true);
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
    ui->timeEdit->setTime(ui->timeEdit->time().addSecs(60*k->getDuration()));
    update_list_timeslots(k->getId());
}

void TimeslotsManager::deleteTimeslot(QListWidgetItem *item) {
    QList<QListWidgetItem*> selection = ui->listTimeslots->selectedItems();
    if(item) {
        selection.clear();
        selection << item;
    }

    if(selection.length() <= 0) {
        QMessageBox::critical(this, "Erreur", "Veuillez sélectionner un horaire de kholle.");
        return;
    }

    QList<QListWidgetItem*> kholleurs = ui->listKholleurs->selectedItems();
    if(kholleurs.length() <= 0) {
        QMessageBox::critical(this, "Erreur", "Aucun kholleur n'est sélectionné. Comment est-ce possible ?");
        return;
    }

    //Confirmation
    QString nbDeletedTimeslots = (selection.length() == 1) ? "un horaire de kholle" : QString::number(selection.length()) + " horaires de kholles";
    int res = QMessageBox::warning(this, "Suppression en cours", "Vous êtes sur le point de supprimer " + nbDeletedTimeslots + " " +
                                                                 "du kholleur <strong>" + ((Kholleur*)kholleurs[0]->data(Qt::UserRole).toULongLong())->getName() + "</strong> "
                                                                 "ainsi que les <strong>kholles</strong> associées. <br /> Voulez-vous continuer ?", QMessageBox::Yes | QMessageBox::No);

    if(res == QMessageBox::Yes) {
        QSqlQuery query(*m_db);
        m_db->transaction();
        for(int i=0; i<selection.length(); i++) {
            Timeslot* ts = (Timeslot*) selection[i]->data(Qt::UserRole).toULongLong();
            //Query
            query.prepare("DELETE FROM tau_kholles WHERE id_timeslots=:id_timeslots");
            query.bindValue(":id_timeslots", ts->getId());
            query.exec();
            query.prepare("DELETE FROM tau_timeslots WHERE id=:id");
            query.bindValue(":id", ts->getId());
            query.exec();
        }
        m_db->commit();
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

void TimeslotsManager::downloadTimeslots() {
    QString name_class = ui->edit_name_class->text();
    if(name_class.length() > 10) {
        QMessageBox::critical(this, "Erreur", "Le nom de la classe doit posséder au plus 10 caractères...");
    } else {
        QSqlQuery qVerif(*m_db);
        qVerif.prepare("SELECT COUNT(*) FROM tau_timeslots "
                        "WHERE date>=:monday_date AND date<=:sunday_date");
        qVerif.bindValue(":monday_date", m_date.toString("yyyy-MM-dd"));
        qVerif.bindValue(":sunday_date", m_date.addDays(6).toString("yyyy-MM-dd"));
        qVerif.exec();

        if(qVerif.next()) {
            int nbSlots = qVerif.value(0).toInt();
            if(nbSlots > 0) {
                QMessageBox msg;
                msg.setWindowTitle("Téléchargement");
                msg.setIcon(QMessageBox::Warning);
                msg.setText("La semaine courante possède déjà des horaires. Voulez-vous supprimer les horaires pour les remplacer par ceux qui vont être téléchargés, ou préférez-vous les garder ? <strong>Attention, si vous remplacez les horaires, les kholles qui leur sont associées vont être supprimées !</strong>");
                QAbstractButton *keep_btn = (QAbstractButton*) msg.addButton("Garder les horaires", QMessageBox::ApplyRole);
                QAbstractButton *replace_btn = (QAbstractButton*) msg.addButton("Remplacer les horaires (!)", QMessageBox::ApplyRole);
                msg.addButton("Annuler", QMessageBox::ApplyRole);
                msg.exec();

                if(msg.clickedButton() == replace_btn) {
                    //Delete all timeslots for this week
                    qVerif.prepare("DELETE FROM tau_kholles WHERE id_timeslots IN "
                                            "(SELECT id FROM tau_timeslots WHERE date>=:monday_date AND date<=:sunday_date)");
                    qVerif.bindValue(":monday_date", m_date.toString("yyyy-MM-dd"));
                    qVerif.bindValue(":sunday_date", m_date.addDays(6).toString("yyyy-MM-dd"));
                    qVerif.exec();
                    qVerif.prepare("DELETE FROM tau_timeslots WHERE date>=:monday_date AND date<=:sunday_date");
                    qVerif.bindValue(":monday_date", m_date.toString("yyyy-MM-dd"));
                    qVerif.bindValue(":sunday_date", m_date.addDays(6).toString("yyyy-MM-dd"));
                    qVerif.exec();
                } else if(msg.clickedButton() != keep_btn)
                    return;
            }
        }


        ui->dowloadButton->setEnabled(false);
        ui->dowloadButton->setText("Téléchargement...");
        Preferences pref;
        ODBSqlQuery* query = NULL;
        if(pref.serverDefault())
                query = new ODBSqlQuery(DEFAULT INTO(this, downloadedTimeslots));
        else    query = new ODBSqlQuery(FROM(pref.serverScript(), pref.serverPassword()) INTO(this, downloadedTimeslots));
        query->prepare("SELECT id, time, time_start, time_end, kholleur, date, nb_pupils, subject FROM spark_timeslots WHERE class=:class AND date>=:start AND date<=:end ORDER BY UPPER(kholleur);");
        query->bindValue(":class", name_class);
        query->bindValue(":start", m_date.toString("yyyy-MM-dd"));
        query->bindValue(":end", m_date.addDays(6).toString("yyyy-MM-dd"));
        query->exec();
        delete query;
    }
}

void TimeslotsManager::downloadedTimeslots(ODBRequest *req) {

    if(req->lastError() == "") {
        QSqlQuery query(*m_db);
        query.exec("SELECT name, id_kholleurs FROM tau_merge_kholleurs");
        QMap<QString, int>* idKholleurs = new QMap<QString, int>;
        while(query.next())
            idKholleurs->insert(query.value(0).toString(), query.value(1).toInt());

        QList<Kholleur*>* anonymousKholleurs = new QList<Kholleur*>;
        QList<QMap<QString, QVariant>*>* res = req->result();
        for(int i=0; i<res->length(); i++) {
            QMap<QString, QVariant>* row = res->at(i);
            // We avoid a "MergeSubjectsManager" concatenating the subjet with the name of the kholleur...
            QString name_khll = row->value("kholleur").toString() + " (" + row->value("subject").toString() + ")";

            if(idKholleurs->contains(name_khll) == false && (anonymousKholleurs->length() == 0 || anonymousKholleurs->last()->getName() != name_khll)) {
                Kholleur* khll = new Kholleur;
                khll->setName(name_khll);
                khll->setPupils(row->value("nb_pupils").toInt());
                khll->setPreparation(row->value("time_start").toTime().secsTo(row->value("time").toTime()) / 60);
                khll->setDuration(row->value("time").toTime().secsTo(row->value("time_end").toTime()) / 60);
                anonymousKholleurs->append(khll);
            }
        }

        if(anonymousKholleurs->length() > 0) {
            MergeKholleursManager manager(m_db, anonymousKholleurs, idKholleurs, this);
            if(manager.exec() == QDialog::Rejected) {
                ui->dowloadButton->setText("Télécharger");
                ui->dowloadButton->setEnabled(true);
                delete req;
                QMessageBox::information(this, "Opération anulée", "Téléchargement annulé...");
                return;
            }
        }

        QString queryInsert_str = "";
        int nbTimeslots = 0;
        for(int i=0; i<res->length(); i++) {
            QMap<QString, QVariant>* row = res->at(i);
            Timeslot* slot = new Timeslot;
            slot->setId(row->value("id").toInt());
            slot->setTime(row->value("time").toTime());
            slot->setTime_start(row->value("time_start").toTime());
            slot->setTime_end(row->value("time_end").toTime());
            // We avoid a "MergeSubjectsManager" concatenating the subjet with the name of the kholleur...
            QString name_khll = row->value("kholleur").toString() + " (" + row->value("subject").toString() + ")";
            slot->setDate(row->value("date").toDate());
            slot->setPupils(row->value("nb_pupils").toInt());

            if(idKholleurs->contains(name_khll)) {
                slot->setId_kholleurs(idKholleurs->value(name_khll));
                queryInsert_str += (queryInsert_str != "") ? ", " : "";
                queryInsert_str += "('"+slot->getTime().toString("hh:mm:ss")+"', '"+slot->getTime_start().toString("hh:mm:ss")+"', '"+slot->getTime_end().toString("hh:mm:ss")+"', ";
                queryInsert_str += QString::number(slot->getId_kholleurs())+", '"+slot->getDate().toString("yyyy-MM-dd")+"', "+QString::number(slot->getPupils())+")";
                nbTimeslots++;
            } else    QMessageBox::information(this, "Error", "FAIL");

            delete slot;
        }

        if(queryInsert_str != "") {
            queryInsert_str = "INSERT INTO tau_timeslots(time, time_start, time_end, id_kholleurs, date, pupils) VALUES"+queryInsert_str+";";
            query.exec(queryInsert_str);
            QMessageBox::information(this, "Succés", "Le téléchargement a réussi : "+QString::number(nbTimeslots)+" horaire(s) de kholle téléchargé(s).");
        } else {
            QMessageBox::information(this, "Succés", "Aucun horaire de kholle n'a été trouvé...");
        }
    } else
        QMessageBox::critical(this, "Error", req->lastError());

    delete req;
    ui->dowloadButton->setText("Télécharger");
    ui->dowloadButton->setEnabled(true);
    getKholleurs();
    QList<QListWidgetItem*> selection = ui->listKholleurs->selectedItems();
    if(selection.length() > 0) {
        update_list_timeslots(((Kholleur*) selection[0]->data(Qt::UserRole).toULongLong())->getId());
    }
}

void TimeslotsManager::displayNameClass() {
    QSqlQuery query(*m_db);
    query.exec("SELECT `value` FROM `tau_general` WHERE `key`='name_class';");

    //Treat
    if(query.next()) {
        ui->edit_name_class->setText(query.value(0).toString());
    } else {
        query.exec("INSERT INTO `tau_general`(`key`, `value`) VALUES('name_class', '');");
        ui->edit_name_class->setText("");
    }
}

void TimeslotsManager::saveNameClass() {
    QSqlQuery query(*m_db);
    query.prepare("UPDATE `tau_general` SET `value`=:value WHERE `key`='name_class';");
    query.bindValue(":value", ui->edit_name_class->text());
    query.exec();
}
