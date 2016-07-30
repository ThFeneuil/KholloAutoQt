#ifndef Kholle_H
#define Kholle_H

#include "storedData/student.h"
#include "storedData/timeslot.h"

class Student;
class Timeslot;

class Kholle
{
public:
    Kholle();
    ~Kholle();

    //Getters
    int getId() const;
    int getId_students() const;
    int getId_timeslots() const;
    Student* student() const; // Interface
    Timeslot* timeslot() const; // Interface

    //Setters
    void setId(int id);
    void setId_students(int id_students);
    void setId_timeslots(int id_timeslots);
    void setStudent(Student* stud); // Interface
    void setTimeslot(Timeslot* slot); // Interface

private:
    int m_id;
    int m_id_students;
    int m_id_timeslots;

    // Interface
    Student* m_student;
    Timeslot* m_timeslot;
};

#endif // Kholle_H
