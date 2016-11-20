#ifndef NOTEPAD_H
#define NOTEPAD_H

#include <QDialog>
#include <QMap>
#include <QDateTime>

namespace Ui {
class NotepadDialog;
}

class NotepadDialog : public QDialog
{
    Q_OBJECT

public:
    explicit NotepadDialog(QString text = "", QString label = "", QWidget *parent = 0);
    ~NotepadDialog();
    void addTextAtTheEnd(QString text);
    void setText(QString text);
    void setLabel(QString label);
    QString text() const;
    QString label() const;

public slots:
    void close();

private:
    Ui::NotepadDialog *ui;
    QString m_label;
    QAction* m_shortcutNotepad;
};

class Notepad : public QObject
{
    Q_OBJECT

public:
    Notepad();
    ~Notepad();

    static NotepadDialog* add(QString label, QString text);
    static bool remove(QString label);
    static QAction* shortcut();

public slots:
    static void add();

private:
    static QMap<QString, NotepadDialog*>* m_notepads;
};


#endif // NOTEPAD_H
