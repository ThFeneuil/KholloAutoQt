#include "managers/subjectsmanager.h"
#include "ui_subjectsmanager.h"

SubjectsManager::SubjectsManager(QSqlDatabase *db, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SubjectsManager)
{
    // Apply the design of the dialog box
    ui->setupUi(this);
    ui->spinBox_weight->setMinimum(0);
    ui->spinBox_weight->setMaximum(MaxWeightSubject);
    connect(ui->pushButton_add, SIGNAL(clicked(bool)), this, SLOT(add_subject()));
    connect(ui->pushButton_delete, SIGNAL(clicked(bool)), this, SLOT(delete_subject()));
    connect(ui->list_subjects, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(update_subject()));

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
    queue_displayedSubjects = SubjectsDBInterface(m_db).load("ORDER BY UPPER(name), UPPER(shortName)");

    // Treat the request
    for (Subject *subj : queue_displayedSubjects) {
        QListWidgetItem *item = new QListWidgetItem(subj->getName() + " (" + subj->getShortName() + ") : " + QString::number(subj->getWeight()), ui->list_subjects);
        item->setData(Qt::UserRole, (qulonglong) subj);
    }

    return true;
}

bool SubjectsManager::add_subject() {
    QString name = ui->lineEdit_long->text();
    QString shortName = ui->lineEdit_short->text();
    int weight = ui->spinBox_weight->value();

    if(name == "" || shortName == "") {
        QMessageBox::critical(this, "Erreur", "Il faut renseigner à la fois le nom long et le nom court.");
        return false;
    } else {
        Subject *s = new Subject();
        s->setName(name);
        s->setShortName(shortName);
        s->setWeight(weight);
        SubjectsDBInterface(m_db).insert(s);
        delete s;

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
            m_db->transaction();
            SubjectsDBInterface(m_db).remove(subj->getId());
            m_db->commit();
            update_list();;
        }
    }

    return true;
}

bool SubjectsManager::update_subject() {
    QListWidgetItem *item = ui->list_subjects->currentItem();

    if(item == NULL) {
        QMessageBox::critical(this, "Erreur", "Veuillez sélectionner un étudiant.");
        return false;
    } else {
        Subject* subj = (Subject*) item->data(Qt::UserRole).toULongLong();
        UpdateSubjectDialog updateBox(m_db, subj, this);
        if(updateBox.exec() == QDialog::Accepted) {
            update_list();
        }
    }

    return true;
}
