#ifndef STUDENT_H
#define STUDENT_H

#include <QString>

class Student
{
    public:
        Student();
        ~Student();

        //Getters
        int getId() const;
        QString getName() const;
        QString getFirst_name() const;
        QString getEmail() const;

        //Setters
        void setId(int id);
        void setName(QString name);
        void setFirst_name(QString first_name);
        void setEmail(QString email);

    private:
        int m_id;
        QString m_name;
        QString m_first_name;
        QString m_email;
};


#endif // STUDENT_H
