#include "effects.h"

#include <QWidget>
#include <QLabel>
#include <QPropertyAnimation>
#include <QSequentialAnimationGroup>
#include <QParallelAnimationGroup>
#include <QGraphicsOpacityEffect>
#include <QFont>
#include <QPoint>

namespace fx {

void floatingText(QWidget* parent, const QPoint& pos, const QString& text,
                  const QColor& color, int fontSize) {
    if (!parent) return;

    QLabel* label = new QLabel(text, parent);
    QFont f = label->font();
    f.setPointSize(fontSize);
    f.setBold(true);
    label->setFont(f);
    label->setStyleSheet(QString(
        "color: %1; background: transparent;").arg(color.name()));
    label->setAttribute(Qt::WA_TransparentForMouseEvents);
    label->adjustSize();
    // 以 pos 为中心
    label->move(pos.x() - label->width() / 2, pos.y());
    label->show();
    label->raise();

    QGraphicsOpacityEffect* opacity = new QGraphicsOpacityEffect(label);
    label->setGraphicsEffect(opacity);

    QPropertyAnimation* moveAnim = new QPropertyAnimation(label, "pos");
    moveAnim->setDuration(750);
    moveAnim->setStartValue(label->pos());
    moveAnim->setEndValue(QPoint(label->x(), label->y() - 45));
    moveAnim->setEasingCurve(QEasingCurve::OutCubic);

    QPropertyAnimation* fadeAnim = new QPropertyAnimation(opacity, "opacity");
    fadeAnim->setDuration(750);
    fadeAnim->setStartValue(1.0);
    fadeAnim->setKeyValueAt(0.5, 1.0);
    fadeAnim->setEndValue(0.0);

    QParallelAnimationGroup* group = new QParallelAnimationGroup(label);
    group->addAnimation(moveAnim);
    group->addAnimation(fadeAnim);
    QObject::connect(group, &QParallelAnimationGroup::finished, label, &QObject::deleteLater);
    group->start(QAbstractAnimation::DeleteWhenStopped);
}

void shake(QWidget* target, int intensity, int durationMs) {
    if (!target) return;

    const QPoint base = target->pos();
    QPropertyAnimation* anim = new QPropertyAnimation(target, "pos", target);
    anim->setDuration(durationMs);
    anim->setKeyValueAt(0.0, base);
    anim->setKeyValueAt(0.15, base + QPoint(intensity, 0));
    anim->setKeyValueAt(0.30, base - QPoint(intensity, 0));
    anim->setKeyValueAt(0.45, base + QPoint(intensity - 2, 0));
    anim->setKeyValueAt(0.60, base - QPoint(intensity - 3, 0));
    anim->setKeyValueAt(0.80, base + QPoint(2, 0));
    anim->setKeyValueAt(1.0, base);
    // 动画结束确保回到原位
    QObject::connect(anim, &QPropertyAnimation::finished, target, [target, base]() {
        target->move(base);
    });
    anim->start(QAbstractAnimation::DeleteWhenStopped);
}

void flash(QWidget* target, const QColor& color, int durationMs) {
    if (!target || !target->parentWidget()) return;

    QWidget* parent = target->parentWidget();
    QLabel* overlay = new QLabel(parent);
    overlay->setGeometry(target->geometry());
    QColor c = color;
    overlay->setStyleSheet(QString("background-color: rgba(%1,%2,%3,140); border-radius: 4px;")
                               .arg(c.red()).arg(c.green()).arg(c.blue()));
    overlay->setAttribute(Qt::WA_TransparentForMouseEvents);
    overlay->show();
    overlay->raise();

    QGraphicsOpacityEffect* opacity = new QGraphicsOpacityEffect(overlay);
    overlay->setGraphicsEffect(opacity);

    QPropertyAnimation* fade = new QPropertyAnimation(opacity, "opacity", overlay);
    fade->setDuration(durationMs);
    fade->setStartValue(1.0);
    fade->setEndValue(0.0);
    QObject::connect(fade, &QPropertyAnimation::finished, overlay, &QObject::deleteLater);
    fade->start(QAbstractAnimation::DeleteWhenStopped);
}

void screenFlash(QWidget* host, const QColor& color, int durationMs) {
    if (!host) return;

    QLabel* overlay = new QLabel(host);
    overlay->setGeometry(host->rect());
    QColor c = color;
    overlay->setStyleSheet(QString("background-color: rgba(%1,%2,%3,90);")
                               .arg(c.red()).arg(c.green()).arg(c.blue()));
    overlay->setAttribute(Qt::WA_TransparentForMouseEvents);
    overlay->show();
    overlay->raise();

    QGraphicsOpacityEffect* opacity = new QGraphicsOpacityEffect(overlay);
    overlay->setGraphicsEffect(opacity);

    QPropertyAnimation* fade = new QPropertyAnimation(opacity, "opacity", overlay);
    fade->setDuration(durationMs);
    fade->setStartValue(0.85);
    fade->setEndValue(0.0);
    QObject::connect(fade, &QPropertyAnimation::finished, overlay, &QObject::deleteLater);
    fade->start(QAbstractAnimation::DeleteWhenStopped);
}

} // namespace fx
