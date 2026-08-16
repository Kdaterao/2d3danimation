#ifndef ANIMATIONMANAGER_H
#define ANIMATIONMANAGER_H

#include <QObject>
#include <QTimer>

class Canvas;

class AnimationManager : public QObject {
    Q_OBJECT

public:
    explicit AnimationManager(Canvas* canvas = nullptr, QObject* parent = nullptr);

    void setCanvas(Canvas* canvas);
    Canvas* canvas() const { return m_canvas; }

    void setFps(int fps);
    int fps() const { return m_fps; }

    bool isPlaying() const { return m_playing; }

public slots:
    void play();
    void stop();
    void toggle();

signals:
    void timeChanged(int time);
    void playingChanged(bool playing);

private slots:
    void tick();

private:
    void applyTimerInterval();

    Canvas* m_canvas = nullptr;
    QTimer m_timer;
    int m_fps = 24;
    bool m_playing = false;
};

#endif
