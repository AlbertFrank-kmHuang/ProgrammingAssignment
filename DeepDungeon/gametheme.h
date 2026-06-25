#ifndef GAMETHEME_H
#define GAMETHEME_H

#include <QString>

// 全局地牢主题样式表。在 main.cpp 中通过 qApp->setStyleSheet(GameTheme::stylesheet()) 应用。
namespace GameTheme {

inline QString stylesheet() {
    return QStringLiteral(R"(
/* ═══════════════════════════════════════════════
   深渊地牢主题：焦黑石壁 × 暗金符文 × 血红火炬
   ═══════════════════════════════════════════════ */

QWidget {
    color: #e8dcc0;
    font-family: "Microsoft YaHei", "SimHei", "Segoe UI", sans-serif;
    font-size: 13px;
}

QDialog, QMainWindow {
    background-color: #0e0b07;
}

/* ═══ 按钮：刻纹石板 + 暗金描边 ═══ */
QPushButton {
    background-color: qlineargradient(x1:0, y1:0, x2:0, y2:1,
        stop:0 #2d2418, stop:0.55 #1e180e, stop:1 #150f07);
    color: #d4aa5a;
    border: 2px solid #6b4e1e;
    border-radius: 6px;
    padding: 7px 16px;
    font-weight: bold;
}
QPushButton:hover {
    background-color: qlineargradient(x1:0, y1:0, x2:0, y2:1,
        stop:0 #3d3020, stop:1 #271d10);
    border-color: #c9a227;
    color: #f5d87a;
}
QPushButton:pressed {
    background-color: #0a0705;
    border-color: #7a5a20;
    color: #b8943f;
}
QPushButton:disabled {
    background-color: #181310;
    color: #4a3f2a;
    border-color: #2e261a;
}

/* ═══ 进度条：魔力纹章 ═══ */
QProgressBar {
    border: 1px solid #3d2e18;
    border-radius: 5px;
    background-color: #080604;
    text-align: center;
    color: #e8dcc0;
    font-weight: bold;
    font-size: 11px;
    min-height: 16px;
}
QProgressBar::chunk {
    border-radius: 4px;
    background-color: qlineargradient(x1:0, y1:0, x2:1, y2:0,
        stop:0 #5aaf4a, stop:1 #3a8f2a);
}

/* ═══ 文字区：泛黄羊皮纸 ═══ */
QTextBrowser, QTextEdit, QPlainTextEdit {
    background-color: rgba(8, 6, 4, 230);
    color: #c8bb9a;
    border: 2px solid #4a3820;
    border-radius: 5px;
    padding: 8px;
    selection-background-color: #5a3e10;
}

/* ═══ 列表：地牢清单 ═══ */
QListWidget, QTreeWidget, QTableWidget {
    background-color: rgba(12, 9, 5, 240);
    color: #d8cba8;
    border: 2px solid #4a3820;
    border-radius: 5px;
    outline: none;
}
QListWidget::item {
    padding: 7px 10px;
    border-bottom: 1px solid #1c1508;
}
QListWidget::item:selected {
    background-color: #5a3e10;
    color: #f0d878;
    border-left: 3px solid #c9a227;
    padding-left: 7px;
}
QListWidget::item:hover:!selected {
    background-color: #271e0f;
}

/* ═══ 标签页：古卷书签 ═══ */
QTabWidget::pane {
    border: 2px solid #4a3820;
    border-radius: 5px;
    background-color: rgba(12, 9, 5, 220);
}
QTabBar::tab {
    background-color: #1c1508;
    color: #a89060;
    border: 1px solid #3d2e18;
    border-bottom: none;
    border-top-left-radius: 5px;
    border-top-right-radius: 5px;
    padding: 9px 22px;
    margin-right: 3px;
    font-weight: bold;
}
QTabBar::tab:selected {
    background-color: #3a2c15;
    color: #f0d878;
    border-color: #6b4e1e;
}
QTabBar::tab:hover:!selected {
    background-color: #251c0c;
}

/* ═══ 滚动条：锈铁滑道 ═══ */
QScrollBar:vertical {
    background: #080604;
    width: 10px;
    margin: 0;
    border: 1px solid #1c1508;
    border-radius: 5px;
}
QScrollBar::handle:vertical {
    background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
        stop:0 #4a3820, stop:1 #6a5028);
    border-radius: 5px;
    min-height: 22px;
}
QScrollBar::handle:vertical:hover { background: #8a6d3b; }
QScrollBar::add-line:vertical,
QScrollBar::sub-line:vertical { height: 0; }
QScrollBar:horizontal {
    background: #080604;
    height: 10px;
    margin: 0;
    border: 1px solid #1c1508;
    border-radius: 5px;
}
QScrollBar::handle:horizontal {
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
        stop:0 #4a3820, stop:1 #6a5028);
    border-radius: 5px;
    min-width: 22px;
}
QScrollBar::handle:horizontal:hover { background: #8a6d3b; }
QScrollBar::add-line:horizontal,
QScrollBar::sub-line:horizontal { width: 0; }

/* ═══ 提示气泡：古符文印记 ═══ */
QToolTip {
    background-color: #1c1508;
    color: #e8d090;
    border: 1px solid #c9a227;
    border-radius: 4px;
    padding: 6px 8px;
    font-size: 12px;
}

/* ═══ 消息框：石碑警示 ═══ */
QMessageBox {
    background-color: #0e0b07;
}
QMessageBox QLabel {
    color: #d8cba8;
    font-size: 13px;
    min-width: 260px;
    min-height: 20px;
}
QMessageBox QPushButton {
    min-width: 90px;
    min-height: 32px;
    margin: 4px;
}

/* ═══ 输入框 ═══ */
QLineEdit {
    background-color: rgba(8, 6, 4, 220);
    color: #c8bb9a;
    border: 2px solid #4a3820;
    border-radius: 4px;
    padding: 4px 8px;
    selection-background-color: #5a3e10;
}
QLineEdit:focus {
    border-color: #8a6d3b;
}
QLineEdit:read-only {
    color: #9a8d6e;
    background-color: rgba(6, 4, 2, 200);
}
)");
}

} // namespace GameTheme

#endif // GAMETHEME_H
