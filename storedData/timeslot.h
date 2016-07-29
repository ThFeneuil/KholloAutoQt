#ifndef TIMESLOT_H
#define TIMESLOT_H

#include <QTime>

class Timeslot
{
public:
    Timeslot();
    ~Timeslot();

    //Getters
    int getId() const;
    QTime getTime_start() const;
    QTime getTime() const;
    QTime getTime_end() const;
    int getId_kholleurs() const;
    int getId_day() const;
    int getPupils() const;

    //Setters
    void setId(int id);
    void setTime_start(QTime time_start);
    void setTime(QTime time);
    void setTime_end(QTime time_end);
    void setId_kholleurs(int id_kholleurs);
    void setId_day(int id_day);
    void setPupils(int pupils);

private:
    int m_id;
    QTime m_time_start;
    QTime m_time;
    QTime m_time_end;
    int m_id_kholleurs;
    int m_id_day;
    int m_pupils;
};

#endif // TIMESLOT_H
