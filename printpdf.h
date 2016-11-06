#ifndef PRINTPDF_H
#define PRINTPDF_H

#include <QList>
#include <QMap>
#include <QFontMetrics>
#include <QFile>
#include <QTextStream>
#include <QDir>
#include <QFileDialog>
#include <QStandardPaths>
#include <QPdfWriter>
#include <QPainter>
#include <QMessageBox>
#include <QCoreApplication>
#include "storedData/student.h"
#include "storedData/kholleur.h"
#include "storedData/timeslot.h"
#include "storedData/kholle.h"
#include "settingsdialog.h"

class PrintPDF
{
public:
    PrintPDF();
    static void printKholles(QList<Student *> *students, QMap<int, Kholleur *> *kholleurs, QMap<int, Timeslot *> *timeslots, QDate monday_date, QMap<int, Kholle *> *kholloscope);
    static bool printKholles_StudentsDays(QList<Student *> *students, QMap<int, Kholleur *> *kholleurs, QMap<int, Timeslot *> *timeslots, QDate monday_date, QMap<int, Kholle *> *kholloscope, QString filename);
    static bool printKholles_StudentsSubjects(QList<Student *> *students, QMap<int, Kholleur *> *kholleurs, QMap<int, Timeslot *> *timeslots, QDate monday_date, QMap<int, Kholle *> *kholloscope, QString filename);

private:
    static int heightStudent(int numStudent, QList<QList<QList<Kholle*>*>*>* grid);
    static bool writeTitle(QPainter* painter, int width, int maxHeight, QDate monday, double ratio = 4./5);
    static double averageWidthStudents(QFontMetrics font, QList<Student *> *students);
    static QString displayStudent(Student* s, int maxWidth, QFont font);
};

#endif // PRINTPDF_Hs
