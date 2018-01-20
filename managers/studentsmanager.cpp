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
    connect(ui->btn_import, SIGNAL(clicked()), this, SLOT(import_students()));

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
        delete queue_displayedStudents.dequeue();
    return true;
}

bool StudentsManager::update_list() {
    // Clear the list
    ui->list_students->clear();
    free_students();

    // Make the request
    queue_displayedStudents = StudentsDBInterface(m_db).load("ORDER BY UPPER(name), UPPER(first_name)");

    // Treat the request
    for (Student *stdnt : queue_displayedStudents) {
        QListWidgetItem *item = new QListWidgetItem(stdnt->getName() + ", " + stdnt->getFirst_name() + ", " + stdnt->getEmail(), ui->list_students);
        item->setData(Qt::UserRole, (qulonglong) stdnt);
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
        Student *s = new Student();
        s->setName(name);
        s->setFirst_name(firstName);
        s->setEmail(email);

        StudentsDBInterface(m_db).insert(s);
        delete s;

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
                "Vous êtes sur le point de supprimer <strong>" + stdnt->getFirst_name() + " " + stdnt->getName() + "</strong> ainsi que ses <strong>kholles</strong>.<br /> Voulez-vous continuer ?",
                QMessageBox::Yes | QMessageBox::Cancel);
        if(res == QMessageBox::Yes) {
            m_db->transaction();
            StudentsDBInterface(m_db).remove(stdnt->getId());
            m_db->commit();

            update_list();
        }
    }

    return true;
}

void StudentsManager::import_students() {
    //Try to load directory preferences
    Preferences pref;
    QString pref_path = pref.dir();

    //Get file name
    QString filename = QFileDialog::getOpenFileName(this, "Ouvrir un fichier", pref_path + QDir::separator(), "CSV (*.csv)");

    if(filename == "")
        return;

    //Use StudentsImportManager
    StudentsImportManager manager(m_db, this);
    if(manager.load(filename))
        QMessageBox::information(this, "Importation", "Importation des étudiants réussie.");
    update_list();
}
