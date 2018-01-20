#ifndef STUDENT_H
#define STUDENT_H

#include <QString>
#include <QList>
#include "storedData/storeddata.h"
#include "storedData/group.h"
#include "storedData/kholle.h"

class Group;
class Kholle;

class Student : public StoredData
{
    public:
        Student();
        ~Student();

        //Getters
        QString getName() const;
        QString getFirst_name() const;
        QString getEmail() const;
        QList<Group*>* groups() const; // Interface
        QList<Kholle*>* kholles() const; // Interface

        //Setters
        void setName(QString name);
        void setName(QVariant name);
        void setFirst_name(QString first_name);
        void setFirst_name(QVariant first_name);
        void setEmail(QString email);
        void setEmail(QVariant email);

    private:
        QString m_name;
        QString m_first_name;
        QString m_email;

        // Interface
        QList<Group*>* m_groups;
        QList<Kholle*>* m_kholles;
};


#endif // STUDENT_H
