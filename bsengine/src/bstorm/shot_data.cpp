#include <bstorm/shot_data.hpp>

#include <bstorm/dnh_const.hpp>
#include <bstorm/parser.hpp>
#include <bstorm/file_util.hpp>
#include <bstorm/texture.hpp>
#include <bstorm/logger.hpp>

#include <algorithm>

namespace bstorm
{
ShotData::ShotData() :
    id(ID_INVALID),
    rect(0, 0, 0, 0),
    render(BLEND_ALPHA),
    filter(FILTER_LINEAR), //FP FILTER
    alpha(0xff),
    delayRect(0, 0, 0, 0),
    useDelayRect(false),
    delayColor(0, 0, 0),
    useDelayColor(false),
	fadeRect(0, 0, 0, 0),
	useFadeRect(false),
    delayRender(BLEND_ADD_ARGB),
	fadeRender(BLEND_ALPHA),
    angularVelocity(0),
    useAngularVelocityRand(false),
    angularVelocityRandMin(0),
    angularVelocityRandMax(0),
    fixedAngle(false),
	delayType(-1),
	fadeType(-1),
	delayData{ 3.0f, 1.0f },
	fadeData{ 1, 1.0f, 0.0f, 255.0f, 255.0f, 1, 1.0f },
	useSelfDelayRect(true),
	useSelfFadeRect(true),
	useExFade(false)
{
}

ShotDataTable::ShotDataTable(Type type, const std::shared_ptr<TextureStore>& textureStore, const std::shared_ptr<FileLoader>& fileLoader) :
    type_(type),
    textureStore_(textureStore),
    fileLoader_(fileLoader)
{
}

ShotDataTable::~ShotDataTable()
{
}

void ShotDataTable::Add(const std::shared_ptr<ShotData>& data)
{
    if (data->texture)
    {
        table_.emplace(data->id, data);
    }
}

const char * ShotDataTable::GetTypeName(Type type)
{
    if (type == Type::PLAYER) return "player";
    if (type == Type::ENEMY) return "enemy";
    return "unknown";
}

static LogParam::Tag getElemTag(ShotDataTable::Type type)
{
    if (type == ShotDataTable::Type::PLAYER) return LogParam::Tag::PLAYER_SHOT_DATA;
    if (type == ShotDataTable::Type::ENEMY) return LogParam::Tag::ENEMY_SHOT_DATA;
    return LogParam::Tag::TEXT;
}

void ShotDataTable::Load(const std::wstring & path, const std::shared_ptr<SourcePos>& srcPos)
{
    if (IsLoaded(path))
    {
        Logger::Write(std::move(
            Log(LogLevel::LV_WARN)
            .Msg(std::string(GetTypeName(type_)) + " shot data already loaded.")
            .Param(LogParam(getElemTag(type_), path))
            .AddSourcePos(srcPos)));
    } else
    {
        Reload(path, srcPos);
    }
}

void ShotDataTable::Reload(const std::wstring & path, const std::shared_ptr<SourcePos>& srcPos)
{
    std::wstring uniqPath = GetCanonicalPath(path);
    auto userShotData = ParseUserShotData(uniqPath, fileLoader_);
    auto& texture = textureStore_->Load(userShotData->imagePath);
    for (auto& entry : userShotData->dataMap)
    {
        auto& data = entry.second;

		float width = std::abs(data.rect.right - data.rect.left);
		float height = std::abs(data.rect.bottom - data.rect.top);

        if (!data.useDelayRect)
        {
            data.delayRect = userShotData->delayRect;
        }

        if (!data.useDelayColor)
        {
            data.delayColor = userShotData->delayColor;
        }

		switch ((int)data.delayType)
		{
			case 0: data.delayData.SetValues(2.5f, 0.5f); data.useSelfDelayRect = false; break; //Standard Delay
			case 1: data.delayData.SetValues(3.0f, 1.0f); data.useSelfDelayRect = false; break; //Medium Shot Delay
			case 2: data.delayData.SetValues(3.0f, 1.0f); data.useSelfDelayRect = true; break; //Large Shot Delay
			case 3: data.delayData.SetValues(4.0f, 1.0f); data.useSelfDelayRect = true; break; //Large Shot Delay 2
			default: data.delayData.SetValues(3.0f, 1.0f); data.useSelfDelayRect = false;
		}

		switch((int)data.fadeType)
		{
		case 0: data.fadeData.SetValues(0, 1.0f, 0.0f, 1.0f, 1.0f, 1, 1.0f); data.useSelfFadeRect = false; data.fadeRender = BLEND_ADD_ARGB; break; //Standard Fade
			case 1: data.fadeData.SetValues(1, 1.0f, 1.5f, 1.0f, 0.0f, 1, 1.0f); data.useSelfFadeRect = true; break; //Medium Shot Fade
			case 2: data.fadeData.SetValues(1, 1.0f, 0.0f, 1.0f, 0.0f, 0, 0.0f); data.useSelfFadeRect = true; break; //Large Shot Fade
			default: data.fadeData.SetValues(0, 1.0f, 0.0f, 1.0f, 0.0f, 0, 0.0f); data.useSelfFadeRect = true;
		}

		if ((int)data.fadeData.vF == 1)
		{
			data.useExFade = true;
		}
		//if (!data.useFadeRect)
		//{
		//	data.fadeRect = userShotData->fadeRect;
		//}

        if (data.collisions.empty())
        {
            float r = std::max(std::max(width, height) / 1.5f - 1.5f, 3.0f) / 2.0f;
            data.collisions.push_back({ r, 0.0f, 0.0f });
        }
        data.texture = texture;
        table_.emplace(data.id, std::make_shared<ShotData>(data));

		/*
		switch (data.delayType)
		{
			case 0: data.delayData.SetValues(2.75f, 0.65f, 0.0f, 0.0f); //Small
			case 1: data.delayData.SetValues(3.0f, 0.65f, 0.0f, 0.0f); //Default
			case 2: data.delayData.SetValues(4.0f, 1.0f, 0.0f, 0.0f); //Medium
			case 3: data.delayData.SetValues(5.0f, 1.0f, 0.0f, 0.0f); //Large
			case 4: data.delayData.SetValues(2.5f, 1.0f, 0.0f, 0.0f); //Same

			case 5: data.delayData.SetValues(2.75f, 0.65f, 0.0f, 0.0f); data.useSelfDelayRect = true; //Small
			case 6: data.delayData.SetValues(3.0f, 0.65f, 0.0f, 0.0f); data.useSelfDelayRect = true; //Default
			case 7: data.delayData.SetValues(4.0f, 1.0f, 0.0f, 0.0f); data.useSelfDelayRect = true; //Medium
			case 8: data.delayData.SetValues(5.0f, 1.0f, 0.0f, 0.0f); data.useSelfDelayRect = true; //Large
			case 9: data.delayData.SetValues(2.5f, 1.0f, 0.0f, 0.0f); data.useSelfDelayRect = true; //Same
			default: data.delayData.SetValues(2.5f, 1.0f, 0.0f, 0.0f); data.useSelfDelayRect = true; //Default
		};

		switch (data.fadeType)
		{
			case 0: data.fadeData.SetValues(1.0f, 1.8f, 0.5f, 0.0f); //Small
			case 1: data.fadeData.SetValues(1.0f, 1.8f, 0.5f, 0.0f); //Default
			case 2: data.fadeData.SetValues(1.0f, 1.8f, 0.5f, 0.0f); //Medium
			case 3: data.fadeData.SetValues(1.0f, 1.8f, 0.5f, 0.0f); //Large
			case 4: data.fadeData.SetValues(1.0f, 1.8f, 0.5f, 0.0f); //Same

			case 5: data.fadeData.SetValues(1.0f, 1.8f, 0.5f, 0.0f); data.useSelfFadeRect = true; //Small
			case 6: data.fadeData.SetValues(1.0f, 1.8f, 0.5f, 0.0f); data.useSelfFadeRect = true; //Default
			case 7: data.fadeData.SetValues(1.0f, 1.8f, 0.5f, 0.0f); data.useSelfFadeRect = true; //Medium
			case 8: data.fadeData.SetValues(1.0f, 1.8f, 0.5f, 0.0f); data.useSelfFadeRect = true; //Large
			case 9: data.fadeData.SetValues(1.0f, 1.8f, 0.5f, 0.0f); data.useSelfFadeRect = true; //Same
			default: data.fadeData.SetValues(1.0f, 1.8f, 0.5f, 0.0f); data.useSelfFadeRect = true; //Default
		};
		*/

    }
    alreadyLoadedPaths_.insert(uniqPath);
    Logger::Write(std::move(
        Log(LogLevel::LV_INFO)
        .Msg("Loaded " + std::string(GetTypeName(type_)) + " shot data.")
        .Param(LogParam(getElemTag(type_), path))
        .AddSourcePos(srcPos)));
}

bool ShotDataTable::IsLoaded(const std::wstring & path) const
{
    return alreadyLoadedPaths_.count(GetCanonicalPath(path)) != 0;
}

NullableSharedPtr<ShotData> ShotDataTable::Get(int id) const
{
    auto it = table_.find(id);
    if (it != table_.end())
    {
        return it->second;
    }
    return nullptr;
}

ShotDataTable::Type ShotDataTable::GetType() const
{
    return type_;
}
}