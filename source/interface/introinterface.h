/*
 *	File:			(Header) introinterface.h
 *  Comment:        INTERFACE FILE
 *	Description:    Class of the dialog which asks necessary information for the interface execution
 *      The information is returned thanks to 'id_week' and 'monday' pointors
 */

#ifndef INTROINTERFACE_H
#define INTROINTERFACE_H

#include <QDialog>
#include <QMessageBox>

namespace Ui {
class IntroInterface;
}

class IntroInterface : public QDialog
{
    Q_OBJECT

public:
    explicit IntroInterface(int* id_week, QDate* monday, QWidget *parent = 0);
    ~IntroInterface();

public slots:
    void save(); // To save information and update the pointers (id_week, monday)

private:
    Ui::IntroInterface *ui; // GUI
    int* m_id_week; // Parity of the week (peer: 1, odd: 2)
    QDate* m_monday; // Monday of the selected week
};

#endif // INTROINTERFACE_H
