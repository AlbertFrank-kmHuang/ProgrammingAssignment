#include "soundmanager.h"

#include <QSoundEffect>
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QUrl>
#include <QByteArray>
#include <QtEndian>
#include <QtGlobal>
#include <cmath>
#include <vector>

namespace {

constexpr int kSampleRate = 44100;

// 简单的合成音：把若干段“音符”拼接/叠加成 16-bit 单声道 PCM
struct Tone {
    double freq;        // 频率(Hz)，0 表示噪声(用于打击感)
    double start;       // 起始时间(秒)
    double duration;    // 持续(秒)
    double amplitude;   // 0~1
    enum Wave { Square, Sine, Noise } wave = Square;
};

// 把一组 Tone 渲染为 wav 字节流
QByteArray renderWav(const std::vector<Tone>& tones, double totalSeconds) {
    const int totalSamples = static_cast<int>(kSampleRate * totalSeconds);
    std::vector<double> buffer(totalSamples, 0.0);

    quint32 noiseState = 0x1234567u;
    auto nextNoise = [&]() {
        // 线性同余 + 取符号，得到方波噪声
        noiseState = noiseState * 1103515245u + 12345u;
        return ((noiseState >> 16) & 1) ? 1.0 : -1.0;
    };

    for (const Tone& t : tones) {
        const int s0 = static_cast<int>(t.start * kSampleRate);
        const int s1 = std::min(totalSamples, static_cast<int>((t.start + t.duration) * kSampleRate));
        const int len = s1 - s0;
        if (len <= 0) continue;

        for (int i = 0; i < len && (s0 + i) < totalSamples; ++i) {
            const double tt = static_cast<double>(i) / kSampleRate;
            const double phase = t.freq * tt;
            // 简单的衰减包络：快速起音 + 线性衰减，听感更像“叮/咚”
            const double env = (1.0 - static_cast<double>(i) / len);
            double v = 0.0;
            switch (t.wave) {
            case Tone::Square: v = (std::sin(2.0 * M_PI * phase) >= 0.0 ? 1.0 : -1.0); break;
            case Tone::Sine:   v = std::sin(2.0 * M_PI * phase); break;
            case Tone::Noise:  v = nextNoise(); break;
            }
            buffer[s0 + i] += v * t.amplitude * env;
        }
    }

    // 写 PCM 16-bit
    QByteArray pcm;
    pcm.reserve(totalSamples * 2);
    for (double sample : buffer) {
        double clamped = std::max(-1.0, std::min(1.0, sample));
        qint16 s = static_cast<qint16>(clamped * 32000);
        char bytes[2];
        qToLittleEndian<qint16>(s, bytes);
        pcm.append(bytes, 2);
    }

    // 组装 WAV 头
    QByteArray wav;
    auto appendStr = [&](const char* s) { wav.append(s, 4); };
    auto appendU32 = [&](quint32 v) { char b[4]; qToLittleEndian<quint32>(v, b); wav.append(b, 4); };
    auto appendU16 = [&](quint16 v) { char b[2]; qToLittleEndian<quint16>(v, b); wav.append(b, 2); };

    const quint32 byteRate = kSampleRate * 1 * 16 / 8;
    appendStr("RIFF");
    appendU32(36 + pcm.size());
    appendStr("WAVE");
    appendStr("fmt ");
    appendU32(16);              // fmt chunk size
    appendU16(1);              // PCM
    appendU16(1);              // mono
    appendU32(kSampleRate);
    appendU32(byteRate);
    appendU16(2);              // block align
    appendU16(16);             // bits per sample
    appendStr("data");
    appendU32(pcm.size());
    wav.append(pcm);
    return wav;
}

// 各音效的合成配方
QByteArray buildEffect(const QString& key) {
    std::vector<Tone> tones;
    double total = 0.3;

    if (key == "hit") {
        total = 0.12;
        tones.push_back({0, 0.0, 0.06, 0.6, Tone::Noise});
        tones.push_back({180, 0.0, 0.10, 0.5, Tone::Square});
    } else if (key == "crit") {
        total = 0.22;
        tones.push_back({0, 0.0, 0.05, 0.7, Tone::Noise});
        tones.push_back({330, 0.0, 0.12, 0.6, Tone::Square});
        tones.push_back({660, 0.06, 0.14, 0.5, Tone::Square});
    } else if (key == "hurt") {
        total = 0.16;
        tones.push_back({0, 0.0, 0.05, 0.5, Tone::Noise});
        tones.push_back({120, 0.0, 0.14, 0.55, Tone::Square});
    } else if (key == "heal") {
        total = 0.4;
        tones.push_back({523, 0.0, 0.14, 0.4, Tone::Sine});  // C5
        tones.push_back({659, 0.12, 0.14, 0.4, Tone::Sine}); // E5
        tones.push_back({784, 0.24, 0.16, 0.4, Tone::Sine}); // G5
    } else if (key == "skill") {
        total = 0.28;
        tones.push_back({440, 0.0, 0.10, 0.45, Tone::Square});
        tones.push_back({587, 0.08, 0.10, 0.45, Tone::Square});
        tones.push_back({880, 0.16, 0.12, 0.4, Tone::Square});
    } else if (key == "victory") {
        total = 0.7;
        tones.push_back({523, 0.0, 0.14, 0.45, Tone::Square});  // C
        tones.push_back({659, 0.14, 0.14, 0.45, Tone::Square}); // E
        tones.push_back({784, 0.28, 0.14, 0.45, Tone::Square}); // G
        tones.push_back({1047, 0.42, 0.26, 0.5, Tone::Square}); // C6
    } else if (key == "defeat") {
        total = 0.7;
        tones.push_back({392, 0.0, 0.18, 0.45, Tone::Square});  // G4
        tones.push_back({330, 0.18, 0.18, 0.45, Tone::Square}); // E4
        tones.push_back({262, 0.36, 0.30, 0.5, Tone::Square});  // C4
    } else if (key == "switch") {
        total = 0.14;
        tones.push_back({440, 0.0, 0.05, 0.4, Tone::Square});
        tones.push_back({660, 0.06, 0.06, 0.4, Tone::Square});
    } else { // click 默认
        total = 0.05;
        tones.push_back({880, 0.0, 0.04, 0.35, Tone::Square});
    }

    return renderWav(tones, total);
}

} // namespace

SoundManager& SoundManager::instance() {
    static SoundManager s;
    return s;
}

SoundManager::SoundManager(QObject* parent) : QObject(parent) {
    ensureAssets();
}

QString SoundManager::assetDir() const {
    QDir dir(QCoreApplication::applicationDirPath());
    dir.mkpath("sounds");
    return dir.absoluteFilePath("sounds");
}

void SoundManager::ensureAssets() {
    const QStringList keys = {"hit", "crit", "hurt", "heal", "skill", "victory", "defeat", "switch", "click"};
    const QString dir = assetDir();
    for (const QString& key : keys) {
        const QString path = dir + "/" + key + ".wav";
        if (!QFile::exists(path)) {
            QByteArray data = buildEffect(key);
            QFile f(path);
            if (f.open(QIODevice::WriteOnly)) {
                f.write(data);
                f.close();
            }
        }
    }
    m_ready = true;
}

QSoundEffect* SoundManager::effectFor(const QString& key) {
    auto it = m_effects.find(key);
    if (it != m_effects.end()) return it.value();

    const QString path = assetDir() + "/" + key + ".wav";
    if (!QFile::exists(path)) return nullptr;

    QSoundEffect* eff = new QSoundEffect(this);
    eff->setSource(QUrl::fromLocalFile(path));
    eff->setVolume(0.6);
    m_effects.insert(key, eff);
    return eff;
}

void SoundManager::play(const QString& key) {
    if (m_muted || !m_ready) return;
    QSoundEffect* eff = effectFor(key);
    if (eff) eff->play();
}
