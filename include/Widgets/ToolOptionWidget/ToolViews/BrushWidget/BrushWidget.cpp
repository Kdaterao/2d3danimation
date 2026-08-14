#include <BrushWidget.h>
#include <BrushPreviewCell.h>
#include <QHBoxLayout>
#include <QVBoxLayout>

static const char* kBrushCatalogPath =
    "/Users/krish/codingStuff/2d3danimation/resources/brushes.xml";

BrushWidget::BrushWidget(int initialBrushSize, QWidget* parent)
    : QWidget(parent) {

    BrushCatalog::loadFromFile(kBrushCatalogPath);

    m_eraserToggle = new QCheckBox("Eraser", this);

    m_sizeLabel = new QLabel(QStringLiteral("Size"), this);
    m_sizeSlider = new QSlider(Qt::Horizontal, this);
    m_sizeSlider->setRange(1, 50);
    m_sizeSlider->setValue(initialBrushSize);

    m_opacityLabel = new QLabel(QStringLiteral("Opacity"), this);
    m_opacitySlider = new QSlider(Qt::Horizontal, this);
    m_opacitySlider->setRange(0, 100);
    m_opacitySlider->setValue(100);

    m_selectedLabel = new QLabel(this);
    m_selectedLabel->setAlignment(Qt::AlignCenter);

    connect(m_eraserToggle, &QCheckBox::toggled, this, &BrushWidget::eraserToggled);
    connect(m_sizeSlider, &QSlider::valueChanged, this, &BrushWidget::brushSizeChanged);
    connect(m_opacitySlider, &QSlider::valueChanged, this, &BrushWidget::brushOpacityChanged);

    auto* scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    auto* container = new QWidget(scrollArea);
    m_grid = new QGridLayout(container);
    m_grid->setSpacing(4);
    m_grid->setContentsMargins(4, 4, 4, 4);

    const auto& brushes = BrushCatalog::entries();
    const int columns = 3;
    int row = 0;
    int col = 0;

    for (const BrushEntry& entry : brushes) {
        auto* cell = new BrushPreviewCell(entry, container);
        m_cells.push_back(cell);
        m_grid->addWidget(cell, row, col);

        connect(cell, &BrushPreviewCell::clicked, this, &BrushWidget::onCellClicked);

        col += 1;
        if (col >= columns) {
            col = 0;
            row += 1;
        }
    }

    scrollArea->setWidget(container);
    scrollArea->setFixedHeight(240);

    auto* sizeRow = new QHBoxLayout();
    sizeRow->addWidget(m_sizeLabel);
    sizeRow->addWidget(m_sizeSlider, 1);

    auto* opacityRow = new QHBoxLayout();
    opacityRow->addWidget(m_opacityLabel);
    opacityRow->addWidget(m_opacitySlider, 1);

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_eraserToggle);
    layout->addLayout(sizeRow);
    layout->addLayout(opacityRow);
    layout->addWidget(m_selectedLabel);
    layout->addWidget(scrollArea);

    if (!brushes.empty()) {
        selectBrush(brushes.front().id);
    }
}

int BrushWidget::brushSize() const {
    return m_sizeSlider->value();
}

void BrushWidget::onCellClicked(const QString& brushId) {
    selectBrush(brushId);
}

void BrushWidget::selectBrush(const QString& brushId) {
    if (m_selectedId == brushId) {
        return;
    }

    m_selectedId = brushId;

    for (BrushPreviewCell* cell : m_cells) {
        cell->setSelected(cell->brushId() == brushId);
    }

    for (const BrushEntry& entry : BrushCatalog::entries()) {
        if (entry.id == brushId) {
            m_selectedLabel->setText(entry.name);

            m_sizeSlider->blockSignals(true);
            m_sizeSlider->setValue(entry.defaultSize);
            m_sizeSlider->blockSignals(false);

            emit brushTypeChanged(entry.type);
            emit brushSelected(entry.id, entry.type, entry.defaultSize);
            emit brushSizeChanged(entry.defaultSize);
            return;
        }
    }
}
