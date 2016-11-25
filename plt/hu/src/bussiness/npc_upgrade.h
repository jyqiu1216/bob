#ifndef _NPC_UPGRADE_H_
#define _NPC_UPGRADE_H_

#include <vector>
#include <string.h>
#include <string>
#include "jsoncpp/json/json.h"
#include <fstream>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>

using namespace std;
using namespace Json;

#define R_MAX_LEVEL 10//�Ƽ����level
#define B_MAX_LEVEL 20//�������level
#define SPACE_NUM 73//����������Ŀ
#define OUTSIDE_SPACE_BEGIN 33//���⽨��������ʼ���
#define MAX_FARM_NUM 22//���farm����
#define MAX_MILL_NUM 6//���mill����
#define MAX_QUARRY_NUM 6//���quarry����
#define MAX_MINE_NUM 6//���mine����
//#define MAX_RESOURCE_BUILDING_NUM 10//�����Դ������Ŀ
#define MAX_HOUSE_NUM 6//���house��Ŀ
#define MAX_CAMP_NUM 11//����Ӫ��Ŀ
#define MAX_HOSPITAL_NUM 5//���ҽԺ��
#define CASTLE_ID 0
#define WALL_ID 1
#define HOUSE_ID 12//house id
#define CAMP_ID 13//camp id
#define HOSPITAL_ID 19
#define FARM_ID 14
#define MILL_ID 15
#define QUARRY_ID 16
#define MINE_ID 17
#define B_NUM 22
#define R_NUM 15
#define T_NUM 12
#define F_NUM 9
//#define FARM_ID 14
//#define MILL_ID 15
//#define QUARY_ID 16
//#define MINE_ID 17


enum SPACE_STATUS
{
	SPACE_BUILT=0,
	SPACE_LOCK,
	SPACE_TO_BUILD
};

struct NPC_RESEARCH
{
	int m_id;
	int m_level;
};

struct NPC_BUILDING
{
	int m_pos;
	int m_id;
	int m_level;
};

struct NPC_CITY_INFO
{
	vector<NPC_BUILDING> m_building_list;
	vector<int> m_resource_num;
	vector<int> m_troop_num;
	vector<int> m_fort_num;
};
struct NPC_INFO
{
	int m_uid;
	int m_level;
	int m_exp;
	vector<int> m_research_level;
	vector<NPC_CITY_INFO> m_city_list;

};

class CNpcUpgrade
{
private:
	Value m_json_root;
	NPC_INFO m_npc_info;
	//int m_target_level;

private:
	vector<NPC_RESEARCH> GetResearchToUpgrade();
	bool CheckReq(Value json,int target_level,int max_level);

	int GetResearchLevel(int id);
	int GetBuildingLevel(int id);
	int ReqLevel(int a1,int a2,int target_level,int max_level);
	int ResearchExp(int id,int target_level);
	//vector<int> ResearchResource(int id,int target_level);
	int ExpToLevel(int exp);

	vector<NPC_BUILDING> GetBuildingToUpgrade();
	int SpaceStatus(int space);
	NPC_BUILDING SpaceInfo(int space);
	int GetBuildingNum(int id);
	int BuildingExp(int id,int target_level,int pos);
	vector<int> ResourceReq(Value json,int target_level);
	void ResourceCost(vector<int> cost,vector<int> &own,int num);

	string IntToString(int i);
	vector<int> TrainTroop();
	vector<int> TrainFort();
public:
	CNpcUpgrade();
	~CNpcUpgrade();
	//return value:0-success,otherwise-error
	int Initalize(const string json_file);
	//return value:0-upgrade success,otherwise-error
	int Upgrade(NPC_INFO &npc_info,int target_level);
	//return value:0-trainning success,otherwise-do not need to train or error
	int Training(NPC_INFO &npc_info);
};

#endif