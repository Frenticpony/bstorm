#include <bstorm/mesh.hpp>

#include <bstorm/file_util.hpp>
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

static void CreateMeshMaterials(const Mqo & mqo, const std::shared_ptr<TextureStore>& textureStore, std::vector<MeshMaterial>& materials)
{
    auto mqoDir = GetParentPath(mqo.path);

    // テクスチャをプレロード
    for (const auto& mqoMat : mqo.materials)
    {
        textureStore->LoadInThread(ConcatPath(mqoDir, mqoMat.tex));
    }

    materials.reserve(mqo.materials.size());

    // 材質情報をコピー
    for (const auto& mqoMat : mqo.materials)
    {
        auto& texture = textureStore->Load(ConcatPath(mqoDir, mqoMat.tex));
        materials.emplace_back(mqoMat.col.r, mqoMat.col.g, mqoMat.col.b, mqoMat.col.a, mqoMat.dif, mqoMat.amb, mqoMat.emi, texture);
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
            auto& meshMat = materials[face.materialIndex];
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
}

Mesh::Mesh(const std::wstring& path, const std::shared_ptr<TextureStore>& textureStore, const std::shared_ptr<FileLoader>& fileLoader) :
    path_(path)
{
    if (auto mqo = ParseMqo(path, fileLoader))
    {
        CreateMeshMaterials(*mqo, textureStore, materials);
    } else
    {
        throw Log(LogLevel::LV_ERROR)
            .Msg("Failed to load mesh.")
            .Param(LogParam(LogParam::Tag::TEXT, path));
    }
}

Mesh::~Mesh()
{
    Logger::Write(std::move(
        Log(LogLevel::LV_INFO)
        .Msg("Released mesh.")
        .Param(LogParam(LogParam::Tag::MESH, path_))));
}


MeshStore::MeshStore(const std::shared_ptr<TextureStore>& textureStore, const std::shared_ptr<FileLoader>& fileLoader) :
    textureStore_(textureStore),
    fileLoader_(fileLoader)
{
}

const std::shared_ptr<Mesh>& MeshStore::Load(const std::wstring & path)
{
    const auto ext = GetLowerExt(path);
    if (ext != L".mqo")
    {
        throw Log(LogLevel::LV_ERROR)
            .Msg("File format not supported.")
            .Param(LogParam(LogParam::Tag::TEXT, path));
    }

    auto uniqPath = GetCanonicalPath(path);
    if (cacheStore_.Contains(uniqPath))
    {
        return cacheStore_.Get(uniqPath);
    }
    auto & mesh = cacheStore_.Load(uniqPath, uniqPath, textureStore_, fileLoader_);
    Logger::Write(std::move(
        Log(LogLevel::LV_INFO).Msg("Loaded mesh.")
        .Param(LogParam(LogParam::Tag::MESH, path))));
    return mesh;
}

void MeshStore::RemoveUnusedMesh()
{
    cacheStore_.RemoveUnused();
}
}