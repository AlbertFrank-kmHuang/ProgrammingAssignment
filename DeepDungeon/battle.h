#ifndef BATTLE_H
#define BATTLE_H

#include <QWidget>
#include <QTimer>
#include <QLabel>
#include <QMouseEvent>
#include <QCursor>
#include "battle_engine.h"
#include "clickablelabel.h"
#include <QMessageBox>

namespace Ui { class battle; }
class battle : public QWidget {
    Q_OBJECT

public:
    explicit battle(BattleEngine* engine = nullptr, QWidget *parent = nullptr);
    ~battle();
    void setEngine(BattleEngine* newEngine);
    // 新增buff处理函数
    void processPreActionBuffs(Character* attacker, Character* defender, bool isPlayer);
    void processPostActionBuffs(Character* attacker, Character* defender, bool isPlayer);
    void processRoundEndBuffs(Character* character, bool isPlayer);
    template<typename T>
    void showBuffDetails(T* character, const QString& title);
    QString getBuffDescription(const buff& b);
    void updateEnemyUI(Character* enemy);
    void checkHpChanges(Character* player, Character* enemy);
    void updatePlayerUI(Character* player);
    void updateSkillButtons(Character* player);
    int showTargetSelectionDialog(bool allowSelf);

signals:
    void battleFinished(bool isPlayerWin);

private slots:
    void onBattleEnded(bool playerWon);
    void onUiNeedsUpdate();
    void onStateChanged(BattleState newState);

    // 明确对应 3 个技能的按钮槽函数
    void attack_clicked();   // 普攻
    void skill1_clicked();   // 特技 1
    void skill2_clicked();   // 特技 2
    void change_clicked();
    void onPlayerStatusClicked();
    void onEnemyStatusClicked();
    void onLogAdded(const QString& log);

private:
    Ui::battle *ui;
    BattleEngine *m_engine;
    // 添加更新UI的函数
    void refreshUI();
    // 用于检测血量/护盾变化并触发特效
    int m_prevPlayerHP = -1;
    int m_prevEnemyHP = -1;
    void* m_prevPlayerPtr = nullptr;  // 前排身份，换人时重置基线，避免误判
    void* m_prevEnemyPtr = nullptr;
    bool m_resultPlayed = false;

    // 根据本帧新日志与血量变化播放特效/音效
    void playCombatFeedback(const std::vector<QString>& newLogs,
                            int playerDelta, int enemyDelta);

    // 扫描日志识别技能释放，按元素染色闪屏
    void triggerSkillEffects(const std::vector<QString>& newLogs);

    // 用布局重排界面，使其随窗口缩放
    void setupResponsiveLayout();

    void paintEvent(QPaintEvent *event) override;
};

// 通用函数显示buff详情
template<typename T>
void battle::showBuffDetails(T* character, const QString& title) {
    if (!character) return;

    auto buffs = character->getbuffs();
    QString details = QString("【%1】的状态详情：\n\n").arg(QString::fromStdString(character->getName()));

    if (buffs.empty()) {
        details += "当前没有任何状态效果。";
    } else {
        details += "当前生效的状态效果：\n";
        details += "────────────────────\n";

        for (const auto& buff : buffs) {
            details += QString("• %1\n").arg(getBuffDescription(buff));
            details += QString("  剩余回合：%1\n").arg(buff.bufftime);
            if (buff.buffvalue != 0) {
                details += QString("  效果值：%1\n").arg(buff.buffvalue);
            }
            details += "\n";
        }

        // 添加状态效果总结
        details += "────────────────────\n";
        details += QString("总计 %1 个状态效果").arg(buffs.size());
    }

    QMessageBox msgBox(this);
    msgBox.setWindowTitle(title);
    msgBox.setText(details);
    msgBox.setIcon(QMessageBox::Information);
    msgBox.setStandardButtons(QMessageBox::Ok);
    msgBox.exec();
}

#endif // BATTLE_H
