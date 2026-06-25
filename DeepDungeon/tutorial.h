#ifndef TUTORIAL_H
#define TUTORIAL_H

#include <QDialog>
#include <QString>
#include <vector>

class QLabel;
class QTextBrowser;
class QPushButton;

// 新手引导：多步骤翻页讲解
class Tutorial : public QDialog {
    Q_OBJECT
public:
    explicit Tutorial(QWidget* parent = nullptr);

private:
    struct Step {
        QString title;
        QString body;       // 支持简单 HTML
        QString image;      // 资源路径，可为空
    };

    void showStep(int index);
    void paintEvent(QPaintEvent* event) override;

    std::vector<Step> m_steps;
    int m_index = 0;

    QLabel* m_title = nullptr;
    QLabel* m_image = nullptr;
    QTextBrowser* m_body = nullptr;
    QLabel* m_progress = nullptr;
    QPushButton* m_prev = nullptr;
    QPushButton* m_next = nullptr;
};

#endif // TUTORIAL_H
