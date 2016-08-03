#include "updatekholleurdialog.h"
#include "ui_updatekholleurdialog.h"

UpdateKholleurDialog::UpdateKholleurDialog(QSqlDatabase *db, Kholleur *khll, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::UpdateKholleurDialog)
{
    // Initialisation
    ui->setupUi(this);
    m_db = db;
    m_kholleur = khll;

    // Set the input field with the data of the kholleur
    ui->lineEdit_name->setText(m_kholleur->getName());
    ui->spinBox_duration->setValue(m_kholleur->getDuration());
    ui->spinBox_preparation->setValue(m_kholleur->getPreparation());
    ui->spinBox_pupils->setValue(m_kholleur->getPupils());

    // Connect the button to update
    connect(ui->pushButton_update, SIGNAL(clicked()), this, SLOT(update_kholleur()));

    // Make the request for the subjects
    QSqlQuery query(*m_db);
    query.exec("SELECT id, name, shortName, color FROM tau_subjects ORDER BY shortName");

    // Treat the request & Display the subjects in the comboBox
    ui->comboBox_subjects->addItem("", (qulonglong) 0);
    while (query.next()) {
        Subject* subj = new Subject();
        subj->setId(query.value(0).toInt());
        subj->setName(query.value(1).toString());
        subj->setShortName(query.value(2).toString());
        subj->setColor(query.value(3).toString());

        ui->comboBox_subjects->addItem(subj->getShortName(), (qulonglong) subj);
        queue_subjects.enqueue(subj);

        // To select the correct subject
        if(m_kholleur->getId_subjects() == subj->getId()) {
            int index = ui->comboBox_subjects->findData((qulonglong) subj);
            if(index != -1)
               ui->comboBox_subjects->setCurrentIndex(index);
        }

    }
}

UpdateKholleurDialog::~UpdateKholleurDialog() {
    delete ui;
    // Free the subjects of the comboBox
    while (!queue_subjects.isEmpty())
        delete queue_subjects.dequeue();
}

bool UpdateKholleurDialog::update_kholleur() {
    // Get the parameters given par the user
    QString name = ui->lineEdit_name->text();
    Subject* subj = (Subject*) ui->comboBox_subjects->currentData().toLongLong();
    int duration = ui->spinBox_duration->value();
    int preparation = ui->spinBox_preparation->value();
    int nbPupils = ui->spinBox_pupils->value();

    if(name == "") { //If there is no name...
        QMessageBox::critical(this, "Erreur", "Il faut renseigner le nom du kholleur.");
        return false;
    } else { // Else update the kholleur
        QSqlQuery query(*m_db);
        query.prepare("UPDATE tau_kholleurs "
                      "SET name=:name, id_subjects=:id_subjects, duration=:duration, preparation=:preparation, pupils=:pupils "
                      "WHERE id=:id");
        query.bindValue(":name", name);
        query.bindValue(":id_subjects", subj ? subj->getId() : 0);
        query.bindValue(":duration", duration);
        query.bindValue(":preparation", preparation);
        query.bindValue(":pupils", nbPupils);
        query.bindValue(":id", m_kholleur->getId());
        query.exec();

        QMessageBox::warning(this, "Attention", "Les horaires de kholles ne sont pas automatiquement mis à jour.\n"
                                                "Pour que ces changements se répercutent sur les horaires, il faut les rentrer à nouveau.");

        accept();
    }

    return true;
}

