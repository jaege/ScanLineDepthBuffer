#include <string>
#include <fstream>
#include <sstream>
#include <cassert>  // assert()
#include <cmath>  // std::lround() std::sqrt()
#include <utility>  // std::swap()
#include "ObjModel.h"
#include "FloatingPoint.h"
#include "DebugPrint.h"
#include "Transformation.h"
#include "Matrix.h"
#include "Tuple.h" // Vector4R

void ObjModel::LoadFromObjFile(const std::wstring & filePath)
{
    // NOTE(jaege): Only polygonal objects are partially supported, free-form
    //     objects are not supported.
    //
    // File format reference: http://paulbourke.net/dataformats/obj/
    //
    // Supported keyword (in parentheses):
    //     geometric vertices (v)
    //     vertex normals (vn)
    //     face (f)

    m_filePath = filePath;

    std::ifstream fileStream(m_filePath);

    if (!fileStream.is_open())
    {
        DebugPrint(L"[WRN] ObjModel::LoadFromObjFile : Fail to open file: %s",
                   m_filePath);
        std::abort();
    }

    std::string line;

    Position3R pos{ };
    m_vertices.push_back(pos);
    //m_vertexNormals.push_back(pos);

    while (std::getline(fileStream, line))
    {
        std::istringstream iss(line);
        std::string keyword, faceStrBuffer;
        iss >> keyword;

        switch (keyword[0])
        {
        case 'v':
            iss >> pos.x >> pos.y >> pos.z;
            switch (keyword[1])
            {
            case '\0':
                // v x y z w
                // w is ignored.
                if (m_box.xmin > pos.x) m_box.xmin = pos.x;
                if (m_box.xmax < pos.x) m_box.xmax = pos.x;
                if (m_box.ymin > pos.y) m_box.ymin = pos.y;
                if (m_box.ymax < pos.y) m_box.ymax = pos.y;
                if (m_box.zmin > pos.z) m_box.zmin = pos.z;
                if (m_box.zmax < pos.z) m_box.zmax = pos.z;
                m_vertices.push_back(pos);
                break;

            //case 'n':
            //    // vn i j k
            //    // vn is ignored.
            //    m_vertexNormals.push_back(pos);
            //    break;

            default:
                // vp and vt are ignored.
                break;
            }
            break;

        case 'f':
            // f  v1/vt1/vn1   v2/vt2/vn2   v3/vt3/vn3 ...
            // Negative indices are not supported.
            // vt and vn are optional.
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
                if (face.size() < 3)
                {
                    DebugPrint(L"[ERR] Face has less than three vertices.");
                    std::abort();
                }
                // Add the first vertex to the last, used for generating
                // edge table.
                face.push_back(face[0]);
                m_faces.push_back(face);
            }
            break;

        default:
            // Ignore other cases.
            break;
        }
    }

    DebugPrint(L"[INF] Model has %d vertices, %d faces.",
               m_vertices.size() - 1, m_faces.size());
}

void ObjModel::TransformModel(INT32 width, INT32 height, REAL scaleFactor,
                              REAL degreeX, REAL degreeY,
                              REAL shiftX, REAL shiftY)
{
    assert(scaleFactor > 0);
    REAL xScale = width / (m_box.xmax - m_box.xmin);
    REAL yScale = height / (m_box.ymax - m_box.ymin);

    // Ensure that the whole object can be seen in screen when scaleFactor <= 1.
    REAL scale = min(xScale, yScale) * scaleFactor;

    // The matrices are mulitplied in reverse order, i.e. last tranformation comes first.
    Matrix4x4R transform =
        Transformation::Translate(width / 2.0f + shiftX, height / 2.0f + shiftY, 0) *
        Transformation::Scale(scale) *
        Transformation::RotateAboutXAxis(degreeX) *
        Transformation::RotateAboutYAxis(degreeY) *
        Transformation::Symmetry(true, false, true) *
        Transformation::Translate(-(m_box.xmin + m_box.xmax) / 2,
                                  -(m_box.ymin + m_box.ymax) / 2,
                                  -(m_box.zmin + m_box.zmax) / 2);

    m_transformedVertices.clear();
    REAL left = REAL_MAX;
    REAL right = REAL_MIN;
    REAL top = REAL_MAX;
    REAL bottom = REAL_MIN;

    for (auto &v : m_vertices)
    {
        Vector4R newPos = transform * Vector4R{v.x, v.y, v.z, 1.0f};
        if (newPos.x < left) left = newPos.x;
        if (newPos.x > right) right = newPos.x;
        if (newPos.y < top) top = newPos.y;
        if (newPos.y > bottom) bottom = newPos.y;
        // TODO(jaege): find out why the following push_back call cannot be
        //     moved before comparing x and y.
        m_transformedVertices.push_back({newPos.x, newPos.y, newPos.z});
    }

    // Round inside the bounding rectangle.
    m_boundingRect.left = static_cast<LONG>(std::ceil(left));  // xmin'
    m_boundingRect.right = static_cast<LONG>(std::floor(right));  // xmax'
    m_boundingRect.top = static_cast<LONG>(std::ceil(top));  // ymin'
    m_boundingRect.bottom = static_cast<LONG>(std::floor(bottom));  // ymax'
}

template <typename T>
ObjModel::Plane<typename T::value_type> ObjModel::GetPlane(T p1, T p2, T p3)
{
    T::value_type a = (p2.y - p1.y) * (p3.z - p1.z) -
        (p3.y - p1.y) * (p2.z - p1.z);
    T::value_type b = (p3.x - p1.x) * (p2.z - p1.z) -
        (p2.x - p1.x) * (p3.z - p1.z);
    T::value_type c = (p2.x - p1.x) * (p3.y - p1.y) -
        (p3.x - p1.x) * (p2.y - p1.y);
    T::value_type d = -(p1.x * a + p1.y * b + p1.z * c);
    T::value_type n = std::sqrt(a * a + b * b + c * c);
    assert(n != 0);
    // Normalize (a, b, c) so that this is the normal vector of the face.
    return{a / n, b / n, c / n, d / n};
}

void ObjModel::InitTables()
{
    m_planes.clear();
    m_planes.resize(m_boundingRect.bottom - m_boundingRect.top + 1);

    m_edges.clear();
    m_edges.resize(m_boundingRect.bottom - m_boundingRect.top + 1);

    REAL lightN = 1 / std::sqrt(m_light.x * m_light.x + m_light.y * m_light.y +
                                m_light.z * m_light.z);

    for (int pid = 0; pid != m_faces.size(); ++pid)
    {
        const auto &face = m_faces[pid];
        PlaneNode pn;

        // Always use first 3 vertices to calculate the plane equation.
        // face[i].v is vertex id.
        pn.plane = GetPlane(m_transformedVertices[face[0].v],
                            m_transformedVertices[face[1].v],
                            m_transformedVertices[face[2].v]);
        // NOTE(jaege): Only plane face is supported, all vertices must in the
        //     same plane. The planse is assured to have at least 3 vertices.
        // BUG(jaege): check why assert fail when it shouldn't.
        //for (auto it = face.cbegin() + 3; it != face.cend(); ++it)
        //{
        //    const auto &p = m_transformedVertices[it->v];
        //    FloatingPoint<REAL> lhs(p.x * pn.plane.a + p.y * pn.plane.b +
        //                            p.z * pn.plane.c + pn.plane.d), rhs(0.0f);
        //    assert(lhs.AlmostEquals(rhs));
        //}

        // Ignore planes that parallel to z axis.
        FloatingPoint<REAL> lhs(pn.plane.c), rhs(0.0f);
        if (lhs.AlmostEquals(rhs)) { continue; }

        pn.id = pid;

        INT32 topyi = m_boundingRect.bottom + 1;
        INT32 btmyi = m_boundingRect.top - 1;
        for (int vid = 0; vid != face.size() - 1; ++vid)
        {
            const auto *ptop = &m_transformedVertices[face[vid].v];
            const auto *pbtm = &m_transformedVertices[face[vid + 1].v];

            if (ptop->y > pbtm->y)
            {
                auto p = ptop;
                ptop = pbtm;
                pbtm = p;
                //std::swap(ptop, pbtm);
            }

            INT32 ptopyi = static_cast<INT32>(std::floor(ptop->y + 1.0f));
            INT32 pbtmyi = static_cast<INT32>(std::floor(pbtm->y));

            if (topyi > ptopyi) topyi = ptopyi;
            if (btmyi < pbtmyi) btmyi = pbtmyi;

            // Ignore horizontal edges that stay between two adjcent scan-lines.
            // Some edges may not parallel to x axis, but their projections on
            // y axis is so small that not intersect with any scan-lines. They
            // should also be omited.
            // NOTE(jaege): When ptopyi==pbtmyi, the edge is still need to add
            //     to the edge tables, because it intersectes with scan-line.
            if (ptopyi > pbtmyi) { continue; }

            EdgeNode edge;

            edge.dx = (ptop->x - pbtm->x) / (ptop->y - pbtm->y);
            edge.planeId = pid;
            edge.xtop = ptop->x - edge.dx * (ptop->y - ptopyi);
            edge.diffy = pbtmyi - ptopyi + 1;
            m_edges[ptopyi - m_boundingRect.top].push_back(edge);
        }

        pn.diffy = btmyi - topyi + 1;
        // Some planes may not parallel to z axis, but their projections on
        // y axis is so small that not intersect with any scan-lines. They
        // should also be ignored.
        if (pn.diffy <= 0) { continue; }

        // Calculate color from the angle of face normal n, which is
        // n(a, b, c), and the light direction normal l(i, j, k). The smaller
        // the angle is, the light the color is.
        //
        //     n(a, b, c) dot l(i, j, k) = |n|*|l|*cos(theta)

        REAL costheta = (pn.plane.a * m_light.x + pn.plane.b * m_light.y +
                         pn.plane.c * m_light.z) * lightN;
        costheta = 0.5f - costheta / 2;

        pn.color.red = static_cast<UINT8>(std::round(m_planeColor.red * costheta));
        pn.color.green = static_cast<UINT8>(std::round(m_planeColor.green * costheta));
        pn.color.blue = static_cast<UINT8>(std::round(m_planeColor.blue * costheta));

        m_planes[topyi - m_boundingRect.top].push_back(pn);
    }
}

void ObjModel::GetBuffer(OffscreenBuffer &buffer, REAL scaleFactor,
                         REAL degreeX, REAL degreeY, REAL shiftX, REAL shiftY)
{
    TransformModel(buffer.GetWidth(), buffer.GetHeight(), scaleFactor,
                   degreeX, degreeY, shiftX, shiftY);

    InitTables();

    std::vector<PlaneNode> activePlanes;
    std::vector<ActiveEdgePairNode> activeEdgePairs;
    INT32 height = buffer.GetHeight();
    INT32 width = buffer.GetWidth();
    Color background{30, 30, 30};
    INT32 ybegin = m_boundingRect.top < 0 ? m_boundingRect.top : 0;
    for (INT32 y = ybegin; y < height; ++y)
    {
        std::vector<Color> frameBuffer(width, background);
        std::vector<REAL> depthBuffer(width, REAL_MAX);

        if (y >= m_boundingRect.top && y <= m_boundingRect.bottom)
        {
            // Add planes from m_planes to activePlanes.
            for (const auto &pl : m_planes[y - m_boundingRect.top])
            {
                activePlanes.push_back(pl);

                // Add edge pair of newly added plane to activeEdgePairs.
                std::vector<EdgeNode> edges;
                for (const auto &edge : m_edges[y - m_boundingRect.top])
                {
                    if (edge.planeId == pl.id) { edges.push_back(edge); }
                }
                // There should be even number of edges.
                assert(edges.size() % 2 == 0);
                if (edges.size() % 2 != 0)
                {
                    DebugPrint(L"[ERR] Find odd number of edge pairs of plane "
                               "#%d at y=%d.", pl.id, y);
                }
                // TODO(jaege): handle the concave polygon case, which
                //     means edges.size()=2n (n>1).

                if (edges.size() == 0)
                {
                    DebugPrint(L"[ERR] Can't find edge pair of plane "
                               "#%d at y=%d.", pl.id, y);
                    continue;
                }

                FloatingPoint<REAL> lhs(edges[0].xtop), rhs(edges[1].xtop);
                if (edges[0].xtop > edges[1].xtop ||
                    lhs.AlmostEquals(rhs) && edges[0].dx > edges[1].dx)
                {
                    std::swap(edges[0], edges[1]);
                }

                ActiveEdgePairNode epn;
                epn.l.x = edges[0].xtop;
                epn.l.dx = edges[0].dx;
                epn.l.diffy = edges[0].diffy;
                epn.r.x = edges[1].xtop;
                epn.r.dx = edges[1].dx;
                epn.r.diffy = edges[1].diffy;
                // NOTE(jaege): zl may lose some precision since y is rounded.
                // TODO(jaege): Test if this is ok.
                epn.zl = -(pl.plane.a * edges[0].xtop + pl.plane.b * y +
                           pl.plane.d) / pl.plane.c;
                epn.dzx = -pl.plane.a / pl.plane.c;
                epn.dzy = -pl.plane.b / pl.plane.c;
                epn.planeId = pl.id;

                activeEdgePairs.push_back(epn);
            }

            for (auto epn = activeEdgePairs.begin();
                 epn != activeEdgePairs.end(); )
            {
                INT32 xl = static_cast<INT32>(std::ceil(epn->l.x));
                INT32 xr = static_cast<INT32>(std::ceil(epn->r.x - 1.0f));
                REAL z = epn->zl;
                for (INT32 x = xl; x <= xr; ++x)
                {
                    // Ignore part of lines that go out of screen border.
                    if (x >= width)
                    {
                        DebugPrint(L"[WRN] edge of plane #%d at y=%d, x=%d "
                                   "posistion out of right boundary",
                                   epn->planeId, y, x);
                        // No need to update depth any more, since we are done
                        // with this scan line.
                        break;
                    }
                    else if (x < 0)
                    {
                        DebugPrint(L"[WRN] edge of plane #%d at y=%d, x=%d "
                                   "posistion out of left boundary",
                                   epn->planeId, y, x);
                    }
                    else if (z < depthBuffer[x])
                    {
                        // Update depthBuffer and frameBuffer.
                        depthBuffer[x] = z;
                        bool foundPlane = false;
                        for (const auto &pl : activePlanes)
                        {
                            if (pl.id == epn->planeId)
                            {
                                frameBuffer[x] = pl.color;
                                foundPlane = true;
                                break;
                            }
                        }
                        if (!foundPlane)
                        {
                            // BUG(jaege): find out why.
                            DebugPrint(L"[ERR] Can't find plane #%d for edge "
                                       "pair at x=%d, y=%d.",
                                       epn->planeId, x, y);
                        }
                    }
                    z += epn->dzx;
                }
                // Update activeEdgePairs.
                --epn->l.diffy;
                --epn->r.diffy;

                // BUG(jaege): when the polygon is concave, below may have bugs.

                // Replace finished edge/edge pairs in active EdgePairs.
                // TODO(jaege): The following if may be optimized. If current
                //     scan-line is the last of this plane, then we don't need
                //     to update epn.
                if (y - m_boundingRect.top + 1 >= 0 &&
                    static_cast<size_t>(y - m_boundingRect.top + 1) <
                    m_edges.size())
                {
                    if (epn->l.diffy == 0 && epn->r.diffy == 0)
                    {
                        std::vector<EdgeNode> edges;
                        for (const auto &edge :
                             m_edges[y - m_boundingRect.top + 1])
                        {
                            if (edge.planeId == epn->planeId)
                            {
                                edges.push_back(edge);
                            }
                        }
                        assert(edges.size() == 2 || edges.size() == 0);
                        if (edges.size() == 2)
                        {
                            FloatingPoint<REAL> lhs(edges[0].xtop),
                                                rhs(edges[1].xtop);
                            if (edges[0].xtop > edges[1].xtop ||
                                lhs.AlmostEquals(rhs) &&
                                edges[0].dx > edges[1].dx)
                            {
                                std::swap(edges[0], edges[1]);
                            }
                            epn->l.x = edges[0].xtop;
                            epn->l.dx = edges[0].dx;
                            epn->l.diffy = edges[0].diffy;
                            epn->r.x = edges[1].xtop;
                            epn->r.dx = edges[1].dx;
                            epn->r.diffy = edges[1].diffy;
                            // TODO(jaege): think if epn->zl need be updated.
                        }
                        else
                        {
                            epn = activeEdgePairs.erase(epn);
                            continue;
                        }
                    }
                    else if (epn->l.diffy == 0)
                    {
                        bool foundEdge = false;
                        for (const auto &edge :
                             m_edges[y - m_boundingRect.top + 1])
                        {
                            if (edge.planeId == epn->planeId)
                            {
                                epn->l.x = edge.xtop;
                                epn->l.dx = edge.dx;
                                epn->l.diffy = edge.diffy;
                                foundEdge = true;
                                break;
                            }
                        }
                        if (!foundEdge)
                        {
                            DebugPrint(L"[ERR] Can't find left edge of plane "
                                       "#%d at y=%d.", epn->planeId, y);
                            epn = activeEdgePairs.erase(epn);
                            continue;
                        }
                        epn->r.x += epn->r.dx;
                    }
                    else if (epn->r.diffy == 0)
                    {
                        bool foundEdge = false;
                        for (const auto &edge :
                             m_edges[y - m_boundingRect.top + 1])
                        {
                            if (edge.planeId == epn->planeId)
                            {
                                epn->r.x = edge.xtop;
                                epn->r.dx = edge.dx;
                                epn->r.diffy = edge.diffy;
                                foundEdge = true;
                                break;
                            }
                        }
                        if (!foundEdge)
                        {
                            DebugPrint(L"[ERR] Can't find right edge of plane "
                                       "#%d at y=%d.", epn->planeId, y);
                            epn = activeEdgePairs.erase(epn);
                            continue;
                        }
                        epn->l.x += epn->l.dx;
                    }
                    else
                    {
                        epn->l.x += epn->l.dx;
                        epn->r.x += epn->r.dx;
                    }
                }
                epn->zl += epn->dzx * epn->l.dx + epn->dzy;
                ++epn;
            }

            // Update activePlanes.
            for (auto it = activePlanes.begin(); it != activePlanes.end(); )
            {
                if (--it->diffy == 0) { it = activePlanes.erase(it); }
                else { ++it; }
            }
        }
        if (y >= 0) { buffer.SetRow(y, frameBuffer); }
    }

    // For debug purpose, draw all vertices.
    for (const auto & v : m_transformedVertices)
    {
        buffer.DebugDrawPoint(std::lround(v.x), std::lround(v.y), Color::GREEN);
    }
    // For debug purpose, draw bounding rectangle.
    buffer.DebugDrawRectangle(m_boundingRect, Color::BLUE);
}