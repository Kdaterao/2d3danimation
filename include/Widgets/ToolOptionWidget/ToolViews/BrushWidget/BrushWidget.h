#ifndef BRUSHWIDGET_H
#define BRUSHWIDGET_H

#include <BrushCatalog.h>
#include <Brush.h>
#include <QCheckBox>
#include <QGridLayout>
#include <QLabel>
#include <QScrollArea>
#include <QSlider>
#include <QWidget>
#include <vector>

class BrushPreviewCell;

class BrushWidget : public QWidget {
    Q_OBJECT

public:
    explicit BrushWidget(int initialBrushSize = 5, QWidget* parent = nullptr);
    int brushSize() const;

private slots:
    void onCellClicked(const QString& brushId);

private:
    void selectBrush(const QString& brushId);

    QGridLayout* m_grid = nullptr;
    QCheckBox* m_eraserToggle = nullptr;
    QSlider* m_sizeSlider = nullptr;
    QSlider* m_opacitySlider = nullptr;
    QLabel* m_sizeLabel = nullptr;
    QLabel* m_opacityLabel = nullptr;
    QLabel* m_selectedLabel = nullptr;
    std::vector<BrushPreviewCell*> m_cells;
    QString m_selectedId;

signals:
    void brushTypeChanged(Brush::RasterTypes type);
    void brushSelected(const QString& brushId, Brush::RasterTypes type, int defaultSize);
    void eraserToggled(bool enabled);
    void brushSizeChanged(int size);
    void brushOpacityChanged(int opacityPercent); // 0–100
};

#endif
