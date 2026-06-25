#include "benchlabel.h"
#include "prepare.h"
#include <QApplication>
#include <QPainter>

BenchLabel::BenchLabel(QWidget *parent)
    : QLabel(parent), m_index(-1)
{
    setAcceptDrops(true);
}

BenchLabel::BenchLabel(int index, QWidget *parent)
    : QLabel(parent), m_index(index)
{
    setAcceptDrops(true);
}

void BenchLabel::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_pressed = true;
        m_pressPos = event->pos();
    }
    QLabel::mousePressEvent(event);
}

void BenchLabel::mouseMoveEvent(QMouseEvent *event)
{
    if (m_pressed && (event->pos() - m_pressPos).manhattanLength() > QApplication::startDragDistance()) {
        m_pressed = false;
        startDrag();
    }
    QLabel::mouseMoveEvent(event);
}

void BenchLabel::mouseReleaseEvent(QMouseEvent *event)
{
    if (m_pressed && event->button() == Qt::LeftButton) {
        m_pressed = false;
        prepare *p = qobject_cast<prepare*>(parentWidget());
        if (p) {
            p->showBenchItemDetail(m_index);
        }
        return;
    }
    QLabel::mouseReleaseEvent(event);
}

void BenchLabel::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasFormat("application/x-benchindex")) {
        event->setDropAction(Qt::MoveAction);
        event->accept();
    } else {
        event->ignore();
    }
}

void BenchLabel::dropEvent(QDropEvent *event)
{
    if (event->mimeData()->hasFormat("application/x-benchindex")) {
        QByteArray data = event->mimeData()->data("application/x-benchindex");
        QDataStream stream(&data, QIODevice::ReadOnly);
        int fromIndex;
        stream >> fromIndex;

        prepare *p = qobject_cast<prepare*>(parentWidget());
        if (p) {
            p->swapBenchItems(fromIndex, m_index);
        }

        event->setDropAction(Qt::MoveAction);
        event->accept();
    } else {
        event->ignore();
    }
}

void BenchLabel::startDrag()
{
    prepare *p = qobject_cast<prepare*>(parentWidget());
    if (!p) return;

    const BenchItem *item = p->getBenchItem(m_index);
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
    stream << QString("bench") << m_index;
    mimeData->setData("application/x-benchindex", data);

    drag->setMimeData(mimeData);
    drag->setPixmap(dragPix);
    drag->setHotSpot(QPoint(40, 40));
    drag->exec(Qt::MoveAction);
}