#include <bstorm/mesh.hpp>

#include <bstorm/util.hpp>
#include <bstorm/logger.hpp>
#include <bstorm/texture.hpp>
#include <bstorm/mqo.hpp>
#include <bstorm/file_loader.hpp>
#include <bstorm/parser.hpp>

#include <array>

namespace bstorm
{
static inline D3DXVECTOR3 ToD3DXVECTOR3(const MqoVec3& vec)
{
    return D3DXVECTOR3(vec.x, vec.y, -vec.z); // 右手系から左手系に変換するのでz座標を反転
}

static D3DXVECTOR3 CalcFaceNormal(const D3DXVECTOR3& a, const D3DXVECTOR3& b, const D3DXVECTOR3& c)
{
    D3DXVECTOR3 n;
    D3DXVECTOR3 ab = b - a;
    D3DXVECTOR3 ac = c - a;
    D3DXVec3Cross(&n, &ab, &ac);
    if (D3DXVec3Length(&n) != 0.0f)
    {
        D3DXVec3Normalize(&n, &n);
    }
    return n;
}

std::shared_ptr<Mesh> MqoToMesh(const Mqo & mqo, const std::shared_ptr<TextureCache>& textureCache, const std::shared_ptr<SourcePos>& srcPos)
{
    auto mesh = std::make_shared<Mesh>(mqo.path);

    // 材質情報をコピー
    for (const auto& mqoMat : mqo.materials)
    {
        auto texture = textureCache->load(concatPath(parentPath(mqo.path), mqoMat.tex), false, srcPos);
        mesh->materials.emplace_back(mqoMat.col.r, mqoMat.col.g, mqoMat.col.b, mqoMat.col.a, mqoMat.dif, mqoMat.amb, mqoMat.emi, texture);
    }

    // 材質別に頂点配列を生成
    for (const auto& obj : mqo.objects)
    {
        // 各頂点の法線を計算
        std::vector<D3DXVECTOR3> vertexNormals(obj.vertices.size(), D3DXVECTOR3(0.0f, 0.0f, 0.0f));
        // 面の法線はキャッシュしておく
        std::vector<std::vector<D3DXVECTOR3>> faceNormals(obj.faces.size());
        for (int faceIdx = 0; faceIdx < obj.faces.size(); faceIdx++)
        {
            const auto& face = obj.faces[faceIdx];
            for (int i = 0; i < face.vertexIndices.size() - 2; i++)
            {
                int vi2 = face.vertexIndices[0];
                int vi1 = face.vertexIndices[i + 1];
                int vi0 = face.vertexIndices[i + 2];
                const D3DXVECTOR3 v0 = ToD3DXVECTOR3(obj.vertices[vi0]);
                const D3DXVECTOR3 v1 = ToD3DXVECTOR3(obj.vertices[vi1]);
                const D3DXVECTOR3 v2 = ToD3DXVECTOR3(obj.vertices[vi2]);
                const auto& faceNormal = CalcFaceNormal(v0, v1, v2);
                vertexNormals[vi0] += faceNormal;
                vertexNormals[vi1] += faceNormal;
                vertexNormals[vi2] += faceNormal;
                faceNormals[faceIdx].push_back(faceNormal);
            }
        }

        // 法線の正規化
        for (auto& normal : vertexNormals)
        {
            // 面に使われてない頂点は無視
            if (normal.x == 0.0f && normal.y == 0.0f && normal.z == 0.0f) continue;
            D3DXVec3Normalize(&normal, &normal);
        }

        // ラジアンに
        const float facet = D3DXToRadian(obj.facet);

        // 頂点生成
        for (int faceIdx = 0; faceIdx < obj.faces.size(); faceIdx++)
        {
            const auto& face = obj.faces[faceIdx];
            auto& meshMat = mesh->materials[face.materialIndex];
            for (int i = 0; i < face.vertexIndices.size() - 2; i++)
            {
                const auto& faceNormal = faceNormals[faceIdx][i];
                for (auto j : std::array<int, 3>{ i + 2, i + 1, 0 })
                {
                    int vIdx = face.vertexIndices[j];
                    // 角sがfacet以下なら面法線を頂点の法線に設定
                    float s = acos(D3DXVec3Dot(&faceNormal, &vertexNormals[vIdx]));
                    const auto& pos = obj.vertices[vIdx];
                    const auto& nor = facet < s ? vertexNormals[vIdx] : faceNormal;
                    const auto& uv = j < face.uvs.size() ? face.uvs[j] : MqoVec2{ 0.0f, 0.0f };
                    meshMat.vertices.emplace_back(pos.x, pos.y, -pos.z, nor.x, nor.y, nor.z, uv.x, uv.y);
                }
            }
        }
    }
    return mesh;
}

Mesh::Mesh(const std::wstring & path) : path_(path)
{
}

Mesh::~Mesh()
{
    Logger::WriteLog(std::move(
        Log(Log::Level::LV_INFO)
        .setMessage("release mesh.")
        .setParam(Log::Param(Log::Param::Tag::MESH, path_))));
}

MeshCache::MeshCache() :
    loader_(std::make_shared<FileLoaderFromTextFile>())
{
}

void MeshCache::SetLoader(const std::shared_ptr<FileLoader>& loader)
{
    this->loader_ = loader;
}

std::shared_ptr<Mesh> MeshCache::Load(const std::wstring & path, const std::shared_ptr<TextureCache>& textureCache, const std::shared_ptr<SourcePos>& srcPos)
{
    const auto ext = getLowerExt(path);
    if (ext != L".mqo")
    {
        throw Log(Log::Level::LV_ERROR)
            .setMessage("this file format is not supported.")
            .setParam(Log::Param(Log::Param::Tag::TEXT, path))
            .addSourcePos(srcPos);
    }

    auto uniqPath = canonicalPath(path);
    auto it = meshMap_.find(uniqPath);
    if (it != meshMap_.end())
    {
        return it->second;
    } else
    {
        if (auto mqo = parseMqo(uniqPath, loader_))
        {
            auto mesh = MqoToMesh(*mqo, textureCache, srcPos);
            Logger::WriteLog(std::move(
                Log(Log::Level::LV_INFO).setMessage("load mesh.")
                .setParam(Log::Param(Log::Param::Tag::MESH, uniqPath))
                .addSourcePos(srcPos)));
            return meshMap_[uniqPath] = std::move(mesh);
        }
        throw Log(Log::Level::LV_ERROR)
            .setMessage("failed to load mesh.")
            .setParam(Log::Param(Log::Param::Tag::TEXT, path))
            .addSourcePos(srcPos);
    }
}

void MeshCache::ReleaseUnusedMesh()
{
    auto it = meshMap_.begin();
    while (it != meshMap_.end())
    {
        auto& mesh = it->second;
        if (mesh.use_count() <= 1)
        {
            meshMap_.erase(it++);
        } else ++it;
    }
}
}