#ifndef TIMELINEWIDGET_H
#define TIMELINEWIDGET_H

#include <QWidget>

class Canvas;
class AnimationManager;
class QPushButton;
class QSpinBox;

class TimelineWidget : public QWidget {
    Q_OBJECT

public:
    explicit TimelineWidget(Canvas* canvas = nullptr, QWidget* parent = nullptr);

    void setCanvas(Canvas* canvas);
    Canvas* canvas() const { return m_canvas; }

    QSize sizeHint() const override { return QSize(800, 200); }
    QSize minimumSizeHint() const override { return QSize(400, 140); }

signals:
    void timeChanged(int time);
    void activeLayerChanged(int layerIndex);
    void timelineEdited();
    void onionSkinChanged(int before, int after);

public slots:
    void refresh();
    void addFrameAtPlayhead();
    void addLayer();

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void leaveEvent(QEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void contextMenuEvent(QContextMenuEvent* event) override;

private:
    enum class DragMode {
        None,
        Scrub,          // also builds a tile selection while dragging
        SelectRange,    // shift-drag / header-drag range select
        MoveFrame,
        ResizeLeft,
        ResizeRight,
        ReorderLayer
    };

    struct HitResult {
        int layerIndex = -1;
        int frameIndex = -1;
        DragMode mode = DragMode::None;
        QRect block;
    };

    int rowFromY(int y) const;
    // Timeline paints top-of-stack first by reverse-reading layerOrder().
    int layerIndexAtRow(int row) const;
    int rowOfLayer(int layerIndex) const;
    int timeFromX(int x) const;
    int xFromTime(int t) const;
    QRect layerStripRect() const;
    QRect timelineRect() const;
    QRect headerRect() const;
    QRect eyeRectForRow(int row) const;
    QRect contentRect() const;
    QRect frameBlockRect(int layerIndex, const class Frame& f) const;
    QRect frameBlockRect(int layerIndex, int startIndex, int duration) const;
    int visibleTimeCount() const;
    HitResult hitTest(const QPoint& pos) const;
    void updateFrameDragPreview(int time);
    void commitFrameDrag();
    void updateHoverCursor(const QPoint& pos);
    void stepFrame(int direction);
    void deleteSelectedRange();
    void deleteLayerAt(int layerIndex);
    void deleteFrameAt(int layerIndex, int frameIndex);
    void setSelectionFromAnchor(int time);
    void clearSelection();
    bool hasSelection() const { return m_selEnd > m_selStart; }
    bool isFrameDrag() const {
        return m_dragMode == DragMode::MoveFrame
            || m_dragMode == DragMode::ResizeLeft
            || m_dragMode == DragMode::ResizeRight;
    }

    Canvas* m_canvas = nullptr;

    QPushButton* m_addLayerBtn = nullptr;
    QPushButton* m_addFrameBtn = nullptr;
    QPushButton* m_playBtn = nullptr;
    QSpinBox* m_fpsSpin = nullptr;
    QSpinBox* m_onionBeforeSpin = nullptr;
    QSpinBox* m_onionAfterSpin = nullptr;
    AnimationManager* m_anim = nullptr;

    int m_toolbarHeight = 32;
    int m_headerHeight = 22;
    int m_layerStripWidth = 120;
    int m_rowHeight = 28;
    int m_pixelsPerFrame = 16;
    int m_minVisibleFrames = 48;
    int m_edgeGrabPx = 6;

    DragMode m_dragMode = DragMode::None;
    int m_dragLayer = -1;
    int m_dragFrame = -1;
    int m_dragOriginTime = 0;
    int m_dragOrigStart = 0;
    int m_dragOrigDuration = 1;
    int m_dragOrigEnd = 1;
    int m_dragGrabOffset = 0;
    int m_dragPreviewStart = 0;
    int m_dragPreviewDuration = 1;
    int m_dragPreviewRow = -1; // visual row while reordering (0 = top of strip)
    bool m_dragEdited = false;

    // Exclusive time range selection [m_selStart, m_selEnd)
    int m_selStart = 0;
    int m_selEnd = 0;
    int m_selAnchor = 0;
};

#endif
