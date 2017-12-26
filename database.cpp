#include "database.h"

DataBase::DataBase(QSqlDatabase *db) {
    m_db = db;
    m_listStudents = new QMap<int, Student*>();
    m_listGroups = new QMap<int, Group*>();
    m_listSubjects = new QMap<int, Subject*>();
    m_listKholleurs = new QMap<int, Kholleur*>();
    m_listCourses = new QMap<int, Course*>();
    m_listTimeslots = new QMap<int, Timeslot*>();
    m_listEvents = new QMap<int, Event*>();
    m_listKholles = new QMap<int, Kholle*>();

    m_conditionCourses = "";
    m_conditionTimeslots = "";
}

DataBase::~DataBase() {
    QMapIterator<int, Student*> iStudents(*m_listStudents);
    while (iStudents.hasNext()) {
        iStudents.next();
        delete iStudents.value();
    }
    QMapIterator<int, Group*> iGroups(*m_listGroups);
    while (iGroups.hasNext()) {
        iGroups.next();
        delete iGroups.value();
    }
    QMapIterator<int, Subject*> iSubjects(*m_listSubjects);
    while (iSubjects.hasNext()) {
        iSubjects.next();
        delete iSubjects.value();
    }
    QMapIterator<int, Kholleur*> iKholleurs(*m_listKholleurs);
    while (iKholleurs.hasNext()) {
        iKholleurs.next();
        delete iKholleurs.value();
    }
    QMapIterator<int, Course*> iCourses(*m_listCourses);
    while (iCourses.hasNext()) {
        iCourses.next();
        delete iCourses.value();
    }
    QMapIterator<int, Timeslot*> iTimeslots(*m_listTimeslots);
    while (iTimeslots.hasNext()) {
        iTimeslots.next();
        delete iTimeslots.value();
    }
    QMapIterator<int, Event*> iEvents(*m_listEvents);
    while (iEvents.hasNext()) {
        iEvents.next();
        delete iEvents.value();
    }
    QMapIterator<int, Kholle*> iKholles(*m_listKholles);
    while (iKholles.hasNext()) {
        iKholles.next();
        delete iKholles.value();
    }

    delete m_listStudents;
    delete m_listGroups ;
    delete m_listSubjects;
    delete m_listKholleurs;
    delete m_listCourses;
    delete m_listTimeslots;
    delete m_listEvents;
    delete m_listKholles;
}

QMap<int, Student*>* DataBase::listStudents() const {
    return m_listStudents;
}
QMap<int, Group*>* DataBase::listGroups() const {
    return m_listGroups;
}
QMap<int, Subject*>* DataBase::listSubjects() const {
    return m_listSubjects;
}
QMap<int, Kholleur*>* DataBase::listKholleurs() const {
    return m_listKholleurs;
}
QMap<int, Course*>* DataBase::listCourses() const {
    return m_listCourses;
}
QMap<int, Timeslot*>* DataBase::listTimeslots() const {
    return m_listTimeslots;
}
QMap<int, Event*>* DataBase::listEvents() const {
    return m_listEvents;
}
QMap<int, Kholle*>* DataBase::listKholles() const {
    return m_listKholles;
}

void DataBase::setConditionCourses(QString condition) {
    m_conditionCourses = condition;
}
void DataBase::setConditionTimeslots(QString condition) {
    m_conditionTimeslots = condition;
}

bool DataBase::load(QProgressBar* progressBar) {
    if(progressBar) progressBar->setValue(0); // Indicator

    QSqlQuery qStudents(*m_db);
    qStudents.exec("SELECT id, name, first_name, email FROM tau_users");
    while (qStudents.next()) {
        Student* stdnt = new Student();
        stdnt->setId(qStudents.value(0).toInt());
        stdnt->setName(qStudents.value(1).toString());
        stdnt->setFirst_name(qStudents.value(2).toString());
        stdnt->setEmail(qStudents.value(3).toString());

        m_listStudents->insert(stdnt->getId(), stdnt);
    }

    QSqlQuery qGroups(*m_db);
    qGroups.exec("SELECT id, name FROM tau_groups");
    while (qGroups.next()) {
        Group* grp = new Group();
        grp->setId(qGroups.value(0).toInt());
        grp->setName(qGroups.value(1).toString());

        m_listGroups->insert(grp->getId(), grp);
    }

    if(progressBar) progressBar->setValue(10); // Indicator

    QSqlQuery qStudentsGroups(*m_db);
    qStudentsGroups.exec("SELECT id, id_groups, id_users FROM tau_groups_users");
    while (qStudentsGroups.next()) {
        int idGroup = qStudentsGroups.value(1).toInt();
        int idStudent = qStudentsGroups.value(2).toInt();
        m_listStudents->value(idStudent)->groups()->append(m_listGroups->value(idGroup));
        m_listGroups->value(idGroup)->students()->append(m_listStudents->value(idStudent));
    }

    if(progressBar) progressBar->setValue(20); // Indicator

    QSqlQuery qSubjects(*m_db);
    qSubjects.exec("SELECT id, name, shortName, color, weight FROM tau_subjects");
    while (qSubjects.next()) {
        Subject* subj = new Subject();
        subj->setId(qSubjects.value(0).toInt());
        subj->setName(qSubjects.value(1).toString());
        subj->setShortName(qSubjects.value(2).toString());
        subj->setColor(qSubjects.value(3).toString());
        subj->setWeight(qSubjects.value(4).toInt());

        m_listSubjects->insert(subj->getId(), subj);
    }

    if(progressBar) progressBar->setValue(30); // Indicator

    QSqlQuery qKholleurs(*m_db);
    qKholleurs.exec("SELECT id, name, id_subjects, duration, preparation, pupils FROM tau_kholleurs");
    while (qKholleurs.next()) {
        Kholleur* khll = new Kholleur();
        khll->setId(qKholleurs.value(0).toInt());
        khll->setName(qKholleurs.value(1).toString());
        khll->setId_subjects(qKholleurs.value(2).toInt());
        khll->setDuration(qKholleurs.value(3).toInt());
        khll->setPreparation(qKholleurs.value(4).toInt());
        khll->setPupils(qKholleurs.value(5).toInt());

        m_listKholleurs->insert(khll->getId(), khll);

        khll->setSubject(m_listSubjects->value(khll->getId_subjects()));
        if(khll->getId_subjects())
            m_listSubjects->value(khll->getId_subjects())->kholleurs()->append(khll);
    }

    if(progressBar) progressBar->setValue(50); // Indicator

    QSqlQuery qCourses(*m_db);
    if(m_conditionCourses != "")
            qCourses.exec("SELECT id, id_subjects, time_start, time_end, id_groups, id_day, id_week FROM tau_courses WHERE " + m_conditionCourses);
    else    qCourses.exec("SELECT id, id_subjects, time_start, time_end, id_groups, id_day, id_week FROM tau_courses");
    while (qCourses.next()) {
        Course* course = new Course();
        course->setId(qCourses.value(0).toInt());
        course->setId_subjects(qCourses.value(1).toInt());
        course->setTime_start(qCourses.value(2).toTime());
        course->setTime_end(qCourses.value(3).toTime());
        course->setId_groups(qCourses.value(4).toInt());
        course->setId_day(qCourses.value(5).toInt());
        course->setId_week(qCourses.value(6).toInt());

        m_listCourses->insert(course->getId(), course);

        course->setSubject(m_listSubjects->value(course->getId_subjects()));
        course->setGroup(m_listGroups->value(course->getId_groups()));
        m_listSubjects->value(course->getId_subjects())->courses()->append(course);
        m_listGroups->value(course->getId_groups())->courses()->append(course);
    }

    if(progressBar) progressBar->setValue(60); // Indicator

    QSqlQuery qTimeslots(*m_db);
    if(m_conditionTimeslots != "")
            qTimeslots.exec("SELECT id, time, time_end, id_kholleurs, date, time_start, pupils FROM tau_timeslots WHERE " + m_conditionTimeslots);
    else    qTimeslots.exec("SELECT id, time, time_end, id_kholleurs, date, time_start, pupils FROM tau_timeslots");
    while (qTimeslots.next()) {
        Timeslot* slot = new Timeslot();
        slot->setId(qTimeslots.value(0).toInt());
        slot->setTime(qTimeslots.value(1).toTime());
        slot->setTime_end(qTimeslots.value(2).toTime());
        slot->setId_kholleurs(qTimeslots.value(3).toInt());
        slot->setDate(qTimeslots.value(4).toDate());
        slot->setTime_start(qTimeslots.value(5).toTime());
        slot->setPupils(qTimeslots.value(6).toInt());

        m_listTimeslots->insert(slot->getId(), slot);

        slot->setKholleur(m_listKholleurs->value(slot->getId_kholleurs()));
        m_listKholleurs->value(slot->getId_kholleurs())->timeslots()->append(slot);
    }

    if(progressBar) progressBar->setValue(70); // Indicator

    QSqlQuery qEvents(*m_db);
    qEvents.exec("SELECT id, name, comment, start, end FROM tau_events");
    while (qEvents.next()) {
        Event* event = new Event();
        event->setId(qEvents.value(0).toInt());
        event->setName(qEvents.value(1).toString());
        event->setComment(qEvents.value(2).toString());
        event->setStart(qEvents.value(3).toDateTime());
        event->setEnd(qEvents.value(4).toDateTime());

        m_listEvents->insert(event->getId(), event);
    }

    if(progressBar) progressBar->setValue(80); // Indicator

    QSqlQuery qEventsGroups(*m_db);
    qEventsGroups.exec("SELECT id, id_groups, id_events FROM tau_events_groups");
    while (qEventsGroups.next()) {
        int idGroup = qEventsGroups.value(1).toInt();
        int idEvent = qEventsGroups.value(2).toInt();
        if(m_listEvents->value(idEvent)) {
            m_listEvents->value(idEvent)->groups()->append(m_listGroups->value(idGroup));
            m_listGroups->value(idGroup)->events()->append(m_listEvents->value(idEvent));
        }
    }

    if(progressBar) progressBar->setValue(90); // Indicator

    QSqlQuery qKholles(*m_db);
    qKholles.exec("SELECT id, id_users, id_timeslots FROM tau_kholles");
    while (qKholles.next()) {
        Kholle* klle = new Kholle();
        klle->setId(qKholles.value(0).toInt());
        klle->setId_students(qKholles.value(1).toInt());
        klle->setId_timeslots(qKholles.value(2).toInt());

        if(m_listTimeslots->value(klle->getId_timeslots())) {
            m_listKholles->insert(klle->getId(), klle);

            klle->setStudent(m_listStudents->value(klle->getId_students()));
            klle->setTimeslot(m_listTimeslots->value(klle->getId_timeslots()));
            m_listStudents->value(klle->getId_students())->kholles()->append(klle);
            m_listTimeslots->value(klle->getId_timeslots())->kholles()->append(klle);
        }
    }

    if(progressBar) progressBar->setValue(100); // Indicator

    return true;
}
