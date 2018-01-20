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
        void setName(QVariant name);
        void setShortName(QString shortName);
        void setShortName(QVariant shortName);
        void setColor(QString color);
        void setColor(QVariant color);
        void setWeight(int weight);
        void setWeight(QVariant weight);

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
