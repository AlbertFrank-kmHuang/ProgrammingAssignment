#ifndef CODEX_H
#define CODEX_H

#include <QDialog>
#include <QString>
#include <vector>

class Character;
class Enemy;
class Weapon;
class QListWidget;
class QLabel;
class QTextBrowser;
class QWidget;

// 图鉴：角色 / 敌人 / 武器 三个分页，左侧列表，右侧立绘 + 详情
class Codex : public QDialog {
    Q_OBJECT
public:
    explicit Codex(QWidget* parent = nullptr);
    ~Codex() override;

private slots:
    void onCharacterRow(int row);
    void onEnemyRow(int row);
    void onWeaponRow(int row);

private:
    QWidget* buildCharacterTab();
    QWidget* buildEnemyTab();
    QWidget* buildWeaponTab();
    void paintEvent(QPaintEvent* event) override;

    // 数据（图鉴持有，析构时统一释放）
    std::vector<Character*> m_characters;
    std::vector<Enemy*> m_enemies;
    std::vector<QString> m_enemyTiers;   // 与 m_enemies 一一对应
    std::vector<Weapon*> m_weapons;

    QListWidget* m_charList = nullptr;
    QLabel* m_charImg = nullptr;
    QTextBrowser* m_charInfo = nullptr;

    QListWidget* m_enemyList = nullptr;
    QLabel* m_enemyImg = nullptr;
    QTextBrowser* m_enemyInfo = nullptr;

    QListWidget* m_weaponList = nullptr;
    QLabel* m_weaponImg = nullptr;
    QTextBrowser* m_weaponInfo = nullptr;
};

#endif // CODEX_H
