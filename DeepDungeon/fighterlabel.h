#ifndef FIGHTERLABEL_H
#define FIGHTERLABEL_H

#include <QLabel>
#include <QDrag>
#include <QMimeData>
#include <QMouseEvent>
#include <QPainter>

class FighterLabel : public QLabel
{
    Q_OBJECT

public:
    explicit FighterLabel(QWidget *parent = nullptr);
    explicit FighterLabel(int index, QWidget *parent = nullptr);
    void setIndex(int idx) { m_index = idx; }
    int getIndex() const { return m_index; }

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    void startDrag();

    int m_index = -1;
    QPoint m_pressPos;
    bool m_pressed = false;
};

#endif // FIGHTERLABEL_H