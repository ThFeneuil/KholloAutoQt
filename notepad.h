#ifndef NOTEPAD_H
#define NOTEPAD_H

#include <QDialog>
#include <QMap>

namespace Ui {
class NotepadDialog;
}

class NotepadDialog : public QDialog
{
    Q_OBJECT

public:
    explicit NotepadDialog(QString text = "", QWidget *parent = 0);
    ~NotepadDialog();
    void addTextAtTheEnd(QString text);
    void setText(QString text);
    QString text() const;

private:
    Ui::NotepadDialog *ui;
};

class Notepad {
public:
    Notepad();
    ~Notepad();

    static NotepadDialog* add(QString label, QString text);

private:
    static QMap<QString, NotepadDialog*>* m_notepads;
};


#endif // NOTEPAD_H
