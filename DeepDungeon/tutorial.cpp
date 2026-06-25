#include "tutorial.h"
#include "soundmanager.h"

#include <QLabel>
#include <QTextBrowser>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPainter>
#include <QPixmap>

Tutorial::Tutorial(QWidget* parent) : QDialog(parent) {
    setWindowTitle("新手引导");
    resize(780, 580);

    m_steps = {
        {
            "欢迎来到地牢冒险",
            "<p>这是一款<b>回合制</b>的地牢探险游戏。你将编成一支小队，"
            "层层深入地牢，击败敌人、触发事件、采购装备，挑战最深处的首领。</p>"
            "<p>点击下方<b>下一步</b>开始了解核心玩法。</p>",
            ":/images/backgrounds/main_background.png"
        },
        {
            "第一步：编成你的队伍",
            "<p>在<b>准备界面</b>，把<b>备战席</b>的英雄拖拽到<b>出战位</b>即可上阵。</p>"
            "<ul>"
            "<li>最前面的英雄是<b>前排</b>，负责直接战斗；</li>"
            "<li>其余出战英雄为<b>后排</b>，弓箭手后排可发动<b>协同攻击</b>；</li>"
            "<li>可以为英雄<b>装备武器</b>提升属性。</li>"
            "</ul>",
            ":/images/backgrounds/prepare_background.png"
        },
        {
            "第二步：善用职业羁绊",
            "<p>队伍中相同职业的英雄达到一定数量会触发<b>羁绊加成</b>：</p>"
            "<ul>"
            "<li><b>剑士</b>：提升全队生命与防御；</li>"
            "<li><b>法师</b>：提升全队法力（及生命）；</li>"
            "<li><b>弓箭手</b>：提升全队攻击与闪避；</li>"
            "<li><b>无名之人</b>：提升全队暴击率与暴击伤害。</li>"
            "</ul>"
            "<p>此外，<b>多职业混搭</b>还能触发组合羁绊，例如：四种职业到齐的"
            "「四职同辉」、剑士+法师的「战法相济」、弓箭手+无名的「疾影连射」等。</p>"
            "<p>合理搭配职业，是通关的关键。</p>",
            ":/images/backgrounds/prepare_background.png"
        },
        {
            "第三步：战斗操作",
            "<p>战斗每回合会先进行<b>速度判定</b>决定出手顺序，然后双方依次行动：</p>"
            "<ul>"
            "<li><b>普攻</b>：基础攻击，不消耗法力；</li>"
            "<li><b>技能1 / 技能2</b>：消耗法力，释放强力效果；</li>"
            "<li><b>切换角色</b>：把后排英雄换到前排（部分英雄可“速切”不耗回合）。</li>"
            "</ul>"
            "<p>暴击会打出更高伤害，闪避则可完全免伤。</p>",
            ":/images/backgrounds/battle_background.png"
        },
        {
            "第四步：状态与护盾",
            "<p>战斗中会出现各种<b>状态效果</b>（buff / debuff）：攻击/防御增减、"
            "中毒、麻痹、睡眠、护盾、吸血、无敌等。</p>"
            "<p>点击血条下方的<b>状态栏</b>，可以查看当前生效的状态及其剩余回合。</p>"
            "<p>护盾会优先承受伤害，状态效果会随回合数衰减。</p>",
            ":/images/backgrounds/battle_background.png"
        },
        {
            "第五步：事件与商店",
            "<p>每通过一场战斗，都有机会触发<b>随机事件</b>：宝藏、商人、陷阱或挑战。</p>"
            "<p>你也可以进入<b>商店</b>采购新的英雄、武器与卡牌，"
            "用积累的金币不断壮大队伍，准备迎接更深层的敌人。</p>",
            ":/images/backgrounds/shop_background.png"
        },
        {
            "准备就绪，出发吧！",
            "<p>随时可以在主菜单打开<b>图鉴</b>查看所有英雄、敌人与武器的详细数据。</p>"
            "<p>祝你在地牢深处好运，勇者！</p>",
            ":/images/backgrounds/main_background.png"
        }
    };

    m_title = new QLabel(this);
    QFont tf = m_title->font();
    tf.setPointSize(18);
    tf.setBold(true);
    m_title->setFont(tf);
    m_title->setStyleSheet("color:#ffd700;");
    m_title->setAlignment(Qt::AlignCenter);

    m_image = new QLabel(this);
    m_image->setFixedHeight(220);
    m_image->setAlignment(Qt::AlignCenter);
    m_image->setStyleSheet("border:2px solid #6b5635; border-radius:6px; background-color:rgba(0,0,0,120);");

    m_body = new QTextBrowser(this);

    m_progress = new QLabel(this);
    m_progress->setAlignment(Qt::AlignCenter);

    m_prev = new QPushButton("上一步", this);
    m_next = new QPushButton("下一步", this);
    QPushButton* skip = new QPushButton("跳过", this);

    connect(m_prev, &QPushButton::clicked, this, [this]() {
        SoundManager::instance().play("click");
        if (m_index > 0) showStep(m_index - 1);
    });
    connect(m_next, &QPushButton::clicked, this, [this]() {
        SoundManager::instance().play("click");
        if (m_index < (int)m_steps.size() - 1) showStep(m_index + 1);
        else accept();
    });
    connect(skip, &QPushButton::clicked, this, &QDialog::accept);

    QHBoxLayout* nav = new QHBoxLayout;
    nav->addWidget(skip);
    nav->addStretch();
    nav->addWidget(m_progress);
    nav->addStretch();
    nav->addWidget(m_prev);
    nav->addWidget(m_next);

    QVBoxLayout* root = new QVBoxLayout(this);
    root->setContentsMargins(20, 18, 20, 18);
    root->addWidget(m_title);
    root->addSpacing(8);
    root->addWidget(m_image);
    root->addSpacing(8);
    root->addWidget(m_body, 1);
    root->addLayout(nav);

    showStep(0);
}

void Tutorial::showStep(int index) {
    if (index < 0 || index >= (int)m_steps.size()) return;
    m_index = index;
    const Step& s = m_steps[index];

    m_title->setText(s.title);
    m_body->setHtml(QString("<div style='font-size:15px; line-height:150%;'>%1</div>").arg(s.body));

    QPixmap pix(s.image);
    if (!pix.isNull()) {
        m_image->setPixmap(pix.scaled(m_image->width() > 0 ? m_image->width() : 700, m_image->height(),
                                      Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
        m_image->show();
    } else {
        m_image->hide();
    }

    m_progress->setText(QString("%1 / %2").arg(index + 1).arg(m_steps.size()));
    m_prev->setEnabled(index > 0);
    m_next->setText(index == (int)m_steps.size() - 1 ? "完成" : "下一步");
}

void Tutorial::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);
    QPainter painter(this);
    painter.fillRect(rect(), QColor(26, 20, 16));
}
