#include "managers/timetable.h"

TimeTable::TimeTable(QSqlDatabase *db, Group *currentGroup, QMap<int, Subject *> subjects)
{   /** CONSTRUCTOR **/

    /// Initalisation of the class properties
    // Sql Database
    m_db = db;
    // General information of the manager
    m_currentGroup = currentGroup;
    m_subjects = subjects;
    // Specific information of the timetable
    m_displayedCourses = new QList<CourseSlot*>;
    m_selectedCourse = NULL;
    // Inputs to modify courses
    m_qtimeedit_start = NULL;
    m_qtimeedit_duration = NULL;
    m_comboBox_subjects = NULL;

    /// Positions of the areas for the days in the khollotable
    m_sizeImg.insert(BeginDays, 42);
    m_sizeImg.insert(BetweenDays, 100);
    m_sizeImg.insert(BeginHours, 33);
    m_sizeImg.insert(BetweenHours, 30);

    /// Display the timetable
    update_scene();
}

TimeTable::~TimeTable() {
    /** DESTRUCTOR **/

    freeDisplayedCourses();
}

bool TimeTable::freeDisplayedCourses() {
    /** TO FREE MEMORY WITH DISPLAYED COURSES **/

    for(int i=0; i < m_displayedCourses->length(); i++) {
        // If it is a double course, delete the second part
        if(m_displayedCourses->at(i)->otherWeek)
            delete m_displayedCourses->at(i);
        delete m_displayedCourses->at(i);
    }
    // Clear the list
    m_displayedCourses->clear();
    return true;
}

bool TimeTable::update_scene(int idSelec) {
    /** TO FREE MEMORY WITH DISPLAYED COURSES **/
    // Reset the timetable data
    m_selectedCourse = NULL;
    freeDisplayedCourses();

    // Get the courses if a group is selected
    if(m_currentGroup) {
        // Execute the query to get the courses of the selected group
        QSqlQuery query(*m_db);
        query.prepare("SELECT id, id_subjects, id_day, time_start, time_end, id_week FROM tau_courses WHERE id_groups=:id_groups ORDER BY id_subjects, id_day, time_start, id_week");
            // The order in the query is to enable to detect double course.
        query.bindValue(":id_groups", m_currentGroup->getId());
        query.exec();

        // Treat the answer of the query
        while(query.next()) {
            // Build the objec "Course"
            Course* course = new Course();
            course->setId(query.value(0).toInt());
            course->setId_subjects(query.value(1).toInt());
            course->setId_day(query.value(2).toInt());
            course->setTime_start(QTime::fromString(query.value(3).toString(), "hh:mm:ss"));
            course->setTime_end(QTime::fromString(query.value(4).toString(), "hh:mm:ss"));
            course->setId_week(query.value(5).toInt());
            course->setId_groups(m_currentGroup->getId());

            // Calculate if it is a double course, i.e., a course on the both weeks
            bool doubleCourse = false;
            if(m_displayedCourses->count() > 0) {
                Course* previous = m_displayedCourses->last()->course;
                doubleCourse = course->getId_subjects() == previous->getId_subjects() && course->getId_day() == previous->getId_day() &&
                               course->getTime_start() == previous->getTime_start() && course->getTime_end() == previous->getTime_end();
            }

            // Build the courseslot, i.e., the object course with the rect which represents it on the timetable
            CourseSlot* slot = new CourseSlot;
            slot->course = course;
            slot->slot = calculateRect(course, doubleCourse);
            if(doubleCourse) {
                slot->otherWeek = m_displayedCourses->last();
                m_displayedCourses->removeLast(); // Remove the previous, because the both course are merged
            } else
                slot->otherWeek = NULL;

            // Append the course in the list of displayed courses
            m_displayedCourses->append(slot);

            // If the course is selected (thanks to its ID), update the variable
            if(course->getId() == idSelec)
                m_selectedCourse = slot;
        }
    }

    // Display the timetable
    return display_scene();
}

QRect* TimeTable::calculateRect(Course* crse, bool doubleCourse) {
    /** TO BUILD THE RECT WHICH WILL REPRESENT A COURSE ON THE TIMETABLE **/

    // Position
    int x = m_sizeImg[BeginDays] + (crse->getId_day()-1)*m_sizeImg[BetweenDays];
    x += (!doubleCourse) ? (crse->getId_week()-1)*m_sizeImg[BetweenDays]/2 : 0;
    int y = m_sizeImg[BeginHours]+m_sizeImg[BetweenHours]*(crse->getTime_start().msecsSinceStartOfDay() - QTime(8, 0).msecsSinceStartOfDay())/3600000;
    // Size
    int h = m_sizeImg[BetweenHours]*(crse->getTime_end().msecsSinceStartOfDay() - crse->getTime_start().msecsSinceStartOfDay())/3600000;
    int w = (doubleCourse) ? m_sizeImg[BetweenDays] : m_sizeImg[BetweenDays]/2;

    // Building
    QRect* rect = new QRect(x, y, w, h);
    return rect;
}

bool TimeTable::display_scene() {
    /** TO DISPLAY THE TIMETABLE **/
    // Reset the timetable
    clear();

    // Display an empty timetable
    QPixmap emptyTable(":/images/emptyTimeTable.png");
    addPixmap(emptyTable);

    /// Display the courses (thanks to the rects)
    for(int i=0; i<m_displayedCourses->length(); i++) {
        // Get the rect and display
        QRect rect = *(m_displayedCourses->at(i)->slot);
        addRect(rect, QPen(Qt::black, 0), QBrush(Qt::gray));
        // If the rect is big enough , display the subject
        if(rect.height() >= 22) {
            QString nameSubject = m_subjects[m_displayedCourses->at(i)->course->getId_subjects()]->getShortName();
            // Truncate the name of the subject if necessary
            while(QFontMetrics(QFont()).width(nameSubject) > m_sizeImg[BetweenDays]/2 - 10)
                nameSubject = nameSubject.left(nameSubject.length()-1);
            // Display the name of the subject
            QGraphicsTextItem *text = addText(nameSubject);
            text->setPos(rect.x() + 3, rect.y() + 3);
        }
    }

    /// Show the selected course
    if(m_selectedCourse)
        addRect(*(m_selectedCourse->slot), QPen(Qt::yellow, 3));

    /// Update the user inputs
    // Hour input
    if(m_qtimeedit_start) {
        // Status (enabled or not)
        m_qtimeedit_start->setEnabled(m_selectedCourse);
        // Value
        if(m_selectedCourse)
                m_qtimeedit_start->setTime(m_selectedCourse->course->getTime_start());
        else    m_qtimeedit_start->setTime(QTime(0,0));
    }
    // Duration input
    if(m_qtimeedit_duration) {
        // Status (enabled or not)
        m_qtimeedit_duration->setEnabled(m_selectedCourse);
        // Value
        if(m_selectedCourse)
                m_qtimeedit_duration->setTime(QTime(0,0).addMSecs(m_selectedCourse->course->getTime_end().msecsSinceStartOfDay() - m_selectedCourse->course->getTime_start().msecsSinceStartOfDay()));
        else    m_qtimeedit_duration->setTime(QTime(0,0));
    }
    // Subject input
    if(m_comboBox_subjects) {
        // Status (enabled or not)
        m_comboBox_subjects->setEnabled(m_selectedCourse);
        // Select the subject of the selected course
        for(int i=0; i<m_comboBox_subjects->count() && m_selectedCourse; i++)
            if(m_selectedCourse->course->getId_subjects() == ((Subject*) m_comboBox_subjects->itemData(i).toULongLong())->getId())
                m_comboBox_subjects->setCurrentIndex(i);
    }

    return true;
}

void TimeTable::setCurrent_group(Group* currentGroup) {
    /** TO SET THE SELECTED GROUP **/
    m_currentGroup = currentGroup;
    // Display  (update) the new timetable
    update_scene();
}

void TimeTable::mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent) {
    /** TO GET THE POSITION OF THE MOUSE WHEN A MOUSE BUTTON IS PRESSED **/

    QPoint pos = mouseEvent->scenePos().toPoint();
    xDown = pos.x();
    yDown = pos.y();
}

void TimeTable::mouseReleaseEvent(QGraphicsSceneMouseEvent *mouseEvent) {
    /** TO SELECT OR CREATE A COURSE WHEN A MOUSE BUTTON IS RELEASED **/

    // Position of the mouse when a button is released
    QPoint pos = mouseEvent->scenePos().toPoint();

    // Determinate if it is a selection or a creation
    bool creating = false;
    if(abs(pos.x()-xDown) > 5 || abs(pos.y()-yDown) > 5)
        creating = true;

    // Position of the selected field
    QPoint tl; // tl = Top Left of the Field
    tl.setX(fmin(xDown, pos.x()));
    tl.setY(fmin(yDown, pos.y()));
    QPoint dr; // dr = Down Right of the Fild
    dr.setX(fmax(xDown, pos.x()));
    dr.setY(fmax(yDown, pos.y()));

    // Get the information of the selected (or creating) course
    int idDays = ((tl.x() + dr.x())/2-m_sizeImg[BeginDays]) / m_sizeImg[BetweenDays] + 1; // ID of the day
    int leftDay = m_sizeImg[BeginDays]+(idDays-1)*m_sizeImg[BetweenDays]; // Left limit of the day
    int rightDay = m_sizeImg[BeginDays]+idDays*m_sizeImg[BetweenDays]; // Right limit of the day
    int middleDay = (leftDay + rightDay) / 2; // Middle of the day
    // Restrict the field on the selected day
    if(tl.x() < leftDay)
        tl.setX(leftDay);
    if(dr.x() > rightDay)
        dr.setX(rightDay);
    // Restrict the field on the possible hour (8:00 -> 20:00)
    if(tl.y() < m_sizeImg[BeginHours])
        tl.setY(m_sizeImg[BeginHours]);
    if(dr.y() > m_sizeImg[BeginHours] + 12*m_sizeImg[BetweenHours])
        dr.setY(m_sizeImg[BeginHours] + 12*m_sizeImg[BetweenHours]);

    if(creating) {
        /// Creation of the course
        // Build the course
        Course crse;
        crse.setId_day(idDays);
        crse.setId_groups(m_currentGroup->getId());
        if(tl.x() < middleDay)
                crse.setId_week((dr.x() > middleDay) ? 0 : 1); // Id_week = 0 : double week
        else    crse.setId_week(2);

        // Calculate the hour (start and end)
        QTime startHour(8,0);
        int secPlus = (tl.y() - m_sizeImg[BeginHours])*3600/m_sizeImg[BetweenHours];
        secPlus = round(secPlus / (15. * 60.)) * 15 * 60; // round all 15 minutes
        startHour = startHour.addSecs(secPlus);
        crse.setTime_start(startHour);
        QTime endHour(8, 0);
        secPlus = (dr.y() - m_sizeImg[BeginHours])*3600/m_sizeImg[BetweenHours];
        secPlus = round(secPlus / (15. * 60.)) * 15 * 60; // round all 15 minutes
        endHour = endHour.addSecs(secPlus);
        if(startHour.msecsTo(endHour) < 30*60*1000) // If the duration of the course is too small
            endHour = startHour.addMSecs(30*60*1000);
        crse.setTime_end(endHour);

        if(m_comboBox_subjects && m_comboBox_subjects->count() > 0)
                crse.setId_subjects( ((Subject*) m_comboBox_subjects->itemData(0).toULongLong())->getId() );
        else { // If there is no subject in the list
            QMessageBox::critical(NULL, "Aucune matière", "Impossible de créer le cours car il n'y a aucune matière.");
            return;
        }

        // Save the course in the Sql Database (and select the created course)
        int idSelec = -1;
        if(crse.getId_week() == 0) { // Double course
            crse.setId_week(1);
            createCourse(&crse);
            crse.setId_week(2);
            idSelec = createCourse(&crse);
        } else // Simple course
            idSelec = createCourse(&crse);

        // Display (update) the timetable
        update_scene(idSelec);
    } else {
        m_selectedCourse = NULL;
        // Search a course on the position of the mouse
        for(int i=0; i<m_displayedCourses->length(); i++)
            if(m_displayedCourses->at(i)->slot->contains(xDown, yDown))
                m_selectedCourse = m_displayedCourses->at(i);
        // Display (a frame) on the selected course
        display_scene();
    }

}

int TimeTable::createCourse(Course* crse) {
    /** TO INSERT A COURSE IN THE SQL DATABASE **/
    /**     RETURN THE ID OF THE INSERTED COURSE **/

    // Make the query to insert the course
    QSqlQuery query(*m_db);
    query.prepare("INSERT INTO tau_courses(id_subjects, time_start, time_end, id_groups, id_day, id_week) VALUES(:id_subjects, :time_start, :time_end, :id_groups, :id_day, :id_week)");
    query.bindValue(":id_subjects", crse->getId_subjects());
    query.bindValue(":time_start", crse->getTime_start().toString("hh:mm:ss"));
    query.bindValue(":time_end", crse->getTime_end().toString("hh:mm:ss"));
    query.bindValue(":id_groups", crse->getId_groups());
    query.bindValue(":id_day", crse->getId_day());
    query.bindValue(":id_week", crse->getId_week());
    query.exec();

    // Return the ID of the inserted course
    return query.lastInsertId().toInt();
}

bool TimeTable::deleteSelectedCourse() {
    /** TO DELETE A COURSE IN THE SQL DATABASE **/

    if(m_selectedCourse) {
        // Make the query to delete the course
        QSqlQuery query(*m_db);
        query.prepare("DELETE FROM tau_courses WHERE id=:id");
        query.bindValue(":id", m_selectedCourse->course->getId());
        query.exec();

        // Make the query to delete the course in the other week if it is a double course
        if(m_selectedCourse->otherWeek) {
            query.prepare("DELETE FROM tau_courses WHERE id=:id");
            query.bindValue(":id", m_selectedCourse->otherWeek->course->getId());
            query.exec();
        }

        // Display (update) the timetable
        update_scene();
    }

    return true;
}

bool TimeTable::setInputs(QTimeEdit* qtimeedit_start, QTimeEdit* qtimeedit_duration, QComboBox *comboBox_subjects) {
    /** TO SET THE USER INPUTS TO MODIFY A COURSE **/

    // Save the user inputs
    m_qtimeedit_start = qtimeedit_start;
    m_qtimeedit_duration = qtimeedit_duration;
    m_comboBox_subjects = comboBox_subjects;

    // Make the connections between wigdet to detect a change
    if(m_qtimeedit_start)
            connect(m_qtimeedit_start, SIGNAL(editingFinished()), this, SLOT(courseModified()));
    else    disconnect(m_qtimeedit_start, SIGNAL(editingFinished()), this, SLOT(courseModified()));

    if(m_qtimeedit_duration)
            connect(m_qtimeedit_duration, SIGNAL(editingFinished()), this, SLOT(courseModified()));
    else    disconnect(m_qtimeedit_duration, SIGNAL(editingFinished()), this, SLOT(courseModified()));

    if(m_comboBox_subjects)
            connect(m_comboBox_subjects, SIGNAL(currentIndexChanged(int)), this, SLOT(courseModified()));
    else    disconnect(m_comboBox_subjects, SIGNAL(currentIndexChanged(int)), this, SLOT(courseModified()));

    // To display the timetable (and to fill the user inputs)
    return display_scene();
}

bool TimeTable::courseModified() {
    /** TO CHANGE THE SELECTED COURSE WHEN A USER INPUT IS MODIFIED **/

    if(m_selectedCourse) {
        // Update the object (in local)
        m_selectedCourse->course->setTime_start(m_qtimeedit_start->time());
        m_selectedCourse->course->setTime_end(m_qtimeedit_start->time().addMSecs(m_qtimeedit_duration->time().msecsSinceStartOfDay()));
        m_selectedCourse->course->setId_subjects(((Subject*) m_comboBox_subjects->currentData().toULongLong())->getId());
        m_selectedCourse->slot = calculateRect(m_selectedCourse->course, m_selectedCourse->otherWeek);

        // Update the Sql database
        QSqlQuery query(*m_db);
        query.prepare("UPDATE tau_courses SET time_start = :time_start, time_end = :time_end, id_subjects = :id_subjects WHERE id=:id");
        query.bindValue(":id", m_selectedCourse->course->getId());
        query.bindValue(":time_start", m_selectedCourse->course->getTime_start().toString("hh:mm:ss"));
        query.bindValue(":time_end", m_selectedCourse->course->getTime_end().toString("hh:mm:ss"));
        query.bindValue(":id_subjects", m_selectedCourse->course->getId_subjects());
        query.exec();

        if(m_selectedCourse->otherWeek) { // If the selected course is double -> the course of the other week is modified too
            // Update the object (in local)
            m_selectedCourse->otherWeek->course->setTime_start(m_qtimeedit_start->time());
            m_selectedCourse->otherWeek->course->setTime_end(m_qtimeedit_start->time().addMSecs(m_qtimeedit_duration->time().msecsSinceStartOfDay()));
            m_selectedCourse->otherWeek->course->setId_subjects(((Subject*) m_comboBox_subjects->currentData().toULongLong())->getId());
            m_selectedCourse->otherWeek->slot = calculateRect(m_selectedCourse->otherWeek->course, false);

            // Update the Sql database
            query.prepare("UPDATE tau_courses SET time_start = :time_start, time_end = :time_end, id_subjects = :id_subjects WHERE id=:id");
            query.bindValue(":id", m_selectedCourse->otherWeek->course->getId());
            query.bindValue(":time_start", m_selectedCourse->otherWeek->course->getTime_start().toString("hh:mm:ss"));
            query.bindValue(":time_end", m_selectedCourse->otherWeek->course->getTime_end().toString("hh:mm:ss"));
            query.bindValue(":id_subjects", m_selectedCourse->course->getId_subjects());
            query.exec();
        }

        // Display the timetable (no need to update because the local object is modified)
        display_scene();
    }

    return true;
}
