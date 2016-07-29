#include "interface/table.h"

Table::Table(QWidget *parent) {
    //QTableWidget(12, 6, parent);
    setParent(parent);
    setRowCount(12);
    setColumnCount(6);
    QStringList days;
    days << "Lundi" << "Mardi" << "Mercredi" << "Jeudi" << "Vendredi" << "Samedi";
    setHorizontalHeaderLabels(days);
    QStringList hours;
    hours << "8:00" << "9:00" << "10:00" << "11:00" << "12:00" << "13:00" << "14:00" << "15:00" << "16:00" << "17:00" << "18:00" << "19:00";
    setVerticalHeaderLabels(hours);
}

Table::~Table() {

}

void Table::paintEvent(QPaintEvent *) {
    QPainter painter(this);
    painter.setPen(Qt::blue);
    painter.setFont(QFont("Arial", 30));
    painter.drawText(rect(), Qt::AlignCenter, "Qt");
}
