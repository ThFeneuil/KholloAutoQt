#ifndef TABLE_H
#define TABLE_H

#include <QTableWidget>
#include <QMessageBox>
#include <QPainter>

class Table : public QTableWidget
{
    Q_OBJECT

public:
    Table(QWidget *parent);
    ~Table();

public slots:

protected:
    void paintEvent(QPaintEvent *) Q_DECL_OVERRIDE;

private:

};

#endif // TABLE_H
