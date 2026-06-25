#include "mainwindow.h"
#include "gametheme.h"
#include "soundmanager.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // 应用全局地牢主题
    a.setStyleSheet(GameTheme::stylesheet());

    // 提前初始化音效（首次运行会生成 wav 素材）
    SoundManager::instance();

    Widget w;
    w.show();
    return QCoreApplication::exec();
}
