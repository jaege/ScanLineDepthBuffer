#include <string>
#include <fstream>
#include <sstream>
#include <cassert>  // assert()
#include <cmath>  // std::lround() std::sqrt()
#include <utility>  // std::swap()
#include <Windows.h>
#include "FloatingPoint.h"
#include "ObjModel.h"
#include "DebugPrint.h"
#include "OffscreenBuffer.h"

void ObjModel::LoadFromObjFile(const std::wstring & filePath)
{
    /* NOTE(jaege): Only polygonal objects are partially supported, free-form
     *              objects are not supported.
     *
     * File format reference: http://paulbourke.net/dataformats/obj/
     *
     * Supported keyword (in parentheses):
     *     geometric vertices (v)
     *     vertex normals (vn)
     *     face (f)
     */ 

    m_filePath = filePath;

    std::ifstream fileStream(m_filePath);

    if (!fileStream.is_open())
    {
        DebugPrint(L"[WRN] ObjModel::LoadFromObjFile : Fail to open file: %s",
                   m_filePath);
        std::abort();
    }

    std::string line;

    PositionF pos{ };
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
}

void ObjModel::SetModelScale(LONG width, LONG height, double scaleFactor)
{
    /* Transform object from object coordinate to screen coordinate, and move
     * it to the center of the screen space.
     *
     *     xScale = width / (xmax - xmin)
     *     yScale = height / (ymax - ymin)
     *     scale = min(xScale, yScale) * scaleFactor
     *
     *     x' = (x - (xmin + xmax) / 2) * scale + width / 2
     *     y' = ((ymin + ymax) / 2 - y) * scale + height / 2
     *     z' = (zmax - z) * scale
     *
     * Depth is the z axis normalized to [0, 1].
     *     depth = (zmax - z) / (zmax - zmin)
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
    //double zScale = 1.0 / (m_box.zmax - m_box.zmin);

    // Ensure that the whole object can be seen in screen when scaleFactor <= 1.
    m_scale = min(xScale, yScale) * scaleFactor;

    // Round to the nearest integer.
    m_boundingRect.left = Pixelate((m_box.xmin -
        (m_box.xmin + m_box.xmax) / 2) * m_scale + 1.0 * width / 2);  // xmin'
    m_boundingRect.right = Pixelate((m_box.xmax -
        (m_box.xmin + m_box.xmax) / 2) * m_scale + 1.0 * width / 2);  // xmax'
    m_boundingRect.top = Pixelate(((m_box.ymin + m_box.ymax) / 2 -
        m_box.ymax) * m_scale + 1.0 * height / 2);  // ymin'
    m_boundingRect.bottom = Pixelate(((m_box.ymin + m_box.ymax) / 2 -
        m_box.ymin) * m_scale + 1.0 * height / 2);  // ymax'

    m_scaledVertices.clear();
    for (auto &v : m_vertices)
    {
        //INT32 xnew = std::lround((v.x - (m_box.xmin + m_box.xmax) / 2) *
        //                         m_scale + 1.0 * width / 2);
        //INT32 ynew = std::lround(((m_box.ymin + m_box.ymax) / 2 - v.y) *
        //                         m_scale + 1.0 * height / 2);
        //INT32 znew = std::lround((m_box.zmax - v.z) * m_scale);
        double xnew = (v.x - (m_box.xmin + m_box.xmax) / 2) *
                                 m_scale + 1.0 * width / 2;
        double ynew = ((m_box.ymin + m_box.ymax) / 2 - v.y) *
                                 m_scale + 1.0 * height / 2;
        double znew = (m_box.zmax - v.z) * m_scale;
        m_scaledVertices.push_back({xnew, ynew, znew});

        //double depth = (m_box.zmax - v.z) * zScale;
        //m_scaledDepth.push_back(depth);
    }
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
    return {a / n, b / n, c / n, d / n};
}

void ObjModel::InitTables()
{
    m_planes.clear();
    m_planes.resize(m_boundingRect.bottom - m_boundingRect.top + 1);

    m_edges.clear();
    m_edges.resize(m_boundingRect.bottom - m_boundingRect.top + 1);

    double lightN = 1.0 / std::sqrt(m_light.x * m_light.x + m_light.y *
                                    m_light.y + m_light.z * m_light.z);

    for (int pid = 0; pid != m_faces.size(); ++pid)
    {
        const auto &face = m_faces[pid];
        PlaneNode pn;

        // Always use first 3 vertices to calculate the plane equation.
        // face[i].v is vertex id.
        pn.plane = GetPlane(m_scaledVertices[face[0].v],
                            m_scaledVertices[face[1].v],
                            m_scaledVertices[face[2].v]);

        // Ignore planes that parallel to z axis.
        Double lhs(pn.plane.c), rhs(0.0);
        if (lhs.AlmostEquals(rhs)) continue;

        pn.id = pid;

        double ytop = m_boundingRect.bottom + 1;
        double ybottom = m_boundingRect.top - 1;
        for (int vid = 0; vid != face.size() - 1; ++vid)
        {
            const auto &p1 = m_scaledVertices[face[vid].v];
            const auto &p2 = m_scaledVertices[face[vid + 1].v];

            // Ignore horizontal edge
            if (Pixelate(p1.y) == Pixelate(p2.y))
            {
                if (ytop > p1.y) ytop = p1.y;
                if (ybottom < p2.y) ybottom = p2.y;
                continue;
            }

            EdgeNode edge;

            edge.dx = (p1.x - p2.x) / (p1.y - p2.y);
            edge.planeId = pid;
            if (p1.y < p2.y)
            {
                edge.xtop = p1.x;
                edge.diffy = Pixelate(p2.y) - Pixelate(p1.y) + 1;
                if (ytop > p1.y) ytop = p1.y;
                if (ybottom < p2.y) ybottom = p2.y;
            }
            else
            {
                edge.xtop = p2.x;
                edge.diffy = Pixelate(p1.y) - Pixelate(p2.y) + 1;
                if (ytop > p2.y) ytop = p2.y;
                if (ybottom < p1.y) ybottom = p1.y;
            }
            if (p1.y < p2.y)
                m_edges[Pixelate(p1.y) - m_boundingRect.top].push_back(edge);
            else
                m_edges[Pixelate(p2.y) - m_boundingRect.top].push_back(edge);
        }

        pn.diffy = Pixelate(ybottom) - Pixelate(ytop) + 1;
        // The plane up to this line of code should not parallel to z axis.
        assert(pn.diffy != 0);

        // Calculate color from the angle of face normal n, which is
        // n(a, b, c), and the light direction normal l(i, j, k). The smaller
        // the angle is, the light the color is.
        //     n(a, b, c) dot l(i, j, k) = |n|*|l|*cos(theta)
        double costheta = (pn.plane.a * m_light.x + pn.plane.b * m_light.y +
                           pn.plane.c * m_light.z) * lightN;
        if (costheta < 0) costheta = -costheta;

        pn.color.red = (UINT8)std::lround(planeColor.red * costheta);
        pn.color.green = (UINT8)std::lround(planeColor.green * costheta);
        pn.color.blue = (UINT8)std::lround(planeColor.blue * costheta);

        m_planes[Pixelate(ytop) - m_boundingRect.top].push_back(pn);
    }
}

void ObjModel::SetBuffer(OffscreenBuffer &buffer)
{
    InitTables();

    std::vector<PlaneNode> activePlanes;
    std::vector<ActiveEdgePairNode> activeEdgePairs;
    INT32 height = buffer.GetHeight();
    INT32 width = buffer.GetWidth();
    Color background = BLACK;
    for (INT32 y = 0; y < height; ++y)
    {
        std::vector<Color> frameBuffer(width, background);
        std::vector<double> depthBuffer(width, DBL_MAX);

        if (y >= m_boundingRect.top && y <= m_boundingRect.bottom)
        {
            // Add planes from m_planes to activePlanes.
            if (m_planes[y - m_boundingRect.top].size() > 0)
            {
                for (const auto &pl : m_planes[y - m_boundingRect.top])
                {
                    activePlanes.push_back(pl);

                    // Add edge pair of newly added plane to activeEdgePairs.
                    std::vector<EdgeNode> edges;
                    for (const auto &edge : m_edges[y - m_boundingRect.top])
                    {
                        if (edge.planeId == pl.id)
                            edges.push_back(edge);
                    }
                    // There should be even number of edges.
                    assert(edges.size() % 2 == 0);
                    // BUG(jaege): teapot_wt.obj desk.obj flowers.obj bunny.obj etc.
                    //      vector edges.size() is 0, index out of range. Find out why.
                    if (edges.size() == 0)
                    {
                        DebugPrint(L"[ERR] Can't find edge pair of plane "
                                   "#%d at y=%d.", pl.id, y);
                        continue;
                    }

                    // TODO(jaege): handle the concave polygon case.
                    Double lhs(edges[0].xtop), rhs(edges[1].xtop);
                    if (edges[0].xtop > edges[1].xtop ||
                        lhs.AlmostEquals(rhs) && edges[0].dx > edges[1].dx)
                        std::swap(edges[0], edges[1]);

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
            }

            for (auto epn = activeEdgePairs.begin(); epn != activeEdgePairs.end(); )
            {
                // BUG(jaege): cube.obj cone.obj etc. edges are aliased.
                // BUG(jaege): torus.obj teapot.obj torusknot.obj strange lines.
                INT32 xl = Pixelate(epn->l.x);
                INT32 xr = Pixelate(epn->r.x);
                double z = epn->zl;
                for (INT32 x = xl; x <= xr; ++x)
                {
                    // BUG(jaege): flowers.obj
                    //     vector depthBuffer[] index out of upper range, negative x.
                    // Ignore part of lines that go out of screen border.
                    if (x >= width)
                    {
                        break;
                    }
                    if (x >= 0 && z < depthBuffer[x])
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
                // Replace finished edge/edge pairs in active EdgePairs.
                // BUG(jaege): when the polygon is concave, below may have bugs.
                if (epn->l.diffy == 0 && epn->r.diffy == 0)
                {
                    std::vector<EdgeNode> edges;
                    for (const auto &edge : m_edges[y - m_boundingRect.top])
                    {
                        if (edge.planeId == epn->planeId)
                            edges.push_back(edge);
                    }
                    assert(edges.size() == 2 || edges.size() == 0);
                    if (edges.size() == 2)
                    {
                        Double lhs(edges[0].xtop), rhs(edges[1].xtop);
                        if (edges[0].xtop > edges[1].xtop ||
                            lhs.AlmostEquals(rhs) && edges[0].dx > edges[1].dx)
                            std::swap(edges[0], edges[1]);
                        epn->l.x = edges[0].xtop;
                        epn->l.dx = edges[0].dx;
                        epn->l.diffy = edges[0].diffy;
                        epn->r.x = edges[1].xtop;
                        epn->r.dx = edges[1].dx;
                        epn->r.diffy = edges[1].diffy;
                        // TODO(jaege): think whether epn->zl need be updated.
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
                    for (const auto &edge : m_edges[y - m_boundingRect.top])
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
                        // BUG(jaege): find out why.
                        DebugPrint(L"[ERR] Can't find left edge of plane #%d "
                                   "at y=%d.", epn->planeId, y);
                    }
                }
                else if (epn->r.diffy == 0)
                {
                    bool foundEdge = false;
                    for (const auto &edge : m_edges[y - m_boundingRect.top])
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
                        // BUG(jaege): find out why.
                        DebugPrint(L"[ERR] Can't find right edge of plane #%d "
                                   "at y=%d.", epn->planeId, y);
                    }
                }
                epn->l.x += epn->l.dx;
                epn->r.x += epn->r.dx;
                epn->zl += epn->dzx * epn->l.dx + epn->dzy;
                ++epn;
            }

            // Update activePlanes.
            for (auto it = activePlanes.begin(); it != activePlanes.end(); )
            {
                if (--it->diffy == 0)
                {
                    it = activePlanes.erase(it);
                }
                else
                {
                    ++it;
                }
            }
        }
        buffer.SetRow(y, frameBuffer);
    }

    // For debug purpose, draw all vertices.
    for (const auto & v : m_scaledVertices)
        buffer.SetPixel(Pixelate(v.x), Pixelate(v.y), GREEN);
}

INT32 ObjModel::Pixelate(double pos)
{
    return std::lround(pos);
}