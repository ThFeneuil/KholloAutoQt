#include "managers/kholleursmanager.h"
#include "ui_kholleursmanager.h"

KholleursManager::KholleursManager(QSqlDatabase *db, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::KholleursManager)
{
    // Initialisation
    ui->setupUi(this);
    m_db = db;

    // Connect the buttons for teachers
    connect(ui->pushButton_addTeacher, SIGNAL(clicked()), this, SLOT(add_teacher()));
    connect(ui->pushButton_updateTeacher, SIGNAL(clicked()), this, SLOT(update_teacher()));
    connect(ui->pushButton_deleteTeacher, SIGNAL(clicked()), this, SLOT(delete_teacher()));
    connect(ui->list_teachers, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(update_teacher()));
    // Connect the buttons for kholleurs
    connect(ui->pushButton_addKholleur, SIGNAL(clicked()), this, SLOT(add_kholleur()));
    connect(ui->pushButton_updateKholleur, SIGNAL(clicked()), this, SLOT(update_kholleur()));
    connect(ui->pushButton_deleteKholleur, SIGNAL(clicked()), this, SLOT(delete_kholleur()));
    connect(ui->list_kholleurs, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(update_kholleur()));

    // Display the teachers and the kholleurs
    update_listTeachers();
    update_listKholleurs();
}

KholleursManager::~KholleursManager() {
    delete ui;
    // Free memories
    free_kholleurs();
    free_teachers();
}

bool KholleursManager::free_teachers() {
    /** To free memories with teachers **/
    while (!queue_displayedTeachers.isEmpty())
        delete queue_displayedTeachers.dequeue();
    return true;
}

bool KholleursManager::free_kholleurs() {
    /** To free memories with kholleurs **/
    while (!queue_displayedKholleurs.isEmpty())
        delete queue_displayedKholleurs.dequeue();
    return true;
}

bool KholleursManager::update_listTeachers() {
    /** To update the list of the teachers **/
    // Clear the list
    ui->list_teachers->clear();
    free_teachers();

    // Make the request
    QSqlQuery query(*m_db);
    query.exec("SELECT T.id, T.name, T.id_subjects, S.shortName "
               "FROM tau_teachers AS T "
               "LEFT OUTER JOIN tau_subjects AS S "
                  "ON T.id_subjects = S.id "
               "ORDER BY T.name");

    // Treat the request & Display the teachers
    while (query.next()) {
        // Get the teacher
        Teacher* tcher = new Teacher();
        tcher->setId(query.value(0).toInt());
        tcher->setName(query.value(1).toString());
        tcher->setId_subjects(query.value(2).toInt());
        QString subject = query.value(3).toString();
        // Display the teacher
        if(subject != "")
            subject = "(" + subject + ")";
        QListWidgetItem *item = new QListWidgetItem(tcher->getName() + " " + subject, ui->list_teachers);
        item->setData(Qt::UserRole, (qulonglong) tcher);
        queue_displayedTeachers.enqueue(tcher);
    }

    return true;
}

bool KholleursManager::add_teacher() {
    /** To add a teacher in the DB **/
    QString name = ui->lineEdit_teacher->text();

    if(name == "") { // If there is no name
        QMessageBox::critical(this, "Erreur", "Il faut renseigner le nom de l'enseignant.");
        return false;
    } else { // Else insert the teacher in the DB
        QSqlQuery query(*m_db);
        query.prepare("INSERT INTO tau_teachers(name, id_subjects) VALUES(:name, 0)");
        query.bindValue(":name", name);
        query.exec();

        // Update the widgets
        ui->lineEdit_teacher->clear();
        update_listTeachers();
    }

    return true;
}

bool KholleursManager::update_teacher() {
    /** (To open a diagol box) To update a teacher in the DB **/
    QListWidgetItem *item = ui->list_teachers->currentItem();

    if(item == NULL) { // If no item is selected
        QMessageBox::critical(this, "Erreur", "Veuillez sélectionner un enseignant.");
        return false;
    } else { // Else ... open the dialog box
        Teacher* tcher = (Teacher*) item->data(Qt::UserRole).toULongLong();
        UpdateTeacherDialog updateBox(m_db, tcher, this);
        if(updateBox.exec() == QDialog::Accepted) {
            // Update the list if there are mofications
            update_listTeachers();
        }
    }

    return true;
}

bool KholleursManager::delete_teacher() {
    /** To delete a teacher in the DB **/
    QListWidgetItem *item = ui->list_teachers->currentItem();

    if(item == NULL) { // If no item is selected
        QMessageBox::critical(this, "Erreur", "Veuillez sélectionner un enseignant.");
        return false;
    } else {
        Teacher* tcher = (Teacher*) item->data(Qt::UserRole).toULongLong();
        // Ask for a confirmation of the deletion
        int res = QMessageBox::warning(this, "Suppression en cours",
                "Vous êtes sur le point de supprimer <strong>" + tcher->getName() + "</strong>.<br /> Voulez-vous continuer ?",
                QMessageBox::Yes | QMessageBox::Cancel);
        if(res == QMessageBox::Yes) {
            // Update the DB if it is a positive answer
            QSqlQuery query(*m_db);
            query.prepare("DELETE FROM tau_teachers WHERE id=:id");
            query.bindValue(":id", tcher->getId());
            query.exec();

            // Update the displayed widgets
            update_listTeachers();;
        }
    }

    return true;
}

bool KholleursManager::update_listKholleurs() {
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
            subject = "(" + subject + ") ";
        QListWidgetItem *item = new QListWidgetItem(khll->getName() + " " + subject + ": " + QString::number(khll->getDuration()) + ", " + QString::number(khll->getPreparation()) + ", " + QString::number(khll->getPupils()), ui->list_kholleurs);
        item->setData(Qt::UserRole, (qulonglong) khll);
        queue_displayedKholleurs.enqueue(khll);
    }

    return true;
}

bool KholleursManager::add_kholleur() {
    /** To add a kholleur in the DB **/
    QString name = ui->lineEdit_kholleur->text();

    if(name == "") { // If there is no name
        QMessageBox::critical(this, "Erreur", "Il faut renseigner le nom du kholleur.");
        return false;
    } else { // Else insert the kholleur in the DB
        QSqlQuery query(*m_db);
        query.prepare("INSERT INTO tau_kholleurs(name, id_subjects, duration, preparation, pupils) VALUES(:name, 0, 0, 0, 0)");
        query.bindValue(":name", name);
        query.exec();

        // Update the widgets
        ui->lineEdit_kholleur->clear();
        update_listKholleurs();
    }

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
        UpdateKholleurDialog updateBox(m_db, khll, this);
        if(updateBox.exec() == QDialog::Accepted) {
            // Update the list if there are mofications
            update_listKholleurs();
        }
    }

    return true;
}

bool KholleursManager::delete_kholleur() {
    /** To delete a teacher in the DB **/
    QListWidgetItem *item = ui->list_kholleurs->currentItem();

    if(item == NULL) { // If no item is selected
        QMessageBox::critical(this, "Erreur", "Veuillez sélectionner un kholleur.");
        return false;
    } else {
        Kholleur* khll = (Kholleur*) item->data(Qt::UserRole).toULongLong();
        // Ask for a confirmation of the deletion
        int res = QMessageBox::warning(this, "Suppression en cours",
                "Vous êtes sur le point de supprimer <strong>" + khll->getName() + "</strong>.<br /> Voulez-vous continuer ?",
                QMessageBox::Yes | QMessageBox::Cancel);
        if(res == QMessageBox::Yes) {
            // Update the DB if it is a positive answer
            QSqlQuery query(*m_db);
            query.prepare("DELETE FROM tau_kholleurs WHERE id=:id");
            query.bindValue(":id", khll->getId());
            query.exec();

            // Update the displayed widgets
            update_listKholleurs();;
        }
    }

    return true;
}
