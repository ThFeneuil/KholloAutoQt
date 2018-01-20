#include "groupsswappingsmanager.h"
#include "ui_groupsswappingsmanager.h"

GroupsSwappingsManager::GroupsSwappingsManager(QSqlDatabase *db, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::GroupsSwappingsManager)
{
    ui->setupUi(this);
    m_db = db;
    m_listGroups = new QList<Group*>();

    *m_listGroups = GroupsDBInterface(m_db).load("ORDER BY UPPER(`name`)");

    for(int i=0; i<m_listGroups->count(); i++) {
        Group* grp = m_listGroups->at(i);
        ui->comboBox_group1->addItem(grp->getName(), (qulonglong) grp);
        ui->comboBox_group2->addItem(grp->getName(), (qulonglong) grp);
    }

    connect(ui->pushButton_valid, SIGNAL(clicked(bool)), this, SLOT(swapGroups()));
    m_shortcutNotepad = Notepad::shortcut();
    this->addAction(m_shortcutNotepad);
}

GroupsSwappingsManager::~GroupsSwappingsManager() {
    while(!m_listGroups->isEmpty()) {
        delete m_listGroups->takeFirst();
    }
    delete m_listGroups;

    delete ui;
    this->removeAction(m_shortcutNotepad);
    delete m_shortcutNotepad;
}

bool GroupsSwappingsManager::swapGroups() {
    Group* gr1 = (Group*) ui->comboBox_group1->currentData().toLongLong();
    Group* gr2 = (Group*) ui->comboBox_group2->currentData().toLongLong();
    bool swapStudents = ui->radioButton_swapStudents->isChecked();

    if(gr1 == NULL || gr2 == NULL) {
        QMessageBox::critical(NULL, "Echange impossible", "Veuillez sélectionner des groupes à échanger...");
        return false;
    } else if(gr1->getId() == gr2->getId()) {
        QMessageBox::critical(NULL, "Echange impossible", "Vous avez sélectionné le même groupe.");
        return false;
    } else if(swapStudents) {
        int res = QMessageBox::information(NULL, "Echange de groupes",
                "Vous êtes sur le point d'échanger les <strong>élèves</strong> entre le groupe <strong>"+gr1->getName()+"</strong> et le groupe <strong>"+gr2->getName()+"</strong>. <br />"
                "Voulez-vous continuez ?", QMessageBox::Yes |QMessageBox::Cancel);
        if(res == QMessageBox::Yes) {
            m_db->transaction();
            StudentGroupLinksDBInterface inter(m_db);
            QList<StudentGroupLink*> grp1 = inter.load("WHERE `id_groups` = " + QString::number(gr1->getId()));
            QList<StudentGroupLink*> grp2 = inter.load("WHERE `id_groups` = " + QString::number(gr2->getId()));
            int numRowsAffected = grp1.count() + grp2.count();

            for(StudentGroupLink *l : grp1) {
                l->setId_groups(gr2->getId());
                inter.update(l);
                delete l;
            }
            grp1.clear();

            for(StudentGroupLink *l : grp2) {
                l->setId_groups(gr1->getId());
                inter.update(l);
                delete l;
            }
            grp2.clear();
            m_db->commit();

            ui->infoArea->setPlainText("Echange effectué : " + QString::number(numRowsAffected) + " élèves affectés...");
        }
    } else {
        int res = QMessageBox::information(NULL, "Echange de groupes",
                "Vous êtes sur le point d'échanger les <strong>cours</strong> entre le groupe <strong>"+gr1->getName()+"</strong> et le groupe <strong>"+gr2->getName()+"</strong>. <br />"
                "Voulez-vous continuez ?", QMessageBox::Yes |QMessageBox::Cancel);
        if(res == QMessageBox::Yes) {
            int numRowsAffected = 0;
            QSqlQuery query(*m_db);
            query.prepare("UPDATE `tau_courses` SET `id_groups` = 0 WHERE `id_groups` = :idGrp1");
            query.bindValue(":idGrp1", gr1->getId());
            query.exec();
            query.prepare("UPDATE `tau_courses` SET `id_groups` = :idGrp1 WHERE `id_groups` = :idGrp2");
            query.bindValue(":idGrp1", gr1->getId());
            query.bindValue(":idGrp2", gr2->getId());
            query.exec();
            numRowsAffected += query.numRowsAffected();
            query.prepare("UPDATE `tau_courses` SET `id_groups` = :idGrp2 WHERE `id_groups` = 0");
            query.bindValue(":idGrp2", gr2->getId());
            query.exec();
            numRowsAffected += query.numRowsAffected();
            ui->infoArea->setPlainText("Echange effectué : " + QString::number(numRowsAffected) + " horaires de cours affectés...");
        }
    }

    return true;
}
