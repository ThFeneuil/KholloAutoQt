#ifndef FIVEWAVESMETHOD_H
#define FIVEWAVESMETHOD_H

#include "generationmethod.h"

#define TIMEOUT_INT 30000 //in msec
#define MAX_ITERATION 100

struct working_index {
    int current_student;
    int current_subject;
    int max;
};

enum ExchangeType {Collisions, Warnings, All};

class FiveWavesMethod : public GenerationMethod
{
    Q_OBJECT

public:
    FiveWavesMethod(QSqlDatabase *db, QDate date, int week);
    void launch(QList<Subject*> *selected_subjects, QMap<int, QList<Student*> > *input);
    virtual void start(QList<Subject*> *selected_subjects, QMap<int, QList<Student*> > *input);

private:
    void constructPoss(QList<Subject*> *selected_subjects, QMap<int, QList<Student*> > *input);
    void clearPoss();

    QMap<int, QList<Timeslot*> > *updatePoss(int id_user, Timeslot *current);
    void resetPoss(int id_user, QMap<int, QList<Timeslot*> > *old);

    working_index* findMax();
    Kholle *create_add_kholle(Student *s, Timeslot* ts);
    bool wave_1();

    void force();
    bool treatImpossible(int index, Kholle::Status stat_correct);
    int remainImpossible(Kholle::Status stat);

    bool exchange(int index, ExchangeType type, int score_limit);

    void displayBlocking();

private slots:
    void timeout();

private:
    QMap<int, QMap<int, QList<Timeslot*> > > m_poss;
    QMap<int, QMap<int, QMap<int, float>* > > m_probabilities;
    working_index last_index;
    QMap<int, bool> m_downgraded;
    bool m_timeout;
};

#endif // FIVEWAVESMETHOD_H
