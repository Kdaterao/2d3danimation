# TimelineWidget

**Note: This widget is AI-generated.** Treat it as a starting point — review and rewrite pieces as needed when behavior or model APIs change.

Files: `TimelineWidget.h` / `TimelineWidget.cpp`  
Wired in `main.cpp` under the canvas; talks to `Canvas` + `RasterLayer`.

---

## Role

UI for scrubbing time, editing frames on the active layer stack, and managing layers (add / reorder / visibility / delete). It does **not** own the animation data — `Canvas` and `RasterLayer` do. The widget paints from that model and calls setters when the user edits.

---

## Layout (top → bottom)

1. **Toolbar** — `+ Layer`, `+ Frame` (Qt buttons).
2. **Content area** (custom-painted):
   - **Header** — frame-number ruler (drag here to range-select).
   - **Layer strip** (left) — eye toggle, layer name; drag to reorder.
   - **Timeline grid** (right) — frame blocks per layer, playhead, tile selection.

Display: timeline reverse-reads `Canvas::layerOrder()` so row 0 is the top of the stack (last entry in the vector). Reorder commits via `moveLayer(layerIndex, toOrder)`.

Time mapping uses `m_pixelsPerFrame` (default 16px per frame index). Frame ends are exclusive (`startIndex` .. `endIndex`).

---

## Signals

| Signal | When |
|--------|------|
| `timeChanged(int)` | Playhead moved |
| `activeLayerChanged(int)` | Active layer changed |
| `timelineEdited()` | Layers/frames/visibility changed (canvas should refresh) |

`GLWidget` listens to these in `main.cpp`.

---

## Interactions

### Playhead / selection
- Click/drag empty timeline → scrub; drag also builds a tile selection `[m_selStart, m_selEnd)`.
- **Shift+drag** or drag on the **header** → range select without needing empty space.
- **Esc** clears selection.
- **← / →** step to previous/next frame start on the current layer.
- **Delete / Backspace** with a selection → `RasterLayer::deleteTimeRange` (carves that range out).

### Frames
- Drag **center** of a block → move (`updateFrame` on release).
- Drag **left/right edge** → resize duration/start.
- While dragging, only a **preview** is drawn; commit happens on mouse release.
- Right-click frame → Delete Frame → `removeFrame`.
- `+ Frame` → `addFrame` at current playhead on the active layer.

### Layers
- Click strip → select layer.
- Click **eye** → toggle visibility.
- Drag layer name → reorder (`moveLayer`).
- Right-click strip → Delete Layer (`removeLayer`, keeps at least one).
- `+ Layer` → `Canvas::addLayer`.

---

## Drag modes (`DragMode`)

`None` · `Scrub` · `SelectRange` · `MoveFrame` · `ResizeLeft` · `ResizeRight` · `ReorderLayer`

`hitTest()` decides move vs edge-resize from the click position (`m_edgeGrabPx`).

---

## Model calls (important)

| UI action | Model API |
|-----------|-----------|
| Add frame | `RasterLayer::addFrame` |
| Delete whole frame | `RasterLayer::removeFrame` |
| Delete selected tiles | `RasterLayer::deleteTimeRange` |
| Move / resize frame | `RasterLayer::updateFrame` |
| Add / remove / reorder / visibility | `Canvas::{addLayer,removeLayer,moveLayer,...}` + layer visibility |
| Scrub | `Canvas::setCurrentTime` (syncs all layers via `frameIndexAtTime`) |

---

## Implementation notes

- Almost everything under the toolbar is **manual `QPainter`** in `paintEvent` (not a Qt item view).
- Geometry helpers: `contentRect`, `layerStripRect`, `timelineRect`, `headerRect`, `frameBlockRect`, `timeFromX` / `xFromTime`.
- Keeps focus (`StrongFocus`) so arrow keys / delete work after clicking the timeline.
