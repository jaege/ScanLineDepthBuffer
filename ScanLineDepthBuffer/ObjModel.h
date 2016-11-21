#pragma once

#include <cfloat>
#include <string>
#include <vector>
#include <Windows.h>

//template<class T> 
//const T& min(const T& a, const T& b)
//{
//    return (b < a) ? b : a;
//}

template <typename T>
struct Triple
{
    T x1;
    T x2;
    T x3;
};

using Position = Triple<double>;

struct FaceNode
{
    int v;
    int vt;
    int vn;
};

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
                    zmin(DBL_MAX), zmax(DBL_MIN) { }
};

class ObjModel
{
public:
    void LoadFromObjFile(const std::wstring &filePath);
    RECT GetBoundingRect() const { return m_boundingRect; }
    void Init();

    // width: screen window width in pixel
    // height: screen window height in pixel
    // scaleFactor: object scale factor on x and y axes
    void ScaleModel(LONG width, LONG height, double scaleFactor = 1.0);

private:
    std::wstring m_filePath;

    // right-hand coordinate system, sequentially numbered, starting with 1
    // This numbering sequence continues even when vertex data is separated by
    // other data.
    std::vector<Position> m_vertices;  // geometric vertices
    std::vector<Position> m_vertexNormals;

    std::vector<std::vector<FaceNode>> m_faces;

    BoundingBox m_box;
    RECT m_boundingRect{ };

};
