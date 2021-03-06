#ifndef Kholle_H
#define Kholle_H

#include <QtSql>
#include "storedData/storeddata.h"
#include "storedData/student.h"
#include "storedData/timeslot.h"
#include "generator/utilities.h"
#include "tools/database.h"

class Student;
class Timeslot;
class Utilities;
class DataBase;

struct stat_info {
    int weeks;
    int status;
    int id_pb_kholle;
};

class Kholle : public StoredData
{

public:
    Kholle();
    ~Kholle();

    enum Status {OK, Warning, Error, Incompatible, Impossible}; //Generation, the status are in order from best to worst

    //Getters
    int getId_students() const;
    int getId_timeslots() const;
    Student* student() const; // Interface
    Timeslot* timeslot() const; // Interface
    int status() const; //Generation
    int weeks() const; //Generation
    QList<int>* past_id_timeslots(); // Generation
    int id_pb_kholle() const; //Generation

    //Setters
    void setId_students(int id_students);
    void setId_timeslots(int id_timeslots);
    void setStudent(Student* stud); // Interface
    void setTimeslot(Timeslot* slot); // Interface
    void setStatus(Status status); //Generation
    void setWeeks(int weeks); //Generation
    void setPast_id_timeslots(QList<int> l); //Generation
    void setId_pb_kholle(int id); //Generation

    //Other functions
    static int nearestKholle(QSqlDatabase *db, QMap<int, Timeslot *> *timeslots, int id_user, Timeslot* t, int id_kholle);
    int nearest(QMap<int, Timeslot *> *timeslots, QSqlDatabase *db);
    static int correspondingStatus(int weeks);
    void updateStatus(DataBase *dbase, QSqlDatabase *db, QList<Kholle *> kholloscope, int week);
    static stat_info* calculateStatus(QSqlDatabase *db, DataBase *dbase, int id_user, Timeslot* t, int week, QList<Kholle*> kholloscope, int id_kholle);
    static bool kholloscope_contains(int id, QList<Kholle*> kholloscope);

private:
    int m_id_students;
    int m_id_timeslots;

    // Interface
    Student* m_student;
    Timeslot* m_timeslot;

    //Generation
    Status m_status;
    int m_weeks;
    QList<int> m_past_id_timeslots;
    int m_id_pb_kholle;
};

#endif // Kholle_H
