/*
 *	File:			(Source) interfaceDialog.cpp
 *  Comment:        INTERFACE FILE
 *	Description:    Class of the dialog which contains the interface
 */

#include "interfacedialog.h"
#ifndef Q_WS_MAC
    #include "ui_interfacedialog.h"
#else
    #include "ui_interfacedialog_mac.h"
#endif


InterfaceDialog::InterfaceDialog(QSqlDatabase *db, int id_week, QDate monday, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::InterfaceDialog)
{   /** CONSTRUCTOR **/

    /// Initalisation of the class properties
    ui->setupUi(this); // GUI
    m_db = db;
    m_students = new QList<Student*>();
    m_id_week = id_week;
    m_monday = monday;
    m_doubleSelectedItem = NULL;
    QString textWeekLabel = "Semaine : " + m_monday.toString("dd/MM/yyyy");
    if(m_id_week == 1)
        textWeekLabel += " (Paire)";
    else if(m_id_week == 2)
        textWeekLabel += " (Impaire)";
    ui->label_week->setText("<html><head/><body><p align=\"center\"><span style=\" color:#005500;\">"+textWeekLabel+"</span></p></body></html>");
    m_lastActions = new QStack<InterfaceAction*>();
    m_shortcutNotepad = Notepad::shortcut();
    this->addAction(m_shortcutNotepad);

    /// To load the DB in the RAM (DataBase object) selecting the courses and the timeslots of the selected week
    m_dbase = new DataBase(m_db);
    m_dbase->setConditionCourses("`id_week` = " + QString::number(m_id_week));
    m_dbase->setConditionTimeslots("`date` >= '"+ m_monday.toString("yyyy-MM-dd") +"' AND `date` < '" + m_monday.addDays(7).toString("yyyy-MM-dd") + "'");
    m_dbase->load();

    /// To initialize the students list (get, display, connect)
    QSqlQuery query(*m_db);
    query.exec("SELECT `id` FROM `tau_users` ORDER BY UPPER(`name`), UPPER(`first_name`)");
    while (query.next()) {
        // To get the student
        Student* stdnt = m_dbase->listStudents()->value(query.value(0).toInt());
        m_students->append(stdnt);

        // To display the student
        QListWidgetItem *item = new QListWidgetItem(stdnt->getName() + " " + stdnt->getFirst_name(), ui->list_students);
        item->setData(Qt::UserRole, (qulonglong) stdnt);
        item->setIcon(QIcon(QPixmap(":/images/none.png")));
    }
    // To connect to detect a click and a double-click on a student
    connect(ui->list_students, SIGNAL(itemSelectionChanged()), this, SLOT(selectStudent()));
    connect(ui->list_students, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(doubleSelectStudent(QListWidgetItem*)));

    /// Creation of the tab (a subject with a kholleur or more = a tab)
    // To select subjects with a kholleur or more
    query.exec("SELECT `id` FROM `tau_subjects` WHERE `id` IN (SELECT DISTINCT `id_subjects` FROM `tau_kholleurs`)");
    while (query.next()) {
        Subject* subj = m_dbase->listSubjects()->value(query.value(0).toInt());
        // To create and to display a tab
        InterfaceTab* tab = new InterfaceTab(subj, m_id_week, m_monday, m_db, m_dbase, NULL, this);
        ui->tabWidget->addTab(tab, subj->getShortName());
    }

    /// To update the students list (according the selected subject)
    if(ui->tabWidget->currentWidget())
        update_list(((InterfaceTab*) ui->tabWidget->currentWidget())->getSubject());
    else {
        // No selected subject
        QMessageBox::warning(this, "Aucune matière sélectionnée", "Aucune matière n'est sélectionnée...<br />Veuillez vérifier si au moins une matière est attribuée à un kholleur.");
        update_list();
    }

    /// To detect a tab change
    connect(ui->tabWidget, SIGNAL(currentChanged(int)), this, SLOT(detectChangeTab(int)));
    /// To detect when user click on a button
    connect(ui->pushButton_print, SIGNAL(clicked(bool)), this, SLOT(printKholloscope()));
    connect(ui->pushButton_review, SIGNAL(clicked(bool)), this, SLOT(openReviewDialog()));

    QShortcut* shortcut_cancel = new QShortcut(QKeySequence("Ctrl+Z"), this);
    connect(shortcut_cancel, SIGNAL(activated()), this, SLOT(cancelAction()));
}

InterfaceDialog::~InterfaceDialog() {
    /** DESTRUCTOR **/
    // Free memory
    delete ui;
    delete m_dbase; // Free DataBase object
    delete m_students;
    this->removeAction(m_shortcutNotepad);
    delete m_shortcutNotepad;
}

bool InterfaceDialog::selectStudent(Student *stud) {
    /** METHOD TO SELECT A STUDENT, TO UPDATE THE TABS AND THE KHOLLOTABLE  **/

    if(stud == NULL) {
        // If the student is selected by the students list
        QListWidgetItem *item = ui->list_students->currentItem();

        if(item == NULL) {
            QMessageBox::critical(this, "Erreur", "Veuillez sélectionner un étudiant.");
            return false;
        } else {
            // Get the student
            Student* stndt = (ui->list_students->selectedItems().length() > 0) ? (Student*) item->data(Qt::UserRole).toULongLong() : NULL;

            // Update the tabs (and the khollotable)
            for(int i=0; i<ui->tabWidget->count(); i++)
                ((InterfaceTab*) ui->tabWidget->widget(i))->selectStudent(stndt);
        }
    } else {
       // If the student is manually selected
       // Find the student in the students list
       for(int i=0; i<ui->list_students->count(); i++) {
            QListWidgetItem *item = ui->list_students->item(i);
            Student* stdnt = (Student*) item->data(Qt::UserRole).toULongLong();
            if(stdnt->getId() == stud->getId()) {
                // Student found --> Set current item in the students list and update the tabs (and the khollotable)
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
    /** METHOD TO PRINT THE KHOLLOSCOPE OF THE SELECTED WEEK IN A PDF FILE **/
    // Use of the PrintPDF Class
    PrintPDF::printKholles(m_students, m_dbase->listKholleurs(), m_dbase->listTimeslots(), m_monday, m_dbase->listKholles());
}

void InterfaceDialog::openReviewDialog() {
    /** METHOD TO OPEN THE REVIEW DIALOG **/
    ReviewDialog dialog(m_db);
    dialog.exec();
}

bool InterfaceDialog::detectChangeTab(int index) {
    /** METHOD TO UPDATE INTERFACE AFTER A TAB CHANGE **/
    // Get the new tab
    InterfaceTab* tab = (InterfaceTab*) ui->tabWidget->widget(index);
    // Update the students list
    update_list(tab->getSubject());
    return true;
}

bool InterfaceDialog::update_list(Subject* subj) {
    /** METHOD TO UPDATE STUDENTS LIST ACCORDING A SUBJECT **/

    for(int i=0; i<ui->list_students->count(); i++) {
        // For each student, get the list item
        QListWidgetItem *item = ui->list_students->item(i);
        if(subj) {
            Student* stdnt = (Student*) item->data(Qt::UserRole).toULongLong();
            // Count the number of the kholles in a selected subject
            QList<Kholle*>* listKholles = stdnt->kholles();
            int nbKholles = 0;
            for(int j=0; j<listKholles->count(); j++) {
                if(listKholles->value(j)->timeslot()->kholleur()->getId_subjects() == subj->getId())
                    nbKholles++;
            }
            // Display the correct icon for the student
            if(nbKholles <= 0)
                item->setIcon(QPixmap(":/images/nbKh0.png"));
            else if(nbKholles == 1)
                item->setIcon(QPixmap(":/images/nbKh1.png"));
            else
                item->setIcon(QPixmap(":/images/nbKh2.png"));
        } else {
            // If no selected subject
            item->setIcon(QPixmap(":/images/none.png"));
        }
    }
    return true;
}

bool InterfaceDialog::doubleSelectStudent(QListWidgetItem* item) {
    /** METHOD TO DETECT A DOUBLE SELECTION (TO COMMUTE KHOLLES BETWEEN STUDENTS) **/

    QColor doubleSelectedColor(255, 100, 150);
    QColor empty(255, 255, 255);

    if(item) {
        if(item != m_doubleSelectedItem) {
            if(m_doubleSelectedItem) {
                // Double-select the item
                item->setBackgroundColor(doubleSelectedColor);

                // Get the subject and the both students
                InterfaceTab* tab = (InterfaceTab*) ui->tabWidget->currentWidget();
                Subject* subj = tab ? tab->getSubject() : NULL;
                if(! subj) {
                    QMessageBox::critical(NULL, "Échange de kholles", "Veuillez sélectionner une matière...");
                } else {
                    Student* stud1 = (Student*) item->data(Qt::UserRole).toULongLong();
                    Student* stud2 = (Student*) m_doubleSelectedItem->data(Qt::UserRole).toULongLong();

                    // Ask for a confirmation
                    int res = QMessageBox::information(NULL, "Échange de kholles", "Vous êtes sur le point d'échanger les kholles de <strong>" + subj->getName() + "</strong> entre <strong>" + stud1->getFirst_name() + " " + stud1->getName() + "</strong> et <strong>" + stud2->getFirst_name() + " " + stud2->getName() + "</strong>. Voulez-vous continuer ?", QMessageBox::Yes | QMessageBox::Cancel);
                    if(res == QMessageBox::Yes)
                        commuteKholle(subj, stud1, stud2);
                }

                // Remove the double-selection for the both items
                item->setBackgroundColor(empty);
                m_doubleSelectedItem->setBackgroundColor(empty);
                m_doubleSelectedItem = NULL;
            } else {
                // Double-select the item
                item->setBackgroundColor(doubleSelectedColor);
                m_doubleSelectedItem = item;
            }
        } else {
            // Double-select a double-selected item = remove the double-selection
            item->setBackgroundColor(empty);
            m_doubleSelectedItem = NULL;
        }
    }

    return true;
}

bool InterfaceDialog::commuteKholle(Subject* subj, Student* stud1, Student* stud2) {
    /** METHOD TO COMMUTE KHOLLES (OF A SUBJECT) BETWEEN 2 STUDENTS  **/

    /// Get the kholles which must be commuted
    QSqlQuery query(*m_db);
    query.prepare("SELECT Kh.`id`, Kh.`id_users` FROM `tau_kholles` AS Kh "
               "WHERE (Kh.`id_users` = :idUser1 OR Kh.`id_users` = :idUser2) AND Kh.`id_timeslots` IN "
                    "(SELECT T.`id` FROM `tau_timeslots` AS T "
                        "JOIN `tau_kholleurs` AS K ON T.`id_kholleurs` = K.`id` "
                        "WHERE (T.`date` >= :dateBegin AND `date` < :dateEnd) AND K.`id_subjects` = :idSubject)");
    query.bindValue(":idUser1", stud1->getId());
    query.bindValue(":idUser2", stud2->getId());
    query.bindValue(":dateBegin", m_monday.toString("yyyy-MM-dd"));
    query.bindValue(":dateEnd", m_monday.addDays(7).toString("yyyy-MM-dd"));
    query.bindValue(":idSubject", subj->getId());
    query.exec();
    /// Commute the kholles
    while (query.next()) {
        // Get the kholle ID and the students
        int id = query.value(0).toInt();
        int idStud_origin = query.value(1).toInt();
        int idStud_target = (idStud_origin == stud1->getId()) ? stud2->getId() : stud1->getId();

        // Update the local database of the commuting (DataBase Object)
        Kholle* khlle = m_dbase->listKholles()->value(id);
        khlle->setId_students(idStud_target);
        khlle->setStudent(m_dbase->listStudents()->value(idStud_target));
        m_dbase->listStudents()->value(idStud_origin)->kholles()->removeAll(khlle);
        m_dbase->listStudents()->value(idStud_target)->kholles()->append(khlle);

        // Update the SQL database
        QSqlQuery queryUpdate(*m_db);
        queryUpdate.exec("UPDATE tau_kholles SET id_users = " + QString::number(idStud_target) + " WHERE id = " + QString::number(id));
    }

    InterfaceAction* act = new InterfaceAction();
    act->setCommuteKholle(stud1, stud2, subj);
    lastActions()->push(act);

    /// Update the interface (students list, tabs, khollotab)
    QListWidgetItem *item = ui->list_students->currentItem();
    Student* selectedStudent = (item != NULL) ? (Student*) item->data(Qt::UserRole).toULongLong() : NULL;
    update_list(subj);
    for(int i=0; i<ui->tabWidget->count(); i++)
        ((InterfaceTab*) ui->tabWidget->widget(i))->selectStudent(selectedStudent);

    return true;
}

QStack<InterfaceAction*>* InterfaceDialog::lastActions() {
    return m_lastActions;
}

bool InterfaceDialog::cancelAction() {
    if(m_lastActions->isEmpty()) {
        QMessageBox::information(this, "Impossible d'annuler", "La pile des actions est vide.");
        return false;
    }
    QListWidgetItem *item = ui->list_students->currentItem();
    Student* selectedStudent = (item != NULL) ? (Student*) item->data(Qt::UserRole).toULongLong() : NULL;

    InterfaceAction* act = m_lastActions->pop();
    switch(act->type()) {
    case InterfaceAction::AddKholle:
        removeKholleInDB(act->student(), act->timeslot());
        delete m_lastActions->pop();
        update_list(((InterfaceTab*) ui->tabWidget->currentWidget())->getSubject());
        for(int i=0; i<ui->tabWidget->count(); i++)
            ((InterfaceTab*) ui->tabWidget->widget(i))->selectStudent(selectedStudent);
        break;
    case InterfaceAction::DeleteKholle:
        addKholleInDB(act->student(), act->timeslot());
        delete m_lastActions->pop();
        update_list(((InterfaceTab*) ui->tabWidget->currentWidget())->getSubject());
        for(int i=0; i<ui->tabWidget->count(); i++)
            ((InterfaceTab*) ui->tabWidget->widget(i))->selectStudent(selectedStudent);
        break;
    case InterfaceAction::CommuteKholle:
        commuteKholle(act->subject(), act->student(1), act->student(2));
        delete m_lastActions->pop();
        break;
    default:
        break;
    }
    delete act;

    return true;
}

bool InterfaceDialog::addKholleInDB(Student* stud, Timeslot* ts) {
    /// Check if the selected student has not already the kholle
    for(int i=0; i<ts->kholles()->count(); i++)
        if(ts->kholles()->at(i)->getId_students() == stud->getId()) {
            QMessageBox::critical(NULL, "Erreur", "Cet étudiant participe déjà à cette kholle.");
            return false;
        }

    /// Create the kholle
    Kholle* klle = new Kholle();
    klle->setId_students(stud->getId());
    klle->setId_timeslots(ts->getId());
    klle->setStudent(stud);
    klle->setTimeslot(ts);

    /// Save the kholle in the databases
    // SQL database
    QSqlQuery query(*m_db);
    query.prepare("INSERT INTO `tau_kholles`(`id_users`, `id_timeslots`) VALUES(:id_users, :id_timeslots)");
    query.bindValue(":id_users", klle->getId_students());
    query.bindValue(":id_timeslots", klle->getId_timeslots());
    query.exec();
    klle->setId(query.lastInsertId().toInt());
    // Local database
    m_dbase->listKholles()->insert(klle->getId(), klle);
    stud->kholles()->append(klle);
    ts->kholles()->append(klle);

    InterfaceAction* act = new InterfaceAction();
    act->setAddKholle(stud, ts);
    lastActions()->push(act);

    return true;
}


bool InterfaceDialog::removeKholleInDB(Student* stud, Timeslot* ts) {
    /// Get the kholle checking if the selected student is in the kholle
    Kholle* klle = NULL;
    for(int i=0; i<ts->kholles()->count() && !klle; i++)
        if(ts->kholles()->at(i)->getId_students() == stud->getId())
            klle = ts->kholles()->at(i);
    if(!klle) {
        QMessageBox::critical(NULL, "Erreur", "Cet étudiant ne participe pas à cette kholle.");
        return false;
    }

    /// Remove the kholle from the databases
    // SQL database
    QSqlQuery query(*m_db);
    query.prepare("DELETE FROM `tau_kholles` WHERE `id` = :id");
    query.bindValue(":id", klle->getId());
    query.exec();
    // Local database
    stud->kholles()->removeOne(klle);
    ts->kholles()->removeOne(klle);
    m_dbase->listKholles()->remove(klle->getId());

    InterfaceAction* act = new InterfaceAction();
    act->setDeleteKholle(stud, ts);
    lastActions()->push(act);

    return true;
}

QTabWidget* InterfaceDialog::tabWidget() const {
    return ui->tabWidget;
}
