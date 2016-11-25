#include "game_command.h"
#include "process_item.h"
#include "process_action.h"
#include "process_map.h"
#include "process_bookmark.h"
#include "process_march.h"
#include "process_mailreport.h"
#include "process_alliance.h"
#include "process_self_system.h"
#include "process_event.h"
#include "process_throne.h"

CClientCmd *CClientCmd::m_poClientCmdInstance = NULL;

// todo: 命令字输出有待优化
static struct SCmdInfo stszClientReqCommand[EN_CLIENT_REQ_COMMAND__END + 1] = 
{
    {EN_CLIENT_REQ_COMMAND__UNKNOW,                 {"unknow", EN_UNKNOW_PROCEDURE, EN_UNKNOW_CMD, NULL, NULL}},
    {EN_CLIENT_REQ_COMMAND__LOGIN_GET,              {"login_get", EN_NORMAL, EN_PLAYER, CProcessPlayer::ProcessCmd_LoginGet, NULL}},
    {EN_CLIENT_REQ_COMMAND__GUIDE_FINISH,           {"guide_finish", EN_NORMAL, EN_PLAYER, CProcessPlayer::ProcessCmd_GuideFinish, NULL}},
    {EN_CLIENT_REQ_COMMAND__LOGIN_FAKE,             {"login_fake", EN_NORMAL, EN_PLAYER, CProcessPlayer::ProcessCmd_LoginFake, NULL}},
    {EN_CLIENT_REQ_COMMAND__LOGIN_CREATE,           {"login_create", EN_NORMAL, EN_PLAYER, CProcessPlayer::ProcessCmd_NewLoginCreate, NULL}},
    {EN_CLIENT_REQ_COMMAND__USER_INFO_CREATE,       {"user_info_create", EN_NORMAL, EN_PLAYER, CProcessPlayer::ProcessCmd_UserInfoCreate, NULL}},
    {EN_CLIENT_REQ_COMMAND__GUIDE_FINISH_STAGE,     {"guide_finish_stage", EN_NORMAL, EN_PLAYER, CProcessPlayer::ProcessCmd_GuideFinishStage, NULL}},
    {EN_CLIENT_REQ_COMMAND__SVR_CHANGE,             {"svr_change", EN_NORMAL, EN_PLAYER, CProcessPlayer::ProcessCmd_SvrChange, NULL}},
    {EN_CLIENT_REQ_COMMAND__USER_INFO_RECOVER,      {"recovery_dead_user", EN_NORMAL, EN_OP, CProcessOperate::ProcessCmd_RecoveryDeadPlayer, NULL}},

    {EN_CLIENT_REQ_COMMAND__BUILDING_UPGRADE,       {"building_upgrade", EN_NORMAL, EN_PLAYER, CProcessPlayer::ProcessCmd_BuildingUpgrade, NULL}},
    {EN_CLIENT_REQ_COMMAND__BUILDING_REMOVE,        {"building_remove", EN_NORMAL, EN_PLAYER, CProcessPlayer::ProcessCmd_BuildingRemove, NULL}},
    {EN_CLIENT_REQ_COMMAND__BUILDING_MOVE,          {"building_move", EN_NORMAL, EN_PLAYER, CProcessPlayer::ProcessCmd_BuildingMove, NULL}},
    {EN_CLIENT_REQ_COMMAND__BUILDING_EDIT,          {"building_edit", EN_NORMAL, EN_PLAYER, CProcessPlayer::ProcessCmd_BuildingEdit, NULL}},
    {EN_CLIENT_REQ_COMMAND__AGE_UPGRADE,            {"age_upgrade", EN_NORMAL, EN_PLAYER, CProcessPlayer::ProcessCmd_AgeUpgrade, NULL}},
    {EN_CLIENT_REQ_COMMAND__RESEARCH_UPGRADE,       {"research_upgrade", EN_NORMAL, EN_PLAYER, CProcessPlayer::ProcessCmd_ResearchUpgrade, NULL}},
    {EN_CLIENT_REQ_COMMAND__REMOVE_OBSTACLE,        {"remove_obstacle", EN_NORMAL, EN_PLAYER, CProcessPlayer::ProcessCmd_RemoveObstacle, NULL}},
    {EN_CLIENT_REQ_COMMAND__SECOND_BUILD_ACTION,    {"active_second_build_action", EN_NORMAL, EN_PLAYER, CProcessPlayer::ProcessCmd_ActiveSecondBuildAction, NULL}},

    {EN_CLIENT_REQ_COMMAND__TROOP_TRAIN,            {"troop_train", EN_NORMAL, EN_PLAYER, CProcessPlayer::ProcessCmd_TroopTrain, NULL}},
    {EN_CLIENT_REQ_COMMAND__FORT_TRAIN,             {"fort_train", EN_NORMAL, EN_PLAYER, CProcessPlayer::ProcessCmd_FortTrain, NULL}},
    {EN_CLIENT_REQ_COMMAND__TROOP_DISMISS,          {"troop_dismiss", EN_NORMAL, EN_PLAYER, CProcessPlayer::ProcessCmd_TroopDismiss, NULL}},
    {EN_CLIENT_REQ_COMMAND__FORT_DISMISS,           {"fort_dismiss", EN_NORMAL, EN_PLAYER, CProcessPlayer::ProcessCmd_FortDismiss, NULL}},
    {EN_CLIENT_REQ_COMMAND__TROOP_HEAL,             {"troop_heal", EN_NORMAL, EN_PLAYER, CProcessPlayer::ProcessCmd_HospitalTroopTreat, NULL}},
    {EN_CLIENT_REQ_COMMAND__FORT_HEAL,              {"dead_fort_heal", EN_NORMAL, EN_PLAYER, CProcessPlayer::ProcessCmd_DeadFortHeal, NULL}},
    {EN_CLIENT_REQ_COMMAND__HOSPITAL_TROOP_ABANDON, {"hospital_troop_abandon", EN_NORMAL, EN_PLAYER, CProcessPlayer::ProcessCmd_AbandWoundTroop, NULL}},
    {EN_CLIENT_REQ_COMMAND__DEAD_FORT_ABANDON,      {"dead_fort_abandon", EN_NORMAL, EN_PLAYER, CProcessPlayer::ProcessCmd_DeadFortAbandon, NULL}},

    {EN_CLIENT_REQ_COMMAND__TROOP_CANCEL,           {"troop_cancel", EN_NORMAL, EN_PLAYER, CProcessAction::ProcessCmd_TroopCancel, NULL}},
    {EN_CLIENT_REQ_COMMAND__FORT_CANCEL,            {"fort_cancel", EN_NORMAL, EN_PLAYER, CProcessAction::ProcessCmd_FortCancel, NULL}},
    {EN_CLIENT_REQ_COMMAND__HOSPITAL_TROOP_TREAT_CANCEL, {"hos_cancel_troop", EN_NORMAL, EN_PLAYER, CProcessAction::ProcessCmd_HospitalTroopTreatCancel, NULL}},
    {EN_CLIENT_REQ_COMMAND__DEAD_FORT_HEAL_CANCEL,  {"fort_heal_cancel", EN_NORMAL, EN_PLAYER, CProcessAction::ProcessCmd_FortRepairCancel, NULL}},

    {EN_CLIENT_REQ_COMMAND__CHANGE_PLAYER_NAME,     {"player_name_change", EN_NORMAL, EN_PLAYER, CProcessPlayer::ProcessCmd_ChangePlayerName, NULL}},
    {EN_CLIENT_REQ_COMMAND__CHANGE_BASE_NAME,       {"base_name_change", EN_NORMAL, EN_PLAYER, CProcessPlayer::ProcessCmd_ChangeBaseName, NULL}},
    {EN_CLIENT_REQ_COMMAND__CHANGE_AVATAR,          {"player_avatar_change", EN_NORMAL, EN_PLAYER, CProcessPlayer::ProcessCmd_PlayerAvatarChange, NULL}},
    {EN_CLIENT_REQ_COMMAND__CHANGE_DRAGON_NAME,     {"dragon_name_change", EN_NORMAL, EN_PLAYER, CProcessPlayer::ProcessCmd_DragonNameChange, NULL}},
    {EN_CLIENT_REQ_COMMAND__CHANGE_DRAGON_AVATAR,   {"dragon_avatar_change", EN_NORMAL, EN_PLAYER, CProcessPlayer::ProcessCmd_DragonAvatarChange, NULL}},
    {EN_CLIENT_REQ_COMMAND__DRAGON_REVIVE,          {"dragon_revive", EN_NORMAL, EN_PLAYER, CProcessPlayer::ProcessCmd_ReviveDragon, NULL}},
    {EN_CLIENT_REQ_COMMAND__DRAGON_KILL,            {"dragon_kill", EN_NORMAL, EN_PLAYER, CProcessPlayer::ProcessCmd_KillDragon, NULL}},
    {EN_CLIENT_REQ_COMMAND__DRAGON_RELEASE,         {"dragon_release", EN_NORMAL, EN_PLAYER, CProcessPlayer::ProcessCmd_ReleaseDragon, NULL}},

    {EN_CLIENT_REQ_COMMAND__PLAYER_INFO_GET,        {"player_info_get", EN_NORMAL, EN_PLAYER, CProcessPlayer::ProcessCmd_PlayerInfoGet, NULL}},
    {EN_CLIENT_REQ_COMMAND__SET_GUIDE_FLAG,         {"set_guide_flag", EN_NORMAL, EN_PLAYER, CProcessPlayer::ProcessCmd_SetGuideFlag, NULL}},

    {EN_CLIENT_REQ_COMMAND__QUEST_START,            {"quest_start", EN_NORMAL, EN_PLAYER, CProcessPlayer::ProcessCmd_QuestStart, NULL}},
    {EN_CLIENT_REQ_COMMAND__QUEST_REWARD_COLLECT,   {"quest_reward_collect", EN_NORMAL, EN_PLAYER, CProcessPlayer::ProcessCmd_QuestRewardCollect, NULL}},
    {EN_CLIENT_REQ_COMMAND__REFRESH_TIME_QUEST,     {"refresh_time_quest", EN_NORMAL, EN_PLAYER, CProcessPlayer::ProcessCmd_RefreshTimeQuest, NULL}},
    {EN_CLIENT_REQ_COMMAND__MISTERY_GIFT_COLLECT,   {"mistery_gift_collect", EN_NORMAL, EN_PLAYER, CProcessPlayer::ProcessCmd_GetTimerGift, NULL}},
    {EN_CLIENT_REQ_COMMAND__QUEST_CLAIM,            {"quest_claim", EN_NORMAL, EN_PLAYER, CProcessPlayer::ProcessCmd_QuestClaim, NULL}},
    {EN_CLIENT_REQ_COMMAND__LORD_LEVEL_UP,          {"lord_level_up", EN_NORMAL, EN_PLAYER, CProcessPlayer::ProcessCmd_LordLevelUp, NULL}},
    {EN_CLIENT_REQ_COMMAND__DRAGON_LEVEL_UP,        {"dragon_level_up", EN_NORMAL, EN_PLAYER, CProcessPlayer::ProcessCmd_DragonLevelUp, NULL}},
    {EN_CLIENT_REQ_COMMAND__TASK_CLEAR_FLAG,        {"task_clear_flag", EN_NORMAL, EN_PLAYER, CProcessPlayer::ProcessCmd_TaskClearFlag, NULL}},
    {EN_CLIENT_REQ_COMMAND__TASK_OPERATE,           {"task_operate", EN_NORMAL, EN_PLAYER, CProcessPlayer::ProcessCmd_TaskOperate, NULL}},
    {EN_CLIENT_REQ_COMMAND__RANDOM_REWARD,          {"random_reward_get", EN_NORMAL, EN_PLAYER, CProcessPlayer::ProcessCmd_RandomReward, NULL}},
    {EN_CLIENT_REQ_COMMAND__OPEN_TASK_WINDOW,       {"open_task_window", EN_NORMAL, EN_PLAYER, CProcessPlayer::ProcessCmd_OpenTaskWinDow, NULL}},

    {EN_CLIENT_REQ_COMMAND__LORD_SKILL_UPGRADE,     {"lord_skill_upgrade", EN_NORMAL, EN_PLAYER, CProcessPlayer::ProcessCmd_LordSkillUpgrade, NULL}},
    {EN_CLIENT_REQ_COMMAND__LORD_SKILL_RESET,       {"lord_skill_reset", EN_NORMAL, EN_PLAYER, CProcessPlayer::ProcessCmd_LordSkillReset, NULL}},

    {EN_CLIENT_REQ_COMMAND__DRAGON_SKILL_UPGRADE,          {"dragon_skill_upgrade", EN_NORMAL, EN_PLAYER, CProcessPlayer::ProcessCmd_DragonSkillUpgrade, NULL}},
    {EN_CLIENT_REQ_COMMAND__DRAGON_SKILL_RESET,            {"dragon_skill_reset", EN_NORMAL, EN_PLAYER, CProcessPlayer::ProcessCmd_DragonSkillReset, NULL}},
    {EN_CLIENT_REQ_COMMAND__DRAGON_MONSTER_SKILL_UPGRADE,  {"dragon_monster_skill_upgrade", EN_NORMAL, EN_PLAYER, CProcessPlayer::ProcessCmd_DragonMonsterSkillUpgrade, NULL}},
    {EN_CLIENT_REQ_COMMAND__DRAGON_MONSTER_SKILL_RESET,    {"dragon_monster_skill_reset", EN_NORMAL, EN_PLAYER, CProcessPlayer::ProcessCmd_DragonMonsterSkillReset, NULL}},

    {EN_CLIENT_REQ_COMMAND__COMPOSE_EQUIP,          {"compose_equip", EN_NORMAL, EN_PLAYER, CProcessPlayer::ProcessCmd_ComposeEquip, NULL}},
    {EN_CLIENT_REQ_COMMAND__COMPOSE_MATERIAL,       {"compose_material", EN_NORMAL, EN_PLAYER, CProcessPlayer::ProcessCmd_ComposeMaterial, NULL}},
    {EN_CLIENT_REQ_COMMAND__COMPOSE_SOUL,           {"compose_soul", EN_NORMAL, EN_PLAYER, CProcessPlayer::ProcessCmd_ComposeSoul, NULL}},
    {EN_CLIENT_REQ_COMMAND__COMPOSE_PARTS,          {"compose_parts", EN_NORMAL, EN_PLAYER, CProcessPlayer::ProcessCmd_ComposeParts, NULL}},
    {EN_CLIENT_REQ_COMMAND__COMPOSE_CRYSTAL,        {"compose_crystal", EN_NORMAL, EN_PLAYER, CProcessPlayer::ProcessCmd_ComposeCrystal, NULL}},
    {EN_CLIENT_REQ_COMMAND__COMPOSE_SP_CRYSTAL,     {"compose_sp_crystal", EN_NORMAL, EN_PLAYER, CProcessPlayer::ProcessCmd_ComposeSpCrystal, NULL}},
    {EN_CLIENT_REQ_COMMAND__ADD_EQUIP_GRID,         {"add_equip_grid", EN_NORMAL, EN_PLAYER, CProcessPlayer::ProcessCmd_AddEquipGride, NULL}},
    {EN_CLIENT_REQ_COMMAND__CRYSTAL_INSERT,         {"crystal_insert", EN_NORMAL, EN_PLAYER, CProcessPlayer::ProcessCmd_CrystalInsert, NULL}},
    {EN_CLIENT_REQ_COMMAND__CRYSTAL_REMOVE,         {"crystal_remove", EN_NORMAL, EN_PLAYER, CProcessPlayer::ProcessCmd_CrystalRemove, NULL}},
    {EN_CLIENT_REQ_COMMAND__PUT_ON_EQUIP,           {"put_on_equip", EN_NORMAL, EN_PLAYER, CProcessPlayer::ProcessCmd_PutOnEquip, NULL}},
    {EN_CLIENT_REQ_COMMAND__PUT_OFF_EQUIP,          {"put_off_equip", EN_NORMAL, EN_PLAYER, CProcessPlayer::ProcessCmd_PutOffEquip, NULL}},
    {EN_CLIENT_REQ_COMMAND__EQUIP_DESTROY,          {"equip_destroy", EN_NORMAL, EN_PLAYER, CProcessPlayer::ProcessCmd_DestroyEquip, NULL}},
    {EN_CLIENT_REQ_COMMAND__BUY_SCROLL,             {"buy_scroll", EN_NORMAL, EN_PLAYER, CProcessPlayer::ProcessCmd_BuyScroll, NULL}},
    {EN_CLIENT_REQ_COMMAND__BUY_SCROLL_NEW,         {"buy_scroll_new", EN_NORMAL, EN_PLAYER, CProcessPlayer::ProcessCmd_BuyScrollNew, NULL}},
    {EN_CLIENT_REQ_COMMAND__SCROLL_DISMISS,         {"scroll_dismiss", EN_NORMAL, EN_PLAYER, CProcessPlayer::ProcessCmd_DropScroll, NULL}},

    {EN_CLIENT_REQ_COMMAND__TRIAL_INIT,             {"trial_init", EN_NORMAL, EN_PLAYER, CProcessPlayer::ProcessCmd_TrialInit, NULL}},
    {EN_CLIENT_REQ_COMMAND__TRIAL_RAGE_MODE,        {"trial_rage_mode", EN_NORMAL, EN_PLAYER, CProcessPlayer::ProcessCmd_TrialRageMode, NULL}},
    {EN_CLIENT_REQ_COMMAND__TRIAL_NORMAL_MODE,      {"trial_normal_mode", EN_NORMAL, EN_PLAYER, CProcessPlayer::ProcessCmd_TrialNormalMode, NULL}},
    {EN_CLIENT_REQ_COMMAND__TRIAL_ATTACK,           {"trial_attack", EN_NORMAL, EN_PLAYER, CProcessPlayer::ProcessCmd_TrialAttack, NULL}},
    {EN_CLIENT_REQ_COMMAND__TRIAL_LUCKY_BAG_NORMAL, {"trial_lucky_bag_normal", EN_NORMAL, EN_PLAYER, CProcessPlayer::ProcessCmd_TrialLuckyBagNormal, NULL}},
    {EN_CLIENT_REQ_COMMAND__TRIAL_LUCKY_BAG_RAGE,   {"trial_lucky_bag_rage", EN_NORMAL, EN_PLAYER, CProcessPlayer::ProcessCmd_TrialLuckyBagRage, NULL}},
    {EN_CLIENT_REQ_COMMAND__TRIAL_GIFT_COLLECT,     {"trial_gift_collect", EN_NORMAL, EN_PLAYER, CProcessPlayer::ProcessCmd_TrialGiftCollect, NULL}},
    {EN_CLIENT_REQ_COMMAND__FINISH_GUIDE,           {"finish_guide", EN_NORMAL, EN_PLAYER, CProcessPlayer::ProcessCmd_FinishGuide, NULL}},

    {EN_CLIENT_REQ_COMMAND__GEM_RECHARGE,           {"gem_recharge", EN_NORMAL, EN_ACCOUNT, CProcessAccount::ProcessCmd_GemRecharge, NULL}},
    {EN_CLIENT_REQ_COMMAND__SVR_GET,                {"svr_get", EN_NORMAL, EN_ACCOUNT, CProcessAccount::ProcessCmd_SvrInfoGet, NULL}},
    {EN_CLIENT_REQ_COMMAND__APNS_UPDATE,            {"apns_update", EN_NORMAL, EN_ACCOUNT, CProcessAccount::ProcessCmd_ApnsUpdate, NULL}},
    {EN_CLIENT_REQ_COMMAND__APNS_TOKEN,             {"apns_token", EN_NORMAL, EN_ACCOUNT, CProcessAccount::ProcessCmd_ApnsToken, NULL}},
    {EN_CLIENT_REQ_COMMAND__APNS_SWITCH,            {"apns_switch", EN_NORMAL, EN_ACCOUNT, CProcessAccount::ProcessCmd_ApnsSwitch, NULL}},

    
    {EN_CLIENT_REQ_COMMAND__ITEM_USE,               {"item_use", EN_NORMAL, EN_ITEM, CProcessItem::ProcessCmd_ItemUse, NULL}},
    {EN_CLIENT_REQ_COMMAND__ITEM_BUY,               {"item_buy", EN_NORMAL, EN_ITEM, CProcessItem::ProcessCmd_ItemBuy, NULL}},
    {EN_CLIENT_REQ_COMMAND__ITEM_BUY_AND_USE,       {"item_buy_and_use", EN_NORMAL, EN_ITEM, CProcessItem::ProcessCmd_ItemBuyAndUse, NULL}},
    {EN_CLIENT_REQ_COMMAND__RANDOM_MOVE_CITY,       {"random_move_city", EN_NORMAL, EN_ITEM, CProcessItem::ProcessCmd_RandomMoveCity, NULL}},
    {EN_CLIENT_REQ_COMMAND__MOVE_CITY,              {"move_city", EN_NORMAL, EN_ITEM, CProcessItem::ProcessCmd_MoveCity, NULL}},
    {EN_CLIENT_REQ_COMMAND__MOVE_CITY_PREPARE,      {"move_city_prepare", EN_NORMAL, EN_ITEM, CProcessItem::ProcessCmd_MoveCityPrepare, NULL}},
    {EN_CLIENT_REQ_COMMAND__OPEN_ALL_CHEST,         {"chest_open_all", EN_NORMAL, EN_ITEM, CProcessItem::ProcessCmd_OpenAllChest, NULL}},

    //EVENT
    {EN_CLIENT_ALL_EVENT_GET,                       {"all_event_get", EN_SPECIAL, EN_EVENT, CProcessEvent::ProcessCmd_AllEventInfoGet, NULL}},
    {EN_CLIENT_EVENT_INFO_GET,                      {"event_info_get", EN_SPECIAL, EN_EVENT, CProcessEvent::ProcessCmd_EventInfoGet, NULL}},
    {EN_CLIENT_HISTORY_EVENT_INFO_GET,              {"event_history_info_get", EN_SPECIAL, EN_EVENT, CProcessEvent::ProcessCmd_HistoryEventInfoGet, NULL}},
    {EN_CLIENT_THEME_HISTORY_EVENT_INFO_GET,        {"theme_event_history_info_get", EN_SPECIAL, EN_EVENT, CProcessEvent::ProcessCmd_ThemeHistoryEventInfoGet, NULL } },

    {EN_CLIENT_REQ_COMMAND__RECOVERY_MERGE_USER,    {"recovery_merge_user", EN_NORMAL, EN_MAIL_REPORT, CProcessPlayer::ProcessCmd_RecoveryMergeUser, NULL}},

    {EN_CLIENT_REQ_COMMAND__ALLIANCE_CREATE,        {"al_create", EN_NORMAL, EN_ALLIANCE, CProcessAlliance::ProcessCmd_AllianceCreate, NULL}},
    {EN_CLIENT_REQ_COMMAND__ALLIANCE_JOIN,          {"al_join", EN_NORMAL, EN_ALLIANCE, CProcessAlliance::ProcessCmd_AllianceRequestJoin, NULL}},
    {EN_CLIENT_REQ_COMMAND__ALLIANCE_LEAVE,         {"al_leave", EN_NORMAL, EN_ALLIANCE, CProcessAlliance::ProcessCmd_AllianceLeave, NULL}},
    {EN_CLIENT_REQ_COMMAND__ALLIANCE_KICKOUT,       {"al_kick", EN_NORMAL, EN_ALLIANCE, CProcessAlliance::ProcessCmd_AllianceKickout, NULL}},
    {EN_CLIENT_REQ_COMMAND__ALLIANCE_REQUEST_GET,   {"al_request_get", EN_NORMAL, EN_ALLIANCE, CProcessAlliance::ProcessCmd_AllianceRequestGet, NULL}},
    {EN_CLIENT_REQ_COMMAND__ALLIANCE_ALLOW_JOIN,    {"al_allow_join", EN_NORMAL, EN_ALLIANCE, CProcessAlliance::ProcessCmd_AllianceRequestAllow, NULL}},
    {EN_CLIENT_REQ_COMMAND__ALLIANCE_REGECT_JOIN,   {"al_reject_join", EN_NORMAL, EN_ALLIANCE, CProcessAlliance::ProcessCmd_AllianceRequestReject, NULL}},
    {EN_CLIENT_REQ_COMMAND__ALLIANCE_MEMBER_GET,    {"al_member_get", EN_NORMAL, EN_ALLIANCE, CProcessAlliance::ProcessCmd_AllianceMemberGet, NULL}},
    {EN_CLIENT_REQ_COMMAND__ALLIANCE_POS_CHANGE,    {"al_pos_change", EN_NORMAL, EN_ALLIANCE, CProcessAlliance::ProcessCmd_AlliancePosChange, NULL}},
    {EN_CLIENT_REQ_COMMAND__ALLIANCE_CHANGE_POLICY, {"al_policy_change", EN_NORMAL, EN_ALLIANCE, CProcessAlliance::ProcessCmd_AllianceChangePolicy, NULL}},
    {EN_CLIENT_REQ_COMMAND__ALLIANCE_CHANGE_DESC,   {"al_desc_change", EN_NORMAL, EN_ALLIANCE, CProcessAlliance::ProcessCmd_AllianceChangeDesc, NULL}},
    {EN_CLIENT_REQ_COMMAND__ALLIANCE_CHANGE_NOTIC,  {"al_notice_change", EN_NORMAL, EN_ALLIANCE, CProcessAlliance::ProcessCmd_AllianceChangeNotic, NULL}},
    {EN_CLIENT_REQ_COMMAND__ALLIANCE_CHANGE_LANGUAGE, {"al_lang_change", EN_NORMAL, EN_ALLIANCE, CProcessAlliance::ProcessCmd_AllianceChangeLanguage, NULL}},
    {EN_CLIENT_REQ_COMMAND__ALLIANCE_WALL_TOP,      {"al_wall_msg_top", EN_NORMAL, EN_ALLIANCE, CProcessAlliance::ProcessCmd_AllianceWallMsgTop, NULL}},
    {EN_CLIENT_REQ_COMMAND__ALLIANCE_WALL_GET,      {"al_wall_msg_get", EN_NORMAL, EN_ALLIANCE, CProcessAlliance::ProcessCmd_AllianceWallMagGet, NULL}},
    {EN_CLIENT_REQ_COMMAND__ALLIANCE_WALL_INSERT,   {"al_wall_msg_add", EN_NORMAL, EN_ALLIANCE, CProcessAlliance::ProcessCmd_WallInsert, NULL}},
    {EN_CLIENT_REQ_COMMAND__ALLIANCE_WALL_DELETE,   {"al_wall_msg_del", EN_NORMAL, EN_ALLIANCE, CProcessAlliance::ProcessCmd_WallDelete, NULL}},
    {EN_CLIENT_REQ_COMMAND__ALLIANCE_TASK_HELP_REQUEST, {"al_help_request", EN_NORMAL, EN_ALLIANCE, CProcessAlliance::ProcessCmd_AlTaskHelpReq, NULL}},
    {EN_CLIENT_REQ_COMMAND__ALLIANCE_TASK_HELP_GET, {"al_help_get", EN_NORMAL, EN_ALLIANCE, CProcessAlliance::ProcessCmd_AlTaskHelpGet, NULL}},
    {EN_CLIENT_REQ_COMMAND__ALLIANCE_TASK_HELP_SPEED_UP, {"al_help", EN_NORMAL, EN_ALLIANCE, CProcessAlliance::ProcessCmd_AlTaskHelpSpeedUp, NULL}},
    {EN_CLIENT_REQ_COMMAND__ALLIANCE_TASK_HELP_SPEED_UP_ALL, {"al_help_all", EN_NORMAL, EN_ALLIANCE, CProcessAlliance::ProcessCmd_AlTaskHelpSpeedUpAll, NULL}},
    {EN_CLIENT_REQ_COMMAND__ALLIANCE_GIFT_OPEN,     {"al_gift_open", EN_NORMAL, EN_ALLIANCE, CProcessAlliance::ProcessCmd_AlGiftOpen, NULL}},
    {EN_CLIENT_REQ_COMMAND__ALLIANCE_GIFT_GET,      {"al_gift_get", EN_NORMAL, EN_ALLIANCE, CProcessAlliance::ProcessCmd_AlGiftGet, NULL}},
    {EN_CLIENT_REQ_COMMAND__ALLIANCE_GIFT_DEL,      {"al_gift_clear", EN_NORMAL, EN_ALLIANCE, CProcessAlliance::ProcessCmd_AlGiftDel, NULL}},
    {EN_CLIENT_REQ_COMMAND__ALLIANCE_GIFT_DEL_ALL,  {"al_gift_clear_all", EN_NORMAL, EN_ALLIANCE, CProcessAlliance::ProcessCmd_AlGiftDelAll, NULL}},
    {EN_CLIENT_REQ_COMMAND__ALLIANCE_ITEM_EXCHANGE, {"al_item_exchange", EN_NORMAL, EN_ALLIANCE, CProcessAlliance::ProcessCmd_AlItemExchange, NULL}},
    {EN_CLIENT_REQ_COMMAND__ALLIANCE_ITEM_BUY,      {"al_item_buy", EN_NORMAL, EN_ALLIANCE, CProcessAlliance::ProcessCmd_AlItemBuy, NULL}},
    {EN_CLIENT_REQ_COMMAND__ALLIANCE_ITEM_MARK,     {"al_item_mark", EN_NORMAL, EN_ALLIANCE, CProcessAlliance::ProcessCmd_AlItemMark, NULL}},
    {EN_CLIENT_REQ_COMMAND__ALLIANCE_ITEM_UNMARK,   {"al_item_unmark", EN_NORMAL, EN_ALLIANCE, CProcessAlliance::ProcessCmd_AlItemUnmark, NULL}},
    {EN_CLIENT_REQ_COMMAND__ALLIANCE_ITEM_MARK_CLEAR, {"al_item_mark_clear", EN_NORMAL, EN_ALLIANCE, CProcessAlliance::ProcessCmd_AlItemMarkClear, NULL}},
    {EN_CLIENT_REQ_COMMAND__ALLIANCE_DIPLOMACY_GET, {"al_diplomacy_get", EN_NORMAL, EN_ALLIANCE, CProcessAlliance::ProcessCmd_AlDiplomacyGet, NULL}},
    {EN_CLIENT_REQ_COMMAND__ALLIANCE_DIPLOMACY_SET, {"al_diplomacy_set", EN_NORMAL, EN_ALLIANCE, CProcessAlliance::ProcessCmd_AlDiplomacySet, NULL}},
	{EN_CLIENT_REQ_COMMAND__ALLIANCE_AVATAR_CHANGE, {"al_avatar_change", EN_NORMAL, EN_ALLIANCE, CProcessAlliance::ProcessCmd_AlAvatarChange, NULL}},
    {EN_CLIENT_REQ_COMMAND__ALLIANCE_ASSIST_SEND,   {"al_assist_send", EN_NORMAL, EN_ALLIANCE, CProcessAlliance::ProcessCmd_AlAssistSend, NULL}},
    {EN_CLIENT_REQ_COMMAND__ALLIANCE_ASSIST_GET,    {"al_assist_get", EN_NORMAL, EN_ALLIANCE, CProcessAlliance::ProcessCmd_AlAssistGet, NULL}},
    {EN_CLIENT_REQ_COMMAND__ALLIANCE_ASSIST_DEL,    {"al_assist_del", EN_NORMAL, EN_ALLIANCE, CProcessAlliance::ProcessCmd_AlAssistDel, NULL}},
    {EN_CLIENT_REQ_COMMAND__ALLIANCE_INFO_GET,      {"al_info_get", EN_NORMAL, EN_ALLIANCE, CProcessAlliance::ProcessCmd_AllianceGetInfo, NULL}},
    {EN_CLIENT_REQ_COMMAND__ALLIANCE_CHANGE_NICK,   {"al_nick_change", EN_NORMAL, EN_ALLIANCE, CProcessAlliance::ProcessCmd_AllianceNickChange, NULL}},
    {EN_CLIENT_REQ_COMMAND__ALLIANCE_CHANGE_NAME,   {"al_name_change", EN_NORMAL, EN_ALLIANCE, CProcessAlliance::ProcessCmd_AllianceNameChange, NULL}},
    {EN_CLIENT_REQ_COMMAND__ALLIANCE_DUB,           {"dub", EN_NORMAL, EN_ALLIANCE, CProcessAlliance::ProcessCmd_DubTitle, NULL}},
    {EN_CLIENT_REQ_COMMAND__ALLIANCE_INVITE,        {"al_invite", EN_NORMAL, EN_ALLIANCE, CProcessAlliance::ProcessCmd_Invite, NULL}},
    {EN_CLIENT_REQ_COMMAND__ALLIANCE_INVITED_JOIN,  {"al_invited_join", EN_NORMAL, EN_ALLIANCE, CProcessAlliance::ProcessCmd_InvitedJoin, NULL}},
    {EN_CLIENT_REQ_COMMAND__PLAYER_RECOMMAND_GET,   {"recommend_player_get", EN_NORMAL, EN_ALLIANCE, CProcessAlliance::ProcessCmd_PlayerRecommendGet, NULL}},
    {EN_CLIENT_REQ_COMMAND__AL_HIVE_POS_SHOW,       {"al_hive_pos_show", EN_NORMAL, EN_ALLIANCE, CProcessAlliance::ProcessCmd_AlHivePosShow, NULL}},
    {EN_CLIENT_REQ_COMMAND__AL_SET_HIVE_POS,        {"al_set_hive_pos", EN_NORMAL, EN_ALLIANCE, CProcessAlliance::ProcessCmd_AlSetHivePos, NULL}},



    {EN_CLIENT_REQ_COMMAND__BOOKMARK_GET,           {"bookmark_get", EN_NORMAL, EN_BOOKMARK, CProcessBookmark::ProcessCmd_BookmarkGet, NULL}},
    {EN_CLIENT_REQ_COMMAND__BOOKMARK_ADD,           {"bookmark_add", EN_NORMAL, EN_BOOKMARK, CProcessBookmark::ProcessCmd_BookmarkAdd, NULL}},
    {EN_CLIENT_REQ_COMMAND__BOOKMARK_UPDT,          {"bookmark_updt", EN_NORMAL, EN_BOOKMARK, CProcessBookmark::ProcessCmd_BookmarkUpdate, NULL}},
    {EN_CLIENT_REQ_COMMAND__BOOKMARK_DEL,           {"bookmark_del", EN_NORMAL, EN_BOOKMARK, CProcessBookmark::ProcessCmd_BookmarkDelete, NULL}},

    {EN_CLIENT_REQ_COMMAND__MARCH_OCCUPY,           {"march_occupy", EN_NORMAL, EN_MARCH, CProcessMarch::ProcessCmd_MarchOccupy, NULL}},
    {EN_CLIENT_REQ_COMMAND__MARCH_SCOUT,            {"march_scout", EN_NORMAL, EN_MARCH, CProcessMarch::ProcessCmd_MarchScout, NULL}},
    {EN_CLIENT_REQ_COMMAND__MARCH_ATTACK,           {"march_attack", EN_NORMAL, EN_MARCH, CProcessMarch::ProcessCmd_MarchAttack, NULL}},
    {EN_CLIENT_REQ_COMMAND__MARCH_TRANSPORT,        {"transport", EN_NORMAL, EN_MARCH, CProcessMarch::ProcessCmd_MarchTransport, NULL}},
    {EN_CLIENT_REQ_COMMAND__MARCH_REINFORCE,        {"reinforce", EN_NORMAL, EN_MARCH, CProcessMarch::ProcessCmd_MarchReinforceNormal, NULL}},
    {EN_CLIENT_REQ_COMMAND__MARCH_RECALL,           {"action_recall", EN_NORMAL, EN_MARCH, CProcessMarch::ProcessCmd_Recall, NULL}},
    {EN_CLIENT_REQ_COMMAND__MARCH_DRAGON_ATTACK,    {"march_dragon_attack", EN_NORMAL, EN_MARCH, CProcessMarch::ProcessCmd_MarchDragonAttack, NULL}},
    {EN_CLIENT_REQ_COMMAND__MARCH_RALLY_ATK,        {"rally_attack", EN_NORMAL, EN_MARCH, CProcessMarch::ProcessCmd_RallyAttack, NULL}},
    {EN_CLIENT_REQ_COMMAND__MARCH_RALLY_REINFORCE,  {"rally_reinforce", EN_NORMAL, EN_MARCH, CProcessMarch::ProcessCmd_RallyReinforce, NULL}},
    {EN_CLIENT_REQ_COMMAND__MARCH_RALLY_RECALL,     {"rally_recall", EN_NORMAL, EN_MARCH, CProcessMarch::ProcessCmd_RallyReinforceRecall, NULL}},
    {EN_CLIENT_REQ_COMMAND__MARCH_RALLY_DISMISS,    {"rally_dismiss", EN_NORMAL, EN_MARCH, CProcessMarch::ProcessCmd_RallyDismiss, NULL}},
    {EN_CLIENT_REQ_COMMAND__MARCH_RALLY_SLOT_BUY,   {"rally_buy_slot", EN_NORMAL, EN_MARCH, CProcessMarch::ProcessCmd_RallySlotBuy, NULL}},

    {EN_CLIENT_REQ_COMMAND__MARCH_RALLY_INFO,       {"rally_war_info", EN_NORMAL, EN_MARCH, CProcessMarch::ProcessCmd_RallyInfo, NULL}},

    {EN_CLIENT_REQ_COMMAND__MARCH_RALLY_HISTORY,    {"rally_history", EN_NORMAL, EN_MARCH, CProcessMarch::ProcessCmd_RallyHistory, NULL}},
    {EN_CLIENT_REQ_COMMAND__ABANDON_MANOR,          {"abandon_manor", EN_NORMAL, EN_MARCH, CProcessMarch::ProcessCmd_ManorAbandon, NULL}},
    {EN_CLIENT_REQ_COMMAND__REPATRIATE,             {"repatriate", EN_NORMAL, EN_MARCH, CProcessMarch::ProcessCmd_Repatriate, NULL}},
    {EN_CLIENT_REQ_COMMAND__RECALL_ALL_REINFORCE,   {"recall_all_reinforce", EN_NORMAL, EN_MARCH, CProcessMarch::ProcessCmd_RecallAllReinforce, NULL}},
    {EN_CLIENT_REQ_COMMAND__SENDBACK_ALL_REINFORCE, {"sendback_all_reinforce", EN_NORMAL, EN_MARCH, CProcessMarch::ProcessCmd_SendbackAllReinforce, NULL}},
    {EN_CLIENT_REQ_COMMAND__MARCH_CAMP,             {"march_camp", EN_NORMAL, EN_MARCH, CProcessMarch::ProcessCmd_MarchCamp, NULL}},

    {EN_CLIENT_REQ_COMMAND__MYSTERY_STORE_BUY,      {"mystery_store_buy", EN_NORMAL, EN_MARCH, CProcessPlayer::ProcessCmd_MysteryStoreBuy, NULL}},
    {EN_CLIENT_REQ_COMMAND__MYSTERY_STORE_PASS,     {"mystery_store_pass", EN_NORMAL, EN_MARCH, CProcessPlayer::ProcessCmd_MysteryStorePass, NULL}},

    {EN_CLIENT_REQ_COMMAND__BLACKUSER_ADD,          {"blackuser_add", EN_NORMAL, EN_PLAYER, CProcessPlayer::ProcessCmd_BlackUserAdd, NULL}},
    {EN_CLIENT_REQ_COMMAND__BLACKUSER_DEL,          {"blackuser_del", EN_NORMAL, EN_PLAYER, CProcessPlayer::ProcessCmd_BlackUserDel, NULL}},

    {EN_CLIENT_REQ_COMMAND__THRONE_TAX_SET,         {"throne_tax_set", EN_NORMAL, EN_PLAYER, CProcessThrone::ProcessCmd_ThroneSetTax, NULL}},

    {EN_CLIENT_REQ_COMMAND__KNIGHT_ASSIGN,          {"knight_assign", EN_NORMAL, EN_PLAYER, CProcessPlayer::ProcessCmd_KnightAssign, NULL}},

    {EN_CLIENT_REQ_COMMAND__PERSON_GUIDE,          {"person_guide", EN_NORMAL, EN_PLAYER, CProcessPlayer::ProcessCmd_PersonGuideClaim, NULL}},

    {EN_CLIENT_REQ_COMMAND__DRAGON_UNLOCK,          {"dragon_unlock", EN_NORMAL, EN_PLAYER, CProcessPlayer::ProcessCmd_DragonUnlock, NULL}},

    {EN_CLIENT_REQ_COMMAND__MAIL_GET,               {"mail_get", EN_NORMAL, EN_MAIL_REPORT, CProcessMailReport::ProcessCmd_MailGet, NULL}},
    {EN_CLIENT_REQ_COMMAND__MAIL_DETAIL_GET,        {"mail_detail_get", EN_NORMAL, EN_MAIL_REPORT, CProcessMailReport::ProcessCmd_MailDetailGet, NULL}},
    {EN_CLIENT_REQ_COMMAND__MAIL_READ,              {"mail_read", EN_NORMAL, EN_MAIL_REPORT, CProcessMailReport::ProcessCmd_MailRead, NULL}},
    {EN_CLIENT_REQ_COMMAND__MAIL_DEL,               {"mail_del", EN_NORMAL, EN_MAIL_REPORT, CProcessMailReport::ProcessCmd_MailDel, NULL}},
    {EN_CLIENT_REQ_COMMAND__MAIL_STAR,              {"mail_star", EN_NORMAL, EN_MAIL_REPORT, CProcessMailReport::ProcessCmd_MailStar, NULL}},
    {EN_CLIENT_REQ_COMMAND__MAIL_UNSTAR,            {"mail_unstar", EN_NORMAL, EN_MAIL_REPORT, CProcessMailReport::ProcessCmd_MailUnstar, NULL}},
    {EN_CLIENT_REQ_COMMAND__MAIL_REWARD_COLLECT,    {"mail_reward_collect", EN_NORMAL, EN_MAIL_REPORT, CProcessMailReport::ProcessCmd_MailRewardCollect, NULL}},
    {EN_CLIENT_REQ_COMMAND__MAIL_SEND,              {"mail_send", EN_NORMAL, EN_MAIL_REPORT, CProcessMailReport::ProcessCmd_MailSend, NULL}},
    {EN_CLIENT_REQ_COMMAND__MAIL_SEND_BY_ID,        {"mail_send_by_id", EN_NORMAL, EN_MAIL_REPORT, CProcessMailReport::ProcessCmd_MailSendById, NULL}},

    {EN_CLIENT_REQ_COMMAND__REPORT_GET,             {"report_get", EN_NORMAL, EN_MAIL_REPORT, CProcessMailReport::ProcessCmd_ReportGet, NULL}},
    {EN_CLIENT_REQ_COMMAND__REPORT_DETAIL_GET,      {"report_detail_get", EN_NORMAL, EN_MAIL_REPORT, CProcessMailReport::ProcessCmd_ReportDetailGet, NULL}},
    {EN_CLIENT_REQ_COMMAND__REPORT_READ,            {"report_read", EN_NORMAL, EN_MAIL_REPORT, CProcessMailReport::ProcessCmd_ReportRead, NULL}},
    {EN_CLIENT_REQ_COMMAND__REPORT_DEL,             {"report_del", EN_NORMAL, EN_MAIL_REPORT, CProcessMailReport::ProcessCmd_ReportDel, NULL}},
    {EN_CLIENT_REQ_COMMAND__ALLIANCE_REPORT_GET,    {"al_report_get", EN_NORMAL, EN_MAIL_REPORT, CProcessMailReport::ProcessCmd_AllianceReportGet, NULL}},

    {EN_CLIENT_REQ_COMMAND__GEM_SPEED_UP,           {"gem_speed_up", EN_NORMAL, EN_ACTION, CProcessAction::ProcessCmd_ActionGemSpeedUp, NULL}},
    {EN_CLIENT_REQ_COMMAND__FREE_SPEED_UP,          {"free_speed_up", EN_NORMAL, EN_ACTION, CProcessAction::ProcessCmd_ActionFreeSpeedUp, NULL}},
    {EN_CLIENT_REQ_COMMAND__ACTION_CANCLE,          {"action_cancel", EN_NORMAL, EN_ACTION, CProcessAction::ProcessCmd_ActionCancel, NULL}},
    
    {EN_CLIENT_REQ_COMMAND__MAP_GET,                {"map_get", EN_SPECIAL, EN_MAP, CProcessMap::ProcessCmd_MapGet, NULL}},
    {EN_CLIENT_REQ_COMMAND__MAP_BLOCK_GET,          {"map_block_get", EN_SPECIAL, EN_MAP, CProcessMap::ProcessCmd_MapBlockGet, NULL}},
    {EN_CLIENT_REQ_COMMAND__MANOR_INFO,             {"manor_info", EN_SPECIAL, EN_MAP, CProcessMap::ProcessCmd_ManorInfo, NULL}},
    {EN_CLIENT_REQ_COMMAND__PRISON_INFO,            {"prison_info", EN_SPECIAL, EN_MAP, CProcessMap::ProcessCmd_PrisionInfo, NULL}},
        
    {EN_CLIENT_REQ_COMMAND__CLEAR_HELP_BUBBLE_STATUS,   {"clear_help_bubble", EN_NORMAL, EN_PLAYER, CProcessPlayer::ProcessCmd_ClearHelpBubble, NULL } },

    {EN_CLIENT_OPERATE_COMMAND__REPORT_GET,         {"op_report_get", EN_SPECIAL, EN_OP, CProcessMailReport::ProcessCmd_OpReportGet, NULL}},
    {EN_CLIENT_OPERATE_COMMAND__MAIL_GET,           {"op_mail_get", EN_SPECIAL, EN_OP, CProcessMailReport::ProcessCmd_OpMailGet, NULL}},
    {EN_CLIENT_OPERATE_COMMAND__MAIL_SEND,          {"operate_mail_send", EN_SPECIAL, EN_OP, CProcessMailReport::ProcessCmd_OpMailSend, NULL}},
    {EN_CLIENT_OPERATE_CMD__LOGIN_GET,              {"operate_login_get", EN_NORMAL, EN_OP, CProcessOperate::ProcessCmd_LoginGet, NULL}}, // todo: 把"operate_log"的逻辑移到op处理类中做处理
    {EN_CLIENT_OPERATE_CMD__LOG,                    {"operate_log", EN_SPECIAL, EN_OP, CProcessOperate::ProcessCmd_OperateLog, NULL}}, // todo: 把"operate_log"的逻辑移到op处理类中做处理
    {EN_CLIENT_REQ_COMMAND__OPRATE_GEM_RECHARGE,    {"operate_gem_recharge", EN_NORMAL, EN_OP, CProcessOperate::ProcessCmd_GemRecharge, NULL}},
    {EN_CLIENT_OPERATE_CMD__ADD_PERSON_AL_GIFT,     {"operate_add_person_al_gift", EN_NORMAL, EN_OP, CProcessOperate::ProcessCmd_AddPersonAlGift, NULL}},
    {EN_CLIENT_OPERATE_CMD__ADD_ALLIANCE_AL_GIFT,   {"operate_add_alliance_al_gift", EN_NORMAL, EN_OP, CProcessOperate::ProcessCmd_AddAllianceAlGift, NULL}},
    {EN_CLIENT_OPERATE_CMD__RECALL_TEST,            {"operate_recall_test", EN_NORMAL, EN_OP, CProcessOperate::ProcessCmd_RecallTest, NULL}},
    
    {EN_CLIENT_OPERATE_CMD__CHANGE_PLAYER_NAME,     {"operate_player_name", EN_NORMAL, EN_OP, CProcessOperate::ProcessCmd_PlayerName, NULL}},
    {EN_CLIENT_OPERATE_CMD__CHANGE_CITY_NAME,       {"operate_city_name", EN_NORMAL, EN_OP, CProcessOperate::ProcessCmd_CityName, NULL}},
    {EN_CLIENT_OPERATE_CMD__ADD_TROOP,              {"operate_add_troop", EN_NORMAL, EN_OP, CProcessOperate::ProcessCmd_AddTroop, NULL}},
    {EN_CLIENT_OPERATE_CMD__ADD_FORT,               {"operate_add_fort", EN_NORMAL, EN_OP, CProcessOperate::ProcessCmd_AddFort, NULL}},
    {EN_CLIENT_OPERATE_CMD__CLEAN_RESOURCE,         {"operate_clear_resource", EN_NORMAL, EN_OP, CProcessOperate::ProcessCmd_ClearUserResource, NULL}},
    {EN_CLIENT_OPERATE_CMD__ADD_RESOURCE,           {"operate_add_resource", EN_NORMAL, EN_OP, CProcessOperate::ProcessCmd_AddResource, NULL}},
    {EN_CLIENT_OPERATE_CMD__ADD_GEM,                {"operate_add_gem", EN_NORMAL, EN_OP, CProcessOperate::ProcessCmd_GemAdd, NULL}},
    {EN_CLIENT_OPERATE_CMD__ADD_ITEM,               {"operate_add_item", EN_NORMAL, EN_OP, CProcessOperate::ProcessCmd_ItemAdd, NULL}},
    {EN_CLIENT_OPERATE_CMD__SET_ITEM,               {"operate_item_set", EN_NORMAL, EN_OP, CProcessOperate::ProcessCmd_ItemSet, NULL}},
    {EN_CLIENT_OPERATE_CMD__AL_MEMBER_GET,          {"operate_al_mem_get", EN_NORMAL, EN_OP, CProcessOperate::ProcessCmd_AllianceMemberGet, NULL}},
    {EN_CLIENT_OPERATE_CMD__AL_INFO_GET,            {"operate_al_get", EN_NORMAL, EN_OP, CProcessOperate::ProcessCmd_AllianceInfoGet, NULL}},

    {EN_CLIENT_OPERATE_CMD__AL_CHANGE_CHANCELLOR,   {"operate_al_change_chancellor", EN_NORMAL, EN_OP, CProcessOperate::ProcessCmd_AllianceChangeChancellor, NULL}},
    {EN_CLIENT_OPERATE_CMD__RESEARCH_LV_SET,        {"operate_research_lv_set", EN_NORMAL, EN_OP, CProcessOperate::ProcessCmd_ResearchLevelSet, NULL}},
    {EN_CLIENT_OPERATE_CMD__BUILDING_LV_SET,        {"operate_building_lv_set", EN_NORMAL, EN_OP, CProcessOperate::ProcessCmd_BuildingLevelSet, NULL}},
    {EN_CLIENT_OPERATE_CMD__IDENTITY_CHANGE,        {"operate_identity_change", EN_NORMAL, EN_OP, CProcessOperate::ProcessCmd_IdentityChange, NULL}},
    {EN_CLIENT_OPERATE_CMD__STATISTICAL_OPEN_AL_GIFT,   {"operate_statistical_open_al_gift", EN_NORMAL, EN_OP, CProcessOperate::ProcessCmd_StatisticalOpenAlGiftWave, NULL}},

    {EN_CLIENT_OPERATE_CMD__ADD_AL_LOYALTY_FUND,    {"operate_add_loyalty_fund", EN_NORMAL, EN_OP, CProcessOperate::Processcmd_AddLoyaltyAndFund, NULL}},
    {EN_CLIENT_OPERATE_CMD__LIST_BUILDING_POS,      {"operate_list_building_pos", EN_NORMAL, EN_OP, CProcessOperate::Processcmd_ListBuildingPos, NULL}},

    {EN_CLIENT_OPERATE_CMD__ACCOUNT_OPERATE,        {"operate_account_operate", EN_NORMAL, EN_OP, CProcessOperate::Processcmd_AccountOperate, NULL}},
    {EN_CLIENT_OPERATE_CMD__CREATE_NEW_PLAYER_ALLIANCE, {"operate_create_new_player_al", EN_NORMAL, EN_OP, CProcessOperate::ProcessCmd_CreateNewPlayerAlliance, NULL}},

    {EN_CLIENT_OPERATE_CMD__RALLY_PRIDICT_SEND,     {"operate_send_rally_pridict", EN_NORMAL, EN_OP, CProcessOperate::ProcessCmd_SendRallyPridict, NULL}},
    {EN_CLIENT_OPERATE_CMD__GEN_TAX_ACTION,         {"operate_gen_tax_action", EN_NORMAL, EN_OP, CProcessOperate::ProcessGenTaxAction, NULL}},
    {EN_CLIENT_OPERATE_CMD__ALLIANCE_LEAVE,         {"op_al_leave", EN_NORMAL, EN_ALLIANCE, CProcessAlliance::ProcessCmd_AllianceLeave, NULL}},
    {EN_CLIENT_OPERATE_CMD__SET_IAP_KEY,            {"op_set_iap_key", EN_NORMAL, EN_OP, CProcessOperate::ProcessSetIapKey, NULL}},
    {EN_CLIENT_OPERATE_CMD__SVR_CHANGE,             {"op_svr_change", EN_NORMAL, EN_OP, CProcessOperate::ProcessCmd_SvrChange, NULL}},

    {EN_CLIENT_OP_CMD__RELEASE_DRAGON,              {"op_release_dragon", EN_NORMAL, EN_OP, CProcessPlayer::ProcessCmd_ReleaseDragon, NULL}},

    //用于活动奖励
    {EN_OPSELF_ADD_REWARD_LIST,                     {"op_add_reward_list", EN_NORMAL, EN_OP, CProcessOperate::Processcmd_AddRewardListNew, NULL}},
    {EN_OPSELF_ADD_AL_REWARD_LIST,                  {"op_add_al_reward_list", EN_NORMAL, EN_OP, CProcessOperate::Processcmd_AddAlRewardList, NULL}},
    {EN_OPSELF_ADD_AL_REWARD_LIST_NEW,              {"op_add_al_reward_list_new", EN_NORMAL, EN_OP, CProcessOperate::Processcmd_AddAlRewardListNew, NULL}},
    {EN_OPSELF_ADD_THEME_REWARD_LIST,               {"op_add_theme_reward_list", EN_NORMAL, EN_OP, CProcessOperate::Processcmd_AddThemeRewardList, NULL}},
    {EN_OPSELF_ADD_THEME_AL_REWARD_LIST,            {"op_add_theme_al_reward_list", EN_NORMAL, EN_OP, CProcessOperate::Processcmd_AddThemeAlRewardList, NULL}},

    //地图脏数据处理
    {EN_OPSELF_CHECK_PLAYER_MAP,                    {"op_player_map_check", EN_NORMAL, EN_OP, CProcessOperate::ProcessCmd_CheckPlayerMap, NULL}},
    {EN_OPSELF_CHECK_PLAYER_SVR_MAP,                {"op_player_svr_map_check", EN_NORMAL, EN_OP, CProcessOperate::ProcessCmd_CheckPlayerSvrMap, NULL}},
    {EN_OPSELF_RECOVER_PLAYER_CITY,                 {"op_player_recover_city", EN_NORMAL, EN_OP, CProcessOperate::ProcessCmd_RecoverPlayerCity, NULL}},

    {EN_OPSELF_CLEAN_EQUIP_GRID,                    {"op_self_clean_equip_grid", EN_NORMAL, EN_OP, CProcessSelfSystem::Processcmd_CleanEquipGrid, NULL}},
    {EN_OPSELF_SET_GEM,                             {"op_self_set_gem", EN_NORMAL, EN_OP, CProcessSelfSystem::Processcmd_SetGemNum, NULL}},
    {EN_OPSELF_SET_RESOURCE,                        {"op_self_set_resource", EN_NORMAL, EN_OP, CProcessSelfSystem::Processcmd_SetResource, NULL}},
    {EN_OPSELF_SET_TROOP,                           {"op_self_set_troop", EN_NORMAL, EN_OP, CProcessSelfSystem::Processcmd_SetTroop, NULL}},
    {EN_OPSELF_SET_FORT,                            {"op_self_set_fort", EN_NORMAL, EN_OP, CProcessSelfSystem::Processcmd_SetFort, NULL}},
    {EN_OPSELF_SET_ITEM,                            {"op_self_set_item", EN_NORMAL, EN_OP, CProcessSelfSystem::Processcmd_SetItem, NULL}},
    {EN_OPSELF_SET_FUND,                            {"op_self_set_fund", EN_NORMAL, EN_OP, CProcessSelfSystem::Processcmd_SetFund, NULL}},
    {EN_OPSELF_SET_LOYALTY,                         {"op_self_set_loyalty", EN_NORMAL, EN_OP, CProcessSelfSystem::Processcmd_SetLoyalty, NULL}},
    {EN_OPSELF_SET_BUILDING_LV,                     {"op_self_set_building_lv", EN_NORMAL, EN_OP, CProcessSelfSystem::Processcmd_SetBuildingLv, NULL}},
    {EN_OPSELF_SET_POS_BUILDING_LV,                 {"op_self_set_pos_building_lv", EN_NORMAL, EN_OP, CProcessSelfSystem::Processcmd_SetPosBuildingLv, NULL}},
    {EN_OPSELF_SET_RESEARCH_LV,                     {"op_self_set_research_lv", EN_NORMAL, EN_OP, CProcessSelfSystem::Processcmd_SetResearchLv, NULL}},

    {EN_OPSELF_SET_LAST_LOGIN_TIME,                 {"op_self_set_last_login_time", EN_NORMAL, EN_OP, CProcessSelfSystem::Processcmd_SetLastLoginTime, NULL}},
    {EN_OPSELF_SET_CUR_LOYTAL,                      {"op_self_set_cur_loytal", EN_NORMAL, EN_OP, CProcessSelfSystem::Processcmd_SetCurLoytal, NULL}},
    {EN_OPSELF_SET_DRAGON_SHARD,                    {"op_self_set_dragon_shard", EN_NORMAL, EN_OP, CProcessSelfSystem::Processcmd_SetDragonShard, NULL}},


    {EN_OPSELF_ADD_CLEAR_SCROLL,                    {"op_self_add_clear_scroll", EN_NORMAL, EN_OP, CProcessSelfSystem::ProcessCmd_AddClearScroll, NULL}},
    {EN_OPSELF_ADD_CLEAR_TROOP,                     {"op_self_add_clear_troop", EN_NORMAL, EN_OP, CProcessSelfSystem::ProcessCmd_AddClearTroop, NULL}},
    {EN_OPSELF_ADD_CLEAR_FORT,                      {"op_self_add_clear_fort", EN_NORMAL, EN_OP, CProcessSelfSystem::ProcessCmd_AddClearFort, NULL}},
    {EN_OPSELF_ADD_CLEAR_RESOURCE,                  {"op_self_add_clear_resource", EN_NORMAL, EN_OP, CProcessSelfSystem::ProcessCmd_AddClearResource, NULL}},
    {EN_OPSELF_CLEAR_ITEM,                          {"op_self_clear_item", EN_NORMAL, EN_OP, CProcessSelfSystem::ProcessCmd_ClearItem, NULL}},
    {EN_OPSELF_SET_ALL_RESEARCH_LV,                 {"op_self_set_all_research_lv", EN_NORMAL, EN_OP, CProcessSelfSystem::Processcmd_SetAllResearchLv, NULL}},
    {EN_OPSELF_SET_ALL_BUILDING_LV,                 {"op_self_set_all_building_lv", EN_NORMAL, EN_OP, CProcessSelfSystem::Processcmd_SetAllBuildingLv, NULL}},

    {EN_OPSELF_ADD_DEAD_FORT,                       {"op_self_add_dead_fort", EN_NORMAL, EN_OP, CProcessSelfSystem::ProcessCmd_AddDeadFort, NULL}},
    {EN_OPSELF_REDUCE_DEAD_FORT,                    {"op_self_reduce_dead_fort", EN_NORMAL, EN_OP, CProcessSelfSystem::ProcessCmd_ReduceDeadFort, NULL}},
    {EN_OPSELF_ADD_HOS_TROOP,                       {"op_self_add_hos_troop", EN_NORMAL, EN_OP, CProcessSelfSystem::ProcessCmd_AddHosTroop, NULL}},
    {EN_OPSELF_REDUCE_HOS_TROOP,                    {"op_self_reduce_hos_troop", EN_NORMAL, EN_OP, CProcessSelfSystem::ProcessCmd_ReduceHosTroop, NULL}},
    {EN_OPSELF_BREAK_NEW_USER_PROTET,               {"op_self_break_new_protect", EN_NORMAL, EN_OP, CProcessSelfSystem::Processcmd_BreakNewUserProtect, NULL}},
    {EN_OPSELF_SET_VIP_LEVEL,                       {"op_self_set_vip_level", EN_NORMAL, EN_OP, CProcessSelfSystem::Processcmd_SetVipLevel, NULL}},
    {EN_OPSELF_SET_VIP_LEFT_TIME,                   {"op_self_set_vip_left_time", EN_NORMAL, EN_OP, CProcessSelfSystem::Processcmd_SetVipLeftTime, NULL}},

    {EN_OPSELF_GET_BUFFER_INFO,                     {"op_get_buffer_info", EN_NORMAL, EN_OP, CProcessSelfSystem::Processcmd_GetBufferInfo, NULL}},
    
    {EN_OPSELF_SET_PLAYER_LV,                       {"op_set_player_lv", EN_NORMAL, EN_OP, CProcessSelfSystem::Processcmd_SetPlayerLv, NULL}},
    {EN_OPSELF_OPEN_CHEST,                          {"op_open_chest", EN_NORMAL, EN_OP, CProcessSelfSystem::Processcmd_OpenChest, NULL}},
    
    {EN_OPSELF_RESET_FINISH_TASK,                   {"op_reset_finish_task", EN_NORMAL, EN_OP, CProcessSelfSystem::Processcmd_ResetFinishTask, NULL}},
    {EN_OPSELF_SET_FINISH_TASK,                     {"op_set_finish_task", EN_NORMAL, EN_OP, CProcessSelfSystem::Processcmd_SetFinishTask, NULL}},
    {EN_OPSELF_REFRESH_TIME_TASK,                   {"op_refresh_time_task", EN_NORMAL, EN_OP, CProcessSelfSystem::Processcmd_RefreshTaskCur, NULL}},
    {EN_OPSELF_ADD_WILD,                            {"op_add_wild", EN_NORMAL, EN_OP, CProcessSelfSystem::Processcmd_AddWild, NULL}},
    {EN_OPSELF_CLEAR_GUIDE_FLAG,                    {"op_self_clear_guide_flag", EN_NORMAL, EN_OP, CProcessSelfSystem::Processcmd_ClearGuideFlag, NULL}},
    {EN_OPSELF_SET_GUIDE_FLAG,                      {"op_self_set_guide_flag", EN_NORMAL, EN_OP, CProcessSelfSystem::Processcmd_SetGuideFlag, NULL}},
    {EN_OPSELF_SET_PERSON_GUIDE_FLAG,               {"op_self_set_person_guide_flag", EN_NORMAL, EN_OP, CProcessSelfSystem::ProcessCmd_SetPersonGuideFlag, NULL}},
    {EN_OPSELF_CLEAN_SECOND_BUILD_ACTION,           {"op_clean_second_build_action", EN_NORMAL, EN_OP, CProcessSelfSystem::Processcmd_CleanSecondBuildingAction, NULL}},

    {EN_OPSELF_ADD_CRYSTAL,                         {"op_set_add_crystal", EN_NORMAL, EN_OP, CProcessSelfSystem::Processcmd_AddCrystal, NULL}},
    {EN_OPSELF_ADD_SP_CRYSTAL,                      {"op_set_add_sp_crystal", EN_NORMAL, EN_OP, CProcessSelfSystem::Processcmd_AddSpCrystal, NULL}},
    {EN_OPSELF_ADD_MATERIAL,                        {"op_set_add_material", EN_NORMAL, EN_OP, CProcessSelfSystem::Processcmd_AddMaterial, NULL}},
    {EN_OPSELF_ADD_SOUL,                            {"op_set_add_soul", EN_NORMAL, EN_OP, CProcessSelfSystem::Processcmd_AddSoul, NULL}},
    {EN_OPSELF_ADD_PARTS,                           {"op_set_add_parts", EN_NORMAL, EN_OP, CProcessSelfSystem::Processcmd_AddParts, NULL}},
    {EN_OPSELF_ADD_NORMAL_EQUIP,                    {"op_set_add_normal_equip", EN_NORMAL, EN_OP, CProcessSelfSystem::Processcmd_AddNormalEquip, NULL}},
    {EN_OPSELF_ADD_SP_EQUIP,                        {"op_set_add_sp_equip", EN_NORMAL, EN_OP, CProcessSelfSystem::Processcmd_AddSpEquip, NULL}},

    {EN_OPSELF_CLEAN_ALL_CRYSTAL,                   {"op_clean_all_crystal", EN_NORMAL, EN_OP, CProcessSelfSystem::Processcmd_CleanAllCrystal, NULL}},
    {EN_OPSELF_CLEAN_ALL_MATERIAL,                  {"op_clean_all_material", EN_NORMAL, EN_OP, CProcessSelfSystem::Processcmd_CleanAllMaterial, NULL}},
    {EN_OPSELF_CLEAN_ALL_SOUL,                      {"op_clean_all_soul", EN_NORMAL, EN_OP, CProcessSelfSystem::Processcmd_CleanAllSoul, NULL}},
    {EN_OPSELF_CLEAN_ALL_PARTS,                     {"op_clean_all_parts", EN_NORMAL, EN_OP, CProcessSelfSystem::Processcmd_CleanAllParts, NULL}},
    {EN_OPSELF_CLEAN_ALL_EQUIP,                     {"op_clean_all_equip", EN_NORMAL, EN_OP, CProcessSelfSystem::Processcmd_CleanAllEquip, NULL}},

    {EN_OPSELF_SET_AL_GIFT_TIME,                    {"op_self_set_al_gift_time", EN_NORMAL, EN_OP, CProcessSelfSystem::Processcmd_SetAlGiftTime, NULL}},
    {EN_OPSELF_SET_AL_GIFT_LEVEL,                   {"op_self_set_al_gift_level", EN_NORMAL, EN_OP, CProcessSelfSystem::Processcmd_SetAlGiftLv, NULL}},
    {EN_OPSELF_ADD_RECOMMEND_PLAYER,                {"op_self_add_recommend_player", EN_NORMAL, EN_OP, CProcessSelfSystem::Processcmd_AddRecommendPlayer, NULL}},
    {EN_OPSELF_DEL_INVITE_RECORD,                   {"op_self_del_invite_record", EN_NORMAL, EN_OP, CProcessSelfSystem::Processcmd_DelInviteRecord, NULL}},
    {EN_OPSELF_SET_AL_STAR,                         {"op_self_set_al_star", EN_NORMAL, EN_OP, CProcessSelfSystem::Processcmd_SetAlStar, NULL}},
    {EN_OPSELF_SET_ASSIST_POST_TIME,                {"op_self_set_assist_time", EN_NORMAL, EN_OP, CProcessSelfSystem::Processcmd_SetAssistPostTime, NULL}},
    {EN_OPSELF_SET_LAST_UPDATE_TIME,                {"op_self_set_update_time", EN_NORMAL, EN_OP, CProcessSelfSystem::Processcmd_SetLastUpdateTime, NULL}},
    {EN_OPSELF_RELEASE_DRAGON,                      {"op_self_release_hero", EN_NORMAL, EN_OP, CProcessSelfSystem::Processcmd_ReleaseSelfDragon, NULL}},
    {EN_OPSELF_SET_THRONE_TROOP,                    {"op_self_set_throne_troop", EN_SPECIAL, EN_OP, CProcessSelfSystem::Processcmd_SetThroneTroop, NULL}},
    //{EN_OPSELF_SET_MAIL_TIME,                       {"op_self_set_mail_time", EN_NORMAL, EN_OP, CProcessMailReport::Processcmd_SetInviteMailTime, NULL}},
    {EN_OPSELF_CLEAR_RECOMMEND,                     {"op_self_clear_recommend", EN_NORMAL, EN_OP, CProcessSelfSystem::ProcessCmd_ClearRecommend, NULL}},
    {EN_OPSELF_CLEAR_INVITED_COUNT,                 {"op_self_clear_invited_count", EN_NORMAL, EN_OP, CProcessSelfSystem::ProcessCmd_ClearInvitedCount, NULL}},

    {EN_OPSELF_SET_DRAGON_EXCUTE_TIME,              {"op_self_set_hero_excute_time", EN_NORMAL, EN_OP, CProcessSelfSystem::Processcmd_SetDragonExcuteTime, NULL}},
    {EN_OPSELF_SET_DRAGON_AUTO_RELEASE_TIME,        {"op_self_set_hero_release_time", EN_NORMAL, EN_OP, CProcessSelfSystem::Processcmd_SetDragonAutoReleaseTime, NULL}},
    {EN_OPSELF_CUT_PREPARE_TIME,                    {"op_self_cut_prepare_time", EN_NORMAL, EN_OP, CProcessSelfSystem::Processcmd_CutPrepareTime, NULL}},
    //{EN_OPSELF_SET_THRONE_TIME,                     {"op_self_set_throne_time", EN_NORMAL, EN_OP, CProcessSelfSystem::Processcmd_SetThroneTime, NULL}},
    //{EN_OPSELF_RECOVER_THRONE_TIME,                 {"op_recover_throne_time", EN_SPECIAL, EN_OP, CProcessSelfSystem::Processcmd_RecoverThroneTime, NULL}},
    {EN_OPSELF_CAPTURE_DRAGON,                      {"op_self_capture_hero", EN_NORMAL, EN_OP, CProcessSelfSystem::Processcmd_CaptureDragon, NULL}},
    {EN_OPSELF_KILL_SELF_DRAGON,                    {"op_self_kill_self_hero", EN_NORMAL, EN_OP, CProcessSelfSystem::Processcmd_KillSelfDragon, NULL}},
    {EN_OPSELF_ADD_GLOBALRES,                       {"op_add_globalres", EN_NORMAL, EN_OP, CProcessSelfSystem::Processcmd_OpAddGlobalres, NULL}},
    {EN_OPSELF_HELP_SELF_ACTION,                    {"op_help_self_action", EN_NORMAL, EN_OP, CProcessSelfSystem::Processcmd_HelpSelfAction, NULL}},
    {EN_OPSELF_SET_ALTAR_BUFF_TIME,                 {"op_self_set_altar_buff_time", EN_NORMAL, EN_OP, CProcessSelfSystem::Processcmd_SetAltarBuffTime, NULL}},
    {EN_OPSELF_ADD_VIP_POINT,                       {"op_self_add_vip_point", EN_NORMAL, EN_OP, CProcessSelfSystem::Processcmd_AddVipPoint, NULL}},
    {EN_OPSELF_SET_QUEST_REFRESH_TIME,              {"op_self_set_quest_refresh_time", EN_NORMAL, EN_OP, CProcessSelfSystem::Processcmd_SetQuestRefresh, NULL}},
    {EN_OPSELF_SET_QUEST_FINISH_TIME,               {"op_self_set_quest_finish_time", EN_NORMAL, EN_OP, CProcessSelfSystem::Processcmd_SetFinishQuest, NULL}},
    {EN_OPSELF_CHANGE_GLOBALRES,                    {"op_change_globalres", EN_NORMAL, EN_OP, CProcessSelfSystem::Processcmd_OpChangeGlobalres, NULL}},
    {EN_OPSELF_SET_GLOBALRES,                       {"op_set_globalres", EN_NORMAL, EN_OP, CProcessSelfSystem::Processcmd_OpSetGlobalres, NULL}},

    {EN_OPSELF_AL_WALL_SET_TIME,                    {"operate_al_wall_set_time", EN_NORMAL, EN_OP, CProcessSelfSystem::ProcessCmd_SetAlWallNewestTime, NULL}},

    {EN_OPSELF_CLEAN_ALL_TREE,                      {"op_self_clean_all_tree", EN_NORMAL, EN_OP, CProcessSelfSystem::Processcmd_CleanAllTree, NULL}},
    {EN_OPSELF_CLEAR_ALL_BUFF,                      {"op_self_clear_all_buff", EN_NORMAL, EN_OP, CProcessSelfSystem::Processcmd_ClearAllBuff, NULL}},
    {EN_OPSELF_CLEAN_ALL_TITLE,                     {"op_self_clean_all_title", EN_NORMAL, EN_OP, CProcessSelfSystem::Processcmd_CleanTitle, NULL}},
    {EN_OPSELF_SEND_BROADCAST,                      {"op_send_broadcast", EN_NORMAL, EN_OP, CProcessSelfSystem::Processcmd_SendBroadcast, NULL}},
    {EN_CLIENT_OPERATE_CMD__RESET_MAP,              {"op_reset_map", EN_SPECIAL, EN_OP, CProcessOperate::ResetMap, NULL}},
    {EN_CLIENT_OPERATE_CMD__CLEAR_NOPLAYER_MAP,     {"op_clear_noplayer_map", EN_SPECIAL, EN_OP, CProcessOperate::ClearNoPlayerMap, NULL}},

    {EN_OPSELF_SET_TRADE_REFRESH_TIME,              {"op_self_set_trade_refresh_time", EN_NORMAL, EN_OP, CProcessSelfSystem::Processcmd_SetTradeRefreshTime, NULL}},
    {EN_OPSELF_SET_TRADE_MARCHING_TIME,             {"op_self_set_trade_march_time", EN_NORMAL, EN_OP, CProcessSelfSystem::Processcmd_SetTradeMarchTime, NULL}},
    {EN_OPSELF_SET_TRADE_WAITING_TIME,              {"op_self_set_trade_waiting_time", EN_NORMAL, EN_OP, CProcessSelfSystem::Processcmd_SetTradeWaitingTime, NULL}},
    {EN_OPSELF_SET_MYSTERY_STORE_REFRESH_TIME,      {"op_self_set_mystery_store_refresh_time", EN_NORMAL, EN_OP, CProcessSelfSystem::Processcmd_SetMysteryStoreRefreshTime, NULL}},
    {EN_OPSELF_SET_VIOLET_GOLD,                     {"op_self_set_violet_gold", EN_NORMAL, EN_OP, CProcessSelfSystem::Processcmd_SetVioletGold, NULL}},
    {EN_OPSELF_SET_USER_CREATE_TIME,                {"op_self_set_user_create_time", EN_NORMAL, EN_OP, CProcessSelfSystem::Processcmd_SetUserCreateTime, NULL }},
    {EN_OPSELF_SET_CONTINUE_LOGIN_DAY,              {"op_self_set_continue_login_day", EN_NORMAL, EN_OP, CProcessSelfSystem::Processcmd_SetContinueLoginDay, NULL }},

    {EN_OPSELF_SET_BOUNTY_REFRESH_TIME,             {"op_self_set_bounty_refresh_time", EN_NORMAL, EN_OP, CProcessSelfSystem::ProcessCmd_SetBountyRefreshTime, NULL}},
    {EN_OPSELF_SET_BOUNTY_NODE_START_NUM,           {"op_self_set_bounty_node_num", EN_NORMAL, EN_OP, CProcessSelfSystem::ProcessCmd_SetBountyNodeStarNum, NULL}},
    {EN_OPSELF_SET_PERSONAL_HELP_BUBBLE,            {"op_self_set_personal_help_bubble", EN_NORMAL, EN_OP, CProcessSelfSystem::ProcessCmd_SetPersonalHelpBubble, NULL}},
    {EN_OPSELF_SET_HELP_BUBBLE_TIMEOUT,             {"op_self_set_help_bubble_timeout", EN_NORMAL, EN_OP, CProcessSelfSystem::ProcessCmd_SetHelpBubbleTimeOut, NULL} },
    //{EN_OPSELF_SET_TAX_TIME,                        {"op_self_set_tax_time", EN_NORMAL, EN_OP, CProcessSelfSystem::ProcessCmd_SetTaxTime, NULL}},
    {EN_OPSELF_CLEAR_SUPPORT_TIME_TAG,              {"op_self_clear_support_timetag", EN_NORMAL, EN_OP, CProcessSelfSystem::ProcessCmd_ClearSupportTimeTag, NULL}},
    {EN_CLIENT_REQ_COMMAND__RATING_REWARD_COLLECT,  {"rating_reward_collect", EN_NORMAL, EN_PLAYER, CProcessPlayer::ProcessCmd_RatingRewardCollect, NULL}},

    {EN_CLIENT_OPERATE_CMD__CLEAR_DEAD_PLAYER,      {"operate_clear_user", EN_NORMAL, EN_OP, CProcessOperate::ProcessCmd_ClearUser, NULL}},
    {EN_CLIENT_OPERATE_CMD__BLOCK_HACKER,           {"operate_block_hacker", EN_NORMAL, EN_OP, CProcessOperate::ProcessCmd_BlockHacker, NULL}},

    {EN_CLIENT_OP_CMD__CHANGE_SVR,                  {"op_change_svr", EN_NORMAL, EN_OP, CProcessOperate::ProcessCmd_ChangeSvr, NULL}},

    //压测用命令字
    {EN_CLIENT_OPERATE_CMD__HU_PRESUSURE_MEASURE,   {"op_hu_pressure_measure", EN_NORMAL, EN_PLAYER, CProcessOperate::ProcessCmd_HuPressureMeasure, NULL}},
    {EN_CLIENT_OPERATE_CMD__AU_PRESUSURE_MEASURE,   {"op_au_pressure_measure", EN_NORMAL, EN_PLAYER, CProcessOperate::ProcessCmd_AuPressureMeasure, NULL}},
    {EN_CLIENT_OPERATE_CMD__CLEAN_AU_PRESUSURE_ACTION,  {"op_clean_au_pressure_action", EN_NORMAL, EN_PLAYER, CProcessOperate::ProcessCmd_CleanAuPressureAction, NULL}},

    // 测试cache
    {EN_CLIENT_OPERATE_CMD__CACHE_TEST,             {"op_cache_test", EN_NORMAL, EN_OP, CProcessOperate::ProcessCmd_CacheTest, NULL}},
    // 测试上报统计信息到agent
    {EN_CLIENT_OPERATE_CMD__WARNING_TEST,           {"op_warning_test", EN_NORMAL, EN_OP, CProcessOperate::ProcessCmd_WarningTest, NULL}},
    // 测试翻译
    {EN_CLIENT_OPERATE_CMD__TRANSLATE_TEST,         {"op_translate_test", EN_NORMAL, EN_OP, CProcessOperate::ProcessCmd_TranslateTest, NULL}},

    // 客服系统命令字
    {EN_CLIENT_OPERATE_CMD__SET_HELP_BUBBLE_STATUS, {"op_set_help_bubble", EN_SPECIAL, EN_OP, CProcessOperate::ProcessCmd_SetHelpBubble, NULL}},

    {EN_OPSELF_SET_DRAGON_LEVEL,                    {"op_self_set_dragon_level", EN_NORMAL, EN_OP, CProcessSelfSystem::Processcmd_SetDragonLevel, NULL}},
    {EN_OPSELF_SET_PLAYER_LEVEL,                    {"op_self_set_player_level", EN_NORMAL, EN_OP, CProcessSelfSystem::Processcmd_SetPlayerLevel, NULL}},
    {EN_OPSELF_SET_KNIGHT_LEVEL,                    {"op_self_set_knight_level", EN_NORMAL, EN_OP, CProcessSelfSystem::Processcmd_SetKnightLevel, NULL}},
    {EN_OPSELF_SET_BUFF_FAILURE_TIME,               {"op_self_set_buff_failure_time", EN_NORMAL, EN_OP, CProcessSelfSystem::Processcmd_SetBuffFailureTime, NULL}},
    {EN_OPSELF_SET_MONSTER_WILD_REFRESH_TIME,       {"op_self_set_monster_wild_refresh_time", EN_NORMAL, EN_OP, CProcessSelfSystem::Processcmd_SetMonsterWildRefreshTime, NULL}},
    {EN_OPSELF_SET_SMOKE_FIRE_DISAPPEAR_TIME,       {"op_self_set_smoke_fire_disappear_time", EN_NORMAL, EN_OP, CProcessSelfSystem::Processcmd_SetSmokeFireDisappearTime, NULL}},
    {EN_OPSELF_SET_CONSECUTIVE,                     {"op_self_set_consecutive_login", EN_NORMAL, EN_OP, CProcessSelfSystem::Processcmd_SetConsecutiveLogin, NULL}},

    // reward 增减接口
    {EN_CLIENT_OP_CMD__REWARD_ADD_MINUS_FOR_OP,     {"op_reward_add_minus_for_op", EN_NORMAL, EN_OP, CProcessOperate::Processcmd_AddRewardListForOp, NULL}},
    {EN_OPSELF_SET_TASK_IN_SHOWLIST,                {"op_self_set_task_in_showlist", EN_NORMAL, EN_OP, CProcessSelfSystem::ProcessCmd_SetTaskInShowList, NULL}},

    {EN_OPSELF_CLEAR_MONSTER_HIT,                   {"op_self_clear_monster_hit", EN_NORMAL, EN_OP, CProcessSelfSystem::ProcessCmd_ResetMonsterHit, NULL}},
    {EN_OPSELF_SET_MAP,                             {"op_self_set_map", EN_NORMAL, EN_OP, CProcessSelfSystem::ProcessCmd_SetMap, NULL}},
    {EN_OPSELF_SET_MAP_EXPIRE,                      {"op_self_set_map_expire", EN_NORMAL, EN_OP, CProcessSelfSystem::ProcessCmd_SetMapExpire, NULL}},
    {EN_OPSELF_SET_RALLY_TIME,                      {"op_self_set_rally_time", EN_NORMAL, EN_OP, CProcessSelfSystem::ProcessCmd_SetRallyTime, NULL}},
    {EN_OPSELF_SET_CAMP_PROTECT_TIME,               {"op_self_set_camp_protect_time", EN_NORMAL, EN_OP, CProcessSelfSystem::ProcessCmd_SetCampProtectTime, NULL}},
    {EN_OPSELF_SET_TRIAL_LOCK,                      {"op_self_set_trial_lock", EN_NORMAL, EN_OP, CProcessSelfSystem::ProcessCmd_SetTrialLock, NULL}},
    {EN_OPSELF_ADD_AL_GIFT_POINT,                   {"op_self_add_al_gift_point", EN_NORMAL, EN_OP, CProcessSelfSystem::ProcessCmd_AddAlGiftPoint, NULL}},
    {EN_OPSELF_ADD_AL_LOYALTY_AND_FUND,             {"op_self_add_al_loyalty_and_fund", EN_NORMAL, EN_OP, CProcessSelfSystem::ProcessCmd_AddAlLoyaltyAndFund, NULL}},
    {EN_OPSELF_SET_ATTACK_MOVE_TIME,                {"op_self_set_attack_move_time", EN_NORMAL, EN_OP, CProcessSelfSystem::ProcessCmd_SetAttackMoveTime, NULL}},
    {EN_OPSELF_SET_RALLY_ATTACK_SLOT_ALL_OPEN,      {"op_self_set_rally_attack_slot_all_open", EN_NORMAL, EN_OP, CProcessSelfSystem::ProcessCmd_SetRallyAttackSlotAllOpen, NULL}},
    {EN_OPSELF_SET_AL_GIFT_DISAPPEAR_TIME,          {"op_self_set_al_gift_disappear_time", EN_NORMAL, EN_OP, CProcessSelfSystem::ProcessCmd_SetAlGiftDisappearTime, NULL}},

    {EN_CLIENT_OP_CMD_SET_SYSTEM_TIME,              {"op_set_system_time", EN_SPECIAL, EN_OP, CProcessSelfSystem::ProcessCmd_SetSystemTime, NULL }},

    {EN_CLIENT_OP_CMD_SET_DRAGON_FLAG,              {"op_set_dragon_flag", EN_SPECIAL, EN_OP, CProcessOperate::ProcessCmd_SetDragonFlag, NULL}},

    {EN_CLIENT_REQ_COMMAND__ADD_RANDOM_REWARD,      {"add_random_reward", EN_SPECIAL, EN_PLAYER, CProcessPlayer::ProcessCmd_AddRandomReward, NULL}},
    {EN_CLIENT_REQ_COMMAND__GET_RANDOM_REWARD,      {"get_random_reward", EN_SPECIAL, EN_PLAYER, CProcessPlayer::ProcessCmd_GetRandomReward, NULL}},

    {EN_CLIENT_REQ_COMMAND__CLAIM_RANDOM_REWARD,    {"claim_random_reward", EN_NORMAL, EN_PLAYER, CProcessPlayer::ProcessCmd_ClaimRandomReward, NULL}},

    {EN_CLIENT_OP_CMD__PRO_SYS_GET_DATA,            {"getdata", EN_NORMAL, EN_OP, CProcessOperate::ProcessCmd_ProSysGetData, NULL}},
    {EN_CLIENT_OP_CMD__GEN_MOVE_ACTION,             {"op_gen_move_action", EN_NORMAL, EN_OP, CProcessOperate::ProcessCmd_GenAttackMoveAction, NULL}},

    {EN_CLIENT_OP_CMD__GEN_IDOL,                    {"op_gen_idol", EN_SPECIAL, EN_OP, CProcessOperate::ProcessCmd_GenIdol, NULL}},
    {EN_CLIENT_OP_CMD__GEN_THRONE,                  {"op_gen_throne", EN_SPECIAL, EN_OP, CProcessOperate::ProcessCmd_GenThrone, NULL}},

    {EN_CLIENT_REQ_COMMAND__IDOL_ATTACK,            {"idol_attack", EN_NORMAL, EN_MARCH, CProcessMarch::ProcessCmd_IdolAttack, NULL}},
    {EN_CLIENT_REQ_COMMAND__GET_IDOL_INFO,          {"get_idol_info", EN_SPECIAL, EN_MAP, CProcessThrone::ProcessCmd_GetIdolInfo, NULL}},

    {EN_CLIENT_REQ_COMMAND__THRONE_ATTACK,          {"throne_attack", EN_NORMAL, EN_MARCH, CProcessMarch::ProcessCmd_ThroneAttack, NULL}},
    {EN_CLIENT_REQ_COMMAND__THRONE_RALLY_WAR,       {"throne_rally_war", EN_NORMAL, EN_MARCH, CProcessMarch::ProcessCmd_ThroneRallyWar, NULL}},
    {EN_CLIENT_REQ_COMMAND__THRONE_REINFORCE,       {"throne_reinforce", EN_NORMAL, EN_MARCH, CProcessMarch::ProcessCmd_ThroneReinforce, NULL}},
    {EN_CLIENT_REQ_COMMAND__THRONE_DISPATCH,        {"throne_dispatch", EN_NORMAL, EN_MARCH, CProcessMarch::ProcessCmd_ThroneDispatch, NULL}},
    //{EN_CLIENT_REQ_COMMAND__THRONE_DISMISS_KNIGHT_DRAGON,   {"throne_dismiss_knight_dragon", EN_NORMAL, EN_MARCH, CProcessMarch::ProcessCmd_ThroneDismissKnightDragon, NULL}},
    {EN_CLIENT_REQ_COMMAND__THRONE_DISMISS_REINFORCE,       {"throne_dismiss_reinforce", EN_NORMAL, EN_MARCH, CProcessMarch::ProcessCmd_ThroneDismissReinforce, NULL}},
    {EN_CLIENT_REQ_COMMAND__THRONE_DISMISS_ALL,             {"throne_dismiss_all", EN_NORMAL, EN_MARCH, CProcessMarch::ProcessCmd_ThroneDismissAll, NULL}},
    {EN_CLIENT_REQ_COMMAND__THRONE_RECALL_KNIGHT_DRAGON,    {"throne_recall_knight_dragon", EN_NORMAL, EN_MARCH, CProcessMarch::ProcessCmd_ThroneRecallKnightDragon, NULL}},
    {EN_CLIENT_REQ_COMMAND__THRONE_RECALL_REINFORCE,        {"throne_recall_reinforce", EN_NORMAL, EN_MARCH, CProcessMarch::ProcessCmd_ThroneRecallReinforce, NULL}},
    {EN_CLIENT_REQ_COMMAND__THRONE_REINFORCE_SPEEDUP,       {"throne_reinforce_speedup", EN_NORMAL, EN_MARCH, CProcessMarch::ProcessCmd_ThroneReinforceSpeedup, NULL}},
    {EN_CLIENT_REQ_COMMAND__GET_THRONE_INFO,        {"get_throne_info", EN_SPECIAL, EN_MAP, CProcessThrone::ProcessCmd_GetThroneInfo, NULL}},
    {EN_CLIENT_REQ_COMMAND__GET_TITLE_INFO,         {"get_title_info", EN_SPECIAL, EN_MAP, CProcessThrone::ProcessCmd_GetTitleInfo, NULL}},
    //{EN_CLIENT_REQ_COMMAND__THRONE_DISMISS_TITLE,   {"throne_dismiss_title", EN_NORMAL, EN_PLAYER, CProcessThrone::ProcessCmd_ThroneDismissTitle, NULL}},
    {EN_CLIENT_REQ_COMMAND__THRONE_DUB_TITLE,       {"throne_dub_title", EN_NORMAL, EN_PLAYER, CProcessThrone::ProcessCmd_ThroneDubTitle, NULL}},
    {EN_CLIENT_REQ_COMMAND__THRONE_ABANDON,         {"throne_abandon", EN_NORMAL, EN_ALLIANCE, CProcessThrone::ProcessCmd_ThroneAbandon, NULL}},
    {EN_CLIENT_REQ_COMMAND__REINFORCE_SPEEDUP,      {"reinforce_speedup", EN_NORMAL, EN_MARCH, CProcessMarch::ProcessCmd_ReinforceSpeedup, NULL}},


    {EN_CLIENT_OP_CMD__NPC_UP_TO_10,                  {"npc_up_to_10", EN_NORMAL, EN_OP, CProcessOperate::ProcessCmd_UpgradeNpcToLv10, NULL}},

    {EN_OPSELF_SET_IDOL_TIME,                       {"op_set_idol_time", EN_NORMAL, EN_OP, CProcessSelfSystem::Processcmd_SetIdolTime, NULL}},
    {EN_OPSELF_FILL_IDOL_RANK,                      {"op_fill_idol_rank", EN_NORMAL, EN_OP, CProcessSelfSystem::Processcmd_FillIdolRank, NULL}},
    {EN_OPSELF_SET_TITLE_EXPIRE_TIME,               {"op_set_title_expire_time", EN_NORMAL, EN_OP, CProcessSelfSystem::Processcmd_SetTitleExpireTime, NULL}},
    {EN_OPSELF_SET_TAX_BEGIN_TIME,                  {"op_set_tax_begin_time", EN_NORMAL, EN_OP, CProcessSelfSystem::Processcmd_SetTaxBeginTime, NULL}},
    {EN_OPSELF_SET_TAX_INTERVAL_TIME,               {"op_set_tax_interval_time", EN_NORMAL, EN_OP, CProcessSelfSystem::Processcmd_SetTaxIntervalTime, NULL}},
    {EN_OPSELF_SET_THRONE_TIME_NEW,                 {"op_set_throne_time", EN_NORMAL, EN_OP, CProcessSelfSystem::Processcmd_SetThroneTimeNew, NULL}},

    {EN_OP_GET_RANK,                                {"op_get_rank", EN_SPECIAL, EN_OP, CProcessOperate::ProcessCmd_GetRank, NULL}},

    {EN_OPSELF_SET_PEACE_TIME,                      {"op_set_peace_time", EN_NORMAL, EN_OP, CProcessSelfSystem::Processcmd_GenPeaceTime, NULL}},

    {EN_CLIENT_REQ_COMMAND__END,                    {"cmdend", EN_UNKNOW_PROCEDURE, EN_UNKNOW_CMD, NULL, NULL}},
    
};

CClientCmd::CClientCmd()
{

}

CClientCmd *CClientCmd::GetInstance()
{
    if(NULL == m_poClientCmdInstance)  //判断是否第一次调用 
    {
        m_poClientCmdInstance = new CClientCmd;
    }
    return m_poClientCmdInstance;
}

TINT32 CClientCmd::Init()
{
    Init_CmdInfo();
    Init_CmdMap();
    Init_CmdEnumMap();

    return 0;
}

TINT32 CClientCmd::Init_CmdMap()
{
    for(TINT32 dwIdx = 0; dwIdx < EN_CLIENT_REQ_COMMAND__END; ++dwIdx)
    {
        m_oCmdMap.insert(make_pair(m_stszClientReqCommand[dwIdx].udwCmdEnum, m_stszClientReqCommand[dwIdx].stFunctionSet));
    }

    return 0;
}

TINT32 CClientCmd::Init_CmdEnumMap()
{
    for(TINT32 dwIdx = 0; dwIdx < EN_CLIENT_REQ_COMMAND__END; ++dwIdx)
    {
        m_oCmdEnumMap.insert(make_pair(m_stszClientReqCommand[dwIdx].stFunctionSet.szCmdName, m_stszClientReqCommand[dwIdx].udwCmdEnum));
    }
    
    return 0;
}

TINT32 CClientCmd::Init_CmdInfo()
{
    // todo: 命令字输出有待优化
    memmove(m_stszClientReqCommand, stszClientReqCommand, sizeof(m_stszClientReqCommand));

    return 0;
}

TUINT32 CClientCmd::GetCommandID( const char* pszCommand )
{
    EClientReqCommand udwCommandID = EN_CLIENT_REQ_COMMAND__UNKNOW;

    map<string, EClientReqCommand>::iterator itCmdEnum;
    itCmdEnum = m_oCmdEnumMap.find(pszCommand);
    if(itCmdEnum != m_oCmdEnumMap.end())
    {
        udwCommandID = itCmdEnum->second;
    }

    return udwCommandID;
}

map<EClientReqCommand, struct SFunctionSet> *CClientCmd::Get_CmdMap()
{
    return &m_oCmdMap;
}

map<string, EClientReqCommand> *CClientCmd::Get_CmdEnumMap()
{
    return &m_oCmdEnumMap;
}

TUINT32 CClientCmd::GetCmdType(TUINT32 udwCommandId)
{
    TUINT32 udwCmdType = EN_UNKNOW_CMD;

    CClientCmd *poCClientCmd = CClientCmd::GetInstance();
    struct SFunctionSet stProcessFunctionSet;
    map<EClientReqCommand, struct SFunctionSet>::iterator itCmdFunctionSet;
    itCmdFunctionSet = poCClientCmd->Get_CmdMap()->find((EClientReqCommand)udwCommandId);
    if (itCmdFunctionSet != poCClientCmd->Get_CmdMap()->end())
    {
        udwCmdType = itCmdFunctionSet->second.dwCmdType;
    }

    return udwCmdType;
}