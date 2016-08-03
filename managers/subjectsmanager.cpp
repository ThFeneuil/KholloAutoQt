#include "managers/subjectsmanager.h"
#include "ui_subjectsmanager.h"

SubjectsManager::SubjectsManager(QSqlDatabase *db, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SubjectsManager)
{
    // Apply the design of the dialog box
    ui->setupUi(this);
    m_subjectColor = new QColor(255, 255, 255);
    connect(ui->pushButton_add, SIGNAL(clicked(bool)), this, SLOT(add_subject()));
    connect(ui->pushButton_delete, SIGNAL(clicked(bool)), this, SLOT(delete_subject()));
    connect(ui->pushButton_color, SIGNAL(clicked(bool)), this, SLOT(select_color()));

    // DB
    m_db = db;
    update_list();
}

SubjectsManager::~SubjectsManager()
{
    delete ui;
    free_subjects();
}

bool SubjectsManager::free_subjects() {
    while (!queue_displayedSubjects.isEmpty())
        delete queue_displayedSubjects.dequeue();
    return true;
}

bool SubjectsManager::update_list() {
    // Clear the list
    ui->list_subjects->clear();
    free_subjects();

    // Make the request
    QSqlQuery query(*m_db);
    query.exec("SELECT id, name, shortName, color FROM tau_subjects ORDER BY name, shortName");

    // Treat the request
    while (query.next()) {
        Subject* subj = new Subject();
        subj->setId(query.value(0).toInt());
        subj->setName(query.value(1).toString());
        subj->setShortName(query.value(2).toString());
        subj->setColor(query.value(3).toString());

        QListWidgetItem *item = new QListWidgetItem(subj->getName() + " (" + subj->getShortName() + ") : " + subj->getColor(), ui->list_subjects);
        item->setData(Qt::UserRole, (qulonglong) subj);
        item->setBackgroundColor(QColor(subj->getColor()));
        queue_displayedSubjects.enqueue(subj);
    }

    return true;
}

bool SubjectsManager::add_subject() {
    QString name = ui->lineEdit_long->text();
    QString shortName = ui->lineEdit_short->text();

    if(name == "" || shortName == "") {
        QMessageBox::critical(this, "Erreur", "Il faut renseigner à la fois le nom long et le nom court.");
        return false;
    } else {
        QSqlQuery query(*m_db);
        query.prepare("INSERT INTO tau_subjects(name, shortName, color) VALUES(:long, :short, :color)");
        query.bindValue(":long", name);
        query.bindValue(":short", shortName);
        query.bindValue(":color", m_subjectColor->name());
        query.exec();

        ui->lineEdit_long->clear();
        ui->lineEdit_short->clear();

        update_list();
    }

    return true;
}

bool SubjectsManager::delete_subject() {
    QListWidgetItem *item = ui->list_subjects->currentItem();

    if(item == NULL) {
        QMessageBox::critical(this, "Erreur", "Veuillez sélectionner une matière.");
        return false;
    } else {
        Subject* subj = (Subject*) item->data(Qt::UserRole).toULongLong();
        int res = QMessageBox::warning(this, "Suppression en cours",
                "Vous êtes sur le point de supprimer la matière <strong>" + subj->getName() + " (" + subj->getShortName() + ")</strong> ainsi que toutes les <strong>données associées</strong> : cours, kholleurs, horaires de kholles, kholles.<br /> Voulez-vous continuer ?",
                QMessageBox::Yes | QMessageBox::Cancel);
        if(res == QMessageBox::Yes) {
            QSqlQuery query(*m_db);
            query.prepare("DELETE FROM tau_courses WHERE id_subjects=:id_subjects");
            query.bindValue(":id_subjects", subj->getId());
            query.exec();
            query.prepare("UPDATE tau_teachers SET id_subjects=0 WHERE id_subjects=:id_subjects");
            query.bindValue(":id_subjects", subj->getId());
            query.exec();
            query.prepare("DELETE FROM tau_kholles WHERE id_timeslots IN "
                            "(SELECT id FROM tau_timeslots WHERE id_kholleurs IN "
                                "(SELECT id FROM tau_kholleurs WHERE id_subjects = :id_subjects))");
            query.bindValue(":id_subjects", subj->getId());
            query.exec();
            query.prepare("DELETE FROM tau_timeslots WHERE id_kholleurs IN "
                            "(SELECT id FROM tau_kholleurs WHERE id_subjects = :id_subjects)");
            query.bindValue(":id_subjects", subj->getId());
            query.exec();
            query.prepare("DELETE FROM tau_kholleurs WHERE id_subjects = :id_subjects");
            query.bindValue(":id_subjects", subj->getId());
            query.exec();
            query.prepare("DELETE FROM tau_subjects WHERE id=:id");
            query.bindValue(":id", subj->getId());
            query.exec();

            update_list();;
        }
    }

    return true;
}

bool SubjectsManager::select_color() {
    *m_subjectColor = QColorDialog::getColor(*m_subjectColor, this);
    // To color the COLOR button
    ui->pushButton_color->setStyleSheet("QPushButton { background-color: "+m_subjectColor->name()+"; }");

    return true;
}
