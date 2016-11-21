#pragma once

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

#include <cfloat>

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

#include <string>
#include <vector>

class ObjModel
{
public:
    void LoadFromObjFile(const std::wstring &filePath);
    void Init();

private:
    std::wstring m_filePath;

    // right-hand coordinate system, sequentially numbered, starting with 1
    // This numbering sequence continues even when vertex data is separated by
    // other data.
    std::vector<Position> m_vertices;  // geometric vertices
    std::vector<Position> m_vertexNormals;

    std::vector<std::vector<FaceNode>> m_faces;

    BoundingBox box;
};
