#ifndef LPMETHOD_H
#define LPMETHOD_H

#include "generationmethod.h"
#include "GLPK/glpk.h"
#include <csetjmp>

class LPMethod : public GenerationMethod
{
public:
    LPMethod(QSqlDatabase *db, QDate date, int week);
    virtual void start(QList<Subject*> *selected_subjects, QMap<int, QList<Student*> > *input);
    bool abortCompleted();
    void setAbortComplete();

private:
    bool generate(QList<Subject*> *selected_subjects, QMap<int, QList<Student*> > *input);
    void set_constraint_row(glp_prob *P, int i, QVector<int> vect);
    void treatCollision(int index, int score_limit);

private:
    QMap<int, QMap<int, float>> m_probabilities;
    QMap<int, bool> m_downgraded;
    bool m_abortComplete;
};

int lpMethodsaveLog(void *info, const char *s);
void lpMethodErrorHook(void *info);

#endif // LPMETHOD_H
