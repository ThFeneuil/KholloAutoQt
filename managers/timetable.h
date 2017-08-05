#ifndef TIMETABLE_H
#define TIMETABLE_H

#include <cmath>
#include <QtSql>
#include <QTimeEdit>
#include <QMessageBox>
#include <QFontMetrics>
#include <QGraphicsScene>
#include <QGraphicsTextItem>
#include <QGraphicsSceneMouseEvent>
#include "storedData/course.h"
#include "storedData/group.h"

// Sructure which associate a rect (in the timetable) for a course
struct CourseSlot
{
    Course* course;
    QRect* slot;
    CourseSlot* otherWeek;
};

class TimeTable : public QGraphicsScene
{
    Q_OBJECT
    enum DataImg { BeginDays, BeginHours, BetweenDays, BetweenHours };

public:
    TimeTable(QSqlDatabase* db, Group* currentGroup, QMap<int, Subject*> subjects);
    // db : Sql Database
    // currentGroup : Selected group in the manager
    // subjects : map associating the id of subjects with the object
    ~TimeTable();
    bool update_scene(int idSelec = -1); // To free memory with displayed courses
    bool display_scene(); // To display the timetable
    QRect *calculateRect(Course* crse, bool doubleCourse); // To build the rect which will represent a course on the timetable
    bool freeDisplayedCourses(); // To free memory with displayed courses

    bool setInputs(QTimeEdit* qtimeedit_start, QTimeEdit* qtimeedit_duration, QComboBox* comboBox_subjects); // To set the user inputs to modify a course
    void setCurrent_group(Group* currentGroup); // To set the selected group

    int createCourse(Course* crse); // To insert a course in the SQL database, return the ID of the inserted course
    bool deleteSelectedCourse(); // To delete a course in the SQL database

public slots:
    bool courseModified(); // To change the selected course when a user input is modified

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent); // To get the position of the mouse when a mouse button is pressed
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *mouseEvent); // To select or create a course when a mouse button is released

private:
    QSqlDatabase* m_db; // Sql database
    Group* m_currentGroup; // The selected group in the manager
    /// General information to run the script
    QMap<DataImg, int> m_sizeImg; // Parameters of the khollotable design
    QMap<int, Subject*> m_subjects; // map associating the id of subjects with the object
    /// Data about course
    QList<CourseSlot*>* m_displayedCourses; // List of the displayed course in the timetable
    CourseSlot* m_selectedCourse; // Selected course in the timetable
    /// Position of the mouse when a mouse button is pressed
    int xDown;
    int yDown;
    /// User inputs
    QTimeEdit* m_qtimeedit_start;
    QTimeEdit* m_qtimeedit_duration;
    QComboBox* m_comboBox_subjects;
};

#endif // TIMETABLE_H
