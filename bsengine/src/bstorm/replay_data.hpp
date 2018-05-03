#pragma once

#include <bstorm/type.hpp>
#include <bstorm/common_data_db.hpp>

#include <string>
#include <memory>
#include <unordered_map>

namespace bstorm
{
class TimePoint;
class ReplayData
{
public:
    ReplayData();

    // =======
    // 再生用
    // =======

    void Load(const std::wstring& filePath) noexcept(false);
    // リプレイの情報を取得する
    const std::unique_ptr<DnhValue>& GetReplayInfo(const CommonDataDB::DataKey& infoKey) const;
    // 全ステージの情報をステージのインデックスリスト順に取得する
    std::unique_ptr<DnhValue> GetStageInfoList(const CommonDataDB::DataKey& infoKey) const;
    // プレイ時のFPSを取得
    float GetFps(StageIndex stageIdx, FrameCount stageElapsedFrame) const;
    // プレイ時のキー入力を取得
    std::unordered_map<VirtualKey, KeyState> GetVirtualKeyStates(StageIndex stageIdx, FrameCount stageElapsedFrame) const;
    // ステージから共通データエリアを読み出す
    bool LoadCommonDataArea(StageIndex stageIdx, const CommonDataDB::DataAreaName& areaName, CommonDataDB& dst) const;

    // ======
    // 保存用
    // ======

    void Save(const std::wstring& filePath,
              const std::wstring& userName,
              GameScore totalScore,
              const std::wstring& playerName) noexcept(false);
    // ステージ情報記録開始
    void StartRecordingStageInfo(StageIndex stageIdx, GameScore stageStartScore, RandValue randSeed, const std::shared_ptr<TimePoint>& startTime);
    // ステージ情報記録終了
    void EndRecordingStageInfo(StageIndex stageIdx, GameScore stageLastScore, const std::shared_ptr<TimePoint>& endTime) noexcept(true);
    // フレーム毎のステージ情報を記録
    void RecordFrameStageInfo(StageIndex stageIdx, float fps, const std::unordered_map<VirtualKey, KeyState>& keyStates);
    // ポーズしたことを記録
    void RecordPause(StageIndex stageIdx);
    // ステージに共通データエリアを保存する
    bool SaveCommonDataArea(StageIndex stageIdx, const CommonDataDB::DataAreaName& areaName, const CommonDataDB& src);
    // コメントを設定
    void SetComment(const std::wstring& comment);

    // ========
    // キー一覧
    // ========

    // リプレイ情報名
    static constexpr wchar_t* FilePathInfoKey = L"REPLAY_FILE_PATH";
    static constexpr wchar_t* DateTimeInfoKey = L"DATE_TIME";
    static constexpr wchar_t* UserNameInfoKey = L"USER_NAME";
    static constexpr wchar_t* TotalScoreInfoKey = L"TOTAL_SCORE";
    static constexpr wchar_t* FpsAverageInfoKey = L"FPS_AVERAGE";
    static constexpr wchar_t* PlayerNameInfoKey = L"PLAYER_NAME";
    static constexpr wchar_t* StageIndexListInfoKey = L"STAGE_INDEX_LIST";
    static constexpr wchar_t* CommentInfoKey = L"COMMENT";
    static constexpr wchar_t* AreaNameListInfoKey = L"AREA_NAME_LIST";
    // ステージ情報名
    static constexpr wchar_t* StageStartScoreInfoKey = L"START_SCORE";
    static constexpr wchar_t* StageLastScoreInfoKey = L"LAST_SCORE";
    static constexpr wchar_t* StageFpsSumInfoKey = L"TOTAL_FPS_SUM";
    static constexpr wchar_t* StageFpsListInfoKey = L"FPS_LIST";
    static constexpr wchar_t* StageVirtualKeyStateListInfoKey = L"VKEY_STATE_LIST";
    static constexpr wchar_t* StageStartRandSeedInfoKey = L"START_RAND_SEED";
    static constexpr wchar_t* StageStartTimeInfoKey = L"START_TIME";
    static constexpr wchar_t* StageEndTimeInfoKey = L"END_TIME";
    static constexpr wchar_t* StagePauseCountInfoKey = L"PAUSE_COUNT";
private:
    // 各種データはすべてCommonDataDBに保存する, メモリが無駄なので中間データ構造は作らない。
    CommonDataDB data_;
    // 各データエリアは以下参照。
    // INFO: リプレイ全体の情報を記録するエリア
    static constexpr wchar_t* ReplayInfoAreaName = CommonDataDB::DefaultDataAreaName;
    // STAGE_<stageIdx>: ステージの情報を記録するエリア
    static std::wstring StageInfoAreaName(StageIndex stageIdx);
    // STAGE_<stageIdx>_<areaName>: ステージに保存された共通データ用エリア
    static std::wstring StageCommonDataAreaName(StageIndex stageIdx, const CommonDataDB::DataAreaName& areaName);
};
}