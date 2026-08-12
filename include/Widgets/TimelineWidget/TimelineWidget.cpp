#include <TimelineWidget.h>

#include <canvas.h>
#include <rasterLayer.h>
#include <frame.h>

#include <algorithm>

#include <QMouseEvent>
#include <QKeyEvent>
#include <QContextMenuEvent>
#include <QMenu>
#include <QPainter>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QResizeEvent>
#include <QCursor>

TimelineWidget::TimelineWidget(Canvas* canvas, QWidget* parent)
    : QWidget(parent)
    , m_canvas(canvas)
{
    setMinimumHeight(140);
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);

    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(4, 4, 4, 4);
    root->setSpacing(2);

    auto* toolbar = new QWidget(this);
    toolbar->setFixedHeight(m_toolbarHeight);
    auto* toolbarLayout = new QHBoxLayout(toolbar);
    toolbarLayout->setContentsMargins(0, 0, 0, 0);
    toolbarLayout->setSpacing(6);

    m_addLayerBtn = new QPushButton("+ Layer", toolbar);
    m_addFrameBtn = new QPushButton("+ Frame", toolbar);
    toolbarLayout->addWidget(m_addLayerBtn);
    toolbarLayout->addWidget(m_addFrameBtn);
    toolbarLayout->addStretch(1);

    root->addWidget(toolbar);
    root->addStretch(1);

    connect(m_addLayerBtn, &QPushButton::clicked, this, &TimelineWidget::addLayer);
    connect(m_addFrameBtn, &QPushButton::clicked, this, &TimelineWidget::addFrameAtPlayhead);
}

void TimelineWidget::setCanvas(Canvas* canvas) {
    m_canvas = canvas;
    refresh();
}

void TimelineWidget::refresh() {
    update();
}

void TimelineWidget::addFrameAtPlayhead() {
    if (!m_canvas) return;
    RasterLayer* layer = m_canvas->currentLayer();
    if (!layer) return;

    layer->addFrame(m_canvas->getCurrentTime());
    m_canvas->setCurrentTime(m_canvas->getCurrentTime());
    emit timelineEdited();
    refresh();
}

void TimelineWidget::addLayer() {
    if (!m_canvas) return;
    m_canvas->addLayer();
    emit activeLayerChanged(m_canvas->getActiveLayerIndex());
    emit timelineEdited();
    refresh();
}

QRect TimelineWidget::contentRect() const {
    const int top = m_toolbarHeight + 6;
    return QRect(4, top, width() - 8, height() - top - 4);
}

QRect TimelineWidget::layerStripRect() const {
    QRect c = contentRect();
    return QRect(c.left(), c.top() + m_headerHeight, m_layerStripWidth, c.height() - m_headerHeight);
}

QRect TimelineWidget::timelineRect() const {
    QRect c = contentRect();
    return QRect(c.left() + m_layerStripWidth, c.top() + m_headerHeight,
                 c.width() - m_layerStripWidth, c.height() - m_headerHeight);
}

QRect TimelineWidget::headerRect() const {
    QRect c = contentRect();
    return QRect(c.left() + m_layerStripWidth, c.top(),
                 c.width() - m_layerStripWidth, m_headerHeight);
}

QRect TimelineWidget::eyeRectForRow(int row) const {
    QRect strip = layerStripRect();
    const int y = strip.top() + row * m_rowHeight + (m_rowHeight - 16) / 2;
    return QRect(strip.left() + 6, y, 16, 16);
}

int TimelineWidget::layerIndexAtRow(int row) const {
    if (!m_canvas) return -1;
    const auto& order = m_canvas->layerOrder();
    const int n = (int)order.size();
    if (row < 0 || row >= n) return -1;
    return order[n - 1 - row]; // reverse-read: row 0 = top of stack
}

int TimelineWidget::rowOfLayer(int layerIndex) const {
    if (!m_canvas) return -1;
    const auto& order = m_canvas->layerOrder();
    const int n = (int)order.size();
    for (int i = 0; i < n; ++i) {
        if (order[i] == layerIndex) return n - 1 - i;
    }
    return -1;
}

QRect TimelineWidget::frameBlockRect(int layerIndex, const Frame& f) const {
    return frameBlockRect(layerIndex, f.startIndex, f.duration);
}

QRect TimelineWidget::frameBlockRect(int layerIndex, int startIndex, int duration) const {
    QRect strip = layerStripRect();
    const int row = rowOfLayer(layerIndex);
    if (row < 0) return {};
    const int y = strip.top() + row * m_rowHeight;
    const int x0 = xFromTime(startIndex);
    const int w = std::max(m_pixelsPerFrame, duration * m_pixelsPerFrame);
    return QRect(x0 + 1, y + 4, w - 2, m_rowHeight - 8);
}

int TimelineWidget::visibleTimeCount() const {
    QRect tl = timelineRect();
    int fromWidth = tl.width() / std::max(1, m_pixelsPerFrame);
    int maxEnd = m_minVisibleFrames;
    if (m_canvas) {
        for (int i = 0; i < m_canvas->layerCount(); ++i) {
            RasterLayer* layer = m_canvas->layerAt(i);
            if (!layer) continue;
            for (const Frame& f : layer->getFrames()) {
                maxEnd = std::max(maxEnd, f.endIndex + 4);
            }
        }
        maxEnd = std::max(maxEnd, m_canvas->getCurrentTime() + 8);
        if (isFrameDrag()) {
            maxEnd = std::max(maxEnd, m_dragPreviewStart + m_dragPreviewDuration + 4);
        }
        if (hasSelection()) {
            maxEnd = std::max(maxEnd, m_selEnd + 4);
        }
    }
    return std::max(fromWidth, maxEnd);
}

int TimelineWidget::rowFromY(int y) const {
    QRect strip = layerStripRect();
    if (y < strip.top() || !m_canvas) return -1;
    int row = (y - strip.top()) / m_rowHeight;
    if (row < 0 || row >= m_canvas->layerCount()) return -1;
    return row;
}

int TimelineWidget::timeFromX(int x) const {
    QRect tl = timelineRect();
    int t = (x - tl.left()) / std::max(1, m_pixelsPerFrame);
    return std::max(0, t);
}

int TimelineWidget::xFromTime(int t) const {
    return timelineRect().left() + t * m_pixelsPerFrame;
}

TimelineWidget::HitResult TimelineWidget::hitTest(const QPoint& pos) const {
    HitResult hit;
    if (!m_canvas) return hit;

    const int row = rowFromY(pos.y());
    if (row < 0) return hit;

    const int layerIndex = layerIndexAtRow(row);
    RasterLayer* layer = m_canvas->layerAt(layerIndex);
    if (!layer) return hit;

    const auto& frames = layer->getFrames();
    for (int i = 0; i < (int)frames.size(); ++i) {
        QRect block = frameBlockRect(layerIndex, frames[i]);
        if (!block.contains(pos)) continue;

        hit.layerIndex = layerIndex;
        hit.frameIndex = i;
        hit.block = block;

        const int edge = std::min(m_edgeGrabPx, std::max(2, block.width() / 3));
        if (pos.x() <= block.left() + edge) {
            hit.mode = DragMode::ResizeLeft;
        } else if (pos.x() >= block.right() - edge) {
            hit.mode = DragMode::ResizeRight;
        } else {
            hit.mode = DragMode::MoveFrame;
        }
        return hit;
    }
    return hit;
}

void TimelineWidget::updateHoverCursor(const QPoint& pos) {
    if (m_dragMode != DragMode::None) return;

    if (layerStripRect().contains(pos) && !eyeRectForRow(rowFromY(pos.y())).contains(pos)
        && rowFromY(pos.y()) >= 0) {
        setCursor(Qt::SizeVerCursor);
        return;
    }

    HitResult hit = hitTest(pos);
    switch (hit.mode) {
        case DragMode::ResizeLeft:
        case DragMode::ResizeRight:
            setCursor(Qt::SizeHorCursor);
            break;
        case DragMode::MoveFrame:
            setCursor(Qt::OpenHandCursor);
            break;
        default:
            setCursor(Qt::ArrowCursor);
            break;
    }
}

void TimelineWidget::updateFrameDragPreview(int time) {
    if (!isFrameDrag()) return;

    int newStart = m_dragOrigStart;
    int newDuration = m_dragOrigDuration;

    if (m_dragMode == DragMode::MoveFrame) {
        newStart = std::max(0, time - m_dragGrabOffset);
        newDuration = m_dragOrigDuration;
    } else if (m_dragMode == DragMode::ResizeRight) {
        newStart = m_dragOrigStart;
        newDuration = std::max(1, time - m_dragOrigStart + 1);
    } else if (m_dragMode == DragMode::ResizeLeft) {
        newStart = std::clamp(time, 0, m_dragOrigEnd - 1);
        newDuration = m_dragOrigEnd - newStart;
    }

    if (newStart == m_dragPreviewStart && newDuration == m_dragPreviewDuration) {
        return;
    }

    m_dragPreviewStart = newStart;
    m_dragPreviewDuration = newDuration;
    m_dragEdited = (newStart != m_dragOrigStart || newDuration != m_dragOrigDuration);
    refresh();
}

void TimelineWidget::commitFrameDrag() {
    if (!m_canvas || !m_dragEdited || m_dragLayer < 0 || m_dragFrame < 0) return;
    RasterLayer* layer = m_canvas->layerAt(m_dragLayer);
    if (!layer || m_dragFrame >= layer->frameCount()) return;

    layer->updateFrame(m_dragFrame, m_dragPreviewStart, m_dragPreviewDuration);
    m_canvas->setCurrentTime(m_dragPreviewStart);
    emit timeChanged(m_dragPreviewStart);
    emit timelineEdited();
}

void TimelineWidget::stepFrame(int direction) {
    if (!m_canvas || direction == 0) return;
    RasterLayer* layer = m_canvas->currentLayer();
    if (!layer || layer->frameCount() == 0) return;

    const int t = m_canvas->getCurrentTime();
    const int fi = layer->frameIndexAtTime(t);
    const auto& frames = layer->getFrames();
    if (fi < 0 || fi >= (int)frames.size()) return;

    int targetTime = t;
    if (direction < 0) {
        // If playhead is past the start of the current frame, snap to its start first
        if (t > frames[fi].startIndex) {
            targetTime = frames[fi].startIndex;
        } else if (fi > 0) {
            targetTime = frames[fi - 1].startIndex;
        } else {
            return;
        }
    } else {
        if (fi + 1 < (int)frames.size()) {
            targetTime = frames[fi + 1].startIndex;
        } else {
            return;
        }
    }

    if (targetTime == t) return;
    m_canvas->setCurrentTime(targetTime);
    emit timeChanged(targetTime);
    refresh();
}

void TimelineWidget::deleteSelectedRange() {
    if (!m_canvas || !hasSelection()) return;
    RasterLayer* layer = m_canvas->currentLayer();
    if (!layer) return;

    layer->deleteTimeRange(m_selStart, m_selEnd);

    int newTime = m_selStart;
    m_canvas->setCurrentTime(newTime);
    emit timeChanged(newTime);
    emit timelineEdited();
    // Keep selection so user sees what was cleared; optional clear — keep it
    refresh();
}

void TimelineWidget::deleteLayerAt(int layerIndex) {
    if (!m_canvas) return;
    if (m_canvas->layerCount() <= 1) return;
    if (layerIndex < 0 || layerIndex >= m_canvas->layerCount()) return;

    m_canvas->removeLayer(layerIndex);
    emit activeLayerChanged(m_canvas->getActiveLayerIndex());
    emit timelineEdited();
    refresh();
}

void TimelineWidget::deleteFrameAt(int layerIndex, int frameIndex) {
    if (!m_canvas) return;
    RasterLayer* layer = m_canvas->layerAt(layerIndex);
    if (!layer) return;
    if (frameIndex < 0 || frameIndex >= layer->frameCount()) return;
    if (layer->frameCount() <= 1) return;

    if (layerIndex != m_canvas->getActiveLayerIndex()) {
        m_canvas->setCurrentLayer(layerIndex);
        emit activeLayerChanged(layerIndex);
    }

    const int oldTime = m_canvas->getCurrentTime();
    layer->removeFrame(frameIndex);

    int newTime = oldTime;
    if (layer->frameCount() > 0) {
        const int newFi = std::clamp(frameIndex, 0, layer->frameCount() - 1);
        const Frame& f = layer->getFrames()[newFi];
        if (newTime < f.startIndex || newTime >= f.endIndex) {
            newTime = f.startIndex;
        }
    }

    m_canvas->setCurrentTime(newTime);
    emit timeChanged(newTime);
    emit timelineEdited();
    refresh();
}

void TimelineWidget::setSelectionFromAnchor(int time) {
    time = std::max(0, time);
    const int a = m_selAnchor;
    m_selStart = std::min(a, time);
    m_selEnd = std::max(a, time) + 1;
}

void TimelineWidget::clearSelection() {
    m_selStart = 0;
    m_selEnd = 0;
}

void TimelineWidget::keyPressEvent(QKeyEvent* event) {
    if (!m_canvas) {
        QWidget::keyPressEvent(event);
        return;
    }

    switch (event->key()) {
        case Qt::Key_Left:
            stepFrame(-1);
            event->accept();
            return;
        case Qt::Key_Right:
            stepFrame(+1);
            event->accept();
            return;
        case Qt::Key_Delete:
        case Qt::Key_Backspace:
            if (hasSelection()) {
                deleteSelectedRange();
            }
            event->accept();
            return;
        case Qt::Key_Escape:
            clearSelection();
            refresh();
            event->accept();
            return;
        default:
            break;
    }
    QWidget::keyPressEvent(event);
}

void TimelineWidget::resizeEvent(QResizeEvent* event) {
    QWidget::resizeEvent(event);
    update();
}

void TimelineWidget::leaveEvent(QEvent* event) {
    QWidget::leaveEvent(event);
    if (m_dragMode == DragMode::None) {
        setCursor(Qt::ArrowCursor);
    }
}

void TimelineWidget::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, false);

    QRect c = contentRect();
    p.fillRect(c, QColor(42, 42, 46));

    QRect header(c.left() + m_layerStripWidth, c.top(), c.width() - m_layerStripWidth, m_headerHeight);
    p.fillRect(QRect(c.left(), c.top(), c.width(), m_headerHeight), QColor(32, 32, 36));
    p.fillRect(QRect(c.left(), c.top(), m_layerStripWidth, m_headerHeight), QColor(36, 36, 40));

    p.setPen(QColor(160, 160, 165));
    p.drawText(QRect(c.left() + 8, c.top(), m_layerStripWidth - 8, m_headerHeight),
               Qt::AlignVCenter | Qt::AlignLeft, QStringLiteral("Layers"));

    const int timeCount = visibleTimeCount();
    for (int t = 0; t <= timeCount; ++t) {
        int x = xFromTime(t);
        if (x > header.right()) break;
        bool major = (t % 5 == 0);
        p.setPen(major ? QColor(120, 120, 128) : QColor(70, 70, 76));
        p.drawLine(x, header.bottom() - (major ? 10 : 5), x, header.bottom());
        if (major) {
            p.setPen(QColor(170, 170, 175));
            p.drawText(QRect(x + 2, header.top(), 28, m_headerHeight - 2),
                       Qt::AlignLeft | Qt::AlignVCenter, QString::number(t));
        }
    }

    if (!m_canvas) {
        p.setPen(QColor(180, 180, 180));
        p.drawText(c, Qt::AlignCenter, QStringLiteral("No canvas"));
        return;
    }

    QRect strip = layerStripRect();
    QRect tl = timelineRect();

    const int layerCount = m_canvas->layerCount();
    const int active = m_canvas->getActiveLayerIndex();
    const int activeRow = rowOfLayer(active);
    const int playhead = m_canvas->getCurrentTime();

    // Soft current-time column (behind frames)
    {
        const int px = xFromTime(playhead);
        if (px >= tl.left() && px <= tl.right()) {
            p.fillRect(QRect(px, c.top(), m_pixelsPerFrame, c.height()), QColor(255, 255, 255, 22));
            p.setPen(QPen(QColor(200, 200, 210, 140), 1));
            p.drawLine(px, header.top() + 3, px, header.bottom());
            p.setBrush(QColor(210, 210, 220, 200));
            p.setPen(Qt::NoPen);
            QPolygon tip;
            tip << QPoint(px - 4, header.top() + 2)
                << QPoint(px + 4, header.top() + 2)
                << QPoint(px, header.top() + 8);
            p.drawPolygon(tip);
        }
    }

    // Tile selection highlight (behind frames)
    if (hasSelection()) {
        const int x0 = xFromTime(m_selStart);
        const int x1 = xFromTime(m_selEnd);
        QRect sel(x0, c.top(), std::max(1, x1 - x0), c.height());
        p.fillRect(sel.intersected(QRect(tl.left(), c.top(), tl.width(), c.height())),
                    QColor(100, 160, 255, 45));
    }

    // Row backgrounds + layer strip (row 0 = top of stack via reverse-read)
    for (int row = 0; row < layerCount; ++row) {
        const int layerIndex = layerIndexAtRow(row);
        RasterLayer* layer = m_canvas->layerAt(layerIndex);
        if (!layer) continue;

        const int y = strip.top() + row * m_rowHeight;
        if (y + m_rowHeight > strip.bottom()) break;

        QRect rowBg(c.left(), y, c.width(), m_rowHeight);
        const bool dropTarget = (m_dragMode == DragMode::ReorderLayer && row == m_dragPreviewRow);
        if (dropTarget) {
            p.fillRect(rowBg, QColor(70, 95, 130));
        } else if (row == activeRow) {
            p.fillRect(rowBg, QColor(55, 70, 95));
        } else if (row % 2 == 0) {
            p.fillRect(rowBg, QColor(46, 46, 50));
        }

        QRect eye = eyeRectForRow(row);
        p.setPen(QColor(200, 200, 205));
        p.setBrush(layer->isVisible() ? QColor(220, 220, 90) : QColor(80, 80, 85));
        p.drawEllipse(eye);

        p.setPen(QColor(220, 220, 225));
        p.drawText(QRect(eye.right() + 8, y, strip.width() - eye.width() - 16, m_rowHeight),
                   Qt::AlignVCenter | Qt::AlignLeft,
                   QString::fromStdString(layer->getName()));

        p.setPen(QColor(60, 60, 66));
        p.drawLine(c.left(), y + m_rowHeight - 1, c.right(), y + m_rowHeight - 1);
    }

    // Tile separators behind frame blocks
    for (int t = 0; t <= timeCount; ++t) {
        int x = xFromTime(t);
        if (x < tl.left() || x > tl.right()) continue;
        p.setPen((t % 5 == 0) ? QColor(65, 65, 72) : QColor(50, 50, 55));
        p.drawLine(x, tl.top(), x, tl.bottom());
    }

    // Frame blocks (dragged frame deferred so it paints on top)
    for (int row = 0; row < layerCount; ++row) {
        const int layerIndex = layerIndexAtRow(row);
        RasterLayer* layer = m_canvas->layerAt(layerIndex);
        if (!layer) continue;

        const int y = strip.top() + row * m_rowHeight;
        if (y + m_rowHeight > strip.bottom()) break;

        for (int fi = 0; fi < layer->frameCount(); ++fi) {
            if (isFrameDrag() && layerIndex == m_dragLayer && fi == m_dragFrame) continue;

            const Frame& f = layer->getFrames()[fi];
            QRect block = frameBlockRect(layerIndex, f);
            if (block.right() < tl.left() || block.left() > tl.right()) continue;

            bool coversPlayhead = (f.startIndex <= playhead && playhead < f.endIndex);

            p.setBrush(coversPlayhead ? QColor(90, 140, 210) : QColor(70, 110, 170));
            p.setPen(QColor(30, 50, 80));
            p.drawRect(block);

            if (block.width() >= 10) {
                p.fillRect(QRect(block.left(), block.top(), 3, block.height()), QColor(255, 255, 255, 40));
                p.fillRect(QRect(block.right() - 2, block.top(), 3, block.height()), QColor(255, 255, 255, 40));
            }

            p.setPen(QColor(230, 230, 235));
            p.drawText(block.adjusted(2, 0, -1, 0), Qt::AlignVCenter | Qt::AlignLeft,
                       QString::number(f.imageIndex));
        }
    }

    // Dragged frame last
    if (isFrameDrag() && m_dragLayer >= 0 && m_dragFrame >= 0) {
        RasterLayer* layer = m_canvas->layerAt(m_dragLayer);
        if (layer && m_dragFrame < layer->frameCount()) {
            const Frame& f = layer->getFrames()[m_dragFrame];

            QRect orig = frameBlockRect(m_dragLayer, f);
            if (orig.right() >= tl.left() && orig.left() <= tl.right()) {
                p.setBrush(QColor(70, 110, 170, 70));
                p.setPen(QColor(30, 50, 80, 90));
                p.drawRect(orig);
            }

            QRect preview = frameBlockRect(m_dragLayer, m_dragPreviewStart, m_dragPreviewDuration);
            if (preview.right() >= tl.left() && preview.left() <= tl.right()) {
                p.setBrush(QColor(110, 160, 230));
                p.setPen(QColor(40, 70, 110));
                p.drawRect(preview);
                if (preview.width() >= 10) {
                    p.fillRect(QRect(preview.left(), preview.top(), 3, preview.height()), QColor(255, 255, 255, 50));
                    p.fillRect(QRect(preview.right() - 2, preview.top(), 3, preview.height()), QColor(255, 255, 255, 50));
                }
                p.setPen(QColor(230, 230, 235));
                p.drawText(preview.adjusted(2, 0, -1, 0), Qt::AlignVCenter | Qt::AlignLeft,
                           QString::number(f.imageIndex));
            }
        }
    }

    p.setPen(QColor(70, 70, 76));
    p.setBrush(Qt::NoBrush);
    p.drawRect(c.adjusted(0, 0, -1, -1));
    p.drawLine(strip.right(), c.top(), strip.right(), c.bottom());
}

void TimelineWidget::mousePressEvent(QMouseEvent* event) {
    if (!m_canvas || event->button() != Qt::LeftButton) {
        QWidget::mousePressEvent(event);
        return;
    }

    setFocus(Qt::MouseFocusReason);

    const QPoint pos = event->pos();
    QRect strip = layerStripRect();
    QRect tl = timelineRect();
    const int row = rowFromY(pos.y());
    const int layerIndex = (row >= 0) ? layerIndexAtRow(row) : -1;

    if (row >= 0 && eyeRectForRow(row).contains(pos)) {
        RasterLayer* layer = m_canvas->layerAt(layerIndex);
        if (layer) {
            layer->setVisible(!layer->isVisible());
            emit timelineEdited();
            refresh();
        }
        return;
    }

    // Layer strip: select + start reorder drag
    if (row >= 0 && strip.contains(pos) && layerIndex >= 0) {
        if (layerIndex != m_canvas->getActiveLayerIndex()) {
            m_canvas->setCurrentLayer(layerIndex);
            emit activeLayerChanged(layerIndex);
        }
        m_dragMode = DragMode::ReorderLayer;
        m_dragLayer = layerIndex;
        m_dragPreviewRow = row;
        m_dragEdited = false;
        setCursor(Qt::SizeVerCursor);
        refresh();
        return;
    }

    const bool inHeader = headerRect().contains(pos);
    const bool inTimeline = tl.contains(pos) || inHeader;

    if (!inTimeline) {
        QWidget::mousePressEvent(event);
        return;
    }

    const int t = timeFromX(pos.x());
    const bool shiftSelect = event->modifiers() & Qt::ShiftModifier;

    // Header drag or Shift+drag → range select (even over frames)
    if (inHeader || shiftSelect) {
        m_dragMode = DragMode::SelectRange;
        m_selAnchor = t;
        setSelectionFromAnchor(t);
        m_canvas->setCurrentTime(t);
        emit timeChanged(t);
        refresh();
        return;
    }

    HitResult hit = hitTest(pos);
    if (hit.mode == DragMode::MoveFrame
        || hit.mode == DragMode::ResizeLeft
        || hit.mode == DragMode::ResizeRight) {

        RasterLayer* layer = m_canvas->layerAt(hit.layerIndex);
        if (!layer) return;
        const Frame& f = layer->getFrames()[hit.frameIndex];

        if (hit.layerIndex != m_canvas->getActiveLayerIndex()) {
            m_canvas->setCurrentLayer(hit.layerIndex);
            emit activeLayerChanged(hit.layerIndex);
        }

        m_dragMode = hit.mode;
        m_dragLayer = hit.layerIndex;
        m_dragFrame = hit.frameIndex;
        m_dragOriginTime = t;
        m_dragOrigStart = f.startIndex;
        m_dragOrigDuration = f.duration;
        m_dragOrigEnd = f.endIndex;
        m_dragGrabOffset = m_dragOriginTime - f.startIndex;
        m_dragPreviewStart = f.startIndex;
        m_dragPreviewDuration = f.duration;
        m_dragEdited = false;

        // Select both the frame and the clicked time tile
        int tile = std::clamp(t, f.startIndex, std::max(f.startIndex, f.endIndex - 1));
        m_selAnchor = tile;
        setSelectionFromAnchor(tile);
        m_canvas->setCurrentTime(tile);
        emit timeChanged(tile);

        if (m_dragMode == DragMode::MoveFrame) {
            setCursor(Qt::ClosedHandCursor);
        } else {
            setCursor(Qt::SizeHorCursor);
        }
        refresh();
        return;
    }

    // Empty timeline → scrub + range select
    m_dragMode = DragMode::Scrub;
    m_selAnchor = t;
    setSelectionFromAnchor(t);
    m_canvas->setCurrentTime(t);
    emit timeChanged(t);
    refresh();
}

void TimelineWidget::mouseMoveEvent(QMouseEvent* event) {
    if (!m_canvas) {
        QWidget::mouseMoveEvent(event);
        return;
    }

    if (!(event->buttons() & Qt::LeftButton) || m_dragMode == DragMode::None) {
        updateHoverCursor(event->pos());
        QWidget::mouseMoveEvent(event);
        return;
    }

    if (m_dragMode == DragMode::ReorderLayer) {
        int row = rowFromY(event->pos().y());
        if (row < 0) {
            // clamp to nearest valid row while dragging outside
            QRect strip = layerStripRect();
            if (event->pos().y() < strip.top()) row = 0;
            else row = m_canvas->layerCount() - 1;
        }
        if (row != m_dragPreviewRow) {
            m_dragPreviewRow = row;
            m_dragEdited = (row != rowOfLayer(m_dragLayer));
            refresh();
        }
        return;
    }

    if (m_dragMode == DragMode::Scrub || m_dragMode == DragMode::SelectRange) {
        const int t = timeFromX(event->pos().x());
        setSelectionFromAnchor(t);
        if (t != m_canvas->getCurrentTime()) {
            m_canvas->setCurrentTime(t);
            emit timeChanged(t);
        }
        refresh();
        return;
    }

    if (isFrameDrag()) {
        updateFrameDragPreview(timeFromX(event->pos().x()));
        return;
    }

    QWidget::mouseMoveEvent(event);
}

void TimelineWidget::mouseReleaseEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton && m_dragMode != DragMode::None) {
        if (isFrameDrag()) {
            commitFrameDrag();
        } else if (m_dragMode == DragMode::ReorderLayer && m_dragEdited
                   && m_dragLayer >= 0 && m_dragPreviewRow >= 0) {
            // Visual row 0 = top → last slot in layerOrder()
            const int n = (int)m_canvas->layerOrder().size();
            m_canvas->moveLayer(m_dragLayer, n - 1 - m_dragPreviewRow);
            emit timelineEdited();
        }

        m_dragMode = DragMode::None;
        m_dragLayer = -1;
        m_dragFrame = -1;
        m_dragPreviewRow = -1;
        m_dragEdited = false;
        updateHoverCursor(event->pos());
        refresh();
        return;
    }
    QWidget::mouseReleaseEvent(event);
}

void TimelineWidget::contextMenuEvent(QContextMenuEvent* event) {
    if (!m_canvas) {
        QWidget::contextMenuEvent(event);
        return;
    }

    const QPoint pos = event->pos();

    // Layer strip → delete layer
    if (layerStripRect().contains(pos)) {
        const int row = rowFromY(pos.y());
        const int layerIndex = (row >= 0) ? layerIndexAtRow(row) : -1;
        if (layerIndex < 0) {
            QWidget::contextMenuEvent(event);
            return;
        }

        if (layerIndex != m_canvas->getActiveLayerIndex()) {
            m_canvas->setCurrentLayer(layerIndex);
            emit activeLayerChanged(layerIndex);
            refresh();
        }

        QMenu menu(this);
        QAction* deleteAction = menu.addAction(QStringLiteral("Delete Layer"));
        deleteAction->setEnabled(m_canvas->layerCount() > 1);

        QAction* chosen = menu.exec(event->globalPos());
        if (chosen == deleteAction) {
            deleteLayerAt(layerIndex);
        }
        event->accept();
        return;
    }

    // Timeline frame block → delete whole frame via removeFrame
    if (timelineRect().contains(pos)) {
        HitResult hit = hitTest(pos);
        if (hit.layerIndex < 0 || hit.frameIndex < 0) {
            QWidget::contextMenuEvent(event);
            return;
        }

        RasterLayer* layer = m_canvas->layerAt(hit.layerIndex);
        if (!layer) {
            QWidget::contextMenuEvent(event);
            return;
        }

        if (hit.layerIndex != m_canvas->getActiveLayerIndex()) {
            m_canvas->setCurrentLayer(hit.layerIndex);
            emit activeLayerChanged(hit.layerIndex);
        }

        // Select a tile inside the frame so playhead/selection stay coherent
        const Frame& f = layer->getFrames()[hit.frameIndex];
        int t = std::clamp(timeFromX(pos.x()), f.startIndex, std::max(f.startIndex, f.endIndex - 1));
        m_selAnchor = t;
        setSelectionFromAnchor(t);
        m_canvas->setCurrentTime(t);
        emit timeChanged(t);
        refresh();

        QMenu menu(this);
        QAction* deleteAction = menu.addAction(QStringLiteral("Delete Frame"));
        deleteAction->setEnabled(layer->frameCount() > 1);

        QAction* chosen = menu.exec(event->globalPos());
        if (chosen == deleteAction) {
            deleteFrameAt(hit.layerIndex, hit.frameIndex);
        }
        event->accept();
        return;
    }

    QWidget::contextMenuEvent(event);
}
