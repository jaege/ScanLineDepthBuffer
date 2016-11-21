#include <string>
#include <fstream>
#include <sstream>
#include "ObjModel.h"
#include "DebugPrint.h"

void ObjModel::LoadFromObjFile(const std::wstring & filePath)
{
    /* NOTE(jaege): Only polygonal objects are partially supported, free-form
     *              objects are not supported.
     *
     * File format reference: http://paulbourke.net/dataformats/obj/
     *
     * Supported keyword (in parentheses):
     *
     * TODO(jaege): Plan to support keyword.
     *     geometric vertices (v)
     *     vertex normals (vn)
     *     face (f)
     *     group name (g)
     */ 

    m_filePath = filePath;

    std::ifstream fileStream(m_filePath);

    if (!fileStream.is_open())
    {
        DebugPrint(L"[ObjModel::LoadFromObjFile] Fail to open file: %s",
                   m_filePath);
        std::abort();
    }

    std::string line;

    Position pos{ };
    m_vertices.push_back(pos);
    m_vertexNormals.push_back(pos);

    while (std::getline(fileStream, line))
    {
        std::istringstream iss(line);
        std::string keyword, faceStrBuffer;
        iss >> keyword;

        switch (keyword[0])
        {
        case 'v':
            iss >> pos.x1 >> pos.x2 >> pos.x3;
            switch (keyword[1])
            {
            case '\0':
                // v x y z w
                // w is ignored.
                if (box.xmin > pos.x1) box.xmin = pos.x1;
                if (box.xmax < pos.x1) box.xmax = pos.x1;
                if (box.ymin > pos.x2) box.ymin = pos.x2;
                if (box.ymax < pos.x2) box.ymax = pos.x2;
                if (box.zmin > pos.x3) box.zmin = pos.x3;
                if (box.zmax < pos.x3) box.zmax = pos.x3;
                m_vertices.push_back(pos);
                break;
            case 'n':
                // vn i j k
                m_vertexNormals.push_back(pos);
                break;
            default:
                // vp and vt are ignored.
                break;
            }
            break;

        case 'f':
            // f  v1/vt1/vn1   v2/vt2/vn2   v3/vt3/vn3 ...

            // Face elements use surface normals to indicate their orientation. If
            // vertices are ordered counterclockwise around the face, both the
            // face and the normal will point toward the viewer. If the vertex
            // ordering is clockwise, both will point away from the viewer. If
            // vertex normals are assigned, they should point in the general
            // direction of the surface normal, otherwise unpredictable results
            // may occur.
            {
                std::vector<FaceNode> face;
                int v, vt, vn;  // 0 means not present in file.
                while (iss >> faceStrBuffer)
                {
                    std::istringstream fsb(faceStrBuffer);
                    faceStrBuffer = "";
                    std::getline(fsb, faceStrBuffer, '/');
                    v = faceStrBuffer == "" ? 0 : std::stoi(faceStrBuffer);
                    faceStrBuffer = "";
                    std::getline(fsb, faceStrBuffer, '/');
                    vt = faceStrBuffer == "" ? 0 : std::stoi(faceStrBuffer);
                    faceStrBuffer = "";
                    std::getline(fsb, faceStrBuffer, '/');
                    vn = faceStrBuffer == "" ? 0 : std::stoi(faceStrBuffer);
                    face.push_back({v, vt, vn});
                }
                m_faces.push_back(face);
            }
            break;
            
        case 'g':
            // Ignored.

            // All grouping statements are state-setting.  This means that once
            // a group statement is set, it alpplies to all elements that follow
            // until the next group statement.

            break;

        default:
            // Ignore other cases.
            break;
        }
    }

    // fileStream goes out of scope
}

void ObjModel::Init()
{
    // TODO(jaege): initialize data structures for z-buffer algorithm
}
