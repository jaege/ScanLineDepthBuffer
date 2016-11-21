#include <string>
#include <fstream>
#include <sstream>
#include <cassert>
#include <Windows.h>
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
                if (m_box.xmin > pos.x1) m_box.xmin = pos.x1;
                if (m_box.xmax < pos.x1) m_box.xmax = pos.x1;
                if (m_box.ymin > pos.x2) m_box.ymin = pos.x2;
                if (m_box.ymax < pos.x2) m_box.ymax = pos.x2;
                if (m_box.zmin > pos.x3) m_box.zmin = pos.x3;
                if (m_box.zmax < pos.x3) m_box.zmax = pos.x3;
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
            // Negative indices are not supported.
            // Index 0 means not present in file.
            {
                std::vector<FaceNode> face;
                int v, vt, vn;  
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
                    assert(v >= 0 && vt >= 0 && vn >= 0);
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

void ObjModel::ScaleModel(LONG width, LONG height, double scaleFactor)
{
    /* Transform object from object coordinate to screen coordinate, and move
     * it to the center of the screen space.
     * This is an affine transformation, because the z axis is not scaled with
     * the same factor.
     *
     *     xScale = width / (xmax - xmin)
     *     yScale = height / (ymax - ymin)
     *     scale = min(xScale, yScale) * scaleFactor
     *
     *     x' = (x - (xmin + xmax) / 2) * scale + width / 2
     *     y' = ((ymin + ymax) / 2 - y) * scale + height / 2
     *     z' = (zmax - z) / (zmax - zmin)
     *
     * Old coordinate: (object space)
     *
     *        y ^
     *          |
     *          |
     *          |
     *          |
     *          +---------> x
     *         /
     *        /
     *       /
     *      /
     *     v z
     *
     * New coordinate: (screen space, x' and y' in pixel)
     *
     *               ^ z'
     *              /
     *             /
     *            /
     *           /
     *          +---------> x'
     *          |
     *          |
     *          |
     *          |
     *       y' v
     *
     */

    assert(scaleFactor > 0);
    double xScale = 1.0 * width / (m_box.xmax - m_box.xmin);
    double yScale = 1.0 * height / (m_box.ymax - m_box.ymin);
    // Ensure that the whole object can be seen in screen when scaleFactor <= 1.
    double scale = min(xScale, yScale) * scaleFactor;
    double zScale = 1.0 / (m_box.zmax - m_box.zmin);

    // Round down to the nearest integer
    m_boundingRect.left = (m_box.xmin - (m_box.xmin + m_box.xmax) / 2) *
        scale + width / 2;
    m_boundingRect.right = (m_box.xmax - (m_box.xmin + m_box.xmax) / 2) *
        scale + width / 2;
    m_boundingRect.top = ((m_box.ymin + m_box.ymax) / 2 - m_box.ymax) *
        scale + height / 2;
    m_boundingRect.bottom = ((m_box.ymin + m_box.ymax) / 2 - m_box.ymin) *
        scale + height / 2;

    //INT32 xnew = (x - (m_box.xmin + m_box.xmax) / 2) * scale + width / 2;
    //INT32 ynew = ((m_box.ymin + m_box.ymax) / 2 - y) * scale + height / 2;
}