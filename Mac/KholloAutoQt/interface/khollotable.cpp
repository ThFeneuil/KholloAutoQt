/*
 *	File:			(Source) khollotable.cpp
 *  Comment:        INTERFACE FILE
 *	Description:    Class of a QGraphicsScene which manage the kholles of the selected week
 */

#include "interface/khollotable.h"

KholloTable::KholloTable(QSqlDatabase* db, DataBase *dbase, int id_week, QDate monday, QWidget *areaKholles, InterfaceDialog* interface, InterfaceTab* tab) : QGraphicsScene() {
    /** CONSTRUCTOR **/

    /// Initalisation of the class properties
    // Database (SQL and local)
    m_db = db;
    m_dbase = dbase;
    // Property of the selected week
    m_id_week = id_week;
    m_monday = monday;
    // Pointors to differents objects of the interface
    m_interface = interface;
    m_tab = tab;
    m_areaKholles = areaKholles;

    /// Positions of the areas for the days in the khollotable
    m_sizeImg.insert(BeginDays, 42);
    m_sizeImg.insert(BetweenDays, 100);
    m_sizeImg.insert(BeginHours, 33);
    m_sizeImg.insert(BetweenHours, 30);

    /// Pointors of the selected parameters (student, kholleur, timeslot, frame)
    m_student = NULL;
    m_kholleur = NULL;
    m_selectedTimeslot = NULL;

    /// Display the empty khollotable
    QPixmap emptyTable(":/images/emptyTimeTable.png");
    addPixmap(emptyTable);

    /// Connect the widgets of the info area
    connect(m_areaKholles->findChild<QPushButton*>("pushButton_addStudent"), SIGNAL(clicked()), this, SLOT(addKholle()));
    connect(m_areaKholles->findChild<QPushButton*>("pushButton_removeStudent"), SIGNAL(clicked()), this, SLOT(removeKholleFromInfoArea()));
    connect(m_areaKholles->findChild<QListWidget*>("list_students"), SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(selectStudentInInterface()));
}

KholloTable::~KholloTable() {
    /** DESTRUCTOR **/
        // empty...
}

void KholloTable::displayKholleur(Kholleur* kll) {
    /** METHOD TO DISPLAY A KHOLLEUR **/
    // If the selected kholleur have changed, remove the selection of timeslot
    if(m_kholleur == NULL || kll == NULL || m_kholleur->getId() != kll->getId())
        removeSelection();

    // Get the new kholleur
    if(kll) m_kholleur = m_dbase->listKholleurs()->value(kll->getId());
    else    m_kholleur = NULL;

    // Update the khollotable
    displayTable();
}

void KholloTable::displayStudent(Student* stud) {
    /** METHOD TO DISPLAY A STUDENT **/
    // Get the new student
    if(stud)    m_student = m_dbase->listStudents()->value(stud->getId());
    else        m_student = NULL;

    // Update the khollotable
    displayTable();
}

void KholloTable::displayTable() {
    /** METHOD TO DISPLAY THE KHOLLOTABLE ACCORDING THE SELECTED STUDENT AND THE SELECTED KHOLLEUR **/
    // Effacer l'ancienne khollotable
    clear();

    // Queue to remember some areas to draw the profile of the frame (at the end)
    QQueue<QRect> areaCourses;
    QQueue<QRect> areaKholles;
    QQueue<QRect> KhollesWhereHeisIn;

    /// Display the timetable of the selected student
    if(m_student) {
        // Display the courses of the student
        QList<Group*>* listGroups = m_student->groups();
        for(int i=0; i<listGroups->count(); i++) { // For each group where there is the student
            QList<Course*>* listCourses = listGroups->at(i)->courses();
            for(int j=0; j<listCourses->count(); j++) { // For each course of the group
                Course* course = listCourses->at(j);

                // Find position of the frame for the course
                int x = m_sizeImg[BeginDays]+m_sizeImg[BetweenDays]*(course->getId_day()-1);
                int y = m_sizeImg[BeginHours]+m_sizeImg[BetweenHours]*(course->getTime_start().msecsSinceStartOfDay() - QTime(8, 0).msecsSinceStartOfDay())/3600000;
                int w = m_sizeImg[BetweenDays];
                int h = m_sizeImg[BetweenHours]*(course->getTime_end().msecsSinceStartOfDay() - course->getTime_start().msecsSinceStartOfDay())/3600000;
                // Display the course
                QRect rect(x,y,w,h);
                addRect(rect, QPen(Qt::black, 0), QBrush(Qt::gray));
                // Save the area to draw the frame profile at the end
                areaCourses.append(rect);
            }
        }

        // Display the kholles of the student
        QList<Kholle*>* listKholles = m_student->kholles();
        QQueue<QRect> khollesInCurrentSubject;
        for(int i=0; i<listKholles->count(); i++) { // For each kholle where there is the student
            Kholle* klle = listKholles->at(i);
            Timeslot* slot = klle->timeslot();

            // Find position of the frame for the kholle
            int x = m_sizeImg[BeginDays]+m_sizeImg[BetweenDays]*m_monday.daysTo(slot->getDate());
            int y = m_sizeImg[BeginHours]+m_sizeImg[BetweenHours]*(slot->getTime().msecsSinceStartOfDay() - QTime(8, 0).msecsSinceStartOfDay())/3600000;
            int w = m_sizeImg[BetweenDays];
            int h = m_sizeImg[BetweenHours]*(slot->getTime_end().msecsSinceStartOfDay() - slot->getTime().msecsSinceStartOfDay())/3600000;
            slot->setArea(new QRect(x,y,w,h));
            // Display the kholle
            QRect rect(x,y,w,h);
            if(slot->kholleur()->getId_subjects() == m_tab->getSubject()->getId())
                    addRect(rect, QPen(Qt::black, 0), QBrush(QColor(255,201,14)));
            else    khollesInCurrentSubject.append(rect);
            // Save the area to draw the frame profile at the end
            areaCourses.append(rect);
        }
        while (!khollesInCurrentSubject.isEmpty()) {
            QRect rect = khollesInCurrentSubject.dequeue();
            addRect(rect, QPen(Qt::black, 0), QBrush(QColor(255,220,100)));
        }
    }

    /// Display the timeslots of the selected kholleur
    if(m_kholleur) {
        // Get the timeslots
        QList<Timeslot*>* listTimeslots = m_dbase->listKholleurs()->value(m_kholleur->getId())->timeslots();
        for(int i=0; i<listTimeslots->count(); i++) { // For each timeslot of this kholleur
            Timeslot* slot = listTimeslots->at(i);

            // Find position of the frame for the course
            int x = m_sizeImg[BeginDays]+m_sizeImg[BetweenDays]*m_monday.daysTo(slot->getDate());
            int y = m_sizeImg[BeginHours]+m_sizeImg[BetweenHours]*(slot->getTime().msecsSinceStartOfDay() - QTime(8, 0).msecsSinceStartOfDay())/3600000;
            int w = m_sizeImg[BetweenDays];
            int h = m_sizeImg[BetweenHours]*(slot->getTime_end().msecsSinceStartOfDay() - slot->getTime().msecsSinceStartOfDay())/3600000;
            slot->setArea(new QRect(x,y,w,h));

            // Display the timeslot (according if the selected student is availble)
            if(m_student == NULL || compatible(m_student, slot))
                addRect(*slot->getArea(), QPen(Qt::black, 0), QBrush(Qt::green));
            else
                addRect(*slot->getArea(), QPen(Qt::black, 0), QBrush(Qt::red, Qt::DiagCrossPattern));
            // Display the filling of the timeslot
            if(slot->getPupils() > 0) {
                int wLoading = slot->kholles()->count()*m_sizeImg[BetweenDays]/slot->getPupils();
                if(wLoading > m_sizeImg[BetweenDays])
                    wLoading = m_sizeImg[BetweenDays];
                addRect(QRect(x,y,wLoading,h), QPen(Qt::blue, 1), QBrush(Qt::blue, Qt::Dense5Pattern));
            }
            // Detect if the selected student is in the timeslot
            bool isInKholle = false;
            for(int j=0; j<slot->kholles()->count(); j++)
                if(m_student && slot->kholles()->at(j)->getId_students() == m_student->getId())
                    isInKholle = true;
            if(isInKholle) {
                // Save the area to draw the "presence symbol" at the end
                int radius = 8;
                int xCenter = x+w/2;
                int yCenter = y+h/2;
                KhollesWhereHeisIn.append(QRect(xCenter-radius,yCenter-radius,radius*2,radius*2));
            }
            // Save the area to draw the frame profile at the end
            areaKholles.append(*slot->getArea());
        }
    }

    /// Add the empty table to draw the line of the timetable
    QPixmap monPixmap(":/images/emptyTimeTable.png");
    addPixmap(monPixmap);

    /// Draw the frame profiles
    // Frame profiles of kholles
    while (!areaKholles.isEmpty()) {
        QRect rect = areaKholles.dequeue();
        addRect(rect, QPen(Qt::blue, 1));
    }
    // Frame profiles of courses
    while (!areaCourses.isEmpty()) {
        QRect rect = areaCourses.dequeue();
        addRect(rect, QPen(Qt::blue, 1));
    }

    /// Display the "selected" frame around the selected timeslot
    if(m_selectedTimeslot)
        addRect(*(m_selectedTimeslot->getArea()), QPen(Qt::yellow, 3));

    /// Draw the "presence symbol" on the kholle where the selected student is
    while (!KhollesWhereHeisIn.isEmpty()) {
        QRect rect = KhollesWhereHeisIn.dequeue();
        addEllipse(rect, QPen(Qt::red, 1), QBrush(Qt::red));
    }

    /// Update the info area
    updateInfoArea();
}

bool KholloTable::compatible(Student* stdnt, Timeslot *timeslot) {
    /** METHOD TO DETERMINE IF A STUDENT AND A TIMESLOT IS COMPATIBLE **/
    //Get all groups of student (the ids)
    QList<Group*> *groups = m_dbase->listStudents()->value(stdnt->getId())->groups();

    if(groups->length() > 0) {
        /// Check if a course interfere with this timeslot
        QString request = "`id_groups`=" + QString::number(groups->at(0)->getId());
        for(int i = 1; i < groups->length(); i++)
            request = request + " OR `id_groups`=" + QString::number(groups->at(i)->getId());

        //Get all courses that can interfere with this timeslot
        QSqlQuery courses_query(*m_db);
        courses_query.prepare("SELECT * FROM `tau_courses` WHERE (" + request + ") AND `id_day`=:id_day AND `id_week`=:id_week AND ("
                                                                              "(`time_start` <= :time_start AND `time_end` > :time_start) OR"
                                                                              "(`time_start` < :time_end AND `time_end` >= :time_end) OR"
                                                                              "(`time_start` >= :time_start AND `time_end` <= :time_end) )");
        courses_query.bindValue(":time_start", timeslot->getTime_start().toString("HH:mm:ss"));
        courses_query.bindValue(":time_end", timeslot->getTime_end().toString("HH:mm:ss"));
        courses_query.bindValue(":id_day", timeslot->getDate().dayOfWeek());
        courses_query.bindValue(":id_week", m_id_week);
        courses_query.exec();

        // If there is a course or more --> Not compatible
        if(courses_query.next())
            return false;

        /// Check if a event interfere with this timeslot
        //Get all events that can interfere with this timeslot
        QSqlQuery event_query(*m_db);
        event_query.prepare("SELECT * FROM `tau_events` AS E JOIN `tau_events_groups` AS G ON E.`id` = G.`id_events` WHERE (" + request + ") AND ("
                                      "(E.`start` <= :start_time AND E.`end` > :start_time) OR"
                                      "(E.`start` < :end_time AND E.`end` >= :end_time) OR"
                                      "(E.`start` >= :start_time AND E.`end` <= :end_time))");
        event_query.bindValue(":start_time", QDateTime(timeslot->getDate(), timeslot->getTime_start()).toString("yyyy-MM-dd HH:mm:ss"));
        event_query.bindValue(":end_time", QDateTime(timeslot->getDate(), timeslot->getTime_end()).toString("yyyy-MM-dd HH:mm:ss"));
        event_query.exec();

        // If there is a event or more --> Not compatible
        if(event_query.next())
            return false;
    }

    /// Check if a kholle interfere with this timeslot
    //Get all kholles that can interfere with this timeslot
    QSqlQuery kholle_query(*m_db);
    kholle_query.prepare("SELECT * FROM `tau_kholles` AS K JOIN `tau_timeslots` AS T ON K.`id_timeslots` = T.`id` WHERE K.`id_users`=:id_users AND T.`date`=:date AND ("
                         "(T.`time_start` <= :time_start AND T.`time_end` > :time_start) OR "
                         "(T.`time_start` < :time_end AND T.`time_end` >= :time_end) OR "
                         "(T.`time_start` >= :time_start AND T.`time_end` <= :time_end))");
    kholle_query.bindValue(":id_users", stdnt->getId());
    kholle_query.bindValue(":date", timeslot->getDate().toString("yyyy-MM-dd"));
    kholle_query.bindValue(":time_start", timeslot->getTime_start().toString("HH:mm:ss"));
    kholle_query.bindValue(":time_end", timeslot->getTime_end().toString("HH:mm:ss"));
    kholle_query.exec();

    // If there is a kholle or more --> Not compatible
    if(kholle_query.next())
        return false;

    /// Else, compatible
    return true;
}

void KholloTable::mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent) {
    /** METHOD TO DETECT WHEN USER CLICKS WITH THE MOUSE (KNOWING BUTTON) **/
    // Select a timeslot or a kholle
    selection(mouseEvent);
}

bool KholloTable::selection(QGraphicsSceneMouseEvent *mouseEvent) {
    /** METHOD TO SELECT A TIMESLOT OR A KHOLLE WITH THE MOUSE **/
    // Get the position of the mouse
    QPoint pos = mouseEvent->scenePos().toPoint();
    m_selectedTimeslot = NULL;

    /// Select a timeslot (Left button)
    if(mouseEvent->button() == Qt::LeftButton && m_kholleur) {
        // Check every timeslot
        QList<Timeslot*>* listTimeslots = m_dbase->listKholleurs()->value(m_kholleur->getId())->timeslots();
        for(int i=0; i<listTimeslots->count(); i++) { // For each timeslot
            Timeslot* slot = listTimeslots->at(i);
            // Check if the area of the timeslot was under the mouse when user clicked
            if(slot->getArea()->contains(pos)) {
                // Select the timeslot
                m_selectedTimeslot = slot;
                break;
            }
        }
    }

    /// Select a kholle (Right button)
    if(mouseEvent->button() == Qt::RightButton && m_selectedTimeslot == NULL) {
        bool selection = false;
        // Check every kholle
        QList<Kholle*>* listKholles = m_student->kholles();
        for(int i=0; i<listKholles->count(); i++) { // For each group
            Kholle* klle = listKholles->at(i);
            Timeslot* slot = klle->timeslot();
            // Check if the area of the kholle was under the mouse when user clicked
                // Only for the kholles of the tab subject
            if(slot->kholleur()->getId_subjects() == m_tab->getSubject()->getId() && slot->getArea()->contains(pos)) {
                m_kholleur = slot->kholleur();
                m_tab->selectKholleur(m_kholleur);
                m_selectedTimeslot = slot;
                selection = true;
            }
        }
        if(! selection) {
            for(int i=0; i<listKholles->count() && ! selection; i++) { // For each group
                Kholle* klle = listKholles->at(i);
                Timeslot* slot = klle->timeslot();
                // Check if the area of the kholle was under the mouse when user clicked
                    // Only for the kholles of the subject of the OTHER tabs
                if(slot->kholleur()->getId_subjects() != m_tab->getSubject()->getId() && slot->getArea()->contains(pos)) {
                    for(int j=0; j<m_interface->tabWidget()->count(); j++) {
                        InterfaceTab* tab = (InterfaceTab*) m_interface->tabWidget()->widget(j);
                        if(tab->getSubject()->getId() == slot->kholleur()->getId_subjects()) {
                            m_interface->tabWidget()->setCurrentWidget(tab);
                            tab->selectKholleur(slot->kholleur());
                            tab->selectTimeslot(slot);
                            selection = true;
                        }
                    }
                }
            }
        }
    }

    /// Update the khollotable and the info area
    displayTable();
    updateInfoArea();

    return true;
}

bool KholloTable::updateInfoArea() {
    /** METHOD TO UPDATE INFO AREA (INFO ABOUT THE SELECTED TIMESLOT) **/
    // Get pointors to widgets for displaying
    QLabel* label_info = m_areaKholles->findChild<QLabel*>("label_info");
    QListWidget* list_students = m_areaKholles->findChild<QListWidget*>("list_students");
    QPushButton* pushButton_add = m_areaKholles->findChild<QPushButton*>("pushButton_addStudent");
    QPushButton* pushButton_remove = m_areaKholles->findChild<QPushButton*>("pushButton_removeStudent");

    if(m_selectedTimeslot) {
        /// If a timeslot is selected
        // Update information labels
        QString text = "<strong>Date :</strong> " + m_selectedTimeslot->getDate().toString("dd/MM/yyyy") + "<br />" +
                       "<strong>Kholleur :</strong> " + m_selectedTimeslot->kholleur()->getName() + "<br />" +
                       "<strong>Horaire :</strong> " + m_selectedTimeslot->getTime().toString("hh:mm") + " >> " + m_selectedTimeslot->getTime_end().toString("hh:mm") + "<br />";
        if(m_selectedTimeslot->getTime_start() != m_selectedTimeslot->getTime())
            text += "<strong> ATTENTION préparation :</strong> Début à " + m_selectedTimeslot->getTime_start().toString("hh:mm") + "<br />";
        text += "<strong>Nombre d'étudiants :</strong> " +
                QString::number(m_selectedTimeslot->kholles()->count()) + " / " + QString::number(m_selectedTimeslot->getPupils()) + "<br />";
        label_info->setText(text);
        label_info->setDisabled(false);

        // Display the students who are in the kholle
        list_students->clear();
        bool isInKholle = false;
        for(int i=0; i<m_selectedTimeslot->kholles()->count(); i++) {
            // For each student in the kholle
            Kholle* klle = m_selectedTimeslot->kholles()->at(i);
            QListWidgetItem *item = new QListWidgetItem(klle->student()->getName() + " " + klle->student()->getFirst_name(), list_students);
            item->setData(Qt::UserRole, (qulonglong) klle->student());
            // Check if the selected student is in this kholle
            if(m_student && klle->getId_students() == m_student->getId())
                isInKholle = true;
        }
        list_students->setDisabled(false);

        // Update buttons (enabled or not)
        if(m_student)   pushButton_add->setText("Ajouter " + m_student->getFirst_name() + " " + m_student->getName() + " à cette kholle");
        else            pushButton_add->setText("Ajouter à cette kholle");
        pushButton_add->setDisabled(isInKholle);
        if(!m_student)
            pushButton_add->setDisabled(true);
        pushButton_remove->setDisabled(m_selectedTimeslot->kholles()->count() == 0);
    } else {
        /// If no timeslot is selected
        // Clear every widget (label, list, buttons)
        label_info->setText("");
        label_info->setDisabled(true);
        list_students->clear();
        list_students->setDisabled(true);
        pushButton_add->setDisabled(true);
        pushButton_remove->setDisabled(true);
    }

    return true;
}

void KholloTable::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *mouseEvent) {
    /** METHOD TO DETECT WHEN USER DOUBLE-CLICKS WITH THE MOUSE (KNOWING BUTTON) **/
    // Select a timeslot or a kholle
    selection(mouseEvent);
    switch(mouseEvent->button()) {
        case Qt::LeftButton:
            // Add a kholle (left button)
            addKholle();
            break;
        case Qt::RightButton:
            // Remove a kholle (right button)
            removeKholle();
            break;
        default:
            break;
    }
}

bool KholloTable::removeSelection() {
    /** METHOD TO REMOVE THE SELECTION ON TIMESLOTS **/
    m_selectedTimeslot = NULL;
    return true;
}

bool KholloTable::addKholle() {
    /** METHOD TO ADD A KHOLLE **/
    if(m_selectedTimeslot && m_student) {
        if(! m_interface->addKholleInDB(m_student, m_selectedTimeslot))
            return false;

        /// Update interface
        updateListKholleurs(); // Kholleurs list
        displayTable(); // Khollotable
        updateInfoArea(); // Info area
        m_interface->selectStudent(m_student); // Students list (selection)
        m_interface->update_list(m_tab->getSubject()); // Students list (displaying)
    } else {
        /// If there is no selected student or no selected timeslot
        if(!m_student)
                QMessageBox::critical(NULL, "Erreur", "Veuillez sélectionner un étudiant...");
        else    QMessageBox::critical(NULL, "Erreur", "Veuillez sélectionner un horaire de kholle...");
        return false;
    }

    return true;
}

bool KholloTable::removeKholle(Student* stud) {
    /** METHOD TO REMOVE A KHOLLE **/
    // Get the student
    if(!stud)
        stud = m_student;

    if(m_selectedTimeslot && stud) {
        if(! m_interface->removeKholleInDB(stud, m_selectedTimeslot))
            return false;

        /// Update interface
        updateListKholleurs(); // Kholleurs list
        displayTable(); // Khollotable
        updateInfoArea(); // Info area
        m_interface->selectStudent(m_student); // Students list (selection)
        m_interface->update_list(m_tab->getSubject()); // Students list (displaying)
    } else {
        /// If there is no selected student or no selected timeslot
        if(!stud)
                QMessageBox::critical(NULL, "Erreur", "Veuillez sélectionner un étudiant...");
        else    QMessageBox::critical(NULL, "Erreur", "Veuillez sélectionner un horaire de kholle...");
        return false;
    }

    return true;
}

bool KholloTable::removeKholleFromInfoArea() {
    /** METHOD TO REMOVE A KHOLLE FROM THE INFO AREA **/
    // Get the selected item
    QListWidget* list_students = m_areaKholles->findChild<QListWidget*>("list_students");
    QListWidgetItem *item = list_students->currentItem();
    if(item) {
        // Get the student
        Student* stud = (Student*) item->data(Qt::UserRole).toULongLong();
        // Remove the kholle
        return removeKholle(stud);
    } else {
        // If there is no selected item
        QMessageBox::critical(NULL, "Erreur", "Veuillez sélectionner un étudiant...");
        return false;
    }
    return true;
}


bool KholloTable::selectStudentInInterface() {
    /** METHOD TO SELECT A STUDENT FROM THE INFO AREA (STUDENTS LIST) **/
    if(m_interface) {
        // Get the selected item
        QListWidget* list_students = m_areaKholles->findChild<QListWidget*>("list_students");
        QListWidgetItem *item = list_students->currentItem();
        if(item) {
            // Get the student
            Student* stud = (Student*) item->data(Qt::UserRole).toULongLong();
            // Select the student
            m_interface->selectStudent(stud);
        } else {
            QMessageBox::critical(NULL, "Erreur", "Veuillez sélectionner un étudiant...");
            return false;
        }
    }
    return true;
}

bool KholloTable::updateListKholleurs() {
    /** METHOD TO UPDATE THE KHOLLEURS LIST OF THE SELECTED TAB **/
    if(m_tab)
        m_tab->selectStudent(m_student);
    return true;
}

bool KholloTable::selectionTimeslot(Timeslot* ts) {
    if(m_kholleur) {
        // Check every timeslot
        QList<Timeslot*>* listTimeslots = m_dbase->listKholleurs()->value(m_kholleur->getId())->timeslots();
        for(int i=0; i<listTimeslots->count(); i++) { // For each timeslot
            Timeslot* slot = listTimeslots->at(i);
            // Check if the area of the timeslot was under the mouse when user clicked
            if(slot->getId() == ts->getId()) {
                // Select the timeslot
                m_selectedTimeslot = slot;
                displayTable();
                updateInfoArea();
                return true;
            }
        }
    }

    return false;
}

bool KholloTable::openReviewWithSelection() {
    if(m_student) {
        ReviewDialog dialog(m_db, m_interface, ReviewDialog::FROMKHOLLEURS, m_student, m_kholleur);
        dialog.exec();
    } else {
        ReviewDialog dialog(m_db, m_interface, ReviewDialog::FROMKHOLLEURS, NULL, m_kholleur);
        dialog.exec();
    }
    return true;
}
