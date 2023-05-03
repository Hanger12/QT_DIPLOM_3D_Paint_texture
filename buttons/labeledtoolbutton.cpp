#include "labeledtoolbutton.h"

#include <QPainter>

LabeledToolButton::LabeledToolButton(QWidget *parent) : QToolButton(parent)
{

}

void LabeledToolButton::paintEvent(QPaintEvent *event)
{
    QToolButton::paintEvent(event);

    drawBackground();

    QPainter p(this);

    QFont textFont = p.font();
    textFont.setPixelSize(12);
    p.setFont(textFont);
    const int textHeight = p.fontMetrics().height();
    QColor textBgColor(200, 200, 200, 240);

//    // draw labels
//    //
//    QString nameText = toolName();
//    if (nameText.length() > 0) {
//        int textWidth = p.fontMetrics().lineWidth()*3;
//        QPoint textP(4, textHeight + 2);
//        p.fillRect(0, 0, textWidth +75, textHeight+4, textBgColor);
//        p.setPen(Qt::black);
//        p.drawText(textP, nameText);
//    }

    QString valueText = toolValue();
    if (valueText.length() > 0) {
        QPoint textP(width()/2-10, height() - 2);
        p.setPen(Qt::black);
        p.drawText(textP, valueText);
    }
}
