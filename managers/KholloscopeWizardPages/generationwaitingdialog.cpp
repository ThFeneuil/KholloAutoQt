#include "generationwaitingdialog.h"
#include "ui_generationwaitingdialog.h"

GenerationWaitingDialog::GenerationWaitingDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::GenerationWaitingDialog)
{
    ui->setupUi(this);
}

GenerationWaitingDialog::~GenerationWaitingDialog()
{
    delete ui;
}

void GenerationWaitingDialog::addLogEvent(QString text) {
    QString newLog = text + ui->textEdit_logArea->toPlainText();
    ui->textEdit_logArea->setPlainText(newLog);
}

void GenerationWaitingDialog::clear() {
     ui->textEdit_logArea->clear();
}
