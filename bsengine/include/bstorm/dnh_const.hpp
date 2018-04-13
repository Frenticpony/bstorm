#pragma once

#undef VK_LEFT
#undef VK_RIGHT
#undef VK_UP
#undef VK_DOWN
#undef VK_CANCEL
#undef VK_PAUSE

namespace bstorm
{
// obj type
constexpr int OBJ_PRIMITIVE_2D = 1;
constexpr int OBJ_SPRITE_2D = 2;
constexpr int OBJ_SPRITE_LIST_2D = 3;
constexpr int OBJ_PRIMITIVE_3D = 4;
constexpr int OBJ_SPRITE_3D = 5;
constexpr int OBJ_TRAJECTORY_3D = 6;
constexpr int OBJ_SHADER = 7;
constexpr int OBJ_MESH = 8;
constexpr int OBJ_TEXT = 9;
constexpr int OBJ_SOUND = 10;
constexpr int OBJ_FILE_TEXT = 11;
constexpr int OBJ_FILE_BINARY = 12;
constexpr int OBJ_PLAYER = 100;
constexpr int OBJ_SPELL_MANAGE = 101;
constexpr int OBJ_SPELL = 102;
constexpr int OBJ_ENEMY = 103;
constexpr int OBJ_ENEMY_BOSS = 104;
constexpr int OBJ_ENEMY_BOSS_SCENE = 105;
constexpr int OBJ_SHOT = 106;
constexpr int OBJ_LOOSE_LASER = 107;
constexpr int OBJ_STRAIGHT_LASER = 108;
constexpr int OBJ_CURVE_LASER = 109;
constexpr int OBJ_ITEM = 110;

// item type
constexpr int ITEM_1UP = -65536;
constexpr int ITEM_1UP_S = -65535;
constexpr int ITEM_SPELL = -65534;
constexpr int ITEM_SPELL_S = -65533;
constexpr int ITEM_POWER = -65532;
constexpr int ITEM_POWER_S = -65531;
constexpr int ITEM_POINT = -65530;
constexpr int ITEM_POINT_S = -65529;
constexpr int ITEM_USER = 0;

// item move type
constexpr int ITEM_MOVE_DOWN = 2;
constexpr int ITEM_MOVE_TOPLAYER = 3;

// blend type
constexpr int BLEND_NONE = 0;
constexpr int BLEND_ALPHA = 1;
constexpr int BLEND_ADD_RGB = 2;
constexpr int BLEND_ADD_ARGB = 3;
constexpr int BLEND_MULTIPLY = 4;
constexpr int BLEND_SUBTRACT = 5;
constexpr int BLEND_SHADOW = 6;
constexpr int BLEND_INV_DESTRGB = 7;

// primitive type
constexpr int PRIMITIVE_TRIANGLEFAN = 6;
constexpr int PRIMITIVE_TRIANGLESTRIP = 5;
constexpr int PRIMITIVE_TRIANGLELIST = 4;
constexpr int PRIMITIVE_LINESTRIP = 3;
constexpr int PRIMITIVE_LINELIST = 2;
constexpr int PRIMITIVE_POINT_LIST = 1;

// ObjText border type
constexpr int BORDER_NONE = 0;
constexpr int BORDER_FULL = 1;
constexpr int BORDER_SHADOW = 2;

// Script type
constexpr int TYPE_SCRIPT_ALL = -1;
constexpr int TYPE_SCRIPT_PLAYER = 1;
constexpr int TYPE_SCRIPT_SINGLE = 2;
constexpr int TYPE_SCRIPT_PLURAL = 3;
constexpr int TYPE_SCRIPT_STAGE = 4;
constexpr int TYPE_SCRIPT_PACKAGE = 5;

// virtual key
constexpr int VK_LEFT = 0;
constexpr int VK_RIGHT = 1;
constexpr int VK_UP = 2;
constexpr int VK_DOWN = 3;
constexpr int VK_OK = 4;
constexpr int VK_CANCEL = 5;
constexpr int VK_SHOT = 6;
constexpr int VK_BOMB = 7;
constexpr int VK_SPELL = 7;
constexpr int VK_SLOWMOVE = 8;
constexpr int VK_USER1 = 9;
constexpr int VK_USER2 = 10;
constexpr int VK_PAUSE = 11;
constexpr int VK_USER_ID_STAGE = 256;
constexpr int VK_USER_ID_PLAYER = 512;

// key map
constexpr int KEY_INVALID = -1;
constexpr int KEY_ESCAPE = 1;
constexpr int KEY_1 = 2;
constexpr int KEY_2 = 3;
constexpr int KEY_3 = 4;
constexpr int KEY_4 = 5;
constexpr int KEY_5 = 6;
constexpr int KEY_6 = 7;
constexpr int KEY_7 = 8;
constexpr int KEY_8 = 9;
constexpr int KEY_9 = 10;
constexpr int KEY_0 = 11;
constexpr int KEY_MINUS = 12;
constexpr int KEY_EQUALS = 13;
constexpr int KEY_BACK = 14;
constexpr int KEY_TAB = 15;
constexpr int KEY_Q = 16;
constexpr int KEY_W = 17;
constexpr int KEY_E = 18;
constexpr int KEY_R = 19;
constexpr int KEY_T = 20;
constexpr int KEY_Y = 21;
constexpr int KEY_U = 22;
constexpr int KEY_I = 23;
constexpr int KEY_O = 24;
constexpr int KEY_P = 25;
constexpr int KEY_LBRACKET = 26;
constexpr int KEY_RBRACKET = 27;
constexpr int KEY_RETURN = 28;
constexpr int KEY_LCONTROL = 29;
constexpr int KEY_A = 30;
constexpr int KEY_S = 31;
constexpr int KEY_D = 32;
constexpr int KEY_F = 33;
constexpr int KEY_G = 34;
constexpr int KEY_H = 35;
constexpr int KEY_J = 36;
constexpr int KEY_K = 37;
constexpr int KEY_L = 38;
constexpr int KEY_SEMICOLON = 39;
constexpr int KEY_APOSTROPHE = 40;
constexpr int KEY_GRAVE = 41;
constexpr int KEY_LSHIFT = 42;
constexpr int KEY_BACKSLASH = 43;
constexpr int KEY_Z = 44;
constexpr int KEY_X = 45;
constexpr int KEY_C = 46;
constexpr int KEY_V = 47;
constexpr int KEY_B = 48;
constexpr int KEY_N = 49;
constexpr int KEY_M = 50;
constexpr int KEY_COMMA = 51;
constexpr int KEY_PERIOD = 52;
constexpr int KEY_SLASH = 53;
constexpr int KEY_RSHIFT = 54;
constexpr int KEY_MULTIPLY = 55;
constexpr int KEY_LMENU = 56;
constexpr int KEY_SPACE = 57;
constexpr int KEY_CAPITAL = 58;
constexpr int KEY_F1 = 59;
constexpr int KEY_F2 = 60;
constexpr int KEY_F3 = 61;
constexpr int KEY_F4 = 62;
constexpr int KEY_F5 = 63;
constexpr int KEY_F6 = 64;
constexpr int KEY_F7 = 65;
constexpr int KEY_F8 = 66;
constexpr int KEY_F9 = 67;
constexpr int KEY_F10 = 68;
constexpr int KEY_NUMLOCK = 69;
constexpr int KEY_SCROLL = 70;
constexpr int KEY_NUMPAD7 = 71;
constexpr int KEY_NUMPAD8 = 72;
constexpr int KEY_NUMPAD9 = 73;
constexpr int KEY_SUBTRACT = 74;
constexpr int KEY_NUMPAD4 = 75;
constexpr int KEY_NUMPAD5 = 76;
constexpr int KEY_NUMPAD6 = 77;
constexpr int KEY_ADD = 78;
constexpr int KEY_NUMPAD1 = 79;
constexpr int KEY_NUMPAD2 = 80;
constexpr int KEY_NUMPAD3 = 81;
constexpr int KEY_NUMPAD0 = 82;
constexpr int KEY_DECIMAL = 83;
constexpr int KEY_F11 = 87;
constexpr int KEY_F12 = 88;
constexpr int KEY_F13 = 100;
constexpr int KEY_F14 = 101;
constexpr int KEY_F15 = 102;
constexpr int KEY_KANA = 112;
constexpr int KEY_CONVERT = 121;
constexpr int KEY_NOCONVERT = 123;
constexpr int KEY_YEN = 125;
constexpr int KEY_NUMPADEQUALS = 141;
constexpr int KEY_CIRCUMFLEX = 144;
constexpr int KEY_AT = 145;
constexpr int KEY_COLON = 146;
constexpr int KEY_UNDERLINE = 147;
constexpr int KEY_KANJI = 148;
constexpr int KEY_STOP = 149;
constexpr int KEY_AX = 150;
constexpr int KEY_UNLABELED = 151;
constexpr int KEY_NUMPADENTER = 156;
constexpr int KEY_RCONTROL = 157;
constexpr int KEY_NUMPADCOMMA = 179;
constexpr int KEY_DIVIDE = 181;
constexpr int KEY_SYSRQ = 183;
constexpr int KEY_RMENU = 184;
constexpr int KEY_PAUSE = 197;
constexpr int KEY_HOME = 199;
constexpr int KEY_UP = 200;
constexpr int KEY_PRIOR = 201;
constexpr int KEY_LEFT = 203;
constexpr int KEY_RIGHT = 205;
constexpr int KEY_END = 207;
constexpr int KEY_DOWN = 208;
constexpr int KEY_NEXT = 209;
constexpr int KEY_INSERT = 210;
constexpr int KEY_DELETE = 211;
constexpr int KEY_LWIN = 219;
constexpr int KEY_RWIN = 220;
constexpr int KEY_APPS = 221;
constexpr int KEY_POWER = 222;
constexpr int KEY_SLEEP = 223;

// mouse button
constexpr int MOUSE_LEFT = 0;
constexpr int MOUSE_RIGHT = 1;
constexpr int MOUSE_MIDDLE = 2;

// key state
constexpr int KEY_FREE = 0;
constexpr int KEY_PUSH = 1;
constexpr int KEY_PULL = 2;
constexpr int KEY_HOLD = 3;

// ObjText alignment
constexpr int ALIGNMENT_LEFT = 0;
constexpr int ALIGNMENT_RIGHT = 1;
constexpr int ALIGNMENT_CENTER = 4;

// ObjSound
constexpr int SOUND_BGM = 0;
constexpr int SOUND_SE = 1;
constexpr int SOUND_VOICE = 2;

// Menu script result
constexpr int RESULT_CANCEL = 23;
constexpr int RESULT_END = 24;
constexpr int RESULT_RETRY = 25;
constexpr int RESULT_SAVE_REPLAY = 26;

// Replay control
constexpr int REPLAY_INDEX_ACTIVE = 0;
constexpr int REPLAY_INDEX_DIGIT_MIN = 1;
constexpr int REPLAY_FILE_PATH = 13;
constexpr int REPLAY_DATE_TIME = 14;
constexpr int REPLAY_USER_NAME = 15;
constexpr int REPLAY_TOTAL_SCORE = 16;
constexpr int REPLAY_FPS_AVERAGE = 17;
constexpr int REPLAY_PLAYER_NAME = 18;
constexpr int REPLAY_STAGE_INDEX_LIST = 19;
constexpr int REPLAY_STAGE_START_SCORE_LIST = 20;
constexpr int REPLAY_STAGE_LAST_SCORE_LIST = 21;
constexpr int REPLAY_COMMENT = 22;
constexpr int REPLAY_INDEX_DIGIT_MAX = 99;
constexpr int REPLAY_INDEX_USER = 100;

// Event
constexpr int EV_REQUEST_LIFE = 1;
constexpr int EV_REQUEST_TIMER = 2;
constexpr int EV_REQUEST_IS_SPELL = 3;
constexpr int EV_REQUEST_IS_LAST_SPELL = 4;
constexpr int EV_REQUEST_IS_DURABLE_SPELL = 5;
constexpr int EV_REQUEST_SPELL_SCORE = 6;
constexpr int EV_REQUEST_REPLAY_TARGET_COMMON_AREA = 7;
constexpr int EV_TIMEOUT = 8;
constexpr int EV_START_BOSS_SPELL = 9;
constexpr int EV_GAIN_SPELL = 10;
constexpr int EV_START_BOSS_STEP = 11;
constexpr int EV_END_BOSS_STEP = 12;
constexpr int EV_PLAYER_SHOOTDOWN = 13;
constexpr int EV_PLAYER_SPELL = 14;
constexpr int EV_PLAYER_REBIRTH = 15;
constexpr int EV_PAUSE_ENTER = 16;
constexpr int EV_PAUSE_LEAVE = 17;
constexpr int EV_DELETE_SHOT_IMMEDIATE = 18;
constexpr int EV_DELETE_SHOT_TO_ITEM = 19;
constexpr int EV_DELETE_SHOT_FADE = 20;
constexpr int EV_REQUEST_SPELL = 1000;
constexpr int EV_GRAZE = 1001;
constexpr int EV_HIT = 1002;
constexpr int EV_GET_ITEM = 1100;
constexpr int EV_USER_COUNT = 100000;
constexpr int EV_USER = 1000000;
constexpr int EV_USER_SYSTEM = 2000000;
constexpr int EV_USER_STAGE = 3000000;
constexpr int EV_USER_PLAYER = 4000000;
constexpr int EV_USER_PACKAGE = 5000000;

// Info
constexpr int INFO_SCRIPT_TYPE = 6;
constexpr int INFO_SCRIPT_PATH = 7;
constexpr int INFO_SCRIPT_ID = 8;
constexpr int INFO_SCRIPT_TITLE = 9;
constexpr int INFO_SCRIPT_TEXT = 10;
constexpr int INFO_SCRIPT_IMAGE = 11;
constexpr int INFO_SCRIPT_REPLAY_NAME = 12;
constexpr int INFO_LIFE = 111;
constexpr int INFO_DAMAGE_RATE_SHOT = 112;
constexpr int INFO_DAMAGE_RATE_SPELL = 113;
constexpr int INFO_SHOT_HIT_COUNT = 114;
constexpr int INFO_TIMER = 115;
constexpr int INFO_TIMERF = 116;
constexpr int INFO_ORGTIMERF = 117;
constexpr int INFO_IS_SPELL = 118;
constexpr int INFO_IS_LAST_SPELL = 119;
constexpr int INFO_IS_DURABLE_SPELL = 120;
constexpr int INFO_SPELL_SCORE = 121;
constexpr int INFO_REMAIN_STEP_COUNT = 122;
constexpr int INFO_ACTIVE_STEP_LIFE_COUNT = 123;
constexpr int INFO_ACTIVE_STEP_TOTAL_MAX_LIFE = 124;
constexpr int INFO_ACTIVE_STEP_TOTAL_LIFE = 125;
constexpr int INFO_ACTIVE_STEP_LIFE_RATE_LIST = 126;
constexpr int INFO_IS_LAST_STEP = 127;
constexpr int INFO_PLAYER_SHOOTDOWN_COUNT = 128;
constexpr int INFO_PLAYER_SPELL_COUNT = 129;
constexpr int INFO_CURRENT_LIFE = 130;
constexpr int INFO_CURRENT_LIFE_MAX = 131;
constexpr int INFO_ITEM_SCORE = 132;
constexpr int INFO_RECT = 133;
constexpr int INFO_DELAY_COLOR = 134;
constexpr int INFO_BLEND = 135;
constexpr int INFO_COLLISION = 136;
constexpr int INFO_COLLISION_LIST = 137;

// player state
constexpr int STATE_NORMAL = 0;
constexpr int STATE_HIT = 1;
constexpr int STATE_DOWN = 2;
constexpr int STATE_END = 3;

// DeleteShot系の引数
constexpr int TYPE_ITEM = 3;
constexpr int TYPE_ALL = 4;
constexpr int TYPE_SHOT = 5;
constexpr int TYPE_CHILD = 6;
constexpr int TYPE_IMMEDIATE = 7;
constexpr int TYPE_FADE = 8;

// GetShot系の引数
constexpr int TARGET_PLAYER = 23;
constexpr int TARGET_ENEMY = 22;
constexpr int TARGET_ALL = 21;

// general
constexpr int ID_INVALID = -1;
constexpr int NO_CHANGE = -(1 << 24);

// stage control(Package only)
constexpr int STAGE_STATE_FINISHED = 1;
constexpr int STAGE_RESULT_BREAK_OFF = 1;
constexpr int STAGE_RESULT_PLAYER_DOWN = 2;
constexpr int STAGE_RESULT_CLEARED = 3;

// for ObjFileB
constexpr int CODE_ACP = 0;
constexpr int CODE_UTF8 = 65001;
constexpr int CODE_UTF16LE = 65002;
constexpr int CODE_UTF16BE = 65003;
constexpr int ENDIAN_LITTLE = 0;
constexpr int ENDIAN_BIG = 1;

// unused
constexpr int CULL_NONE = 1;
constexpr int CULL_CW = 2;
constexpr int CULL_CCW = 3;
}