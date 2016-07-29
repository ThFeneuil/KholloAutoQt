#include "interface/khollotable.h"
#include <QPixmap>

KholloTable::KholloTable(QSqlDatabase* db, int id_week) : QGraphicsScene() {
    m_db = db;
    m_id_week = id_week;

    sizeImg.insert(BeginDays, 42);
    sizeImg.insert(BetweenDays, 100);
    sizeImg.insert(BeginHours, 33);
    sizeImg.insert(BetweenHours, 30);

    m_student = NULL;
    m_kholleur = NULL;

    /*
     * addRect(310,50,100, 150,QPen(QColor(255,0,0)), QBrush(Qt::red));
    QPixmap monPixmap(":/images/emptyTimeTable.png");
    addPixmap(monPixmap);
    addRect(110,70,100,150,QPen(QColor(255,0,0)), QBrush(Qt::green));
    */
/*
    Kholleur* lafitte = new Kholleur();
    lafitte->setId(13);
    displayKholleur(lafitte);*/
    QPixmap monPixmap(":/images/emptyTimeTable.png");
    addPixmap(monPixmap);
}

KholloTable::~KholloTable() {

}

void KholloTable::displayKholleur(Kholleur* kll) {
    m_kholleur = kll;
    clear();

    QQueue<QRect> areaKholles;

    // Get the list of all the students
    QSqlQuery query(*m_db);
    query.prepare("SELECT id, time, time_end, id_kholleurs, id_day, time_start FROM `tau_timeslots` WHERE `id_kholleurs` = :id_kholleur");
    query.bindValue(":id_kholleur", kll->getId());
    query.exec();
    while (query.next()) {
        Timeslot* slot = new Timeslot();
        slot->setId(query.value(0).toInt());
        slot->setTime(query.value(1).toTime());
        slot->setTime_end(query.value(2).toTime());
        slot->setId_kholleurs(query.value(3).toInt());
        slot->setId_day(query.value(4).toInt());
        slot->setTime_start(query.value(5).toTime());

        // Display the kholle
        int x = sizeImg[BeginDays]+sizeImg[BetweenDays]*(slot->getId_day()-1);
        int y = sizeImg[BeginHours]+sizeImg[BetweenHours]*(slot->getTime().msecsSinceStartOfDay() - QTime(8, 0).msecsSinceStartOfDay())/3600000;
        int w = sizeImg[BetweenDays];
        int h = sizeImg[BetweenHours]*(slot->getTime_end().msecsSinceStartOfDay() - slot->getTime().msecsSinceStartOfDay())/3600000;
        QRect rect(x,y,w,h);
        addRect(rect, QPen(Qt::black, 0), QBrush(Qt::green));
        areaKholles.append(rect);
    }

    QPixmap monPixmap(":/images/emptyTimeTable.png");
    addPixmap(monPixmap);

    while (!areaKholles.isEmpty()) {
        QRect rect = areaKholles.dequeue();
        addRect(rect, QPen(Qt::blue, 1));
    }
}

void KholloTable::displayStudent(Student* stud) {
    m_student = stud;
    clear();

    QQueue<QRect> areaKholles;

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
        int x = sizeImg[BeginDays]+sizeImg[BetweenDays]*(course->getId_day()-1);
        int y = sizeImg[BeginHours]+sizeImg[BetweenHours]*(course->getTime_start().msecsSinceStartOfDay() - QTime(8, 0).msecsSinceStartOfDay())/3600000;
        int w = sizeImg[BetweenDays];
        int h = sizeImg[BetweenHours]*(course->getTime_end().msecsSinceStartOfDay() - course->getTime_start().msecsSinceStartOfDay())/3600000;
        QRect rect(x,y,w,h);
        addRect(rect, QPen(Qt::black, 0), QBrush(Qt::gray));
        areaKholles.append(rect);
    }

    QPixmap monPixmap(":/images/emptyTimeTable.png");
    addPixmap(monPixmap);

    while (!areaKholles.isEmpty()) {
        QRect rect = areaKholles.dequeue();
        addRect(rect, QPen(Qt::blue, 1));
    }
}
