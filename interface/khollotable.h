/*
 *	File:			(Header) khollotable.h
 *  Comment:        INTERFACE FILE
 *	Description:    Class of a QGraphicsScene which manage the kholles of the selected week
 */

#ifndef KHOLLOTABLE_H
#define KHOLLOTABLE_H

#include <QGraphicsScene>
#include <QMessageBox>
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
    KholloTable(QSqlDatabase* db, DataBase* dbase,  int id_week, QDate monday, QWidget* areaKholles,InterfaceDialog* interface = NULL, InterfaceTab* tab = NULL);
    // db, dbase : databases (SQL, local)
    // id_week, monday : Properties of the selected week (parity, first day)
    // interface, tab, areaKholles : Parent widgets (interface, tab, info area)
    ~KholloTable();
    bool compatible(Student* stdnt, Timeslot *timeslot); // To determine if a student and a timeslot is compatible

public slots:
    void displayKholleur(Kholleur* kll); // To display a kholleur in the khollotable
    void displayStudent(Student* stud); // To display a student in the khollotable
    void displayTable(); // To display the khollotable according the selected student and the selected kholleur
    bool removeSelection(); // To remove the selection on timeslots
    bool selection(QGraphicsSceneMouseEvent *mouseEvent); // To select a timeslot or a kholle with the mouse
    bool updateInfoArea(); // To update info area (info about the selected timeslot)
    bool addKholle(); // To add a kholle
    bool removeKholle(Student *stud = NULL); // To remove a kholle
    bool removeKholleFromInfoArea(); // To remove a kholle from the info area
    bool selectStudentInInterface(); // To select a student from the info area (students list)
    bool updateListKholleurs(); // To update the kholleurs list of the selected tab
    bool selectionTimeslot(Timeslot* ts);
    bool openReviewWithSelection();

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent); // To detect when user clicks with the mouse (knowing button)
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *mouseEvent); // To detect when user double-clicks with the mouse (knowing button)

private:
    /// Databases
    QSqlDatabase* m_db; // SQL database
    DataBase* m_dbase; // Local database
    /// Properties of the selected week
    int m_id_week;
    QDate m_monday;
    /// Parameters of the khollotable design
    QMap<DataImg, int> m_sizeImg;
    /// Pointors of selection
    Kholleur* m_kholleur; // Selected kholleur
    Student* m_student; // Selected student
    Timeslot *m_selectedTimeslot; // Selected timeslot
    /// Parent widget
    InterfaceDialog* m_interface; // Interface
    InterfaceTab* m_tab; // Tab
    QWidget* m_areaKholles; // Info area of the tab
};

#endif // KHOLLOTABLE_H
