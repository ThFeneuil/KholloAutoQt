#ifndef EVENT_H
#define EVENT_H

#include <QString>
#include <QTime>

class Event
{
public:
    Event();
    ~Event();

    //Getters
    int getId() const;
    QString getName() const;
    QString getComment() const;
    QDateTime getStart() const;
    QDateTime getEnd() const;

    //Setters
    void setId(int id);
    void setName(QString name);
    void setComment(QString comment);
    void setStart(QDateTime start);
    void setEnd(QDateTime end);

private:
    int m_id;
    QString m_name;
    QString m_comment;
    QDateTime m_start;
    QDateTime m_end;
};

#endif // EVENT_H
