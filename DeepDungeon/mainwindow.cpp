#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "prepare.h"
#include "codex.h"
#include "tutorial.h"
#include "soundmanager.h"
#include <QStyle>
#include <QVBoxLayout>
#include <QPixmap>
#include <QFile>
#include <QDir>
#include <QPushButton>
#include <QStandardPaths>

QString buttonStyle =
    "QPushButton {"
    "   background-color: qlineargradient(x1:0,y1:0,x2:0,y2:1,"
    "       stop:0 rgba(45,36,24,210), stop:1 rgba(21,15,7,210));"
    "   color: #d4aa5a;"
    "   border: 2px solid #6b4e1e;"
    "   border-radius: 10px;"
    "   font-size: 16px;"
    "   font-weight: bold;"
    "   padding: 10px 0px;"
    "   letter-spacing: 2px;"
    "}"
    "QPushButton:hover {"
    "   background-color: qlineargradient(x1:0,y1:0,x2:0,y2:1,"
    "       stop:0 rgba(61,48,32,220), stop:1 rgba(39,29,16,220));"
    "   border-color: #c9a227;"
    "   color: #f5d87a;"
    "}"
    "QPushButton:pressed {"
    "   background-color: rgba(10,7,5,220);"
    "   border-color: #7a5a20;"
    "   color: #b8943f;"
    "}";

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);
    this->showFullScreen();

    // 获取 UI 中的控件指针
    QLabel *titleLabel = ui->label;
    QPushButton *newGameBtn = ui->newgamebutton;
    QPushButton *continueBtn = ui->continuebutton;
    QPushButton *exitBtn = ui->exitbutton;

    // 新增：图鉴与新手引导按钮
    QPushButton *codexBtn = new QPushButton("图鉴", this);
    QPushButton *tutorialBtn = new QPushButton("新手引导", this);

    // 创建垂直布局
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addStretch();              // 顶部弹性空间
    layout->addWidget(titleLabel, 0, Qt::AlignCenter);  // 标题居中
    layout->addSpacing(30);            // 标题与按钮之间的固定间距
    layout->addWidget(newGameBtn, 0, Qt::AlignCenter);
    layout->addWidget(continueBtn, 0, Qt::AlignCenter);
    layout->addWidget(tutorialBtn, 0, Qt::AlignCenter);
    layout->addWidget(codexBtn, 0, Qt::AlignCenter);
    layout->addWidget(exitBtn, 0, Qt::AlignCenter);
    layout->addStretch();              // 底部弹性空间

    // 为按钮设置固定宽度
    int btnWidth = 200;
    newGameBtn->setFixedWidth(btnWidth);
    continueBtn->setFixedWidth(btnWidth);
    tutorialBtn->setFixedWidth(btnWidth);
    codexBtn->setFixedWidth(btnWidth);
    exitBtn->setFixedWidth(btnWidth);

    newGameBtn->setStyleSheet(buttonStyle);
    continueBtn->setStyleSheet(buttonStyle);
    tutorialBtn->setStyleSheet(buttonStyle);
    codexBtn->setStyleSheet(buttonStyle);
    exitBtn->setStyleSheet(buttonStyle);

    connect(codexBtn, &QPushButton::clicked, this, &Widget::openCodex);
    connect(tutorialBtn, &QPushButton::clicked, this, &Widget::openTutorial);

}

void Widget::openCodex()
{
    SoundManager::instance().play("click");
    Codex dlg(this);
    dlg.exec();
}

void Widget::openTutorial()
{
    SoundManager::instance().play("click");
    Tutorial dlg(this);
    dlg.exec();
}

void Widget::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    QPixmap background(":/images/backgrounds/main_background.png");
    if (!background.isNull()) {
        // 获取当前窗口的设备像素比（Retina 屏幕通常为 2.0）
        qreal dpr = this->devicePixelRatioF();
        // 目标物理尺寸 = 窗口逻辑尺寸 × 像素比
        QSize targetSize = size() * dpr;
        // 将原图缩放到物理尺寸
        QPixmap scaled = background.scaled(targetSize, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
        // 设置 Pixmap 的像素比，使其与窗口匹配
        scaled.setDevicePixelRatio(dpr);
        // 绘制到窗口（rect() 是逻辑矩形，但 pixmap 的高分辨率会自动适配）
        painter.drawPixmap(rect(), scaled);
    } else {
        painter.fillRect(rect(), Qt::darkGray);
    }
}

Widget::~Widget()
{

    if (currentGame) {
        delete currentGame;
        currentGame = nullptr;
    }

    delete ui;
}

void Widget::on_newgamebutton_clicked()
{
    // 首次开始新游戏时自动弹出新手引导（用标记文件判断）
    QString flagDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    if (flagDir.isEmpty()) flagDir = QCoreApplication::applicationDirPath();
    QDir flagDirObj(flagDir);
    if (!flagDirObj.exists()) flagDirObj.mkpath(".");
    QString tutorialFlag = flagDirObj.absoluteFilePath(".tutorial_shown");
    if (!QFile::exists(tutorialFlag)) {
        Tutorial t(this);
        t.exec();
        QFile f(tutorialFlag);
        if (f.open(QIODevice::WriteOnly)) f.close();
    }

    if (currentGame) {
        delete currentGame;
        currentGame = nullptr;
    }

    // 创建新游戏
    currentGame = new prepare();

    // 检查prepare对象是否有效
    if (!currentGame) {
        return;
    }

    currentGame->showFullScreen();

    // 连接返回主界面的信号
    connect(currentGame, &prepare::backToMain, this, [this]() {
        if (!currentGame) {
            this->show();
            return;
        }

        // 1. 立即保存状态到局部变量
        bool wasDefeated = currentGame->isDefeated();
        QString savePath = getSaveFilePath();

        // 2. 立即清理指针，避免后续访问
        prepare* gameToDelete = currentGame;
        currentGame = nullptr;  // 立即置空
        // 3. 根据之前保存的状态决定是否保存
        if (gameToDelete && !wasDefeated) {
            bool saveResult = gameToDelete->saveToFile(savePath);
        }

        // 4. 安全删除
        if (gameToDelete) {
            gameToDelete->deleteLater();
        }

        // 5. 显示主界面
        this->showFullScreen();
    });

    this->hide();

}

void Widget::on_continuebutton_clicked()
{

    if (checkSaveExists()) {

        // 如果有存档，加载并继续游戏
        if (currentGame) {
            delete currentGame;
            currentGame = nullptr;
        }
        currentGame = new prepare();

        if (!currentGame) {
            return;
        }

        connect(currentGame, &prepare::backToMain, this, [this]() {

            // 当prepare界面返回时，保存游戏
            if (currentGame && !currentGame->isDefeated()) {
                currentGame->saveToFile(getSaveFilePath());
            } else {
            }

            if (currentGame) {
                currentGame->deleteLater();
            }

            currentGame = nullptr;
            this->showFullScreen();
        });


        // 加载存档
        if (currentGame->loadFromFile(getSaveFilePath())) {
            currentGame->show();
            this->hide();
        } else {
            QMessageBox::warning(this, "加载失败", "存档文件已损坏，无法加载！");

            if (currentGame) {
                delete currentGame;
                currentGame = nullptr;
            }
        }
    } else {
        showNoSaveDialog();
    }

}

bool Widget::checkSaveExists()
{
    QString savePath = getSaveFilePath();
    bool exists = QFile::exists(savePath);
    return exists;
}

QString Widget::getSaveFilePath()
{
    QString appDataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    if (appDataPath.isEmpty()) {
        appDataPath = QCoreApplication::applicationDirPath();
    }
    QDir dir(appDataPath);
    if (!dir.exists()) {
        dir.mkpath(".");
    }
    return dir.absoluteFilePath("autosave.json");
}

void Widget::showNoSaveDialog()
{
    QMessageBox msgBox(this);
    msgBox.setWindowTitle("无存档");
    msgBox.setText("没有找到可用的游戏存档。");
    msgBox.setInformativeText("请开始新游戏或确认存档文件是否存在。");
    msgBox.setIcon(QMessageBox::Information);
    msgBox.addButton("确定", QMessageBox::AcceptRole);
    msgBox.exec();
}

void Widget::on_exitbutton_clicked()
{

    // 退出前保存当前游戏
    if (currentGame && !currentGame->isDefeated()) {
        currentGame->saveToFile(getSaveFilePath());
    }

    QApplication::quit();
}

void Widget::closeEvent(QCloseEvent *event)
{

    // 窗口关闭时保存当前游戏
    if (currentGame && !currentGame->isDefeated()) {
        currentGame->saveToFile(getSaveFilePath());
    }
    event->accept();
}