#pragma once

#include <cfloat>  // DBL_MIN DBL_MAX
#include <string>
#include <vector>
#include <Windows.h>
#include "Color.h"
#include "OffscreenBuffer.h"

template <typename T>
struct Triple
{
    using value_type = T;
    T x;
    T y;
    T z;
};

using PositionF = Triple<double>;
using PositionI = Triple<INT32>;
using VectorF = Triple<double>;

class ObjModel
{
public:
    void LoadFromObjFile(const std::wstring &filePath);

    RECT GetBoundingRect() const { return m_boundingRect; }

    // width: screen window width in pixel
    // height: screen window height in pixel
    // scaleFactor: object scale factor on x and y axes
    void SetModelScale(LONG width, LONG height, double scaleFactor = 1.0);

    void SetBuffer(OffscreenBuffer &buffer);

private:
    std::wstring m_filePath;

    // right-hand coordinate system, sequentially numbered, starting with 1
    // This numbering sequence continues even when vertex data is separated by
    // other data.
    std::vector<PositionF> m_vertices;  // geometric vertices
    std::vector<PositionF> m_vertexNormals;

    std::vector<PositionF> m_scaledVertices;

    double m_scale = 1.0;  // Model scale factor

    INT32 Pixelate(double pos);

    struct FaceNode
    {
        int v;
        int vt;
        int vn;
    };

    std::vector<std::vector<FaceNode>> m_faces;

    struct BoundingBox
    {
        double xmin;
        double xmax;
        double ymin;
        double ymax;
        double zmin;
        double zmax;

        BoundingBox() : xmin(DBL_MAX), xmax(DBL_MIN),
                        ymin(DBL_MAX), ymax(DBL_MIN),
                        zmin(DBL_MAX), zmax(DBL_MIN)
        { }
    };

    BoundingBox m_box;
    RECT m_boundingRect{ };

    template <typename T = double>
    struct Plane
    {
        T a;
        T b;
        T c;
        T d;
    };

    template <typename T>
    Plane<typename T::value_type> GetPlane(T p1, T p2, T p3);

    struct PlaneNode
    {
        Plane<> plane;
        UINT32 id;
        UINT32 diffy;
        Color color;
    };

    std::vector<std::vector<PlaneNode>> m_planes;

    struct EdgeNode
    {
        double xtop;
        double dx;
        UINT32 diffy;
        UINT32 planeId;
    };

    std::vector<std::vector<EdgeNode>> m_edges;

    struct ActiveEdgePairNode
    {
        struct Edge
        {
            double x;
            double dx;
            UINT32 diffy;
        } l, r;
        double zl;
        double dzx;
        double dzy;
        UINT32 planeId;
    };

    VectorF m_light{1, 1, 1};  // Light direction vector

    Color m_planeColor = WHITE;

    // Initialize plane tables and edge tables.
    void InitTables();

};
