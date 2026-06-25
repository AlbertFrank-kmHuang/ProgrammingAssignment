#ifndef SOUNDMANAGER_H
#define SOUNDMANAGER_H

#include <QObject>
#include <QString>
#include <QHash>

class QSoundEffect;

// 简易音效管理器
// 由于项目不附带音频素材，这里在首次运行时用代码合成 8-bit 风格的 wav 音效，
// 写入到可执行文件同目录的 sounds/ 下，再通过 QSoundEffect 播放。
class SoundManager : public QObject {
    Q_OBJECT
public:
    static SoundManager& instance();

    // key 取值见 .cpp：hit/crit/heal/skill/victory/defeat/click/switch/hurt
    void play(const QString& key);

    void setMuted(bool muted) { m_muted = muted; }
    bool isMuted() const { return m_muted; }

private:
    explicit SoundManager(QObject* parent = nullptr);
    void ensureAssets();                 // 确保 wav 文件已生成
    QString assetDir() const;
    QSoundEffect* effectFor(const QString& key);

    QHash<QString, QSoundEffect*> m_effects;
    bool m_ready = false;
    bool m_muted = false;
};

#endif // SOUNDMANAGER_H
