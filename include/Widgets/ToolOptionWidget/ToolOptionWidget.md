# ToolOptionWidget

**Note: This widget is AI-generated.** Treat it as a starting point — review and rewrite pieces as needed when tools or brush APIs change.

Files: `ToolOptionWidget.h` / `ToolOptionWidget.cpp`  
Views live under `ToolViews/` (currently `BrushWidget/`).  
Brush list data: `BrushCatalog` + `resources/brushes.xml`.  
Wired in `main.cpp` beside the canvas; signals go to `GLWidget`.

---

## Role

Sidebar panel for **options of the current tool** — not tool *selection* (that’s a separate concern), and not color (also separate: `ColorTriangleWidget`).

It hosts one options UI per tool and switches between them. Right now only the brush view exists.

---

## Structure

```
ToolOptionWidget          ← shell: QStackedWidget + signal forwarding
└── ToolViews/
    └── BrushWidget/      ← eraser, size slider, brush grid
        └── BrushPreviewCell
BrushCatalog              ← loads brushes.xml → BrushEntry list
```

`ToolOptionView` enum indexes the stack (`Brush = 0`). Add future tools as new enum values + stacked pages.

---

## How it works

1. Constructor builds a `QStackedWidget`, adds `BrushWidget`, shows `ToolOptionView::Brush`.
2. `BrushWidget` loads `BrushCatalog` from `brushes.xml`, builds a grid of `BrushPreviewCell`s, plus eraser checkbox and size slider.
3. BrushWidget signals are **forwarded** unchanged through `ToolOptionWidget` (no logic in the shell).
4. `onToolSelected(int toolIndex)` sets `m_stack->setCurrentIndex(toolIndex)` so a future tool picker can flip pages by enum index.

---

## Signals (forwarded to canvas)

| Signal | Meaning |
|--------|---------|
| `brushSelected(id, type, defaultSize)` | User picked a catalog brush |
| `brushTypeChanged(type)` | Same selection’s raster brush type |
| `brushSizeChanged(size)` | Size slider (or default size on brush pick) |
| `eraserToggled(enabled)` | Eraser checkbox |

`main.cpp` connects these to `GLWidget::{selectBrush, updateBrushSize, toggleEraser}`.

---

## BrushWidget details

- **Eraser** — checkbox → `eraserToggled`
- **Size** — horizontal slider 1–50 → `brushSizeChanged`
- **Catalog grid** — 3 columns in a fixed-height scroll area; each cell is a `BrushPreviewCell` (preview + click)
- On select: highlights cell, updates label, sets slider to that brush’s `defaultSize`, emits type/selection/size

`BrushCatalog` parses XML entries (`id`, `name`, `type`, `defaultSize`, preview paths, etc.) into a static list shared by the UI.

---

## Adding another tool view

1. Add a value to `ToolOptionView`.
2. Create `ToolViews/YourToolWidget/`.
3. `m_stack->addWidget(...)` in the same enum order.
4. Forward any new signals from that view through `ToolOptionWidget`.
5. Call `onToolSelected((int)ToolOptionView::YourTool)` from the tool picker when that tool is active.
