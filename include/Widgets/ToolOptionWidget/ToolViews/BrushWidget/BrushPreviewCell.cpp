#include <BrushPreviewCell.h>
#include <UIShapes.h>

BrushPreviewCell::BrushPreviewCell(const BrushEntry& entry, QWidget* parent)
    : QOpenGLWidget(parent), m_entry(entry) {
    setFixedSize(72, 72);
    setToolTip(m_entry.name);
}

void BrushPreviewCell::setSelected(bool selected) {

    //---- change toggle -----
    if (m_selected == selected) {
        return;
    }
    m_selected = selected;
    if (!m_painter) {
        return;
    }


    //---- update celll ----
    rebuildGeometry();
    update();
}

void BrushPreviewCell::rebuildGeometry() {
    m_painter->cleanUp();

    float bg = m_selected ? 0.18f : 0.28f;

    VertexRGBM32 outerTL(-0.9f,  0.9f, bg, bg, bg, 1.0f);
    VertexRGBM32 outerTR( 0.9f,  0.9f, bg, bg, bg, 1.0f);
    VertexRGBM32 outerBL(-0.9f, -0.9f, bg, bg, bg, 1.0f);
    VertexRGBM32 outerBR( 0.9f, -0.9f, bg, bg, bg, 1.0f);
    m_painter->addGeometry(Box(outerTL, outerTR, outerBL, outerBR));

    // Placeholder inner region for a future brush-stroke preview image.
    float innerR = 0.75f;
    float innerG = 0.75f;
    float innerB = 0.75f;
    VertexRGBM32 innerTL(-0.55f,  0.55f, innerR, innerG, innerB, 1.0f);
    VertexRGBM32 innerTR( 0.55f,  0.55f, innerR, innerG, innerB, 1.0f);
    VertexRGBM32 innerBL(-0.55f, -0.55f, innerR, innerG, innerB, 1.0f);
    VertexRGBM32 innerBR( 0.55f, -0.55f, innerR, innerG, innerB, 1.0f);
    m_painter->addGeometry(Box(innerTL, innerTR, innerBL, innerBR));
}

void BrushPreviewCell::initializeGL() {
    initializeOpenGLFunctions();
    setUpdateBehavior(QOpenGLWidget::PartialUpdate);

    m_shader = new toonzShader(
        "/Users/krish/codingStuff/2d3danimation/include/2D/Painter/General/Shader/ShaderScripts/vsColorTriangle.txt",
        "/Users/krish/codingStuff/2d3danimation/include/2D/Painter/General/Shader/ShaderScripts/fsColorTriangle.txt"
    );

    m_painter = new UIPainter(width(), height(), GL_NEAREST, GL_NEAREST, false, m_shader);
    rebuildGeometry();
}

void BrushPreviewCell::paintGL() {
    if (!m_painter) {
        return;
    }
    m_painter->Paint(defaultFramebufferObject(), true);
}

void BrushPreviewCell::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        emit clicked(m_entry.id);
    }
    event->accept();
}
