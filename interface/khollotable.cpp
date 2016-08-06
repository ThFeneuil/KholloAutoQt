#include "interface/khollotable.h"
#include <QPixmap>

KholloTable::KholloTable(QSqlDatabase* db, int id_week, QDate monday, QWidget *areaKholles, DataBase *dbase, InterfaceDialog* interface, InterfaceTab* tab) : QGraphicsScene() {
    m_db = db;
    m_id_week = id_week;
    m_monday = monday;
    m_areaKholles = areaKholles;
    m_dbase = dbase;
    m_interface = interface;
    m_tab = tab;

    m_sizeImg.insert(BeginDays, 42);
    m_sizeImg.insert(BetweenDays, 100);
    m_sizeImg.insert(BeginHours, 33);
    m_sizeImg.insert(BetweenHours, 30);

    m_student = NULL;
    m_kholleur = NULL;
    m_selectedTimeslot = NULL;
    m_selectedFrame = NULL;

    QPixmap monPixmap(":/images/emptyTimeTable.png");
    addPixmap(monPixmap);

    connect(m_areaKholles->findChild<QPushButton*>("pushButton_addStudent"), SIGNAL(clicked()), this, SLOT(addKholle()));
    connect(m_areaKholles->findChild<QPushButton*>("pushButton_removeStudent"), SIGNAL(clicked()), this, SLOT(removeKholleFromInfoArea()));
    connect(m_areaKholles->findChild<QListWidget*>("list_students"), SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(selectStudentInInterface()));
}

KholloTable::~KholloTable() {

}

void KholloTable::displayKholleur(Kholleur* kll) {
    if(m_kholleur == NULL || kll == NULL || m_kholleur->getId() != kll->getId())
        removeSelection();

    if(kll) m_kholleur = m_dbase->listKholleurs()->value(kll->getId());
    else    m_kholleur = NULL;

    displayTable();
}

void KholloTable::displayStudent(Student* stud) {
    if(stud)    m_student = m_dbase->listStudents()->value(stud->getId());
    else        m_student = NULL;

    displayTable();
}

void KholloTable::displayTable() {
    clear();

    QQueue<QRect> areaCourses;
    QQueue<QRect> areaKholles;
    QQueue<QRect> KhollesWhereHeisIn;

    if(m_student) {
        // Display the timetable of the student selected
        QList<Group*>* listGroups = m_student->groups();
        for(int i=0; i<listGroups->count(); i++) { // For each group
            QList<Course*>* listCourses = listGroups->at(i)->courses();
            for(int j=0; j<listCourses->count(); j++) { // For each course
                Course* course = listCourses->at(j);

                // Display the course
                int x = m_sizeImg[BeginDays]+m_sizeImg[BetweenDays]*(course->getId_day()-1);
                int y = m_sizeImg[BeginHours]+m_sizeImg[BetweenHours]*(course->getTime_start().msecsSinceStartOfDay() - QTime(8, 0).msecsSinceStartOfDay())/3600000;
                int w = m_sizeImg[BetweenDays];
                int h = m_sizeImg[BetweenHours]*(course->getTime_end().msecsSinceStartOfDay() - course->getTime_start().msecsSinceStartOfDay())/3600000;
                QRect rect(x,y,w,h);
                addRect(rect, QPen(Qt::black, 0), QBrush(Qt::gray));
                areaCourses.append(rect);
            }
        }

        QList<Kholle*>* listKholles = m_student->kholles();
        for(int i=0; i<listKholles->count(); i++) { // For each group
            Kholle* klle = listKholles->at(i);
            Timeslot* slot = klle->timeslot();

            // Display only the kholle of the subjects of the tab
            if(slot->kholleur()->getId_subjects() == m_tab->getSubject()->getId()) {
                // Display the kholle
                int x = m_sizeImg[BeginDays]+m_sizeImg[BetweenDays]*m_monday.daysTo(slot->getDate());
                int y = m_sizeImg[BeginHours]+m_sizeImg[BetweenHours]*(slot->getTime().msecsSinceStartOfDay() - QTime(8, 0).msecsSinceStartOfDay())/3600000;
                int w = m_sizeImg[BetweenDays];
                int h = m_sizeImg[BetweenHours]*(slot->getTime_end().msecsSinceStartOfDay() - slot->getTime().msecsSinceStartOfDay())/3600000;
                QRect rect(x,y,w,h);
                addRect(rect, QPen(Qt::black, 0), QBrush(QColor(255,201,14)));
                areaCourses.append(rect);
            }
        }
    }

    if(m_kholleur) {
        // Display the timeslot of the kholleur selected
        QList<Timeslot*>* listTimeslots = m_dbase->listKholleurs()->value(m_kholleur->getId())->timeslots();
        for(int i=0; i<listTimeslots->count(); i++) { // For each timeslot
            Timeslot* slot = listTimeslots->at(i);

            // Display the timeslot
            int x = m_sizeImg[BeginDays]+m_sizeImg[BetweenDays]*m_monday.daysTo(slot->getDate());
            int y = m_sizeImg[BeginHours]+m_sizeImg[BetweenHours]*(slot->getTime().msecsSinceStartOfDay() - QTime(8, 0).msecsSinceStartOfDay())/3600000;
            int w = m_sizeImg[BetweenDays];
            int h = m_sizeImg[BetweenHours]*(slot->getTime_end().msecsSinceStartOfDay() - slot->getTime().msecsSinceStartOfDay())/3600000;
            slot->setArea(new QRect(x,y,w,h));
            if(m_student == NULL || compatible(m_student, slot))
                addRect(*slot->getArea(), QPen(Qt::black, 0), QBrush(Qt::green));
            else
                addRect(*slot->getArea(), QPen(Qt::black, 0), QBrush(Qt::red, Qt::DiagCrossPattern));
            int wLoading = slot->kholles()->count()*m_sizeImg[BetweenDays]/slot->getPupils();
            if(wLoading > m_sizeImg[BetweenDays])
                wLoading = m_sizeImg[BetweenDays];
            addRect(QRect(x,y,wLoading,h), QPen(Qt::blue, 1), QBrush(Qt::blue, Qt::Dense5Pattern));
            bool isInKholle = false;
            for(int j=0; j<slot->kholles()->count(); j++)
                if(m_student && slot->kholles()->at(j)->getId_students() == m_student->getId())
                    isInKholle = true;
            if(isInKholle) {
                int radius = 8;
                int xCenter = x+w/2;
                int yCenter = y+h/2;
                KhollesWhereHeisIn.append(QRect(xCenter-radius,yCenter-radius,radius*2,radius*2));
            }
            areaKholles.append(*slot->getArea());
        }
    }

    QPixmap monPixmap(":/images/emptyTimeTable.png");
    addPixmap(monPixmap);

    while (!areaKholles.isEmpty()) {
        QRect rect = areaKholles.dequeue();
        addRect(rect, QPen(Qt::blue, 1));
    }
    while (!areaCourses.isEmpty()) {
        QRect rect = areaCourses.dequeue();
        addRect(rect, QPen(Qt::blue, 1));
    }

    // Display the "selected" frame around the selected timeslot
    if(m_selectedTimeslot)
        addRect(*(m_selectedTimeslot->getArea()), QPen(Qt::yellow, 3));

    while (!KhollesWhereHeisIn.isEmpty()) {
        QRect rect = KhollesWhereHeisIn.dequeue();
        addEllipse(rect, QPen(Qt::red, 1), QBrush(Qt::red));
    }

    updateInfoArea();
}

bool KholloTable::compatible(Student* stdnt, Timeslot *timeslot) {
    //Get all groups of user (the ids)
    QList<Group*> *groups = m_dbase->listStudents()->value(stdnt->getId())->groups();

    int i;
    if(groups->length() > 0) {
        QString request = "id_groups=" + QString::number(groups->at(0)->getId());
        for(i = 1; i < groups->length(); i++) {
            request = request + " OR id_groups=" + QString::number(groups->at(i)->getId());
        }

        //Get all courses that can interfere with this timeslot
        QSqlQuery courses_query(*m_db);
        courses_query.prepare("SELECT * FROM tau_courses WHERE (" + request + ") AND id_day=:id_day AND id_week=:id_week AND ("
                                                                              "(time_start <= :time_start AND time_end > :time_start) OR"
                                                                              "(time_start < :time_end AND time_end >= :time_end) OR"
                                                                              "(time_start >= :time_start AND time_end <= :time_end) )");
        courses_query.bindValue(":time_start", timeslot->getTime_start().toString("HH:mm:ss"));
        courses_query.bindValue(":time_end", timeslot->getTime_end().toString("HH:mm:ss"));
        courses_query.bindValue(":id_day", timeslot->getDate().dayOfWeek());
        courses_query.bindValue(":id_week", m_id_week);
        courses_query.exec();

        if(courses_query.next()) {
            return false;
        }

        //Get all events that can interfere with this timeslot
        QSqlQuery event_query(*m_db);
        event_query.prepare("SELECT * FROM tau_events AS E JOIN tau_events_groups AS G ON E.`id` = G.`id_events` WHERE (" + request + ") AND ("
                                      "(E.`start` <= :start_time AND E.`end` > :start_time) OR"
                                      "(E.`start` < :end_time AND E.`end` >= :end_time) OR"
                                      "(E.`start` >= :start_time AND E.`end` <= :end_time))");
        event_query.bindValue(":start_time", QDateTime(timeslot->getDate(), timeslot->getTime_start()).toString("yyyy-MM-dd HH:mm:ss"));
        event_query.bindValue(":end_time", QDateTime(timeslot->getDate(), timeslot->getTime_end()).toString("yyyy-MM-dd HH:mm:ss"));
        event_query.exec();

        if(event_query.next()) {
            return false;
        }

        //Get all kholles that can interfere with this timeslot
        QSqlQuery kholle_query(*m_db);
        kholle_query.prepare("SELECT * FROM tau_kholles AS K JOIN tau_timeslots AS T ON K.`id_timeslots` = T.`id` WHERE K.`id_users`=:id_users AND T.`date`=:date AND ("
                             "(T.`time_start` <= :time_start AND T.`time_end` > :time_start) OR "
                             "(T.`time_start` < :time_end AND T.`time_end` >= :time_end) OR "
                             "(T.`time_start` >= :time_start AND T.`time_end` <= :time_end))");
        kholle_query.bindValue(":id_users", stdnt->getId());
        kholle_query.bindValue(":date", timeslot->getDate().toString("yyyy-MM-dd"));
        kholle_query.bindValue(":time_start", timeslot->getTime_start().toString("HH:mm:ss"));
        kholle_query.bindValue(":time_end", timeslot->getTime_end().toString("HH:mm:ss"));
        kholle_query.exec();

        if(kholle_query.next()) {
            return false;
        }
    }

    return true;
}

void KholloTable::mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent) {
    selection(mouseEvent);
}

bool KholloTable::selection(QGraphicsSceneMouseEvent *mouseEvent) {
    QPoint pos = mouseEvent->scenePos().toPoint();
    m_selectedTimeslot = NULL;

    if(!m_kholleur)
        return false;

    QList<Timeslot*>* listTimeslots = m_dbase->listKholleurs()->value(m_kholleur->getId())->timeslots();
    for(int i=0; i<listTimeslots->count(); i++) { // For each timeslot
        Timeslot* slot = listTimeslots->at(i);

        if(slot->getArea()->contains(pos)) {
            m_selectedTimeslot = slot;
            break;
        }
    }

    displayTable();
    updateInfoArea();

    return true;
}

bool KholloTable::updateInfoArea() {
    QLabel* label_info = m_areaKholles->findChild<QLabel*>("label_info");
    QListWidget* list_students = m_areaKholles->findChild<QListWidget*>("list_students");
    QPushButton* pushButton_add = m_areaKholles->findChild<QPushButton*>("pushButton_addStudent");
    QPushButton* pushButton_remove = m_areaKholles->findChild<QPushButton*>("pushButton_removeStudent");

    if(m_selectedTimeslot) {
        // Information label
        QString text = "<strong>Date :</strong> " + m_selectedTimeslot->getDate().toString("dd/MM/yyyy") + "<br />" +
                       "<strong>Kholleur :</strong> " + m_selectedTimeslot->kholleur()->getName() + "<br />" +
                       "<strong>Horaire :</strong> " + m_selectedTimeslot->getTime().toString("hh:mm") + " >> " + m_selectedTimeslot->getTime_end().toString("hh:mm") + "<br />";
        if(m_selectedTimeslot->getTime_start() != m_selectedTimeslot->getTime())
            text += "<strong> ATTENTION préparation :</strong> Début à " + m_selectedTimeslot->getTime_start().toString("hh:mm") + "<br />";
        text += "<strong>Nombre d'étudiants attendus :</strong> " + QString::number(m_selectedTimeslot->getPupils()) + "<br />";
        text += "<strong>Nombre d'étudiants présents :</strong> " + QString::number(m_selectedTimeslot->kholles()->count());
        label_info->setText(text);
        label_info->setDisabled(false);

        // Students in the kholle
        list_students->clear();
        bool isInKholle = false;
        for(int i=0; i<m_selectedTimeslot->kholles()->count(); i++) {
            Kholle* klle = m_selectedTimeslot->kholles()->at(i);
            QListWidgetItem *item = new QListWidgetItem(klle->student()->getName() + " " + klle->student()->getFirst_name(), list_students);
            item->setData(Qt::UserRole, (qulonglong) klle->student());
            if(m_student && klle->getId_students() == m_student->getId())
                isInKholle = true;
        }
        list_students->setDisabled(false);

        // Buttons
        if(m_student)   pushButton_add->setText("Ajouter " + m_student->getFirst_name() + " " + m_student->getName() + " à cette kholle");
        else            pushButton_add->setText("Ajouter à cette kholle");
        pushButton_add->setDisabled(isInKholle);
        if(!m_student)
            pushButton_add->setDisabled(true);
        pushButton_remove->setDisabled(m_selectedTimeslot->kholles()->count() == 0);
    } else {
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
    selection(mouseEvent);
    switch(mouseEvent->button()) {
        case Qt::LeftButton:
            addKholle();
            break;
        case Qt::RightButton:
            removeKholle();
            break;
        default:
            break;
    }
}

bool KholloTable::removeSelection() {
    m_selectedTimeslot = NULL;
    return true;
}

bool KholloTable::addKholle() {
    if(m_selectedTimeslot && m_student) {
        for(int i=0; i<m_selectedTimeslot->kholles()->count(); i++)
            if(m_selectedTimeslot->kholles()->at(i)->getId_students() == m_student->getId()) {
                QMessageBox::critical(NULL, "Erreur", "Cet étudiant participe déjà à cette kholle.");
                return false;
            }

        Kholle* klle = new Kholle();
        klle->setId_students(m_student->getId());
        klle->setId_timeslots(m_selectedTimeslot->getId());
        klle->setStudent(m_student);
        klle->setTimeslot(m_selectedTimeslot);

        QSqlQuery query(*m_db);
        query.prepare("INSERT INTO `tau_kholles`(`id_users`, `id_timeslots`) VALUES(:id_users, :id_timeslots)");
        query.bindValue(":id_users", klle->getId_students());
        query.bindValue(":id_timeslots", klle->getId_timeslots());
        query.exec();
        klle->setId(query.lastInsertId().toInt());

        m_dbase->listKholles()->insert(klle->getId(), klle);
        m_student->kholles()->append(klle);
        m_selectedTimeslot->kholles()->append(klle);

        updateListKholleurs();

        displayTable();
        updateInfoArea();
    } else {
        if(!m_student)
                QMessageBox::critical(NULL, "Erreur", "Veuillez sélectionner un étudiant...");
        else    QMessageBox::critical(NULL, "Erreur", "Veuillez sélectionner un horaire de kholle...");
        return false;
    }

    return true;
}

bool KholloTable::removeKholle(Student* stud) {
    if(!stud)
        stud = m_student;

    if(m_selectedTimeslot && stud) {
        // Get the kholle
        Kholle* klle = NULL;
        for(int i=0; i<m_selectedTimeslot->kholles()->count() && !klle; i++)
            if(m_selectedTimeslot->kholles()->at(i)->getId_students() == stud->getId())
                klle = m_selectedTimeslot->kholles()->at(i);
        if(!klle) {
            QMessageBox::critical(NULL, "Erreur", "Cet étudiant ne participe pas à cette kholle.");
            return false;
        }

        // Remove in the database
        QSqlQuery query(*m_db);
        query.prepare("DELETE FROM `tau_kholles` WHERE `id` = :id");
        query.bindValue(":id", klle->getId());
        query.exec();

        stud->kholles()->removeOne(klle);
        m_selectedTimeslot->kholles()->removeOne(klle);
        m_dbase->listKholles()->remove(klle->getId());

        updateListKholleurs();

        displayTable();
        updateInfoArea();
    } else {
        if(!stud)
                QMessageBox::critical(NULL, "Erreur", "Veuillez sélectionner un étudiant...");
        else    QMessageBox::critical(NULL, "Erreur", "Veuillez sélectionner un horaire de kholle...");
        return false;
    }

    return true;
}

bool KholloTable::removeKholleFromInfoArea() {
    QListWidget* list_students = m_areaKholles->findChild<QListWidget*>("list_students");
    QListWidgetItem *item = list_students->currentItem();
    if(item) {
        Student* stud = (Student*) item->data(Qt::UserRole).toULongLong();
        return removeKholle(stud);
    } else {
        QMessageBox::critical(NULL, "Erreur", "Veuillez sélectionner un étudiant...");
        return false;
    }
    return true;
}


bool KholloTable::selectStudentInInterface() {
    if(m_interface) {
        QListWidget* list_students = m_areaKholles->findChild<QListWidget*>("list_students");
        QListWidgetItem *item = list_students->currentItem();
        if(item) {
            Student* stud = (Student*) item->data(Qt::UserRole).toULongLong();
            m_interface->selectStudent(stud);
        } else {
            QMessageBox::critical(NULL, "Erreur", "Veuillez sélectionner un étudiant...");
            return false;
        }
    }
    return true;
}

bool KholloTable::updateListKholleurs() {
    if(m_tab)
        m_tab->selectStudent(m_student);
    return true;
}
