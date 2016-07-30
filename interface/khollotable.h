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
#include <QScrollArea>
#include <QVBoxLayout>
#include <QGraphicsRectItem>
#include <QLabel>
#include "storedData/kholleur.h"
#include "storedData/student.h"
#include "storedData/timeslot.h"
#include "storedData/course.h"

class KholloTable : public QGraphicsScene
{
    Q_OBJECT
    enum DataImg { BeginDays, BeginHours, BetweenDays, BetweenHours };

public:
    KholloTable(QSqlDatabase* db, int id_week, QDate monday, QWidget* areaKholles);
    ~KholloTable();
    bool compatible(Student* stdnt, Timeslot *timeslot);

public slots:
    void displayKholleur(Kholleur* kll);
    void displayStudent(Student* stud);
    void displayKholleurAndStudent(Kholleur* kll, Student* stud);
    bool add_selection();
    bool remove_selection();

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent);

private:
    QSqlDatabase* m_db;
    QMap<DataImg, int> m_sizeImg;
    Kholleur* m_kholleur;
    Student* m_student;
    int m_id_week;
    QDate m_monday;
    QList<Timeslot*> m_timeslots;
    Timeslot *m_selectedTimeslot;
    QGraphicsRectItem* m_selectedFrame;
    QWidget* m_areaKholles;
};

#endif // KHOLLOTABLE_H
