#ifndef GENERATIONWAITINGDIALOG_H
#define GENERATIONWAITINGDIALOG_H

#include <QDialog>

namespace Ui {
class GenerationWaitingDialog;
}

class GenerationWaitingDialog : public QDialog
{
    Q_OBJECT

public:
    explicit GenerationWaitingDialog(QWidget *parent = 0);
    ~GenerationWaitingDialog();
    void clear();

public slots:
    void addLogEvent(QString text);
    void reject();

signals:
    void cancelled();

private:
    Ui::GenerationWaitingDialog *ui;
};

#endif // GENERATIONWAITINGDIALOG_H
