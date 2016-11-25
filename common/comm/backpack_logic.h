#ifndef _BACKPACK_LOGIC_H_
#define _BACKPACK_LOGIC_H_

#include "player_info.h"

class CBackpack
{

public:
    //id
    static TUINT64 GenEquipId(TbLogin *pstLogin);

    //gen equip
    static TINT32 ComposeNormalEquip(SUserInfo *pstUser, TUINT32 udwEquipType, TUINT32 udwScrollId, const vector<TUINT32> &vMaterial, 
        SEquipMentInfo *pstEquip, TBOOL bInstallFinish, TINT32 dwLv);
    static TINT32 ComposeSpecialEquip(SUserInfo *pstUser, TINT32 dwResearchParam, TUINT32 udwSoulId, 
        vector<TUINT32> *pvParts, SEquipMentInfo *pstEquip, TBOOL bInstallFinish,TBOOL bMistery);

    //info
    static TINT32 GetEquipInfoById(TbEquip *patBackpack, TUINT32 udwEquipNum, TUINT64 uddwEquipId, SEquipMentInfo *pstEquip);
    static TINT32 GetEquipBaseInfoByEid(TUINT32 udwEquipId, SEquipMentInfo *pstEquip);
    static TINT32 GetCrystalInfoById(TUINT32 udwCrystalId, SCrystalInfo *pstCrystal);
    static TINT32 GetMaterialInfoById(TUINT32 udwMaterialId, SMaterialInfo *pstCrystal);
    static TINT32 GetSoulInfoById(TUINT32 udwCrystalId, SSoulInfo *pstCrystal);
    static TINT32 GetPartsInfoById(TUINT32 udwpartsId, SSoulInfo *pstPart);
    static TINT32 GetSpCrystalInfoById(TUINT32 udwCrystalId, SCrystalInfo *pstCrystal);

    //add
    static TVOID AddEquip(SUserInfo *pstUser, SEquipMentInfo *pstEquip);
    static TVOID AddCrystal(SUserInfo *pstUser, TUINT32 udwCrystalId, TUINT32 udwNum = 1);
    static TVOID SetCrystal(SUserInfo *pstUser, TUINT32 udwCrystalId, TUINT32 udwNum);
    static TVOID AddSoul(SUserInfo *pstUser, TUINT32 udwSoulId, TUINT32 udwNum = 1);
    static TVOID AddParts(SUserInfo *pstUser, TUINT32 udwPartsId, TUINT32 udwNum = 1);
    static TVOID AddMaterial(SUserInfo *pstUser, TUINT32 udwMaterialId, TUINT32 udwNum = 1);
    static TVOID AddSpCrystal(SUserInfo *pstUser, TUINT32 udwCrystalId, TUINT32 udwNum = 1);
    static TVOID AddScroll(SUserInfo *pstUser, TUINT32 udwScrollId, TUINT32 udwNum = 1);
    static TVOID SetScroll(SUserInfo *pstUser, TUINT32 udwScrollId, TUINT32 udwNum = 1);
    static TVOID OpAddMaterial(SUserInfo *pstUser, TUINT32 udwMaterialId, TINT64 ddwNum);
    static TVOID OpSetMaterial(SUserInfo *pstUser, TUINT32 udwMaterialId, TUINT32 udwNum);

    static TINT32 AddNormalEquip(SUserInfo *pstUser, TUINT32 udwEquipType, TUINT32 udwEquipLv, TINT32 udwNum = 1);

    //has enough 
    static TBOOL HasEnoughCrystal(SUserInfo *pstUser, TUINT32 udwCrystalId, TUINT32 udwNum = 1);
    static TBOOL HasEnoughSoul(SUserInfo *pstUser, TUINT32 udwSoulId, TUINT32 udwNum = 1);
    static TBOOL HasEnoughParts(SUserInfo *pstUser, TUINT32 udwPartsId, TUINT32 udwNum = 1);
    static TBOOL HasEnoughMaterial(SUserInfo *pstUser, TUINT32 udwMaterialId, TUINT32 udwNum = 1);
    static TBOOL HasEnoughSpCrystal(SUserInfo *pstUser, TUINT32 udwCrystalId, TUINT32 udwNum = 1);
    static TBOOL HasEnoughScroll(SUserInfo *pstUser, TUINT32 udwScrollId, TUINT32 udwNum = 1);

    //cost
    static TVOID CostCrystal(SUserInfo *pstUser, TUINT32 udwCrystalId, TUINT32 udwNum = 1);
    static TVOID CostSoul(SUserInfo *pstUser, TUINT32 udwSoulId, TUINT32 udwNum = 1);
    static TVOID CostParts(SUserInfo *pstUser, TUINT32 udwPartsId, TUINT32 udwNum = 1);
    static TVOID CostMaterial(SUserInfo *pstUser, TUINT32 udwMaterialId, TUINT32 udwNum = 1);
    static TVOID CostSpCrystal(SUserInfo *pstUser, TUINT32 udwCrystalId, TUINT32 udwNum = 1);
    static TVOID CostScroll(SUserInfo *pstUser, TUINT32 udwScrollId, TUINT32 udwNum = 1);
    static TVOID DropScroll(SUserInfo *pstUser, TUINT32 udwScrollId);

    //get num
    static TINT32 GetCrystalNumById(SUserInfo *pstUser, TUINT32 udwCrystalId);
    static TINT32 GetSoulNumById(SUserInfo *pstUser, TUINT32 udwCrystalId);
    static TINT32 GetPartsNumById(SUserInfo *pstUser, TUINT32 udwCrystalId);
    static TINT32 GetMaterialNumById(SUserInfo *pstUser, TUINT32 udwCrystalId);
    static TINT32 GetSpCrystalNumById(SUserInfo *pstUser, TUINT32 udwCrystalId);

    //destroy equip function
    static TINT32 GetMaterialIdByTypeAndLv(TUINT32 udwType,TUINT32 udwLv);
    static TINT32 GetMaterialTypeByEquipType(TUINT32 udwEquipType);

    //soul equip 
    static TINT32 CheckSoulEquip(SUserInfo *pstUser, TbPlayer *pstPlayer);


    static TINT32 GetEquipIdByTypeAndLv(TUINT32 udwType, TUINT32 udwLv);
    static TINT32 GetNormalEquipLv(TUINT32 udwMaterialLvTotal);
    static TINT32 GenRandomValue(TINT32 dwInputNum, TUINT32 *audwOutputValue, TUINT32 udwOutPutNum);
    static TINT32 GetRandomBuffNum(TINT32 dwResearchNum,TINT32 dwMinNum,TINT32 dwMaxNum);

    static TINT32 GetNormalEquipCostTime(TUINT32 udwEid, SMaterialInfo *aMaterialInfo, TUINT32 udwMaterialNum, TUINT64 uddwBuffNum);

    static TBOOL CompareMaterial(SMaterialInfo stMaterialA, SMaterialInfo stMaterialB);

    static TINT32 GetNewNormalEquipLv(vector<TUINT32> vMaterialLv);
    static TVOID GetMaterialLvStr(vector<TUINT32> vMaterialLv, string& sBuff);
    static TINT32 GetNormalEquipType(vector<TUINT32> vMaterialType);
    static TINT32 GetEquipPos(TUINT32 udwEquipType);
};

#endif

