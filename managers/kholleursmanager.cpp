#include "managers/kholleursmanager.h"
#include "ui_kholleursmanager.h"

KholleursManager::KholleursManager(QSqlDatabase *db, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::KholleursManager)
{
    // Initialisation
    ui->setupUi(this);
    m_db = db;

    // Connect the buttons for kholleurs
    connect(ui->pushButton_addKholleur, SIGNAL(clicked()), this, SLOT(add_kholleur()));
    connect(ui->pushButton_updateKholleur, SIGNAL(clicked()), this, SLOT(update_kholleur()));
    connect(ui->pushButton_deleteKholleur, SIGNAL(clicked()), this, SLOT(delete_kholleur()));
    connect(ui->list_kholleurs, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(update_kholleur()));

    // Display the kholleurs
    update_listKholleurs();
}

KholleursManager::~KholleursManager() {
    delete ui;
    // Free memories
    free_kholleurs();
}

bool KholleursManager::free_kholleurs() {
    /** To free memories with kholleurs **/
    while (!queue_displayedKholleurs.isEmpty())
        delete queue_displayedKholleurs.dequeue();
    return true;
}

bool KholleursManager::update_listKholleurs(int idSelected) {
    /** To update the list of the kholleurs **/
    // Clear the list
    ui->list_kholleurs->clear();
    free_kholleurs();

    // Make the request
    QSqlQuery query(*m_db);
    query.exec("SELECT K.id, K.name, K.id_subjects, K.duration, K.preparation, K.pupils, S.shortName "
               "FROM tau_kholleurs AS K "
               "LEFT OUTER JOIN tau_subjects AS S "
                  "ON K.id_subjects = S.id "
               "ORDER BY UPPER(K.name)");

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
        if(subject == "")
            subject = "???";
        QListWidgetItem *item = new QListWidgetItem(khll->getName() + " : " + subject + ", " + QString::number(khll->getDuration()) + ", " + QString::number(khll->getPreparation()) + ", " + QString::number(khll->getPupils()), ui->list_kholleurs);
        item->setData(Qt::UserRole, (qulonglong) khll);
        if(idSelected == khll->getId())
            ui->list_kholleurs->setCurrentItem(item);
        queue_displayedKholleurs.enqueue(khll);
    }

    return true;
}

bool KholleursManager::add_kholleur() {
    /** To add a kholleur in the DB **/
    QString name = ui->lineEdit_kholleur->text();
    int idKholleur = 0;

    if(name == "") { // If there is no name
        QMessageBox::critical(this, "Erreur", "Il faut renseigner le nom du kholleur.");
        return false;
    } else { // Else insert the kholleur in the DB
        QSqlQuery query(*m_db);
        query.prepare("INSERT INTO tau_kholleurs(name, id_subjects, duration, preparation, pupils) VALUES(:name, 0, 0, 0, 0)");
        query.bindValue(":name", name);
        query.exec();
        idKholleur = query.lastInsertId().toInt();
        // Update the widgets
        ui->lineEdit_kholleur->clear();
        update_listKholleurs(idKholleur);
    }

    if(idKholleur > 0)
        update_kholleur();

    return true;
}

bool KholleursManager::update_kholleur() {
    /** (To open a diagol box) To update a kholleur in the DB **/
    QListWidgetItem *item = ui->list_kholleurs->currentItem();

    if(item == NULL) { // If no item is selected
        QMessageBox::critical(this, "Erreur", "Veuillez sélectionner un kholleur.");
        return false;
    } else { // Else ... open the dialog box
        Kholleur* khll = (Kholleur*) item->data(Qt::UserRole).toULongLong();
        UpdateKholleurDialog updateBox(m_db, khll, true, this);
        if(updateBox.exec() == QDialog::Accepted) {
            // Update the list if there are mofications
            update_listKholleurs();
        }
    }

    return true;
}

bool KholleursManager::delete_kholleur() {
    /** To delete a kholleur in the DB **/
    QListWidgetItem *item = ui->list_kholleurs->currentItem();

    if(item == NULL) { // If no item is selected
        QMessageBox::critical(this, "Erreur", "Veuillez sélectionner un kholleur.");
        return false;
    } else {
        Kholleur* khll = (Kholleur*) item->data(Qt::UserRole).toULongLong();
        // Ask for a confirmation of the deletion
        int res = QMessageBox::warning(this, "Suppression en cours",
                "Vous êtes sur le point de supprimer <strong>" + khll->getName() + "</strong>, ses <strong>horaires de kholles</strong> ainsi que ses <strong>kholles</strong>. Voulez-vous continuer ?",
                QMessageBox::Yes | QMessageBox::Cancel);
        if(res == QMessageBox::Yes) {
            // Update the DB if it is a positive answer
            QSqlQuery query(*m_db);
            //Done by SQLITE
            /*query.prepare("DELETE FROM tau_kholles WHERE id_timeslots IN "
                            "(SELECT id FROM tau_timeslots WHERE id_kholleurs=:id_kholleurs)");
            query.bindValue(":id_kholleurs", khll->getId());
            query.exec();
            query.prepare("DELETE FROM tau_timeslots WHERE id_kholleurs=:id_kholleurs");
            query.bindValue(":id_kholleurs", khll->getId());
            query.exec();
            query.prepare("DELETE FROM tau_merge_kholleurs WHERE id_kholleurs=:id_kholleurs");
            query.bindValue(":id_kholleurs", khll->getId());
            query.exec();*/
            query.prepare("DELETE FROM tau_kholleurs WHERE id=:id");
            query.bindValue(":id", khll->getId());
            query.exec();

            // Update the displayed widgets
            update_listKholleurs();;
        }
    }

    return true;
}
