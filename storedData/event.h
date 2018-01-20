#ifndef EVENT_H
#define EVENT_H

#include <QString>
#include <QTime>
#include <QList>
#include "storedData/storeddata.h"
#include "storedData/group.h"

class Group;

class Event : public StoredData
{
public:
    Event();
    ~Event();

    //Getters
    QString getName() const;
    QString getComment() const;
    QDateTime getStart() const;
    QDateTime getEnd() const;
    QList<Group*>* groups() const; // Interface

    //Setters
    void setName(QString name);
    void setComment(QString comment);
    void setStart(QDateTime start);
    void setEnd(QDateTime end);

private:
    QString m_name;
    QString m_comment;
    QDateTime m_start;
    QDateTime m_end;

    // Interface
    QList<Group*>* m_groups;
};

#endif // EVENT_H
