#pragma once

#include <bstorm/type.hpp>
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
class TextureCache;
class ShotDataTable
{
public:
    enum class Type
    {
        PLAYER,
        ENEMY
    };
    static const char* getTypeName(Type type);
    ShotDataTable(Type type);
    ~ShotDataTable();
    void add(const std::shared_ptr<ShotData>& data);
    void load(const std::wstring& path, const std::shared_ptr<FileLoader>& loader, const std::shared_ptr<TextureCache>& textureCache, const std::shared_ptr<SourcePos>& srcPos);
    void reload(const std::wstring& path, const std::shared_ptr<FileLoader>& loader, const std::shared_ptr<TextureCache>& textureCache, const std::shared_ptr<SourcePos>& srcPos);
    bool isLoaded(const std::wstring& path) const;
    std::shared_ptr<ShotData> Get(int id) const;
    Type GetType() const;
    /* backdoor */
    template <typename T>
    void backDoor() const {}
private:
    Type type;
    std::unordered_set<std::wstring> loadedPaths;
    std::map<int, std::shared_ptr<ShotData>> table;
};
}