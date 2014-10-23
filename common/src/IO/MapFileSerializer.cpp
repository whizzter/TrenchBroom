/*
 Copyright (C) 2010-2014 Kristian Duske
 
 This file is part of TrenchBroom.
 
 TrenchBroom is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 
 TrenchBroom is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with TrenchBroom. If not, see <http://www.gnu.org/licenses/>.
 */

#include "MapFileSerializer.h"
#include "Exceptions.h"
#include "IO/DiskFileSystem.h"
#include "IO/Path.h"
#include "Model/BrushFace.h"

namespace TrenchBroom {
    namespace IO {
        class StandardFileSerializer : public MapFileSerializer {
        private:
            String ShortFaceFormat;
            String LongFaceFormat;
        public:
            StandardFileSerializer(const IO::Path& path) :
            MapFileSerializer(path) {
                StringStream str;
                str <<
                "( %." << FloatPrecision << "g " <<
                "%." << FloatPrecision << "g " <<
                "%." << FloatPrecision << "g ) " <<
                "( %." << FloatPrecision << "g " <<
                "%." << FloatPrecision << "g " <<
                "%." << FloatPrecision << "g ) " <<
                "( %." << FloatPrecision << "g " <<
                "%." << FloatPrecision << "g " <<
                "%." << FloatPrecision << "g ) " <<
                "%s %.6g %.6g %.6g %.6g %.6g";
                ShortFaceFormat = str.str();
                
                str <<
                " %d %d %.6g";
                LongFaceFormat = str.str();
            }
        private:
            size_t doWriteBrushFace(FILE* stream, Model::BrushFace* face) {
                const String& textureName = face->textureName().empty() ? Model::BrushFace::NoTextureName : face->textureName();
                const Model::BrushFace::Points& points = face->points();
                
                if (face->hasSurfaceAttributes()) {
                    std::fprintf(stream, LongFaceFormat.c_str(),
                                 points[0].x(),
                                 points[0].y(),
                                 points[0].z(),
                                 points[1].x(),
                                 points[1].y(),
                                 points[1].z(),
                                 points[2].x(),
                                 points[2].y(),
                                 points[2].z(),
                                 textureName.c_str(),
                                 face->xOffset(),
                                 face->yOffset(),
                                 face->rotation(),
                                 face->xScale(),
                                 face->yScale(),
                                 face->surfaceContents(),
                                 face->surfaceFlags(),
                                 face->surfaceValue());
                } else {
                    std::fprintf(stream, ShortFaceFormat.c_str(),
                                 points[0].x(),
                                 points[0].y(),
                                 points[0].z(),
                                 points[1].x(),
                                 points[1].y(),
                                 points[1].z(),
                                 points[2].x(),
                                 points[2].y(),
                                 points[2].z(),
                                 textureName.c_str(),
                                 face->xOffset(),
                                 face->yOffset(),
                                 face->rotation(),
                                 face->xScale(),
                                 face->yScale());
                }
                std::fprintf(stream, "\n");
                return 1;
            }
        };
        
        class Hexen2FileSerializer : public MapFileSerializer {
        private:
            String FaceFormat;
        public:
            Hexen2FileSerializer(const IO::Path& path) :
            MapFileSerializer(path) {
                StringStream str;
                str <<
                "( %." << FloatPrecision << "g " <<
                "%." << FloatPrecision << "g " <<
                "%." << FloatPrecision << "g ) " <<
                "( %." << FloatPrecision << "g " <<
                "%." << FloatPrecision << "g " <<
                "%." << FloatPrecision << "g ) " <<
                "( %." << FloatPrecision << "g " <<
                "%." << FloatPrecision << "g " <<
                "%." << FloatPrecision << "g ) " <<
                "%s %.6g %.6g %.6g %.6g %.6g 0\n"; // the extra value is written here
                
                FaceFormat = str.str();
            }
        private:
            size_t doWriteBrushFace(FILE* stream, Model::BrushFace* face) {
                const String& textureName = face->textureName().empty() ? Model::BrushFace::NoTextureName : face->textureName();
                const Model::BrushFace::Points& points = face->points();
                
                std::fprintf(stream, FaceFormat.c_str(),
                             points[0].x(),
                             points[0].y(),
                             points[0].z(),
                             points[1].x(),
                             points[1].y(),
                             points[1].z(),
                             points[2].x(),
                             points[2].y(),
                             points[2].z(),
                             textureName.c_str(),
                             face->xOffset(),
                             face->yOffset(),
                             face->rotation(),
                             face->xScale(),
                             face->yScale());
                return 1;
            }
        };
        
        class ValveFileSerializer : public MapFileSerializer {
        private:
            String FaceFormat;
        public:
            ValveFileSerializer(const IO::Path& path) :
            MapFileSerializer(path) {
                StringStream str;
                str <<
                "( %." << FloatPrecision << "g " <<
                "%." << FloatPrecision << "g " <<
                "%." << FloatPrecision << "g ) " <<
                "( %." << FloatPrecision << "g " <<
                "%." << FloatPrecision << "g " <<
                "%." << FloatPrecision << "g ) " <<
                "( %." << FloatPrecision << "g " <<
                "%." << FloatPrecision << "g " <<
                "%." << FloatPrecision << "g ) " <<
                "%s " <<
                "[ %.6g %.6g %.6g %.6g ] " <<
                "[ %.6g %.6g %.6g %.6g ] " <<
                "%.6g %.6g %.6g\n";
                
                FaceFormat = str.str();
            }
        private:
            size_t doWriteBrushFace(FILE* stream, Model::BrushFace* face) {
                const String& textureName = face->textureName().empty() ? Model::BrushFace::NoTextureName : face->textureName();
                const Vec3 xAxis = face->textureXAxis();
                const Vec3 yAxis = face->textureYAxis();
                const Model::BrushFace::Points& points = face->points();
                
                std::fprintf(stream, FaceFormat.c_str(),
                             points[0].x(),
                             points[0].y(),
                             points[0].z(),
                             points[1].x(),
                             points[1].y(),
                             points[1].z(),
                             points[2].x(),
                             points[2].y(),
                             points[2].z(),
                             
                             textureName.c_str(),
                             
                             xAxis.x(),
                             xAxis.y(),
                             xAxis.z(),
                             face->xOffset(),
                             
                             yAxis.x(),
                             yAxis.y(),
                             yAxis.z(),
                             face->yOffset(),
                             
                             face->rotation(),
                             face->xScale(),
                             face->yScale());
                return 1;
            }
        };

        MapSerializer::Ptr MapFileSerializer::create(const Model::MapFormat::Type format, const IO::Path& path) {
            switch (format) {
                case Model::MapFormat::Standard:
                    return MapSerializer::Ptr(new StandardFileSerializer(path));
                case Model::MapFormat::Valve:
                    return MapSerializer::Ptr(new ValveFileSerializer(path));
                case Model::MapFormat::Hexen2:
                    return MapSerializer::Ptr(new Hexen2FileSerializer(path));
                case Model::MapFormat::Unknown:
                default:
                    throw new FileFormatException("Unknown map file format");
            }
        }
        
        MapFileSerializer::MapFileSerializer(const IO::Path& path) :
        m_line(1) {
            // ensure that the directory actually exists or is created if it doesn't
            const Path directoryPath = path.deleteLastComponent();
            WritableDiskFileSystem fs(directoryPath, true);
            
            m_stream = fopen(path.asString().c_str(), "w");
            if (m_stream == NULL)
                throw FileSystemException("Cannot open file: " + path.asString());
        }
        
        MapFileSerializer::~MapFileSerializer() {
            if (m_stream != NULL)
                fclose(m_stream);
        }
        
        void MapFileSerializer::doBeginEntity(const Model::Node* node) {
            m_startLineStack.push_back(m_line);
            std::fprintf(m_stream, "{\n");
            ++m_line;
        }
        
        void MapFileSerializer::doEndEntity(Model::Node* node) {
            std::fprintf(m_stream, "}\n");
            ++m_line;
            setFilePosition(node);
        }
        
        void MapFileSerializer::doEntityAttribute(const Model::EntityAttribute& attribute) {
            std::fprintf(m_stream, "\"%s\" \"%s\"\n", attribute.name().c_str(), attribute.value().c_str());
            ++m_line;
        }
        
        void MapFileSerializer::doBeginBrush(const Model::Brush* brush) {
            m_startLineStack.push_back(m_line);
            std::fprintf(m_stream, "{\n");
            ++m_line;
        }
        
        void MapFileSerializer::doEndBrush(Model::Brush* brush) {
            std::fprintf(m_stream, "}\n");
            ++m_line;
            setFilePosition(brush);
        }
        
        void MapFileSerializer::doBrushFace(Model::BrushFace* face) {
            const size_t lines = doWriteBrushFace(m_stream, face);
            face->setFilePosition(m_line, lines);
            m_line += lines;
        }
        
        void MapFileSerializer::setFilePosition(Model::Node* node) {
            const size_t start = startLine();
            node->setFilePosition(start, m_line - start);
        }

        size_t MapFileSerializer::startLine() {
            assert(!m_startLineStack.empty());
            const size_t result = m_startLineStack.back();
            m_startLineStack.pop_back();
            return result;
        }
    }
}