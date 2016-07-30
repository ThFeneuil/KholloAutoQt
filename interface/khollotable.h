#ifndef KHOLLOTABLE_H
#define KHOLLOTABLE_H

#include <QGraphicsScene>
#include <QMessageBox>
#include <QSql>
#include <QSqlQuery>
#include <QPainter>
#include <QPixmap>
#include <QMap>
#include <QVariant>
#include <QQueue>
#include <QGraphicsSceneMouseEvent>
#include "storedData/kholleur.h"
#include "storedData/student.h"
#include "storedData/timeslot.h"
#include "storedData/course.h"

class KholloTable : public QGraphicsScene
{
    Q_OBJECT
    enum DataImg { BeginDays, BeginHours, BetweenDays, BetweenHours };

public:
    KholloTable(QSqlDatabase* db, int id_week, QDate monday);
    ~KholloTable();
    bool compatible(Student* stdnt, Timeslot *timeslot);

public slots:
    void displayKholleur(Kholleur* kll);
    void displayStudent(Student* stud);
    void displayKholleurAndStudent(Kholleur* kll, Student* stud);

protected:
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *mouseEvent);

private:
    QSqlDatabase* m_db;
    QMap<DataImg, int> sizeImg;
    Kholleur* m_kholleur;
    Student* m_student;
    int m_id_week;
    QDate m_monday;
};

#endif // KHOLLOTABLE_H
