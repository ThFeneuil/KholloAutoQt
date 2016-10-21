/*
 *	File:			(Source) interfaceDialog.cpp
 *  Comment:        INTERFACE FILE
 *	Description:    Class of the dialog which contains the interface
 *
 */

#include "interfacedialog.h"
#include "ui_interfacedialog.h"

InterfaceDialog::InterfaceDialog(QSqlDatabase *db, int id_week, QDate monday, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::InterfaceDialog)
{
    /// Initalisation of the class properties
    ui->setupUi(this); // GUI
    m_db = db;
    m_students = new QList<Student*>();
    m_id_week = id_week;
    m_monday = monday;
    m_doubleSelectedItem = NULL;

    /// Load the DB in the RAM (DataBase object) selecting the courses and the timeslots of the selected week
    m_dbase = new DataBase(m_db);
    m_dbase->setConditionCourses("id_week = " + QString::number(m_id_week));
    m_dbase->setConditionTimeslots("date >= '"+ m_monday.toString("yyyy-MM-dd") +"' AND date < '" + m_monday.addDays(7).toString("yyyy-MM-dd") + "'");
    m_dbase->load();

    /// Update the students list (get, display, connect)
    QSqlQuery query(*m_db);
    query.exec("SELECT id FROM tau_users ORDER BY name, first_name");
    while (query.next()) {
        // Get the student
        Student* stdnt = m_dbase->listStudents()->value(query.value(0).toInt());
        m_students->append(stdnt);

        // Display the student
        QListWidgetItem *item = new QListWidgetItem(stdnt->getName() + " " + stdnt->getFirst_name(), ui->list_students);
        item->setData(Qt::UserRole, (qulonglong) stdnt);
        item->setIcon(QIcon(QPixmap(":/images/none.png")));
    }
    // Connect to detect a click and a double-click on a student
    connect(ui->list_students, SIGNAL(itemSelectionChanged()), this, SLOT(selectStudent()));
    connect(ui->list_students, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(doubleSelectStudent(QListWidgetItem*)));

    query.exec("SELECT `id`, `name`, `shortName`, `color` FROM `tau_subjects` WHERE `id` IN (SELECT DISTINCT `id_subjects` FROM `tau_kholleurs`)");
    while (query.next()) {
        Subject* subj = new Subject();
        subj->setId(query.value(0).toInt());
        subj->setName(query.value(1).toString());
        subj->setShortName(query.value(2).toString());
        subj->setColor(query.value(3).toString());

        InterfaceTab* tab = new InterfaceTab(subj, m_id_week, m_monday, m_db, m_dbase, NULL, this);
        ui->tabWidget->addTab(tab, subj->getShortName());
    }

    if(ui->tabWidget->currentWidget()) {
        update_list(((InterfaceTab*) ui->tabWidget->currentWidget())->getSubject());
    } else {
        QMessageBox::warning(this, "Aucune matière sélectionnée", "Aucune matière n'est sélectionnée...<br />Veuillez vérifier si au moins une matière est attribuée à un kholleur.");
        update_list();
    }
    connect(ui->tabWidget, SIGNAL(currentChanged(int)), this, SLOT(detectChangeTab(int)));
    connect(ui->pushButton_print, SIGNAL(clicked(bool)), this, SLOT(printKholloscope()));
    connect(ui->pushButton_review, SIGNAL(clicked(bool)), this, SLOT(openReviewDialog()));
}

InterfaceDialog::~InterfaceDialog() {
    delete ui;
    delete m_dbase;
    delete m_students;
}

bool InterfaceDialog::selectStudent(Student *stud) {
    if(stud == NULL) {
        QListWidgetItem *item = ui->list_students->currentItem();

        if(item == NULL) {
            QMessageBox::critical(this, "Erreur", "Veuillez sélectionner un étudiant.");
            return false;
        } else {
            Student* stud = (Student*) item->data(Qt::UserRole).toULongLong();
            for(int i=0; i<ui->tabWidget->count(); i++)
                ((InterfaceTab*) ui->tabWidget->widget(i))->selectStudent(stud);
        }
    } else {
       for(int i=0; i<ui->list_students->count(); i++) {
            QListWidgetItem *item = ui->list_students->item(i);
            Student* stdnt = (Student*) item->data(Qt::UserRole).toULongLong();
            if(stdnt->getId() == stud->getId()) {
                ui->list_students->setCurrentItem(item);
                for(int j=0; j<ui->tabWidget->count(); j++)
                    ((InterfaceTab*) ui->tabWidget->widget(j))->selectStudent(stdnt);
                return true;
            }
       }
    }

    return true;
}

void InterfaceDialog::printKholloscope() {
    PrintPDF::printKholles(m_students, m_dbase->listKholleurs(), m_dbase->listTimeslots(), m_monday, m_dbase->listKholles());
}

void InterfaceDialog::openReviewDialog() {
    ReviewDialog dialog(m_db);
    dialog.exec();
}

bool InterfaceDialog::detectChangeTab(int index) {
    InterfaceTab* tab = (InterfaceTab*) ui->tabWidget->widget(index);
    update_list(tab->getSubject());
    return true;
}

bool InterfaceDialog::update_list(Subject* subj) {
    for(int i=0; i<ui->list_students->count(); i++) {
        QListWidgetItem *item = ui->list_students->item(i);
        if(subj) {
            Student* stdnt = (Student*) item->data(Qt::UserRole).toULongLong();
            QList<Kholle*>* listKholles = stdnt->kholles();
            int nbKholles = 0;
            for(int j=0; j<listKholles->count(); j++) {
                if(listKholles->value(j)->timeslot()->kholleur()->getId_subjects() == subj->getId())
                    nbKholles++;
            }
            if(nbKholles <= 0)
                item->setIcon(QPixmap(":/images/nbKh0.png"));
            else if(nbKholles == 1)
                item->setIcon(QPixmap(":/images/nbKh1.png"));
            else
                item->setIcon(QPixmap(":/images/nbKh2.png"));
        } else {
            item->setIcon(QPixmap(":/images/none.png"));
        }
    }
    return true;
}

bool InterfaceDialog::doubleSelectStudent(QListWidgetItem* item) {
    QColor doubleSelectedColor(255, 100, 150);
    QColor empty(255, 255, 255);

    if(item) {
        if(item != m_doubleSelectedItem) {
            if(m_doubleSelectedItem) {
                item->setBackgroundColor(doubleSelectedColor);
                // Change
                Subject* subj = ((InterfaceTab*) ui->tabWidget->currentWidget())->getSubject();
                Student* stud1 = (Student*) item->data(Qt::UserRole).toULongLong();
                Student* stud2 = (Student*) m_doubleSelectedItem->data(Qt::UserRole).toULongLong();

                int res = QMessageBox::information(NULL, "Échange de kholles", "Vous êtes sur le point d'échanger les kholles de <strong>" + subj->getName() + "</strong> entre <strong>" + stud1->getFirst_name() + " " + stud1->getName() + "</strong> et <strong>" + stud2->getFirst_name() + " " + stud2->getName() + "</strong>. Voulez-vous continuer ?", QMessageBox::Yes | QMessageBox::Cancel);
                if(res == QMessageBox::Yes) {
                    QSqlQuery query(*m_db);
                    query.exec("SELECT Kh.id, Kh.id_users FROM tau_kholles AS Kh "
                               "WHERE (Kh.id_users = " + QString::number(stud1->getId()) + " OR Kh.id_users = " + QString::number(stud2->getId()) + ") AND Kh.id_timeslots IN "
                                    "(SELECT T.id FROM tau_timeslots AS T "
                                        "JOIN tau_kholleurs AS K ON T.id_kholleurs = K.id "
                                        "WHERE (T.date >= '"+ m_monday.toString("yyyy-MM-dd") +"' AND date < '" + m_monday.addDays(7).toString("yyyy-MM-dd") + "') AND K.id_subjects = " + QString::number(subj->getId())+ ")");
                    while (query.next()) {
                        int id = query.value(0).toInt();
                        int idStud_origin = query.value(1).toInt();
                        int idStud_target = (idStud_origin == stud1->getId()) ? stud2->getId() : stud1->getId();

                        Kholle* khlle = m_dbase->listKholles()->value(id);
                        khlle->setId_students(idStud_target);
                        khlle->setStudent(m_dbase->listStudents()->value(idStud_target));
                        m_dbase->listStudents()->value(idStud_origin)->kholles()->removeAll(khlle);
                        m_dbase->listStudents()->value(idStud_target)->kholles()->append(khlle);

                        QSqlQuery queryUpdate(*m_db);
                        queryUpdate.exec("UPDATE tau_kholles SET id_users = " + QString::number(idStud_target) + " WHERE id = " + QString::number(id));
                    }
                    update_list(subj);
                    for(int i=0; i<ui->tabWidget->count(); i++)
                        ((InterfaceTab*) ui->tabWidget->widget(i))->selectStudent(stud1);
                }
                item->setBackgroundColor(empty);
                m_doubleSelectedItem->setBackgroundColor(empty);
                m_doubleSelectedItem = NULL;
            } else {
                item->setBackgroundColor(doubleSelectedColor);
                m_doubleSelectedItem = item;
            }
        } else {
            item->setBackgroundColor(empty);
            m_doubleSelectedItem = NULL;
        }
    }

    return true;
}
