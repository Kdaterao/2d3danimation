#include <BrushCatalog.h>
#include <tinyxml2.h>
#include <cstring>
#include <iostream>

std::vector<BrushEntry> BrushCatalog::s_entries;

Brush::RasterTypes BrushCatalog::parseType(const char* typeStr) {
    if (!typeStr) {
        return Brush::RasterTypes::NONE;
    }
    if (std::strcmp(typeStr, "BRUSH_BGRM32") == 0) {
        return Brush::RasterTypes::BRUSH_BGRM32;
    }
    if (std::strcmp(typeStr, "MPL_BRUSH") == 0) {
        return Brush::RasterTypes::MPL_BRUSH;
    }
    return Brush::RasterTypes::NONE;
}

bool BrushCatalog::loadFromFile(const QString& path) {
    s_entries.clear();

    tinyxml2::XMLDocument doc;
    if (doc.LoadFile(path.toUtf8().constData()) != tinyxml2::XML_SUCCESS) {
        std::cerr << "BrushCatalog: failed to load " << path.toStdString() << std::endl;
        return false;
    }

    tinyxml2::XMLElement* root = doc.FirstChildElement("brushCatalog");
    if (!root) {
        std::cerr << "BrushCatalog: missing <brushCatalog> root element" << std::endl;
        return false;
    }

    for (tinyxml2::XMLElement* brushEl = root->FirstChildElement("brush");
         brushEl;
         brushEl = brushEl->NextSiblingElement("brush")) {

        const char* id = brushEl->Attribute("id");
        const char* name = brushEl->Attribute("name");
        const char* type = brushEl->Attribute("type");

        if (!id || !name || !type) {
            std::cerr << "BrushCatalog: skipping brush with missing id/name/type" << std::endl;
            continue;
        }

        BrushEntry entry;
        entry.id = QString::fromUtf8(id);
        entry.name = QString::fromUtf8(name);
        entry.type = parseType(type);

        if (entry.type == Brush::RasterTypes::NONE) {
            std::cerr << "BrushCatalog: unknown type \"" << type << "\" for brush \""
                      << id << "\"" << std::endl;
            continue;
        }

        int defaultSize = 5;
        brushEl->QueryIntAttribute("defaultSize", &defaultSize);
        entry.defaultSize = defaultSize;

        if (const char* brushFile = brushEl->Attribute("brushFile")) {
            entry.brushFile = QString::fromUtf8(brushFile);
        }
        if (const char* preview = brushEl->Attribute("preview")) {
            entry.preview = QString::fromUtf8(preview);
        }

        s_entries.push_back(std::move(entry));
    }

    return !s_entries.empty();
}

const std::vector<BrushEntry>& BrushCatalog::entries() {
    return s_entries;
}
