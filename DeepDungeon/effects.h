#ifndef EFFECTS_H
#define EFFECTS_H

#include <QString>
#include <QColor>
#include <QPoint>

class QWidget;

// 轻量战斗特效：浮动文字（伤害/治疗数字）、控件抖动、闪光
namespace fx {

// 在 parent 上 pos 处冒出一段会上浮并淡出的文字
void floatingText(QWidget* parent, const QPoint& pos, const QString& text,
                  const QColor& color, int fontSize = 20);

// 让 target 控件左右抖动（受击反馈）
void shake(QWidget* target, int intensity = 8, int durationMs = 280);

// 在 target 上覆盖一层半透明颜色并快速淡出（受击/暴击闪光）
void flash(QWidget* target, const QColor& color, int durationMs = 220);

// 在 host 整个区域覆盖一层淡色并淡出（技能释放的元素闪屏）
void screenFlash(QWidget* host, const QColor& color, int durationMs = 320);

} // namespace fx

#endif // EFFECTS_H
