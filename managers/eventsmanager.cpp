#include "eventsmanager.h"
#include "ui_eventsmanager.h"

EventsManager::EventsManager(QSqlDatabase *db, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::EventsManager)
{
    // Initialisation
    ui->setupUi(this);
    m_db = db;

    // Connect the buttons
    connect(ui->list_events, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(display_event(QListWidgetItem*)));
    connect(ui->checkBox_old, SIGNAL(clicked(bool)), this, SLOT(update_list()));
    connect(ui->pushButton_add, SIGNAL(clicked(bool)), this, SLOT(add_event()));
    connect(ui->pushButton_update, SIGNAL(clicked(bool)), this, SLOT(update_event()));
    connect(ui->list_events, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(update_event()));
    connect(ui->pushButton_delete, SIGNAL(clicked(bool)), this, SLOT(delete_event()));

    // Display the events
    update_list();
}

EventsManager::~EventsManager() {
    delete ui;
    free_events();
}

bool EventsManager::free_events() {
    /** To free memories with events **/
    while (!queue_displayedEvents.isEmpty())
        delete queue_displayedEvents.dequeue();
    return true;
}

bool EventsManager::update_list() {
    /** To update the list of the events **/
    // Clear the list
    ui->list_events->clear();
    free_events();

    // Make the request
    QSqlQuery query(*m_db);
    if( ! ui->checkBox_old->isChecked())
            query.exec("SELECT id, name, comment, start, end FROM tau_events WHERE end >= datetime('NOW') ORDER BY start");
    else    query.exec("SELECT id, name, comment, start, end FROM tau_events WHERE end <  datetime('NOW') ORDER BY start DESC");

    // Treat the request & Display the events
    while (query.next()) {
        // Get the event
        Event* event = new Event();
        event->setId(query.value(0).toInt());
        event->setName(query.value(1).toString());
        event->setComment(query.value(2).toString());
        event->setStart(query.value(3).toDateTime());
        event->setEnd(query.value(4).toDateTime());

        // Get the groups of the event
        QSqlQuery queryGroups(*m_db);
        queryGroups.prepare("SELECT G.`id`, G.`name` FROM `tau_events_groups` AS L JOIN `tau_groups` AS G ON L.`id_groups` = G.`id` WHERE L.`id_events` = :id_events");
        queryGroups.bindValue(":id_events", event->getId());
        queryGroups.exec();
        // Add the groups in the event
        while (queryGroups.next()) {
            Group* grp = new Group();
            grp->setId(queryGroups.value(0).toInt());
            grp->setName(queryGroups.value(1).toString());
            event->getGroups()->append(grp);
        }

        // Display the event
        QListWidgetItem *item = new QListWidgetItem(event->getName() + " : " + event->getStart().toString("dd/MM/yyyy hh:mm") + " >> " + event->getEnd().toString("dd/MM/yyyy hh:mm"), ui->list_events);
        item->setData(Qt::UserRole, (qulonglong) event);
        queue_displayedEvents.enqueue(event);
    }

    return true;
}

bool EventsManager::display_event(QListWidgetItem* item) {
    /** Display the event (name, comment, groups...) in the information area**/
    if(item == NULL) { // If no item is selected
        QMessageBox::critical(this, "Erreur", "Veuillez sélectionner un évènement.");
        return false;
    } else { // Else ... update the information area
        Event* event = (Event*) item->data(Qt::UserRole).toULongLong();
        ui->label_name->setText(event->getName());
        ui->label_comment->setText(event->getComment());
        ui->label_start->setText(event->getStart().toString("hh:mm dd/MM/yyyy"));
        ui->label_end->setText(event->getEnd().toString("hh:mm dd/MM/yyyy"));
        QString textGroups = "";
        for(int i=0; i<event->getGroups()->size(); ++i)
            textGroups += (*event->getGroups())[i]->getName() + "<br />";
        ui->label_groups->setText(textGroups);
    }

    return true;
}

bool EventsManager::add_event() {
    /** To add an event in the DB **/
    QString name = ui->lineEdit_name->text();

    if(name == "") { // If there is no name
        QMessageBox::critical(this, "Erreur", "Il faut renseigner l'intitulé de l'évènement.");
        return false;
    } else {
        // Create the event
        Event* event = new Event();
        event->setName(name);

        // Open the manager
        UpdateEventDialog manager(m_db, event, this);
        int res = manager.exec();

        // Update the widgets
        if(res == QDialog::Accepted) {
            ui->lineEdit_name->clear();
            update_list();
        }

        free(event);
    }

    return true;
}

bool EventsManager::update_event() {
    /** To update an event **/
    QListWidgetItem *item = ui->list_events->currentItem();

    if(item == NULL) { // If no event is selected...
        QMessageBox::critical(this, "Erreur", "Veuillez sélectionner un évènement.");
        return false;
    } else {
        Event* event = (Event*) item->data(Qt::UserRole).toULongLong();
        // Open the manager to update the event
        UpdateEventDialog updateBox(m_db, event, this);
        if(updateBox.exec() == QDialog::Accepted) {
            // Update the widgets
            update_list();

            ui->label_name->setText("");
            ui->label_comment->setText("");
            ui->label_start->setText("");
            ui->label_end->setText("");
            ui->label_groups->setText("");
        }
    }

    return true;
}

bool EventsManager::delete_event() {
    /** To delete an event in the DB **/
    QListWidgetItem *item = ui->list_events->currentItem();

    if(item == NULL) { // If there is no name
        QMessageBox::critical(this, "Erreur", "Veuillez sélectionner un évènement.");
        return false;
    } else {
        // Get the event
        Event* event = (Event*) item->data(Qt::UserRole).toULongLong();
        // Ask for a confirmation
        int res = QMessageBox::warning(this, "Suppression en cours",
                "Vous êtes sur le point de supprimer l'évènement <strong>" + event->getName() + "</strong>.<br /> Voulez-vous continuer ?",
                QMessageBox::Yes | QMessageBox::Cancel);
        if(res == QMessageBox::Yes) {
            // Delete the event if the answer is YES
            QSqlQuery query(*m_db);
            query.prepare("DELETE FROM tau_events WHERE id=:id");
            query.bindValue(":id", event->getId());
            query.exec();
            query.prepare("DELETE FROM tau_events_groups WHERE id_events=:id_events");
            query.bindValue(":id_events", event->getId());
            query.exec();

            // Update the widgets
            update_list();
        }
    }

    return true;
}
