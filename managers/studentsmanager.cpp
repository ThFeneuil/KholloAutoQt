#include "managers/studentsmanager.h"
#include "ui_studentsmanager.h"

StudentsManager::StudentsManager(QSqlDatabase *db, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::StudentsManager)
{
    // Apply the design of the dialog box
    ui->setupUi(this);
    connect(ui->pushButton_add, SIGNAL(clicked()), this, SLOT(add_student()));
    connect(ui->pushButton_update, SIGNAL(clicked()), this, SLOT(update_student()));
    connect(ui->pushButton_delete, SIGNAL(clicked()), this, SLOT(delete_student()));
    connect(ui->list_students, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(update_student()));

    // DB
    m_db = db;
    update_list();
}

StudentsManager::~StudentsManager() {
    delete ui;
    free_students();
}

bool StudentsManager::free_students() {
    while (!queue_displayedStudents.isEmpty())
        free(queue_displayedStudents.dequeue());
    return true;
}

bool StudentsManager::update_list() {
    // Clear the list
    ui->list_students->clear();
    free_students();

    // Make the request
    QSqlQuery query(*m_db);
    query.exec("SELECT id, name, first_name, email FROM tau_users ORDER BY name, first_name");

    // Treat the request
    while (query.next()) {
        Student* stdnt = new Student();
        stdnt->setId(query.value(0).toInt());
        stdnt->setName(query.value(1).toString());
        stdnt->setFirst_name(query.value(2).toString());
        stdnt->setEmail(query.value(3).toString());
        QListWidgetItem *item = new QListWidgetItem(stdnt->getName() + ", " + stdnt->getFirst_name() + ", " + stdnt->getEmail(), ui->list_students);
        item->setData(Qt::UserRole, (qulonglong) stdnt);
        queue_displayedStudents.enqueue(stdnt);
    }

    return true;
}

bool StudentsManager::add_student() {
    QString name = ui->lineEdit_name->text();
    QString firstName = ui->lineEdit_firstName->text();
    QString email = ui->lineEdit_email->text();

    if(name == "" || firstName == "") {
        QMessageBox::critical(this, "Erreur", "Il faut renseigner le nom et le prénom.");
        return false;
    } else {
        QSqlQuery query(*m_db);
        query.prepare("INSERT INTO tau_users(is_account, name, first_name, email) VALUES(1, :name, :firstName, :email)");
        query.bindValue(":name", name);
        query.bindValue(":firstName", firstName);
        query.bindValue(":email", email);
        query.exec();

        ui->lineEdit_name->clear();
        ui->lineEdit_firstName->clear();
        ui->lineEdit_email->clear();

        update_list();
    }

    return true;
}

bool StudentsManager::update_student() {
    QListWidgetItem *item = ui->list_students->currentItem();

    if(item == NULL) {
        QMessageBox::critical(this, "Erreur", "Veuillez sélectionner un étudiant.");
        return false;
    } else {
        Student* stdnt = (Student*) item->data(Qt::UserRole).toULongLong();
        UpdateStudentDialog updateBox(m_db, stdnt, this);
        if(updateBox.exec() == QDialog::Accepted) {
            update_list();
        }
    }

    return true;
}

bool StudentsManager::delete_student() {
    QListWidgetItem *item = ui->list_students->currentItem();

    if(item == NULL) {
        QMessageBox::critical(this, "Erreur", "Veuillez sélectionner un étudiant.");
        return false;
    } else {
        Student* stdnt = (Student*) item->data(Qt::UserRole).toULongLong();
        int res = QMessageBox::warning(this, "Suppression en cours",
                "Vous êtes sur le point de supprimer <strong>" + stdnt->getFirst_name() + " " + stdnt->getName() + "</strong>.<br /> Voulez-vous continuer ?",
                QMessageBox::Yes | QMessageBox::Cancel);
        if(res == QMessageBox::Yes) {
            QSqlQuery query(*m_db);
            query.prepare("DELETE FROM tau_users WHERE id=:id");
            query.bindValue(":id", stdnt->getId());
            query.exec();

            update_list();
        }
    }

    return true;
}
