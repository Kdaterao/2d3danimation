#ifndef PIPELINE_LOGGER_H
#define PIPELINE_LOGGER_H

#include <chrono>
#include <array>
#include <iostream>
#include <iomanip>
#include <cstdint>
#include <vector>

// Flip this to PipelineLogMode::Off to disable all timing (scopes become no-ops).
enum class PipelineLogMode {
    Off,
    On
};

inline constexpr PipelineLogMode kPipelineLogMode = PipelineLogMode::On; // On, Off

enum class PipelineStage {
    Frame = 0,
    Idle,
    MousePress,
    MouseMove,
    MouseRelease,
    EnsureFrame,
    UpdateCanvas,
    Bezier,
    DrawBrush,
    PaintEvent,
    PaintGL,
    BatchBuild,
    TileStack,
    GetTile,
    PreviewLookup,
    Onion,
    PaintRaster,
    AtlasPack,
    GlGetError,
    TexGen,
    TexAlloc,
    TexUpload,
    InstanceUpload,
    GlState,
    ShaderBind,
    GlDraw,
    GpuSync,
    TexDelete,
    UnmarkDirty,
    Commit,
    FlushBuffers,
    ClearBuffers,
    Count
};

inline const char* pipelineStageName(PipelineStage stage) {
    switch (stage) {
        case PipelineStage::Frame:          return "frame";
        case PipelineStage::Idle:           return "idle";
        case PipelineStage::MousePress:     return "mousePress";
        case PipelineStage::MouseMove:      return "mouseMove";
        case PipelineStage::MouseRelease:   return "mouseRelease";
        case PipelineStage::EnsureFrame:    return "ensureFrame";
        case PipelineStage::UpdateCanvas:   return "updateCanvas";
        case PipelineStage::Bezier:         return "bezier";
        case PipelineStage::DrawBrush:      return "drawBrush";
        case PipelineStage::PaintEvent:     return "paintEvent";
        case PipelineStage::PaintGL:        return "paintGL";
        case PipelineStage::BatchBuild:     return "batchBuild";
        case PipelineStage::TileStack:      return "tileStack";
        case PipelineStage::GetTile:        return "getTile";
        case PipelineStage::PreviewLookup:  return "previewLookup";
        case PipelineStage::Onion:          return "onion";
        case PipelineStage::PaintRaster:    return "paintRaster";
        case PipelineStage::AtlasPack:      return "atlasPack";
        case PipelineStage::GlGetError:     return "glGetError";
        case PipelineStage::TexGen:         return "texGen";
        case PipelineStage::TexAlloc:       return "texAlloc";
        case PipelineStage::TexUpload:      return "texUpload";
        case PipelineStage::InstanceUpload: return "instanceUpload";
        case PipelineStage::GlState:        return "glState";
        case PipelineStage::ShaderBind:     return "shaderBind";
        case PipelineStage::GlDraw:         return "glDraw";
        case PipelineStage::GpuSync:        return "gpuSync";
        case PipelineStage::TexDelete:      return "texDelete";
        case PipelineStage::UnmarkDirty:    return "unmarkDirty";
        case PipelineStage::Commit:         return "commit";
        case PipelineStage::FlushBuffers:   return "flushBuffers";
        case PipelineStage::ClearBuffers:   return "clearBuffers";
        default:                            return "unknown";
    }
}

class PipelineLogger {
    static constexpr int kCount = static_cast<int>(PipelineStage::Count);

    struct SamplePoint {
        float x = 0.0f;
        float y = 0.0f;
    };

    std::array<double, kCount> ms{};
    std::array<int, kCount> calls{};
    std::vector<SamplePoint> points;
    int tilesThisFrame = 0;
    int frameIndex = 0;
    int scopeDepth = 0;
    bool haveLastTopEnd = false;
    bool haveFrameT0 = false;
    std::chrono::steady_clock::time_point frameT0;
    std::chrono::steady_clock::time_point lastTopEnd;

    double stageMs(PipelineStage stage) const {
        return ms[static_cast<int>(stage)];
    }

    void resetAccumulators() {
        ms.fill(0.0);
        calls.fill(0);
        points.clear();
        tilesThisFrame = 0;
    }

public:
    static PipelineLogger& instance() {
        static PipelineLogger log;
        return log;
    }

    static bool enabled() {
        return kPipelineLogMode == PipelineLogMode::On;
    }

    void enterScope() {
        if (!enabled()) return;
        if (scopeDepth == 0 && haveLastTopEnd) {
            const auto now = std::chrono::steady_clock::now();
            add(PipelineStage::Idle, std::chrono::duration<double, std::milli>(now - lastTopEnd).count());
        }
        if (!haveFrameT0) {
            frameT0 = std::chrono::steady_clock::now();
            haveFrameT0 = true;
        }
        scopeDepth += 1;
    }

    void leaveScope() {
        if (!enabled()) return;
        scopeDepth -= 1;
        if (scopeDepth < 0) scopeDepth = 0;
        if (scopeDepth == 0) {
            lastTopEnd = std::chrono::steady_clock::now();
            haveLastTopEnd = true;
        }
    }

    void addPoint(float x, float y) {
        if (!enabled()) return;
        if (!points.empty()) {
            const SamplePoint& last = points.back();
            if (last.x == x && last.y == y) return;
        }
        points.push_back(SamplePoint{x, y});
    }

    void addSegment(float ax, float ay, float bx, float by) {
        addPoint(ax, ay);
        addPoint(bx, by);
    }

    void addPaintGLTiles(int tileCount) {
        if (!enabled()) return;
        tilesThisFrame = tileCount;
    }

    void add(PipelineStage stage, double elapsedMs) {
        if (!enabled()) return;
        const int i = static_cast<int>(stage);
        ms[i] += elapsedMs;
        calls[i] += 1;
    }

    void endFrame() {
        if (!enabled()) return;

        const auto now = std::chrono::steady_clock::now();
        if (!haveFrameT0) {
            frameT0 = now;
            haveFrameT0 = true;
        }

        const bool drew =
            calls[static_cast<int>(PipelineStage::DrawBrush)] > 0;

        if (drew) {
            const double frameMs = std::chrono::duration<double, std::milli>(now - frameT0).count();
            const double fps = (frameMs > 0.0) ? (1000.0 / frameMs) : 0.0;
            const double qtAroundGL =
                stageMs(PipelineStage::PaintEvent)
                - stageMs(PipelineStage::PaintGL)
                - stageMs(PipelineStage::Commit)
                - stageMs(PipelineStage::ClearBuffers)
                - stageMs(PipelineStage::FlushBuffers);

            ++frameIndex;

            std::cout << std::fixed << std::setprecision(3);
            std::cout << "[pipeline] ---- frame " << frameIndex
                      << "  " << frameMs << " ms  "
                      << std::setprecision(1) << fps << " fps"
                      << std::setprecision(3)
                      << "  " << tilesThisFrame << " tiles"
                      << "  " << points.size() << " pts ----\n";
            for (int i = 0; i < kCount; ++i) {
                if (calls[i] == 0) continue;
                if (static_cast<PipelineStage>(i) == PipelineStage::Frame) continue;
                std::cout << "  " << std::setw(16) << pipelineStageName(static_cast<PipelineStage>(i))
                          << "  " << std::setw(8) << ms[i] << " ms"
                          << "  (" << calls[i] << " calls)\n";
            }
            std::cout << "  " << std::setw(16) << "qtAroundGL"
                      << "  " << std::setw(8) << qtAroundGL << " ms"
                      << "  (paintEvent - paintGL: makeCurrent/present)\n";
            std::cout.flush();
        }

        resetAccumulators();
        frameT0 = now;
        haveFrameT0 = true;
        haveLastTopEnd = true;
        lastTopEnd = now;
        scopeDepth = 0;
    }
};

class PipelineScope {
    PipelineStage stage;
    std::chrono::steady_clock::time_point t0;
    bool active;

public:
    explicit PipelineScope(PipelineStage s)
        : stage(s)
        , active(PipelineLogger::enabled())
    {
        if (active) {
            PipelineLogger::instance().enterScope();
            t0 = std::chrono::steady_clock::now();
        }
    }

    ~PipelineScope() {
        if (!active) return;
        const auto t1 = std::chrono::steady_clock::now();
        const double ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
        PipelineLogger::instance().add(stage, ms);
        PipelineLogger::instance().leaveScope();
    }

    PipelineScope(const PipelineScope&) = delete;
    PipelineScope& operator=(const PipelineScope&) = delete;
};

#endif
