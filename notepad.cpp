#include "notepad.h"
#include "ui_notepad.h"

QMap<QString, NotepadDialog*>* Notepad::m_notepads = new QMap<QString, NotepadDialog*>();

Notepad::Notepad() {

}

Notepad::~Notepad() {

}

NotepadDialog* Notepad::add(QString label, QString text) {
    if(m_notepads->contains(label)) {
        NotepadDialog* pad = m_notepads->value(label);
        pad->addTextAtTheEnd(text);
        pad->show();
        pad->raise();
        pad->setFocus();
        pad->activateWindow();
        return pad;
    } else {
        NotepadDialog* pad = new NotepadDialog(text);
        pad->show();
        pad->raise();
        pad->activateWindow();
        m_notepads->insert(label, pad);
        return pad;
    }
    return NULL;
}

NotepadDialog::NotepadDialog(QString text, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::NotepadDialog)
{
    ui->setupUi(this);
    ui->textEdit->setPlainText(text);
}

NotepadDialog::~NotepadDialog()
{
    delete ui;
}

void NotepadDialog::addTextAtTheEnd(QString text) {
    ui->textEdit->setPlainText(ui->textEdit->toPlainText() + "\n\n" + text);
}

void NotepadDialog::setText(QString text) {
    ui->textEdit->setPlainText(text);
}

QString NotepadDialog::text() const {
    return ui->textEdit->toPlainText();
}
