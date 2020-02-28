#include "notepad.h"
#include "ui_notepad.h"
#include <QMessageBox>

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
        pad->activateWindow();
        return pad;
    } else {
        NotepadDialog* pad = new NotepadDialog(text, label);
        pad->show();
        pad->raise();
        pad->activateWindow();
        m_notepads->insert(label, pad);
        return pad;
    }
    return NULL;
}

void Notepad::add() {
    add(QString::number(QDateTime::currentMSecsSinceEpoch()), "");
}

bool Notepad::remove(QString label) {
    if(m_notepads->contains(label)) {
        delete m_notepads->value(label);
        m_notepads->remove(label);
        return true;
    }
    return false;
}

NotepadDialog::NotepadDialog(QString text, QString label, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::NotepadDialog)
{
    ui->setupUi(this);
    ui->textEdit->setPlainText(text);
    m_label = label;

    connect(this, SIGNAL(finished(int)), this, SLOT(close()));

    m_shortcutNotepad = Notepad::shortcut();
    this->addAction(m_shortcutNotepad);
}

NotepadDialog::~NotepadDialog() {
    delete ui;
    this->removeAction(m_shortcutNotepad);
    delete m_shortcutNotepad;
}

void NotepadDialog::addTextAtTheEnd(QString text) {
    ui->textEdit->setPlainText(ui->textEdit->toPlainText() + "\n\n" + text);
}

void NotepadDialog::setText(QString text) {
    ui->textEdit->setPlainText(text);
}

void NotepadDialog::setLabel(QString label) {
    m_label = label;
}

QString NotepadDialog::text() const {
    return ui->textEdit->toPlainText();
}

QString NotepadDialog::label() const {
    return m_label;
}

void NotepadDialog::close() {
    if(m_label != "")
        Notepad::remove(m_label);
}

QAction* Notepad::shortcut() {
    QAction* act = new QAction("Bloc-notes");
    act->setShortcut(QKeySequence("Ctrl+B"));
    connect(act, &QAction::triggered, [=] () {
        add(QString::number(QDateTime::currentMSecsSinceEpoch()), "");
    });
    return act;
}
