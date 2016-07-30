#ifndef KHOLLEUR_H
#define KHOLLEUR_H

#include <QString>
#include <QList>
#include "storedData/subject.h"
#include "storedData/timeslot.h"

class Subject;
class Timeslot;

class Kholleur
{
    public:
        Kholleur();
        ~Kholleur();

        //Getters
        int getId() const;
        QString getName() const;
        int getId_subjects() const;
        int getDuration() const;
        int getPreparation() const;
        int getPupils() const;
        Subject* subject() const; // Interface
        QList<Timeslot*>* timeslots() const; // Interface

        //Setters
        void setId(int id);
        void setName(QString name);
        void setId_subjects(int id_subjects);
        void setDuration(int duration);
        void setPreparation(int preparation);
        void setPupils(int pupils);
        void setSubject(Subject* subj);

    private:
        int m_id;
        QString m_name;
        int m_id_subjects;
        int m_duration;
        int m_preparation;
        int m_pupils;

        // Interface
        Subject* m_subject;
        QList<Timeslot*>* m_timeslots;
};

#endif // KHOLLEUR_H
