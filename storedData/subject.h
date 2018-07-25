#ifndef SUBJECT_H
#define SUBJECT_H

#include <QString>
#include <QList>
#include "storedData/storeddata.h"
#include "storedData/kholleur.h"
#include "storedData/course.h"

#define MaxWeightSubject 10

class Kholleur;
class Course;

class Subject : public StoredData
{
    public:
        Subject();
        ~Subject();

        //Getters
        QString getName() const;
        QString getShortName() const;
        QString getColor() const;
        int getWeight() const;
        QList<Kholleur*>* kholleurs() const;
        QList<Course*>* courses() const;

        //Setters
        void setName(QString name);
        void setShortName(QString shortName);
        void setColor(QString color);
        void setWeight(int weight);

    private:
        QString m_name;
        QString m_shortName;
        QString m_color;
        int m_weight;

        //Interface
        QList<Kholleur*>* m_kholleurs;
        QList<Course*>* m_courses;
};

#endif // SUBJECT_H
