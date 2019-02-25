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
    return D3DXVECTOR3(vec.x, vec.y, -vec.z); // �E��n���獶��n�ɕϊ�����̂�z���W�𔽓]
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

    // �e�N�X�`�����v�����[�h
    for (const auto& mqoMat : mqo.materials)
    {
        textureStore->LoadInThread(ConcatPath(mqoDir, mqoMat.tex));
    }

    materials.reserve(mqo.materials.size());

    // �ގ������R�s�[
    for (const auto& mqoMat : mqo.materials)
    {
        auto& texture = textureStore->Load(ConcatPath(mqoDir, mqoMat.tex));
        materials.emplace_back(mqoMat.col.r, mqoMat.col.g, mqoMat.col.b, mqoMat.col.a, mqoMat.dif, mqoMat.amb, mqoMat.emi, texture);
    }

    // �ގ��ʂɒ��_�z��𐶐�
    for (const auto& obj : mqo.objects)
    {
        // �e���_�̖@�����v�Z
        std::vector<D3DXVECTOR3> vertexNormals(obj.vertices.size(), D3DXVECTOR3(0.0f, 0.0f, 0.0f));
        // �ʂ̖@���̓L���b�V�����Ă���
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

        // �@���̐��K��
        for (auto& normal : vertexNormals)
        {
            // �ʂɎg���ĂȂ����_�͖���
            if (normal.x == 0.0f && normal.y == 0.0f && normal.z == 0.0f) continue;
            D3DXVec3Normalize(&normal, &normal);
        }

        // ���W�A����
        const float facet = D3DXToRadian(obj.facet);

        // ���_����
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
                    // �ps��facet�ȉ��Ȃ�ʖ@���𒸓_�̖@���ɐݒ�
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