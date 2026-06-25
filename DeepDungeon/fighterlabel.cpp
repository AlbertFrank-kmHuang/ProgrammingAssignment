#include "fighterlabel.h"
#include "prepare.h"
#include <QApplication>

FighterLabel::FighterLabel(QWidget *parent)
    : QLabel(parent), m_index(-1)
{
    setAcceptDrops(false);
    setScaledContents(true);
    setAlignment(Qt::AlignCenter);
}

FighterLabel::FighterLabel(int index, QWidget *parent)
    : QLabel(parent), m_index(index)
{
    setAcceptDrops(false);
    setScaledContents(true);
    setAlignment(Qt::AlignCenter);
}

void FighterLabel::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_pressed = true;
        m_pressPos = event->pos();
    }
    QLabel::mousePressEvent(event);
}

void FighterLabel::mouseMoveEvent(QMouseEvent *event)
{
    if (m_pressed && (event->pos() - m_pressPos).manhattanLength() > QApplication::startDragDistance()) {
        m_pressed = false;
        startDrag();
    }
    QLabel::mouseMoveEvent(event);
}

void FighterLabel::mouseReleaseEvent(QMouseEvent *event)
{
    if (m_pressed && event->button() == Qt::LeftButton) {
        m_pressed = false;
        prepare *p = qobject_cast<prepare*>(parentWidget());
        if (p) {
            p->showFighterDetail(m_index);
        }
        return;
    }
    QLabel::mouseReleaseEvent(event);
}

void FighterLabel::startDrag()
{
    prepare *p = qobject_cast<prepare*>(parentWidget());
    if (!p) return;

    const BenchItem *item = p->getFighterItem(m_index);
    if (!item) return;

    QPixmap dragPix(80, 80);
    dragPix.fill(Qt::transparent);
    QPainter painter(&dragPix);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);

    QPixmap original(item->imagePath);
    if (!original.isNull()) {
        QPixmap scaled = original.scaled(70, 70, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        painter.drawPixmap((dragPix.width() - scaled.width()) / 2,
                           (dragPix.height() - scaled.height()) / 2,
                           scaled);
    } else {
        painter.setPen(Qt::black);
        painter.setFont(QFont("微软雅黑", 10));
        painter.drawText(dragPix.rect(), Qt::AlignCenter, item->name);
    }
    painter.end();

    QDrag *drag = new QDrag(this);
    QMimeData *mimeData = new QMimeData;
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);
    stream << QString("fighter") << m_index;
    mimeData->setData("application/x-benchindex", data);

    drag->setMimeData(mimeData);
    drag->setPixmap(dragPix);
    drag->setHotSpot(QPoint(40, 40));
    drag->exec(Qt::MoveAction);
}