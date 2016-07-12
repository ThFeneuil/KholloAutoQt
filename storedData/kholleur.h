#ifndef KHOLLEUR_H
#define KHOLLEUR_H

#include <QString>

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

        //Setters
        void setId(int id);
        void setName(QString name);
        void setId_subjects(int id_subjects);
        void setDuration(int duration);
        void setPreparation(int preparation);
        void setPupils(int pupils);

    private:
        int m_id;
        QString m_name;
        int m_id_subjects;
        int m_duration;
        int m_preparation;
        int m_pupils;
};

#endif // KHOLLEUR_H
