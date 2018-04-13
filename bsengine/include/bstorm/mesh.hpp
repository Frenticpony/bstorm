#pragma once

#include <vector>
#include <memory>
#include <unordered_map>
#include <d3dx9.h>

namespace bstorm
{
struct MeshVertex
{
    MeshVertex() : x(0), y(0), z(0), nx(0), ny(0), nz(0), u(0), v(0) {}
    MeshVertex(float x, float y, float z, float nx, float ny, float nz, float u, float v) : x(x), y(y), z(z), nx(nx), ny(ny), nz(nz), u(u), v(v) {}
    float x, y, z;
    float nx, ny, nz;
    float u, v;
    static constexpr DWORD Format = D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1;
};

class Texture;
class MeshMaterial
{
public:
    MeshMaterial(float r, float g, float b, float a, float dif, float amb, float emi, const std::shared_ptr<Texture>& texture) :
        col({ r, g, b, a }), dif(dif), amb(amb), emi(emi), texture(texture)
    {
    }
    struct
    {
        float r;
        float g;
        float b;
        float a;
    } col;
    float dif;
    float amb;
    float emi;
    std::vector<MeshVertex> vertices;
    std::shared_ptr<Texture> texture;
};

class Mesh
{
public:
    Mesh(const std::wstring& path);
    ~Mesh();
    std::vector<MeshMaterial> materials;
    const std::wstring& getPath() const;
private:
    std::wstring path;
};

struct SourcePos;
class TextureCache;
struct Mqo;
std::shared_ptr<Mesh> mqoToMesh(const Mqo& mqo, const std::shared_ptr<TextureCache>& textureCache, const std::shared_ptr<SourcePos>& srcPos);

class FileLoader;
class TextureCache;
class MeshCache
{
public:
    MeshCache();
    void setLoader(const std::shared_ptr<FileLoader>& loader);
    std::shared_ptr<Mesh> load(const std::wstring& path, const std::shared_ptr<TextureCache>& textureCache, const std::shared_ptr<SourcePos>& srcPos);
    void releaseUnusedMesh();
    // backdoor
    template <typename T>
    void backDoor() const {}
private:
    std::unordered_map<std::wstring, std::shared_ptr<Mesh>> meshMap;
    std::shared_ptr<FileLoader> loader;
};
}