#pragma once

#include <bstorm/rect.hpp>
#include <bstorm/animation.hpp>
#include <bstorm/color_rgb.hpp>
#include <bstorm/nullable_shared_ptr.hpp>
#include <bstorm/file_loader.hpp>

#include <memory>
#include <map>
#include <string>
#include <unordered_set>
#include <unordered_map>

namespace bstorm
{
struct ShotCollision
{
    float r;
    float x;
    float y;
};

class Texture;
class ShotData
{
public:
    ShotData();
    int id;
    Rect<int> rect;
    int render;
    int alpha;
    Rect<int> delayRect;
    bool useDelayRect;
    ColorRGB delayColor;
    bool useDelayColor;
    int delayRender;
    float angularVelocity;
    float angularVelocityRandMin;
    float angularVelocityRandMax;
    bool useAngularVelocityRand;
    bool fixedAngle;
    std::vector<ShotCollision> collisions;
    AnimationData animationData;
    std::shared_ptr<Texture> texture;
};

class UserShotData
{
public:
    UserShotData() :
        delayRect(0, 0, 0, 0),
        delayColor(0xff, 0xff, 0xff)
    {
    }
    std::wstring path;
    std::wstring imagePath;
    Rect<int> delayRect;
    ColorRGB delayColor;
    std::unordered_map<int, ShotData> dataMap;
};

struct SourcePos;
class TextureStore;
class ShotDataTable
{
public:
    enum class Type
    {
        PLAYER,
        ENEMY
    };
    static const char* GetTypeName(Type type);
    ShotDataTable(Type type, const std::shared_ptr<TextureStore>& textureStore, const std::shared_ptr<FileLoader>& fileLoader);
    ~ShotDataTable();
    void Add(const std::shared_ptr<ShotData>& data);
    void Load(const std::wstring& path, const std::shared_ptr<SourcePos>& srcPos);
    void Reload(const std::wstring& path, const std::shared_ptr<SourcePos>& srcPos);
    bool IsLoaded(const std::wstring& path) const;
    NullableSharedPtr<ShotData> Get(int id) const;
    Type GetType() const;
    /* backdoor */
    template <typename T>
    void BackDoor() const {}
private:
    const Type type_;
    const std::shared_ptr<TextureStore> textureStore_;
    const std::shared_ptr<FileLoader> fileLoader_;
    std::unordered_set<std::wstring> alreadyLoadedPaths_;
    std::map<int, std::shared_ptr<ShotData>> table_;
};
}