#ifndef TIMETABLE_H
#define TIMETABLE_H

#include <QGraphicsScene>
#include <QtSql>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsTextItem>
#include <QDebug>
#include <cmath>
#include <QTimeEdit>
#include "storedData/course.h"
#include "storedData/group.h"

#define max(a,b) \
  ({ __typeof__ (a) _a = (a); \
      __typeof__ (b) _b = (b); \
    _a > _b ? _a : _b; })

#define min(a,b) \
  ({ __typeof__ (a) _a = (a); \
      __typeof__ (b) _b = (b); \
    _a < _b ? _a : _b; })

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
    ~TimeTable();
    bool update_scene(int idSelec = -1);
    bool display_scene();
    bool setInputs(QTimeEdit* qtimeedit_start, QTimeEdit* qtimeedit_duration, QComboBox* comboBox_subjects);
    void setCurrent_group(Group* currentGroup);
    int createCourse(Course* crse);
    bool deleteSelectedCourse();
    bool freeDisplayedCourses();
    QRect *calculateRect(Course* crse, bool doubleCourse);

public slots:
    bool courseModified();

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *mouseEvent);

private:
    QSqlDatabase* m_db;
    Group* m_currentGroup;
    QMap<DataImg, int> m_sizeImg;
    QMap<int, Subject*> m_subjects;

    QList<CourseSlot*>* m_displayedCourses;
    CourseSlot* m_selectedCourse;

    int xDown;
    int yDown;

    QTimeEdit* m_qtimeedit_start;
    QTimeEdit* m_qtimeedit_duration;
    QComboBox* m_comboBox_subjects;
};

#endif // TIMETABLE_H
