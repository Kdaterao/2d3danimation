#ifndef BRUSHPREVIEWCELL_H
#define BRUSHPREVIEWCELL_H

#include <BrushCatalog.h>
#include <QMouseEvent>
#include <QOpenGLFunctions>
#include <QOpenGLWidget>
#include <UIPainter.h>
#include <toonzShader.h>

class BrushPreviewCell : public QOpenGLWidget, protected QOpenGLFunctions {
    Q_OBJECT

public:
    explicit BrushPreviewCell(const BrushEntry& entry, QWidget* parent = nullptr);

    void setSelected(bool selected);
    const QString& brushId() const { return m_entry.id; }

protected:
    void initializeGL() override;
    void paintGL() override;
    void mousePressEvent(QMouseEvent* event) override;

signals:
    void clicked(const QString& brushId);

private:
    void rebuildGeometry();

    BrushEntry m_entry;
    toonzShader* m_shader = nullptr;
    UIPainter* m_painter = nullptr;
    bool m_selected = false;
};

#endif
