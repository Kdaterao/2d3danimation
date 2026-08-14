#ifndef TOOLOPTIONWIDGET_H
#define TOOLOPTIONWIDGET_H

#include <Brush.h>
#include <QStackedWidget>
#include <QWidget>

class BrushWidget;

enum class ToolOptionView {
    Brush = 0,
    // Future tool option views go here.
};

class ToolOptionWidget : public QWidget {
    Q_OBJECT

public:
    explicit ToolOptionWidget(int initialBrushSize = 5, QWidget* parent = nullptr);

public slots:
    void onToolSelected(int toolIndex);

signals:
    void brushTypeChanged(Brush::RasterTypes type);
    void brushSelected(const QString& brushId, Brush::RasterTypes type, int defaultSize);
    void eraserToggled(bool enabled);
    void brushSizeChanged(int size);
    void brushOpacityChanged(int opacityPercent); // 0–100

private:
    QStackedWidget* m_stack = nullptr;
    BrushWidget* m_brushView = nullptr;
};

#endif
