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

    update_list();
}

EventsManager::~EventsManager() {
    delete ui;
    free_events();
}

bool EventsManager::free_events() {
    /** To free memories with events **/
    while (!queue_displayedEvents.isEmpty())
        free(queue_displayedEvents.dequeue());
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
        query.exec("SELECT id, name, comment, start, end FROM tau_events WHERE end >= NOW() ORDER BY start");
    else
        query.exec("SELECT id, name, comment, start, end FROM tau_events WHERE end < NOW() ORDER BY start DESC");

    // Treat the request & Display the events
    while (query.next()) {
        // Get the event
        Event* event = new Event();
        event->setId(query.value(0).toInt());
        event->setName(query.value(1).toString());
        event->setComment(query.value(2).toString());
        event->setStart(query.value(3).toDateTime());
        event->setEnd(query.value(4).toDateTime());
        QListWidgetItem *item = new QListWidgetItem(event->getName() + " : " + event->getStart().toString("dd.MM.yyyy hh:mm") + " >> " + event->getEnd().toString("dd.MM.yyyy hh:mm"), ui->list_events);
        item->setData(Qt::UserRole, (qulonglong) event);
        queue_displayedEvents.enqueue(event);
    }

    return true;
}

bool EventsManager::display_event(QListWidgetItem* item) {
    if(item == NULL) { // If no item is selected
        QMessageBox::critical(this, "Erreur", "Veuillez sélectionner un évènement.");
        return false;
    } else { // Else ... open the dialog box
        Event* event = (Event*) item->data(Qt::UserRole).toULongLong();
        ui->label_name->setText(event->getName());
        ui->label_comment->setText(event->getComment());
        ui->label_start->setText(event->getStart().toString("hh:mm dd/MM/yyyy"));
        ui->label_end->setText(event->getEnd().toString("hh:mm dd/MM/yyyy"));
        ui->label_groups->setText("");
    }

    return true;
}

bool EventsManager::add_event() {
    /** To add a teacher in the DB **/
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
    QListWidgetItem *item = ui->list_events->currentItem();

    if(item == NULL) {
        QMessageBox::critical(this, "Erreur", "Veuillez sélectionner un évènement.");
        return false;
    } else {
        Event* event = (Event*) item->data(Qt::UserRole).toULongLong();
        UpdateEventDialog updateBox(m_db, event, this);
        if(updateBox.exec() == QDialog::Accepted) {
            update_list();
        }
    }

    return true;
}

bool EventsManager::delete_event() {
    QListWidgetItem *item = ui->list_events->currentItem();

    if(item == NULL) {
        QMessageBox::critical(this, "Erreur", "Veuillez sélectionner un évènement.");
        return false;
    } else {
        Event* event = (Event*) item->data(Qt::UserRole).toULongLong();
        int res = QMessageBox::warning(this, "Suppression en cours",
                "Vous êtes sur le point de supprimer l'évènement <strong>" + event->getName() + "</strong>.<br /> Voulez-vous continuer ?",
                QMessageBox::Yes | QMessageBox::Cancel);
        if(res == QMessageBox::Yes) {
            QSqlQuery query(*m_db);
            query.prepare("DELETE FROM tau_events WHERE id=:id");
            query.bindValue(":id", event->getId());
            query.exec();

            update_list();
        }
    }

    return true;
}
