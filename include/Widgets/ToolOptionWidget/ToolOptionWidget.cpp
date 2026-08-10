#include <ToolOptionWidget.h>
#include <BrushWidget.h>
#include <QVBoxLayout>

ToolOptionWidget::ToolOptionWidget(int initialBrushSize, QWidget* parent)
    : QWidget(parent) {

    m_stack = new QStackedWidget(this);
    m_brushView = new BrushWidget(initialBrushSize, this);
    m_stack->addWidget(m_brushView);

    connect(m_brushView, &BrushWidget::brushTypeChanged,
            this, &ToolOptionWidget::brushTypeChanged);
    connect(m_brushView, &BrushWidget::brushSelected,
            this, &ToolOptionWidget::brushSelected);
    connect(m_brushView, &BrushWidget::eraserToggled,
            this, &ToolOptionWidget::eraserToggled);
    connect(m_brushView, &BrushWidget::brushSizeChanged,
            this, &ToolOptionWidget::brushSizeChanged);

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_stack);

    m_stack->setCurrentIndex(static_cast<int>(ToolOptionView::Brush));
}

void ToolOptionWidget::onToolSelected(int toolIndex) {
    m_stack->setCurrentIndex(toolIndex);
}
