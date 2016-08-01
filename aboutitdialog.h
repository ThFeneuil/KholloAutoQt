#ifndef ABOUTITDIALOG_H
#define ABOUTITDIALOG_H

#include <QDialog>

namespace Ui {
class AboutItDialog;
}

class AboutItDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AboutItDialog(QWidget *parent = 0);
    ~AboutItDialog();

private:
    Ui::AboutItDialog *ui;
};

#endif // ABOUTITDIALOG_H
