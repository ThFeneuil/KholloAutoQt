/*
 *	File:			(Source) interfacetab.cpp
 *  Comment:        INTERFACE FILE
 *	Description:    Class of a tab which displays the khollotable for a subject
 */

#include "interfacetab.h"
#ifndef Q_WS_MAC
    #include "ui_interfacetab.h"
#else
    #include "ui_interfacetab_mac.h"
#endif



InterfaceTab::InterfaceTab(Subject* subj, int id_week, QDate monday, QSqlDatabase *db, DataBase *dbase, QWidget *parent, InterfaceDialog* interface) :
    QWidget(parent),
    ui(new Ui::InterfaceTab)
{   /** CONSTRUCTOR **/

    /// Initalisation of the class properties
    ui->setupUi(this); // GUI
    m_interface = interface;
    m_subject = subj;
    m_db = db; // SQL database
    m_dbase = dbase; // Local database
    m_id_week = id_week; // Parity of the week
    m_monday = monday; // Monday of the selected week

    /// Create the khollotable
    KholloTable* scene = new KholloTable(m_db, m_dbase, id_week, m_monday, ui->areaKholles, m_interface, this);
    ui->viewTable->setScene(scene);

    /// Initilize the kholleurs list with the kholleurs of the selected subject
    QSqlQuery query(*m_db);
    query.prepare("SELECT `id` FROM `tau_kholleurs` WHERE `id_subjects`=:id_subjects ORDER BY UPPER(`name`)");
    query.bindValue(":id_subjects", m_subject->getId());
    query.exec();
    while (query.next()) {
        // Get the kholleur
        Kholleur* khll = m_dbase->listKholleurs()->value(query.value(0).toInt());
        // Display the kholleur
        QListWidgetItem *item = new QListWidgetItem(khll->getName(), ui->list_kholleurs);
        item->setData(Qt::UserRole, (qulonglong) khll);
    }
    // Connect to detect a new selected kholleur
    connect(ui->list_kholleurs, SIGNAL(itemSelectionChanged()), this, SLOT(displayKholleur()));
    connect(ui->list_kholleurs, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(openReviewWithSelection()));
}

InterfaceTab::~InterfaceTab() {
    /** DESTRUCTOR **/
    delete ui;
}

Subject* InterfaceTab::getSubject() const {
    /** GETTER RETURNING THE SUBJECT PROPERTY **/
    return m_subject;
}

bool InterfaceTab::displayKholleur() {
    /** METHOD TO UPDATE THE KHOLLOTABLE ACCORDING THE SELECTED KHOLLEUR **/
    QListWidgetItem* item = ui->list_kholleurs->currentItem();

    if(item == NULL) {
        // S'il n'y aucun kholleur sélectionné
        QMessageBox::critical(this, "Erreur", "Veuillez sélectionner un kholleur.");
        return false;
    } else {
        // Get the selected kholleur
        Kholleur* kll = (ui->list_kholleurs->selectedItems().length() > 0) ? (Kholleur*) item->data(Qt::UserRole).toULongLong() : NULL;
        // Display the selected kholleur
        ((KholloTable*) ui->viewTable->scene())->displayKholleur(kll);
    }

    return true;
}

bool InterfaceTab::selectStudent(Student* stud) {
    /** METHOD TO UPDATE THE KHOLLEURS LIST AND THE KHOLLOTABLE ACCORDING THE SELECTED STUDENT **/

    /// Update the data displayed with the kholleurs (last kholle, nb kholles)
    for(int i=0; i<ui->list_kholleurs->count(); i++) {
        // For each kholleur
        QListWidgetItem* item =  ui->list_kholleurs->item(i);
        Kholleur* kll = (Kholleur*) item->data(Qt::UserRole).toULongLong();
        if(stud) { // If a student is selected
            // Get the date of kholles with this kholleur
            QSqlQuery query(*m_db);
            query.prepare("SELECT T.`date` "
                          "FROM `tau_kholles` AS K "
                          "JOIN `tau_timeslots` AS T "
                            "ON T.`id` = K.`id_timeslots` "
                          "WHERE `id_users` = :id_users AND T.`id_kholleurs` = :id_kholleurs");
            query.bindValue(":id_users", stud->getId());
            query.bindValue(":id_kholleurs", kll->getId());
            query.exec();

            // Count the number of kholles and find the last kholle
            int lastKholles = 0;
            int nbKholles = 0;
            QDate today = m_monday;
            while(query.next()) {
                // Check if it is the last kholle
                int nbDays = query.value(0).toDate().daysTo(today);
                if(nbKholles == 0 || abs(nbDays) < abs(lastKholles))
                    lastKholles = nbDays;
                // Increase the number of kholles
                nbKholles++;
            }

            // Display the kholleur and the number of kholles with the selected students
            item->setText(kll->getName() + " (" + QString::number(nbKholles) + ")");
            // Display the last kholle thanks to a tooltip
            if(nbKholles) {
                int week = (lastKholles > 0) ? (lastKholles-1) / 7 + 1 : lastKholles / 7;
                if(0 >= lastKholles && lastKholles > -7)
                    item->setToolTip("Cette semaine...");
                else if(abs(week) == 1)
                    item->setToolTip(QString::number(week) + " semaine (" + QString::number(lastKholles) + ")");
                else
                    item->setToolTip(QString::number(week) + " semaines (" + QString::number(lastKholles) + ")");
            } else
                item->setToolTip("");
        } else {
            // If no student is selected --> Just display the names of kholleurs
            item->setText(kll->getName());
            item->setToolTip("");
        }
    }

    /// Update the khollotable (displaying student courses and kholles)
    ((KholloTable*) ui->viewTable->scene())->displayStudent(stud);

    return true;
}

bool InterfaceTab::selectKholleur(Kholleur* khll) {
    /** METHOD TO SELECT MANUALLY A KHOLLEUR IN THE LIST **/
    if(khll) {
        for(int i=0; i<ui->list_kholleurs->count(); i++) {
            // For each kholleur of the list
            QListWidgetItem* item =  ui->list_kholleurs->item(i);
            Kholleur* khll_item = (Kholleur*) item->data(Qt::UserRole).toULongLong();
            // Test if it is the searched kholleur
            if(khll->getId() == khll_item->getId()) {
                // If YES --> select the item
                ui->list_kholleurs->setCurrentItem(item);
                return true;
            }
        }
    }
    return false;
}

bool InterfaceTab::selectTimeslot(Timeslot* slot) {
    if(slot)
        return ((KholloTable*) ui->viewTable->scene())->selectionTimeslot(slot);
    return false;
}

bool InterfaceTab::openReviewWithSelection() {
    return ((KholloTable*) ui->viewTable->scene())->openReviewWithSelection();
}
