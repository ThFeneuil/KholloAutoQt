#ifndef TEACHER_H
#define TEACHER_H

#include <QString>

class Teacher
{
    public:
        Teacher();
        ~Teacher();

        //Getters
        int getId() const;
        QString getName() const;
        int getId_subjects() const;

        //Setters
        void setId(int id);
        void setName(QString name);
        void setId_subjects(int id_subjects);

    private:
        int m_id;
        QString m_name;
        int m_id_subjects;
};

#endif // TEACHER_H
