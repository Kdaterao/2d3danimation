#ifndef BRUSHCATALOG_H
#define BRUSHCATALOG_H

#include <Brush.h>
#include <QString>
#include <vector>

struct BrushEntry {
    QString id;
    QString name;
    Brush::RasterTypes type = Brush::RasterTypes::NONE;
    int defaultSize = 5;
    QString brushFile;
    QString preview;
};

class BrushCatalog {
public:
    static bool loadFromFile(const QString& path);
    static const std::vector<BrushEntry>& entries();

private:
    static std::vector<BrushEntry> s_entries;
    static Brush::RasterTypes parseType(const char* typeStr);
};

#endif
