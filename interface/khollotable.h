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
#include <QSqlRecord>
#include <QPushButton>
#include <QListWidget>
#include "storedData/kholleur.h"
#include "storedData/student.h"
#include "storedData/timeslot.h"
#include "storedData/course.h"
#include "database.h"
#include "interface/interfacedialog.h"
#include "interface/interfacetab.h"

class InterfaceDialog;
class InterfaceTab;

class KholloTable : public QGraphicsScene
{
    Q_OBJECT
    enum DataImg { BeginDays, BeginHours, BetweenDays, BetweenHours };

public:
    KholloTable(QSqlDatabase* db, int id_week, QDate monday, QWidget* areaKholles, DataBase* dbase, InterfaceDialog* interface = NULL, InterfaceTab* tab = NULL);
    ~KholloTable();
    bool compatible(Student* stdnt, Timeslot *timeslot);

public slots:
    void displayKholleur(Kholleur* kll);
    void displayStudent(Student* stud);
    void displayTable();
    bool removeSelection();
    bool selection(QGraphicsSceneMouseEvent *mouseEvent);
    bool updateInfoArea();
    bool addKholle();
    bool removeKholle(Student *stud = NULL);
    bool removeKholleFromInfoArea();
    bool selectStudentInInterface();
    bool updateListKholleurs();

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent);
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *mouseEvent);

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
    DataBase* m_dbase;
    InterfaceDialog* m_interface;
    InterfaceTab* m_tab;
};

#endif // KHOLLOTABLE_H
