#ifndef SUBJECT_H
#define SUBJECT_H

#include <QString>

class Subject
{
    public:
        Subject();
        ~Subject();

        //Getters
        int getId() const;
        QString getName() const;
        QString getShortName() const;
        QString getColor() const;

        //Setters
        void setId(int id);
        void setName(QString name);
        void setShortName(QString shortName);
        void setColor(QString color);

    private:
        int m_id;
        QString m_name;
        QString m_shortName;
        QString m_color;
};

#endif // SUBJECT_H
