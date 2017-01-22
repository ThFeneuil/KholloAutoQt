#include "updateeventdialog.h"
#include "ui_updateeventdialog.h"

UpdateEventDialog::UpdateEventDialog(QSqlDatabase *db, Event *event, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::UpdateEventDialog)
{
    // Initialisation
    ui->setupUi(this);
    m_db = db;
    m_event = event;
    m_list_groups = new QList<Group*>();

    // Connect the button
    connect(ui->pushButton_update, SIGNAL(clicked()), this, SLOT(update_event()));
    connect(ui->pushButton_groups, SIGNAL(clicked(bool)), this, SLOT(select_groups()));

    // Display the event
    ui->lineEdit_name->setText(event->getName());
    ui->plainTextEdit_comment->setPlainText(event->getComment());
    ui->dateTimeEdit_start->setDateTime(event->getStart());
    ui->dateTimeEdit_end->setDateTime(event->getEnd());

    // Get the groups of the event
    QSqlQuery query(*m_db);
    query.prepare("SELECT G.`id`, G.`name` FROM `tau_events_groups` AS L JOIN `tau_groups` AS G ON L.`id_groups` = G.`id` WHERE L.`id_events` = :id_events");
    query.bindValue(":id_events", m_event->getId());
    query.exec();

    // Display the groups (in the combo box)
    while (query.next()) {
        Group* grp = new Group();
        grp->setId(query.value(0).toInt());
        grp->setName(query.value(1).toString());
        m_list_groups->append(grp);
    }
    display_groups();
}

UpdateEventDialog::~UpdateEventDialog() {
    delete ui;
    // Free the groups
    for(int i=0; i<m_list_groups->size(); ++i)
        delete (*m_list_groups)[i];
    free(m_list_groups);
}

bool UpdateEventDialog::update_event() {
    /** Update an event in the DB **/
    // Get the parameters given par the user
    QString name = ui->lineEdit_name->text();
    QString comment = ui->plainTextEdit_comment->toPlainText();
    QDateTime start = ui->dateTimeEdit_start->dateTime();
    QDateTime end = ui->dateTimeEdit_end->dateTime();

    if(name == "") { // If there is no name
        QMessageBox::critical(this, "Erreur", "Il faut renseigner l'intitulé de l'évènement.");
        return false;
    } else if(start > end) { // If the dates are not compatible
        QMessageBox::critical(this, "Erreur", "En toute logique, le début doit être avant la fin !!");
        return false;
    } else { // Else insert the event in the DB
        QSqlQuery query(*m_db);

        if(m_event->getId()) { // If the event has a ID
            query.prepare("UPDATE tau_events "
                          "SET name=:name, comment=:comment, start=:start, end=:end "
                          "WHERE id=:id");
            query.bindValue(":id", m_event->getId());
        } else // If the event has not a ID
            query.prepare("INSERT INTO tau_events(name, comment, start, end) VALUES(:name, :comment, :start, :end)");
        query.bindValue(":name", name);
        query.bindValue(":comment", comment);
        query.bindValue(":start", start.toString("yyyy-MM-dd hh:mm"));
        query.bindValue(":end", end.toString("yyyy-MM-dd hh:mm"));
        query.exec();

        int id_event = m_event->getId() ? m_event->getId() : query.lastInsertId().toInt();

        // Delete the old groups of the event
        query.prepare("DELETE FROM tau_events_groups WHERE id_events = :id_events");
        query.bindValue(":id_events", id_event);
        query.exec();

        // Add the new groups of the event
        for(int i=0; i < m_list_groups->size(); ++i) {
            query.prepare("INSERT INTO tau_events_groups(id_events, id_groups) VALUES(:id_events, :id_groups)");
            query.bindValue(":id_events", id_event);
            query.bindValue(":id_groups", m_list_groups->at(i)->getId());
            query.exec();
        }

        accept();
    }

    return true;
}

bool UpdateEventDialog::select_groups() {
    /** To select the groups of the event **/
    // Open the manager to select the groups
    SelectGroupsDialog manager(m_db, m_list_groups, this);
    manager.exec();

    // Update the widgets
    display_groups();

    return true;
}

bool UpdateEventDialog::display_groups() {
    /** Update the widgets of the groups **/
    // Update the button (to select the groups)
    if(m_list_groups->size()>1)
        ui->pushButton_groups->setText(QString::number(m_list_groups->size()) + " groupes");
    else if(m_list_groups->size() == 1)
        ui->pushButton_groups->setText(QString::number(m_list_groups->size()) + " groupe");
    else
        ui->pushButton_groups->setText("Ajouter des groupes");

    // Clear the combo box
    ui->comboBox_groups->clear();
    // Fill the combo box
    for(int i=0; i<m_list_groups->size(); ++i)
        ui->comboBox_groups->addItem(m_list_groups->at(i)->getName(), 0);

    return true;
}
