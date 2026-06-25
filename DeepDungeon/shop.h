#ifndef SHOP_H
#define SHOP_H

#include <QWidget>
#include <QMap>
#include <QLabel>
#include <QResizeEvent>

#include "character_config.h"
#include "weapon_config.h"
#include "card_config.h"

class prepare;

namespace Ui {
class shop;
}

struct ShopSlot {
    QString name;
    int price;
    QString imagePath;
    QString type;
    bool isSoldOut = false;
};


class shop : public QWidget
{
    Q_OBJECT

public:
    explicit shop(prepare *parentPrepare, QWidget *parent = nullptr);
    ~shop();

    void updateMoneyUI();

    void notEnoughMoney();

    void setLegendaryWeaponMode(bool legendaryMode);  // 新增：设置传说武器模式
    void setLineEditsAndTextEditsReadOnly();  // 新增：设置所有 QLineEdit 和 QTextEdit 为只读

private slots:
    void on_exitshop_clicked();


    void on_new_character_clicked();

    void on_new_weapons_clicked();

    void on_new_card_clicked();

private:
    Ui::shop *ui;
    prepare *mainPrepare;

    void paintEvent(QPaintEvent *event) override;

    QVector<ShopSlot> currentCharacters;
    QVector<ShopSlot> currentWeapons;
    QVector<ShopSlot> currentCards;
    bool legendaryWeaponMode;  // 新增：传说武器模式标志

    void refreshCharacters();
    void refreshWeapons();
    void refreshCards();

    void displayCharacters();
    void displayWeapons();
    void displayCards();


    // 用于事件过滤器：记录每个标签对应的是哪个商品（索引 + 类型）
    QMap<QLabel*, int>     m_imageIndex;   // 图片标签 -> 商品索引
    QMap<QLabel*, QString>  m_imageType;    // 图片标签 -> 商品类型 ("character"/"weapon"/"card")
    QMap<QLabel*, int>     m_nameIndex;    // 名字标签 -> 商品索引
    QMap<QLabel*, QString>  m_nameType;     // 名字标签 -> 商品类型


    void showDetail(const ShopSlot &slot);
    void buyItem(const ShopSlot &slot, int index, const QString &type);

    QMap<QWidget*, QRect> m_originalGeometries;
    static constexpr int DESIGN_W = 634;
    static constexpr int DESIGN_H = 525;

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;
    void resizeEvent(QResizeEvent* event) override;




signals:
    void backToPrepare();
};

#endif
