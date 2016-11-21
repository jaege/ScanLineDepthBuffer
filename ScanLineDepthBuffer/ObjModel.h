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

#include <string>
#include <vector>

class ObjModel
{
public:
    void LoadFromObjFile(const std::wstring &filePath);

private:
    std::wstring m_filePath;

    // right-hand coordinate system, sequentially numbered, starting with 1
    // This numbering sequence continues even when vertex data is separated by
    // other data.
    std::vector<Position> m_vertices;  // geometric vertices
    std::vector<Position> m_vertexNormals;

    std::vector<std::vector<FaceNode>> m_faces;

};
