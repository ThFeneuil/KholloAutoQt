#include "managers/timetable.h"

TimeTable::TimeTable(QSqlDatabase *db, Group *currentGroup, QMap<int, Subject *> subjects)
{
    m_db = db;
    m_currentGroup = currentGroup;
    m_displayedCourses = new QList<CourseSlot*>;
    m_selectedCourse = NULL;
    m_subjects = subjects;

    m_qtimeedit_start = NULL;
    m_qtimeedit_duration = NULL;
    m_comboBox_subjects = NULL;

    /// Positions of the areas for the days in the khollotable
    m_sizeImg.insert(BeginDays, 42);
    m_sizeImg.insert(BetweenDays, 100);
    m_sizeImg.insert(BeginHours, 33);
    m_sizeImg.insert(BetweenHours, 30);

    /// Display the empty khollotable
    QPixmap emptyTable(":/images/emptyTimeTable.png");
    addPixmap(emptyTable);

    update_scene();
}

TimeTable::~TimeTable() {
    freeDisplayedCourses();
}

bool TimeTable::freeDisplayedCourses() {
    for(int i=0; i < m_displayedCourses->length(); i++) {
        if(m_displayedCourses->at(i)->otherWeek)
            delete m_displayedCourses->at(i);
        delete m_displayedCourses->at(i);
    }
    m_displayedCourses->clear();
    return true;
}

bool TimeTable::update_scene(int idSelec) {
    qDebug() << 2;
    clear();
    m_selectedCourse = NULL;

    QPixmap emptyTable(":/images/emptyTimeTable.png");
    addPixmap(emptyTable);

    freeDisplayedCourses();

    if(m_currentGroup == NULL)
        return display_scene();

    QSqlQuery query(*m_db);
    query.prepare("SELECT id, id_subjects, id_day, time_start, time_end, id_week FROM tau_courses WHERE id_groups=:id_groups ORDER BY id_subjects, id_day, time_start, id_week");
    query.bindValue(":id_groups", m_currentGroup->getId());
    query.exec();

    while(query.next()) {
        Course* course = new Course();
        course->setId(query.value(0).toInt());
        course->setId_subjects(query.value(1).toInt());
        course->setId_day(query.value(2).toInt());
        course->setTime_start(QTime::fromString(query.value(3).toString(), "hh:mm:ss"));
        course->setTime_end(QTime::fromString(query.value(4).toString(), "hh:mm:ss"));
        course->setId_week(query.value(5).toInt());
        course->setId_groups(m_currentGroup->getId());

        bool doubleCourse = false;
        if(m_displayedCourses->count() > 0) {
            Course* previous = m_displayedCourses->last()->course;
            doubleCourse = course->getId_subjects() == previous->getId_subjects() && course->getId_day() == previous->getId_day() &&
                           course->getTime_start() == previous->getTime_start() && course->getTime_end() == previous->getTime_end();
        }

        CourseSlot* slot = new CourseSlot;
        slot->course = course;
        slot->slot = calculateRect(course, doubleCourse);
        if(doubleCourse) {
            slot->otherWeek = m_displayedCourses->last();
            m_displayedCourses->removeLast();
        } else
            slot->otherWeek = NULL;

        m_displayedCourses->append(slot);

        if(course->getId() == idSelec)
            m_selectedCourse = slot;
    }

    return display_scene();
}

QRect* TimeTable::calculateRect(Course* crse, bool doubleCourse) {
    int x = m_sizeImg[BeginDays] + (crse->getId_day()-1)*m_sizeImg[BetweenDays];
    x += (!doubleCourse) ? (crse->getId_week()-1)*m_sizeImg[BetweenDays]/2 : 0;
    int y = m_sizeImg[BeginHours]+m_sizeImg[BetweenHours]*(crse->getTime_start().msecsSinceStartOfDay() - QTime(8, 0).msecsSinceStartOfDay())/3600000;
    int h = m_sizeImg[BetweenHours]*(crse->getTime_end().msecsSinceStartOfDay() - crse->getTime_start().msecsSinceStartOfDay())/3600000;
    int w = (doubleCourse) ? m_sizeImg[BetweenDays] : m_sizeImg[BetweenDays]/2;

    QRect* rect = new QRect(x, y, w, h);
    return rect;
}

bool TimeTable::display_scene() {
    qDebug() << 1;
    clear();

    QPixmap emptyTable(":/images/emptyTimeTable.png");
    addPixmap(emptyTable);

    for(int i=0; i<m_displayedCourses->length(); i++) {
        QRect rect = *(m_displayedCourses->at(i)->slot);
        addRect(rect, QPen(Qt::black, 0), QBrush(Qt::gray));
        QGraphicsTextItem *text = addText(m_subjects[m_displayedCourses->at(i)->course->getId_subjects()]->getShortName());
        text->setPos(rect.x() + 3, rect.y() + 3);
    }

    if(m_selectedCourse) {
        addRect(*(m_selectedCourse->slot), QPen(Qt::yellow, 3));
    }

    if(m_qtimeedit_start) {
        m_qtimeedit_start->setEnabled(m_selectedCourse);
        if(m_selectedCourse)
                m_qtimeedit_start->setTime(m_selectedCourse->course->getTime_start());
        else    m_qtimeedit_start->setTime(QTime(0,0));
    }
    if(m_qtimeedit_duration) {
        m_qtimeedit_duration->setEnabled(m_selectedCourse);
        if(m_selectedCourse)
                m_qtimeedit_duration->setTime(QTime(0,0).addMSecs(m_selectedCourse->course->getTime_end().msecsSinceStartOfDay() - m_selectedCourse->course->getTime_start().msecsSinceStartOfDay()));
        else    m_qtimeedit_duration->setTime(QTime(0,0));
    }

    if(m_comboBox_subjects) {
        m_comboBox_subjects->setEnabled(m_selectedCourse);
        for(int i=0; i<m_comboBox_subjects->count() && m_selectedCourse; i++)
            if(m_selectedCourse->course->getId_subjects() == ((Subject*) m_comboBox_subjects->itemData(i).toULongLong())->getId())
                m_comboBox_subjects->setCurrentIndex(i);
    }


    return true;
}

void TimeTable::setCurrent_group(Group* currentGroup) {
    m_currentGroup = currentGroup;
    update_scene();
}

void TimeTable::mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent) {
    QPoint pos = mouseEvent->scenePos().toPoint();
    xDown = pos.x();
    yDown = pos.y();
}

void TimeTable::mouseReleaseEvent(QGraphicsSceneMouseEvent *mouseEvent) {
    QPoint pos = mouseEvent->scenePos().toPoint();

    bool creating = false;
    if(abs(pos.x()-xDown) > 5 || abs(pos.y()-yDown) > 5)
        creating = true;

    QPoint tl;
    tl.setX(min(xDown, pos.x()));
    tl.setY(min(yDown, pos.y()));
    QPoint dr;
    dr.setX(max(xDown, pos.x()));
    dr.setY(max(yDown, pos.y()));

    int idDays = ((tl.x() + dr.x())/2-m_sizeImg[BeginDays]) / m_sizeImg[BetweenDays] + 1;
    int leftDay = m_sizeImg[BeginDays]+(idDays-1)*m_sizeImg[BetweenDays];
    int rightDay = m_sizeImg[BeginDays]+idDays*m_sizeImg[BetweenDays];
    int middleDay = (leftDay + rightDay) / 2;
    if(tl.x() < leftDay)
        tl.setX(leftDay);
    if(dr.x() > rightDay)
        dr.setX(rightDay);
    if(tl.y() < m_sizeImg[BeginHours])
        tl.setY(m_sizeImg[BeginHours]);
    if(dr.y() > m_sizeImg[BeginHours] + 12*m_sizeImg[BetweenHours])
        dr.setY(m_sizeImg[BeginHours] + 12*m_sizeImg[BetweenHours]);

    QTime startHour(8,0);
    int secPlus = (tl.y() - m_sizeImg[BeginHours])*3600/m_sizeImg[BetweenHours];
    secPlus = round(secPlus / (15. * 60.)) * 15 * 60;
    startHour = startHour.addSecs(secPlus);
    QTime endHour(8, 0);
    secPlus = (dr.y() - m_sizeImg[BeginHours])*3600/m_sizeImg[BetweenHours];
    secPlus = round(secPlus / (15. * 60.)) * 15 * 60;
    endHour = endHour.addSecs(secPlus);

    if(creating) {
        Course crse;
        crse.setId_day(idDays);
        if(tl.x() < middleDay)
                crse.setId_week((dr.x() > middleDay) ? 0 : 1);
        else    crse.setId_week(2);
        crse.setId_groups(m_currentGroup->getId());
        crse.setTime_start(startHour);
        crse.setTime_end(endHour);
        if(m_comboBox_subjects && m_comboBox_subjects->count() > 0)
                crse.setId_subjects( ((Subject*) m_comboBox_subjects->itemData(0).toULongLong())->getId() );
        else    crse.setId_subjects(0);

        int idSelec = -1;
        if(crse.getId_week() == 0) {
            crse.setId_week(1);
            createCourse(&crse);
            crse.setId_week(2);
            idSelec = createCourse(&crse);
        } else
            idSelec = createCourse(&crse);

        update_scene(idSelec);
    } else /*if(xDown == pos.x() && yDown == pos.y())*/ {
        m_selectedCourse = NULL;
        for(int i=0; i<m_displayedCourses->length(); i++)
            if(m_displayedCourses->at(i)->slot->contains(xDown, yDown))
                m_selectedCourse = m_displayedCourses->at(i);
        display_scene();
    }

}

int TimeTable::createCourse(Course* crse) {
    QSqlQuery query(*m_db);
    query.prepare("INSERT INTO tau_courses(id_subjects, time_start, time_end, id_groups, id_teachers, id_day, id_week) VALUES(:id_subjects, :time_start, :time_end, :id_groups, 0, :id_day, :id_week)");
    query.bindValue(":id_subjects", crse->getId_subjects());
    query.bindValue(":time_start", crse->getTime_start().toString("hh:mm:ss"));
    query.bindValue(":time_end", crse->getTime_end().toString("hh:mm:ss"));
    query.bindValue(":id_groups", crse->getId_groups());
    query.bindValue(":id_day", crse->getId_day());
    query.bindValue(":id_week", crse->getId_week());
    query.exec();

    return query.lastInsertId().toInt();
}

bool TimeTable::deleteSelectedCourse() {
    if(m_selectedCourse) {
        QSqlQuery query(*m_db);
        query.prepare("DELETE FROM tau_courses WHERE id=:id");
        query.bindValue(":id", m_selectedCourse->course->getId());
        query.exec();

        if(m_selectedCourse->otherWeek) {
            query.prepare("DELETE FROM tau_courses WHERE id=:id");
            query.bindValue(":id", m_selectedCourse->otherWeek->course->getId());
            query.exec();
        }

        update_scene();
    }

    return true;
}

bool TimeTable::setInputs(QTimeEdit* qtimeedit_start, QTimeEdit* qtimeedit_duration, QComboBox *comboBox_subjects) {
    m_qtimeedit_start = qtimeedit_start;
    m_qtimeedit_duration = qtimeedit_duration;
    m_comboBox_subjects = comboBox_subjects;

    if(m_qtimeedit_start)
            connect(m_qtimeedit_start, SIGNAL(editingFinished()), this, SLOT(courseModified()));
    else    disconnect(m_qtimeedit_start, SIGNAL(editingFinished()), this, SLOT(courseModified()));

    if(m_qtimeedit_duration)
            connect(m_qtimeedit_duration, SIGNAL(editingFinished()), this, SLOT(courseModified()));
    else    disconnect(m_qtimeedit_duration, SIGNAL(editingFinished()), this, SLOT(courseModified()));

    if(m_comboBox_subjects)
            connect(m_comboBox_subjects, SIGNAL(currentIndexChanged(int)), this, SLOT(courseModified()));
    else    disconnect(m_comboBox_subjects, SIGNAL(currentIndexChanged(int)), this, SLOT(courseModified()));

    return display_scene();
}

bool TimeTable::courseModified() {

    if(m_selectedCourse) {
        m_selectedCourse->course->setTime_start(m_qtimeedit_start->time());
        m_selectedCourse->course->setTime_end(m_qtimeedit_start->time().addMSecs(m_qtimeedit_duration->time().msecsSinceStartOfDay()));
        m_selectedCourse->course->setId_subjects(((Subject*) m_comboBox_subjects->currentData().toULongLong())->getId());
        m_selectedCourse->slot = calculateRect(m_selectedCourse->course, m_selectedCourse->otherWeek);

        QSqlQuery query(*m_db);
        query.prepare("UPDATE tau_courses SET time_start = :time_start, time_end = :time_end, id_subjects = :id_subjects WHERE id=:id");
        query.bindValue(":id", m_selectedCourse->course->getId());
        query.bindValue(":time_start", m_selectedCourse->course->getTime_start().toString("hh:mm:ss"));
        query.bindValue(":time_end", m_selectedCourse->course->getTime_end().toString("hh:mm:ss"));
        query.bindValue(":id_subjects", m_selectedCourse->course->getId_subjects());
        query.exec();

        if(m_selectedCourse->otherWeek) {
            m_selectedCourse->otherWeek->course->setTime_start(m_qtimeedit_start->time());
            m_selectedCourse->otherWeek->course->setTime_end(m_qtimeedit_start->time().addMSecs(m_qtimeedit_duration->time().msecsSinceStartOfDay()));
            m_selectedCourse->otherWeek->course->setId_subjects(((Subject*) m_comboBox_subjects->currentData().toULongLong())->getId());
            m_selectedCourse->otherWeek->slot = calculateRect(m_selectedCourse->otherWeek->course, false);

            query.prepare("UPDATE tau_courses SET time_start = :time_start, time_end = :time_end, id_subjects = :id_subjects WHERE id=:id");
            query.bindValue(":id", m_selectedCourse->otherWeek->course->getId());
            query.bindValue(":time_start", m_selectedCourse->otherWeek->course->getTime_start().toString("hh:mm:ss"));
            query.bindValue(":time_end", m_selectedCourse->otherWeek->course->getTime_end().toString("hh:mm:ss"));
            query.bindValue(":id_subjects", m_selectedCourse->course->getId_subjects());
            query.exec();
        }

        display_scene();
    }

    return true;
}
