#include <AnimationManager.h>
#include <canvas.h>

#include <algorithm>

AnimationManager::AnimationManager(Canvas* canvas, QObject* parent)
    : QObject(parent)
    , m_canvas(canvas)
{
    m_timer.setTimerType(Qt::PreciseTimer);
    connect(&m_timer, &QTimer::timeout, this, &AnimationManager::tick);
    applyTimerInterval();
}

void AnimationManager::setCanvas(Canvas* canvas) {
    if (m_playing) {
        stop();
    }
    m_canvas = canvas;
}

void AnimationManager::setFps(int fps) {
    m_fps = std::clamp(fps, 1, 60);
    applyTimerInterval();
}

void AnimationManager::play() {
    if (!m_canvas || m_playing) return;
    if (m_canvas->playbackEnd() <= 0) return;

    m_playing = true;
    applyTimerInterval();
    m_timer.start();
    emit playingChanged(true);
}

void AnimationManager::stop() {
    if (!m_playing) return;

    m_timer.stop();
    m_playing = false;
    emit playingChanged(false);
}

void AnimationManager::toggle() {
    if (m_playing) {
        stop();
    } else {
        play();
    }
}

void AnimationManager::tick() {
    if (!m_canvas) {
        stop();
        return;
    }

    const int end = m_canvas->playbackEnd();
    if (end <= 0) {
        stop();
        return;
    }

    int t = m_canvas->getCurrentTime() + 1;
    if (t >= end) {
        t = 0;
    }
    emit timeChanged(t);
}

void AnimationManager::applyTimerInterval() {
    m_timer.setInterval(std::max(1, 1000 / m_fps));
}
