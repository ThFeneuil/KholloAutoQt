#ifndef TIMESLOT_H
#define TIMESLOT_H

#include <QTime>
#include <QRect>
#include <QList>
#include "storedData/storeddata.h"
#include "storedData/kholle.h"
#include "storedData/kholleur.h"

class Kholleur;
class Kholle;

class Timeslot : public StoredData
{
public:
    Timeslot();
    ~Timeslot();

    //Getters
    QTime getTime_start() const;
    QTime getTime() const;
    QTime getTime_end() const;
    int getId_kholleurs() const;
    QDate getDate() const;
    int getPupils() const;
    QRect* getArea() const;
    QList<Kholle*>* kholles() const;
    Kholleur* kholleur() const;
    bool isDeleted() const;

    //Setters
    void setTime_start(QTime time_start);
    void setTime(QTime time);
    void setTime_end(QTime time_end);
    void setId_kholleurs(int id_kholleurs);
    void setDate(QDate date);
    void setPupils(int pupils);
    void setArea(QRect* area);
    void setKholleur(Kholleur* kll);
    void setIsDeleted(bool is_deleted);

    //Other functions
    int weeksTo(Timeslot* ts);

private:
    QTime m_time_start;
    QTime m_time;
    QTime m_time_end;
    int m_id_kholleurs;
    QDate m_date;
    int m_pupils;

    // UpdateTimeslotDialog
    bool m_is_deleted;

    // Interface
    QRect* m_area;
    QList<Kholle*>* m_kholles;
    Kholleur* m_kholleur;
};

#endif // TIMESLOT_H
