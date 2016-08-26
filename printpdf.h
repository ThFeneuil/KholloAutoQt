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

class PrintPDF
{
public:
    PrintPDF();
    static void printKholles(QList<Student *> *students, QMap<int, Kholleur *> *kholleurs, QMap<int, Timeslot *> *timeslots, QDate monday_date, QMap<int, Kholle *> *kholloscope);

private:
    static int longestUser(QFontMetrics font, QList<Student *> *students);
    static int longestKholleur(QFontMetrics font, QMap<int, Kholleur*> *kholleurs);
};

#endif // PRINTPDF_H
