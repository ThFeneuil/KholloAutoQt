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

private:
    Ui::IntroInterface *ui;
    int* m_id_week;
    QDate* m_monday;
};

#endif // INTROINTERFACE_H
