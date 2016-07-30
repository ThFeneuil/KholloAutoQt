#ifndef Kholle_H
#define Kholle_H

class Kholle
{
public:
    Kholle();
    ~Kholle();

    //Getters
    int getId() const;
    int getId_students() const;
    int getId_timeslots() const;

    //Setters
    void setId(int id);
    void setId_students(int id_students);
    void setId_timeslots(int id_timeslots);

private:
    int m_id;
    int m_id_students;
    int m_id_timeslots;
};

#endif // Kholle_H
