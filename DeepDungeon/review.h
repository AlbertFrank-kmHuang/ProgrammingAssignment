#ifndef REVIEW_H
#define REVIEW_H

#include <QWidget>
#include <QMap>
#include <QResizeEvent>
#include <vector>

class Enemy; // 前向声明
class prepare;

namespace Ui { class review; }

class review : public QWidget {
    Q_OBJECT

public:
    explicit review(prepare *parentPrepare = nullptr, QWidget *parent = nullptr);
    ~review();

    // 核心接口：更新情报
    void updateIntel(int currentFloor);

signals:
    void backToPrepare();

protected:
    void resizeEvent(QResizeEvent* event) override;

private slots:
    void on_btn_back_clicked();

private:
    Ui::review *ui;
    prepare *mainPrepare;
    void paintEvent(QPaintEvent *event) override;

    QMap<QWidget*, QRect> m_originalGeometries;
    static constexpr int DESIGN_W = 660;
    static constexpr int DESIGN_H = 560;
};

#endif // REVIEW_H