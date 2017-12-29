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
public:
    FiveWavesMethod(QSqlDatabase *db, QDate date, int week);
    virtual void start(QList<Subject*> *selected_subjects, QMap<int, QList<Student*> > *input);

private:
    working_index* findMax();
    bool exchange(int index, ExchangeType type, int score_limit);
    bool generate();

private:
    QMap<int, QMap<int, QList<Timeslot*> > > m_poss;
};

#endif // FIVEWAVESMETHOD_H
