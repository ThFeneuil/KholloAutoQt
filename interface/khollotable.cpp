#include "interface/khollotable.h"
#include <QPixmap>

KholloTable::KholloTable(QSqlDatabase* db, int id_week, QDate monday, QWidget *areaKholles) : QGraphicsScene() {
    m_db = db;
    m_id_week = id_week;
    m_monday = monday;
    m_areaKholles = areaKholles;

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
}

KholloTable::~KholloTable() {

}

void KholloTable::displayKholleur(Kholleur* kll) {
    if(m_student != NULL) {
        displayKholleurAndStudent(kll, m_student);
        return;
    }

    if(!m_kholleur || m_kholleur->getId() != kll->getId())
        remove_selection();
    m_kholleur = kll;
    clear();
    m_timeslots.clear();

    QQueue<QRect> areaKholles;

    // Get the list of all the timeslots
    QSqlQuery query(*m_db);
    query.prepare("SELECT id, time, time_end, id_kholleurs, date, time_start FROM `tau_timeslots` "
                    "WHERE `id_kholleurs` = :id_kholleur AND (date >= :start AND date < :end)");
    query.bindValue(":id_kholleur", kll->getId());
    query.bindValue(":start", m_monday.toString("yyyy-MM-dd"));
    query.bindValue(":end", m_monday.addDays(7).toString("yyyy-MM-dd"));
    query.exec();
    while (query.next()) {
        Timeslot* slot = new Timeslot();
        slot->setId(query.value(0).toInt());
        slot->setTime(query.value(1).toTime());
        slot->setTime_end(query.value(2).toTime());
        slot->setId_kholleurs(query.value(3).toInt());
        slot->setDate(query.value(4).toDate());
        slot->setTime_start(query.value(5).toTime());

        QSqlQuery queryKholles(*m_db);
        queryKholles.prepare("SELECT * "
                      "FROM `tau_kholles` AS K "
                      "JOIN `tau_users` AS U "
                          "ON U.`id` = K.`id_users` "
                      "WHERE K.`id_timeslots` = :id_timeslots");
        queryKholles.bindValue(":id_timeslots", m_selectedTimeslot->getId());
        queryKholles.exec();

        // Display the kholle
        int x = m_sizeImg[BeginDays]+m_sizeImg[BetweenDays]*m_monday.daysTo(slot->getDate());
        int y = m_sizeImg[BeginHours]+m_sizeImg[BetweenHours]*(slot->getTime().msecsSinceStartOfDay() - QTime(8, 0).msecsSinceStartOfDay())/3600000;
        int w = m_sizeImg[BetweenDays];
        int h = m_sizeImg[BetweenHours]*(slot->getTime_end().msecsSinceStartOfDay() - slot->getTime().msecsSinceStartOfDay())/3600000;
        slot->setArea(new QRect(x,y,w,h));
        addRect(*slot->getArea(), QPen(Qt::black, 0), QBrush(Qt::green));
        areaKholles.append(*slot->getArea());
        m_timeslots.append(slot);
    }

    QPixmap monPixmap(":/images/emptyTimeTable.png");
    addPixmap(monPixmap);

    while (!areaKholles.isEmpty()) {
        QRect rect = areaKholles.dequeue();
        addRect(rect, QPen(Qt::blue, 1));
    }
}

void KholloTable::displayStudent(Student* stud) {
    if(m_kholleur != NULL) {
        displayKholleurAndStudent(m_kholleur, stud);
        return;
    }

    m_student = stud;
    clear();

    QQueue<QRect> areaCourses;

    // Get the list of all the students
    QSqlQuery query(*m_db);
    query.prepare("SELECT id, id_subjects, time_start, time_end, id_groups, id_teachers, id_day, id_week "
                  "FROM `tau_courses` AS C "
                  "WHERE C.`id_groups` IN "
                      "(SELECT `id_groups` "
                       "FROM `tau_groups_users` AS L "
                       "WHERE L.`id_users` = :id_users "
                        "AND id_week = :id_week "
                                ")");
    query.bindValue(":id_users", stud->getId());
    query.bindValue(":id_week", m_id_week);
    query.exec();
    while (query.next()) {
        Course* course = new Course();
        course->setId(query.value(0).toInt());
        course->setId_subjects(query.value(1).toInt());
        course->setTime_start(query.value(2).toTime());
        course->setTime_end(query.value(3).toTime());
        course->setId_groups(query.value(4).toInt());
        course->setId_teachers(query.value(5).toInt());
        course->setId_day(query.value(6).toInt());
        course->setId_week(query.value(7).toInt());

        // Display the course
        int x = m_sizeImg[BeginDays]+m_sizeImg[BetweenDays]*(course->getId_day()-1);
        int y = m_sizeImg[BeginHours]+m_sizeImg[BetweenHours]*(course->getTime_start().msecsSinceStartOfDay() - QTime(8, 0).msecsSinceStartOfDay())/3600000;
        int w = m_sizeImg[BetweenDays];
        int h = m_sizeImg[BetweenHours]*(course->getTime_end().msecsSinceStartOfDay() - course->getTime_start().msecsSinceStartOfDay())/3600000;
        QRect rect(x,y,w,h);
        addRect(rect, QPen(Qt::black, 0), QBrush(Qt::gray));
        areaCourses.append(rect);
    }

    QPixmap monPixmap(":/images/emptyTimeTable.png");
    addPixmap(monPixmap);

    while (!areaCourses.isEmpty()) {
        QRect rect = areaCourses.dequeue();
        addRect(rect, QPen(Qt::blue, 1));
    }

    add_selection();
}

void KholloTable::displayKholleurAndStudent(Kholleur* kll, Student* stud) {
    m_student = stud;
    if(!m_kholleur || m_kholleur->getId() != kll->getId())
        remove_selection();
    m_kholleur = kll;
    clear();
    m_timeslots.clear();

    QQueue<QRect> areaCourses;

    // Get the list of all the students
    QSqlQuery query(*m_db);
    query.prepare("SELECT id, id_subjects, time_start, time_end, id_groups, id_teachers, id_day, id_week "
                  "FROM `tau_courses` AS C "
                  "WHERE C.`id_groups` IN "
                      "(SELECT `id_groups` "
                       "FROM `tau_groups_users` AS L "
                       "WHERE L.`id_users` = :id_users "
                        "AND id_week = :id_week "
                                ")");
    query.bindValue(":id_users", stud->getId());
    query.bindValue(":id_week", m_id_week);
    query.exec();
    while (query.next()) {
        Course* course = new Course();
        course->setId(query.value(0).toInt());
        course->setId_subjects(query.value(1).toInt());
        course->setTime_start(query.value(2).toTime());
        course->setTime_end(query.value(3).toTime());
        course->setId_groups(query.value(4).toInt());
        course->setId_teachers(query.value(5).toInt());
        course->setId_day(query.value(6).toInt());
        course->setId_week(query.value(7).toInt());

        // Display the course
        int x = m_sizeImg[BeginDays]+m_sizeImg[BetweenDays]*(course->getId_day()-1);
        int y = m_sizeImg[BeginHours]+m_sizeImg[BetweenHours]*(course->getTime_start().msecsSinceStartOfDay() - QTime(8, 0).msecsSinceStartOfDay())/3600000;
        int w = m_sizeImg[BetweenDays];
        int h = m_sizeImg[BetweenHours]*(course->getTime_end().msecsSinceStartOfDay() - course->getTime_start().msecsSinceStartOfDay())/3600000;
        QRect rect(x,y,w,h);
        addRect(rect, QPen(Qt::black, 0), QBrush(Qt::gray));
        areaCourses.append(rect);
    }

    QQueue<QRect> areaKholles;

    // Get the list of all the students
    QSqlQuery query2(*m_db);
    query2.prepare("SELECT id, time, time_end, id_kholleurs, date, time_start FROM `tau_timeslots` "
                    "WHERE `id_kholleurs` = :id_kholleur AND (date >= :start AND date < :end)");
    query2.bindValue(":id_kholleur", kll->getId());
    query2.bindValue(":start", m_monday.toString("yyyy-MM-dd"));
    query2.bindValue(":end", m_monday.addDays(7).toString("yyyy-MM-dd"));
    query2.exec();
    while (query2.next()) {
        Timeslot* slot = new Timeslot();
        slot->setId(query2.value(0).toInt());
        slot->setTime(query2.value(1).toTime());
        slot->setTime_end(query2.value(2).toTime());
        slot->setId_kholleurs(query2.value(3).toInt());
        slot->setDate(query2.value(4).toDate());
        slot->setTime_start(query2.value(5).toTime());

        // Display the kholle
        int x = m_sizeImg[BeginDays]+m_sizeImg[BetweenDays]*m_monday.daysTo(slot->getDate());
        int y = m_sizeImg[BeginHours]+m_sizeImg[BetweenHours]*(slot->getTime().msecsSinceStartOfDay() - QTime(8, 0).msecsSinceStartOfDay())/3600000;
        int w = m_sizeImg[BetweenDays];
        int h = m_sizeImg[BetweenHours]*(slot->getTime_end().msecsSinceStartOfDay() - slot->getTime().msecsSinceStartOfDay())/3600000;
        slot->setArea(new QRect(x,y,w,h));
        if(compatible(stud, slot))
            addRect(*slot->getArea(), QPen(Qt::black, 0), QBrush(Qt::green));
        else
            addRect(*slot->getArea(), QPen(Qt::black, 0), QBrush(Qt::red, Qt::DiagCrossPattern));
        areaKholles.append(*slot->getArea());
        m_timeslots.append(slot);
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

    add_selection();
}

bool KholloTable::compatible(Student* stdnt, Timeslot *timeslot) {
    //Get all groups of user (the ids)
    QSqlQuery query(*m_db);
    query.prepare("SELECT id_groups FROM tau_groups_users WHERE id_users=:id_users");
    query.bindValue(":id_users", stdnt->getId());
    query.exec();

    //Get all courses that can interfere with this timeslot
    if(query.next()) {
        QString request = "id_groups=" + QString::number(query.value(0).toInt());
        while(query.next())
            request = request + " OR id_groups=" + QString::number(query.value(0).toInt());

        QSqlQuery courses_query(*m_db);
        courses_query.prepare("SELECT * FROM tau_courses WHERE (" + request + ") AND id_day=:id_day AND id_week=:id_week AND ("
                                                                              "(time_start <= :time_start AND time_end > :time_start) OR"
                                                                              "(time_start < :time_end AND time_end >= :time_end) OR"
                                                                              "(time_start >= :time_start AND time_end <= :time_end) )");
        courses_query.bindValue(":time_start", timeslot->getTime_start());
        courses_query.bindValue(":time_end", timeslot->getTime_end());
        courses_query.bindValue(":id_day", timeslot->getDate().dayOfWeek());
        courses_query.bindValue(":id_week", m_id_week);
        courses_query.exec();

        if(courses_query.next())
            return false;

        //Get all events that can interfere with this timeslot
        QSqlQuery event_query(*m_db);
        event_query.prepare("SELECT * FROM tau_events AS E JOIN tau_events_groups AS G ON E.`id` = G.`id_events` WHERE (" + request + ") AND ("
                                      "(E.`start` <= :start_time AND E.`end` > :start_time) OR"
                                      "(E.`start` < :end_time AND E.`end` >= :end_time) OR"
                                      "(E.`start` >= :start_time AND E.`end` <= :end_time))");

        event_query.bindValue(":start_time", QDateTime(timeslot->getDate(), timeslot->getTime_start()).toString("yyyy-MM-dd HH:mm:ss"));
        event_query.bindValue(":end_time", QDateTime(timeslot->getDate(), timeslot->getTime_end()).toString("yyyy-MM-dd HH:mm:ss"));
        event_query.exec();

        if(event_query.next())
            return false;
    }

    return true;
}

void KholloTable::mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent) {
    QPoint pos = mouseEvent->scenePos().toPoint();

    Timeslot* selectedTimeslot = NULL;

    for(int i=0; i<m_timeslots.count(); i++) {
        Timeslot * slot = m_timeslots.at(i);
        if(slot->getArea()->contains(pos)) {
            selectedTimeslot = slot;
            break;
        }
    }

    if(selectedTimeslot) {
        add_selection();

        if(m_areaKholles->layout()) {
            for(int i=0; i<m_areaKholles->layout()->count(); i++)
                delete m_areaKholles->layout()->itemAt(i)->widget();
            delete m_areaKholles->layout();
        }
        QVBoxLayout* layout = new QVBoxLayout();
        QString text = "<strong>Date :</strong> " + m_selectedTimeslot->getDate().toString("dd/MM/yyyy") + "<br />" +
                       "<strong>Horaire :</strong> " + m_selectedTimeslot->getTime().toString("hh:mm") + " >> " + m_selectedTimeslot->getTime_end().toString("hh:mm") + "<br />";
        if(m_selectedTimeslot->getTime_start() != m_selectedTimeslot->getTime())
            text += "<strong> ATTENTION préparation :</strong> Début à " + m_selectedTimeslot->getTime_start().toString("hh:mm");
        QLabel* title = new QLabel(text);
        layout->addWidget(title);
        m_areaKholles->setLayout(layout);
    }
}

bool KholloTable::add_selection() {
    if(m_selectedTimeslot) {
        Timeslot* selectedTimeslot = m_selectedTimeslot;
        remove_selection();
        m_selectedTimeslot = selectedTimeslot;

        m_selectedFrame = addRect(*(m_selectedTimeslot->getArea()), QPen(Qt::yellow, 3));
    }
    return true;
}

bool KholloTable::remove_selection() {
    QMessageBox::information(NULL, "INFO", "42");
    m_selectedTimeslot = NULL;
    if(m_selectedFrame) {
        removeItem(m_selectedFrame);
        delete m_selectedFrame;
        m_selectedFrame = NULL;
    }
    return true;
}
