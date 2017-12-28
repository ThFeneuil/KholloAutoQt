#ifndef FIVEWAVESMETHOD_H
#define FIVEWAVESMETHOD_H

#include "generationmethod.h"

#define TIMEOUT_INT 30000 //in msec
#define MAX_ITERATION 100

enum ExchangeType {Collisions, Warnings, All};

class FiveWavesMethod : public GenerationMethod
{
public:
    FiveWavesMethod(QSqlDatabase *db, QDate date, int week);
    virtual bool start(QList<Subject*> *selected_subjects, QMap<int, QList<Student*> > *input);

private:
    bool exchange(int index, ExchangeType type, int score_limit);
};

#endif // FIVEWAVESMETHOD_H
