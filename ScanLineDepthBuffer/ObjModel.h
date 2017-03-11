#pragma once

#include <string>
#include <vector>
#include <Windows.h>  // RECT
#include "Types.h"
#include "Color.h"
#include "Tuple.h"  // Vector3R
#include "OffscreenBuffer.h"

class ObjModel
{
public:
    void LoadFromObjFile(const std::wstring & filePath);

    // scaleFactor: object scale factor, must be positive, 1 means original size
    // degreeX: rotate about x axis of object, mesured in degree
    // degreeX: rotate about y axis of object, mesured in degree
    // shiftX: translate in x axis of screen, mesured in pixel
    // shiftY: translate in y axis of screen, mesured in pixel
    void GetBuffer(OffscreenBuffer & buffer, REAL scaleFactor,
                   REAL degreeX, REAL degreeY, REAL shiftX, REAL shiftY);

private:
    std::wstring m_filePath;

    // Right-hand coordinate system, sequentially numbered, index start from 1.
    // This number sequence continues even when vertex data is separated by
    // other data.
    std::vector<Position3R> m_vertices;  // geometric vertices

    //std::vector<Position3R> m_vertexNormals;

    std::vector<Position3R> m_transformedVertices;

    // width: buffer width in pixel
    // height: buffer height in pixel
    void TransformModel(INT32 width, INT32 height, REAL scaleFactor,
                        REAL degreeX, REAL degreeY, REAL shiftX, REAL shiftY);

    struct FaceNode
    {
        int v;
        int vt;
        int vn;
    };

    std::vector<std::vector<FaceNode>> m_faces;

    struct BoundingBox
    {
        REAL xmin; REAL xmax;
        REAL ymin; REAL ymax;
        REAL zmin; REAL zmax;

        BoundingBox() :
            xmin(REAL_MAX), xmax(REAL_MIN),
            ymin(REAL_MAX), ymax(REAL_MIN),
            zmin(REAL_MAX), zmax(REAL_MIN)
        { }
    };

    BoundingBox m_box;
    RECT m_boundingRect{ };

    template <typename T = REAL>
    struct Plane
    {
        T a;
        T b;
        T c;
        T d;
    };

    template <typename T>
    static Plane<typename T::value_type> GetPlane(T p1, T p2, T p3);

    struct PlaneNode
    {
        Plane<REAL> plane;
        UINT32 id;
        UINT32 diffy;
        Color color;
    };

    std::vector<std::vector<PlaneNode>> m_planes;

    struct EdgeNode
    {
        REAL xtop;
        REAL dx;
        UINT32 diffy;
        UINT32 planeId;
    };

    std::vector<std::vector<EdgeNode>> m_edges;

    struct ActiveEdgePairNode
    {
        struct Edge
        {
            REAL x;
            REAL dx;
            UINT32 diffy;
        } l, r;
        REAL zl;
        REAL dzx;
        REAL dzy;
        UINT32 planeId;
    };

    Vector3R m_light{1.0f, 1.5f, 1.0f};  // Light direction vector

    Color m_planeColor = Color::WHITE;

    // Initialize plane tables and edge tables.
    void InitTables();
};
