#include "aboutitdialog.h"
#include "ui_aboutitdialog.h"

AboutItDialog::AboutItDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AboutItDialog)
{
    ui->setupUi(this);
}

AboutItDialog::~AboutItDialog()
{
    delete ui;
}
