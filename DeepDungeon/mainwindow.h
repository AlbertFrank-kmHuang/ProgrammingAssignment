#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QMessageBox>

QT_BEGIN_NAMESPACE
namespace Ui {
class Widget;
}
QT_END_NAMESPACE

class prepare;  // 前向声明

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = nullptr);
    ~Widget() override;

private slots:
    void on_newgamebutton_clicked();
    void on_continuebutton_clicked();
    void on_exitbutton_clicked();
    void openCodex();
    void openTutorial();

private:
    Ui::Widget *ui;
    prepare* currentGame = nullptr;  // 当前游戏实例指针

    // 存档相关功能
    bool checkSaveExists();  // 检查存档是否存在
    QString getSaveFilePath();  // 获取存档文件路径
    void showNoSaveDialog();  // 显示无存档对话框

protected:
    void paintEvent(QPaintEvent *event) override;
    void closeEvent(QCloseEvent *event) override;  // 添加关闭事件处理
};
#endif // MAINWINDOW_H
