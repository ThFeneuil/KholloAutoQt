#include "updateeventdialog.h"
#include "ui_updateeventdialog.h"

UpdateEventDialog::UpdateEventDialog(QSqlDatabase *db, Event *event, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::UpdateEventDialog)
{
    ui->setupUi(this);
    m_db = db;
    m_event = event;

    connect(ui->pushButton_update, SIGNAL(clicked()), this, SLOT(update_event()));

    ui->lineEdit_name->setText(event->getName());
    ui->plainTextEdit_comment->setPlainText(event->getComment());
    ui->dateTimeEdit_start->setDateTime(event->getStart());
    ui->dateTimeEdit_end->setDateTime(event->getEnd());
}

UpdateEventDialog::~UpdateEventDialog() {
    delete ui;
    // Free the groups of the comboBox
    while (!queue_groups.isEmpty())
        free(queue_groups.dequeue());
}

bool UpdateEventDialog::update_event() {
    // Get the parameters given par the user
    QString name = ui->lineEdit_name->text();
    QString comment = ui->plainTextEdit_comment->toPlainText();
    QDateTime start = ui->dateTimeEdit_start->dateTime();
    QDateTime end = ui->dateTimeEdit_end->dateTime();

    if(name == "") { // If there is no name
        QMessageBox::critical(this, "Erreur", "Il faut renseigner l'intitulé de l'évènement.");
        return false;
    } else if(start > end) {
        QMessageBox::critical(this, "Erreur", "En toute logique, le début doit être avant la fin !!");
        return false;
    } else { // Else insert the teacher in the DB
        QSqlQuery query(*m_db);
        if(m_event->getId()) { // If the event has a ID
            query.prepare("UPDATE tau_events "
                          "SET name=:name, comment=:comment, start=:start, end=:end "
                          "WHERE id=:id");
            query.bindValue(":id", m_event->getId());
        } else { // If the event has not a ID
            query.prepare("INSERT INTO tau_events(name, comment, start, end) VALUES(:name, :comment, :start, :end)");
        }
        query.bindValue(":name", name);
        query.bindValue(":comment", comment);
        query.bindValue(":start", start.toString("yyyy-MM-dd hh:mm"));
        query.bindValue(":end", end.toString("yyyy-MM-dd hh:mm"));
        query.exec();

        accept();
    }

    return true;
}
