#ifndef LPMETHOD_H
#define LPMETHOD_H

#include "generationmethod.h"
#include "GLPK/glpk.h"

class LPMethod : public GenerationMethod
{
public:
    LPMethod(QSqlDatabase *db, QDate date, int week);
    virtual bool start(QList<Subject*> *selected_subjects, QMap<int, QList<Student*> > *input);

private:
    bool generate(QList<Subject*> *selected_subjects, QMap<int, QList<Student*> > *input);
    void set_constraint_row(glp_prob *P, int i, QVector<int> vect);
};

int lpMethodsaveLog(void *info, const char *s);

#endif // LPMETHOD_H
