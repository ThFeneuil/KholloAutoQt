#ifndef Kholle_H
#define Kholle_H

#include <QtSql>
#include "storedData/student.h"
#include "storedData/timeslot.h"

class Student;
class Timeslot;

class Kholle
{

public:
    Kholle();
    ~Kholle();

    enum Status {OK, Warning, Error}; //Generation

    //Getters
    int getId() const;
    int getId_students() const;
    int getId_timeslots() const;
    Student* student() const; // Interface
    Timeslot* timeslot() const; // Interface
    int status() const; //Generation
    int weeks() const; //Generation

    //Setters
    void setId(int id);
    void setId_students(int id_students);
    void setId_timeslots(int id_timeslots);
    void setStudent(Student* stud); // Interface
    void setTimeslot(Timeslot* slot); // Interface
    void setStatus(Status status); //Generation
    void setWeeks(int weeks); //Generation

    //Other functions
    static int nearestKholle(QSqlDatabase *db, QMap<int, Timeslot *> *timeslots, int id_user, Timeslot* t, int id_kholle);
    int nearest(QMap<int, Timeslot *> *timeslots, QSqlDatabase *db);
    void updateStatus(QMap<int, Timeslot *> *timeslots, QSqlDatabase *db);

private:
    int m_id;
    int m_id_students;
    int m_id_timeslots;

    // Interface
    Student* m_student;
    Timeslot* m_timeslot;

    //Generation
    Status m_status;
    int m_weeks;
};

#endif // Kholle_H
