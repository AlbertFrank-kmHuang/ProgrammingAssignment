#ifndef BENCHLABEL_H
#define BENCHLABEL_H

#include <QLabel>
#include <QDrag>
#include <QMimeData>
#include <QMouseEvent>
#include <QPainter>

class BenchLabel : public QLabel
{
    Q_OBJECT

public:
    explicit BenchLabel(QWidget *parent = nullptr);
    explicit BenchLabel(int index, QWidget *parent = nullptr);
    void setIndex(int idx) { m_index = idx; }
    int getIndex() const { return m_index; }

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;

private:
    void startDrag();

    int m_index = -1;
    QPoint m_pressPos;
    bool m_pressed = false;
};

#endif // BENCHLABEL_H