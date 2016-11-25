#include "npc_upgrade.h"
#include <math.h>

static int g_building_weight[B_NUM]={1000,100,10,10,3,7,7,10,0,10,0,10,100,100,100,100,100,100,10,100,5,0};
static int g_research_weight[R_NUM]={10,10,15,30,10,10,10,1000,10,5,500,10,10,5,500};
//static int g_building_weight[18]={84,70,76,60,27,49,37,54,57,67,56,24,68,40,75,69,56,52};

//static int g_building_weight[B_NUM]={1000,500,100,10,3,7,7,500,0,0,0,500,100,100,100,100,100,100,10,100,15,0};
//static int g_research_weight[R_NUM]={50,45,40,60,100,10,100,100,100,25,20,70,60,100,300};


CNpcUpgrade::CNpcUpgrade()
{
}

CNpcUpgrade::~CNpcUpgrade()
{
}

int CNpcUpgrade::Initalize(const string json_file)
{
	//random seed
	srand((unsigned int)time(NULL));

	Json::Reader reader;
	std::ifstream is;
	is.open(json_file.c_str(), std::ios::binary);
	if (reader.parse(is, m_json_root) == false)
	{
		return -1;
	}
	else
	{
		is.close();
		return 0;
	}
	//return 0;
}

int CNpcUpgrade::Upgrade(NPC_INFO &npc_info,int target_level)
{
	srand((unsigned int)time(NULL));
	bool has_changed=false;
	vector<NPC_RESEARCH> upgrade_research_list;
	vector<NPC_BUILDING> upgrade_building_list;
	NPC_RESEARCH research;
	NPC_BUILDING building;
	int rand_num;
	int exp;
	vector<int> resource_req;
	//printf("upgrade to level %d\n",target_level);
	while(npc_info.m_level< target_level)
	{
		m_npc_info=npc_info;
		//m_target_level=target_level;
		upgrade_research_list=GetResearchToUpgrade();
		upgrade_building_list=GetBuildingToUpgrade();

		if(upgrade_research_list.size()<=0 && upgrade_building_list.size()<=0)
		{
			break;
		}

		has_changed=true;

		rand_num=rand()%2;

		//upgrade building
		if(upgrade_research_list.size()<=0)
		{
			rand_num=1;
		}
		//upgrade research
		if(upgrade_building_list.size()<=0)
		{
			rand_num=0;
		}

		if(0==rand_num)
		{
			rand_num=rand()%upgrade_research_list.size();
			research=upgrade_research_list[rand_num];
			exp=ResearchExp(research.m_id,research.m_level+1);
			npc_info.m_research_level[research.m_id]=research.m_level+1;
			npc_info.m_exp=npc_info.m_exp+exp;
			npc_info.m_level=ExpToLevel(npc_info.m_exp);

			//printf("*******************research %d to level %d,exp=%d,total exp=%d,level=%d\n",research.m_id,research.m_level+1,exp,npc_info.m_exp,npc_info.m_level);
			ResourceCost( ResourceReq(m_json_root["game_research"][research.m_id]["r"]["r0"],research.m_level+1),npc_info.m_city_list[0].m_resource_num,1);
		}
		else
		{
			rand_num=rand()%upgrade_building_list.size();
			building=upgrade_building_list[rand_num];
			exp=BuildingExp(building.m_id,building.m_level+1,building.m_pos);
			/*if(building.m_id==WALL_ID)
			{
				printf("@@@@@@@@@@@@@@@@@@@@@@@@@@@@ wall pos=%d,level=%d\n",building.m_pos,building.m_level); 
			}
			*/
			ResourceCost( ResourceReq(m_json_root["game_building"][building.m_id]["r"]["r0"],building.m_level+1),npc_info.m_city_list[0].m_resource_num,1);
			if(0==building.m_level && building.m_id!=WALL_ID)
			{
				building.m_level++;
				npc_info.m_city_list[0].m_building_list.push_back(building);

				
			}
			else
			{
				for(unsigned int i=0; i < npc_info.m_city_list[0].m_building_list.size(); i++)
				{
					if(npc_info.m_city_list[0].m_building_list[i].m_pos==building.m_pos)
					{
						building.m_level++;
						npc_info.m_city_list[0].m_building_list[i].m_level=building.m_level;

					}
				}
			}
			npc_info.m_exp=npc_info.m_exp+exp;
			npc_info.m_level=ExpToLevel(npc_info.m_exp);

			//printf("*****************building %d to level %d,exp=%d,total exp=%d,level=%d\n",building.m_id,building.m_level,exp,npc_info.m_exp,npc_info.m_level);
		}

		/*
		printf("resource->");
		for(int i=0;i<npc_info.m_city_list[0].m_resource_num.size();i++)
		{
			printf("%d:",npc_info.m_city_list[0].m_resource_num[i]);
		}
		printf("\n");
		*/
	}
	

	if(has_changed)
	{
		return 0;
	}
	else
	{
		return -1;
	}
}

int CNpcUpgrade::Training(NPC_INFO &npc_info)
{
	srand((unsigned int)time(NULL));
	m_npc_info=npc_info;
	unsigned int i;
	bool has_changed=false;
	vector<int> troop_num=TrainTroop();
	for(i=0;i<troop_num.size();i++)
	{
		if(troop_num[i]>0)
		{
			npc_info.m_city_list[0].m_troop_num[i]+=troop_num[i];
			ResourceCost( ResourceReq(m_json_root["game_troop"][i]["r"]["r0"],1),npc_info.m_city_list[0].m_resource_num,troop_num[i]);
			has_changed=true;
		}
	}
	if(npc_info.m_uid%1==0)
	{
		vector<int> fort_num=TrainFort();

		for(i=0;i<fort_num.size();i++)
		{
			if(fort_num[i]>0)
			{
				npc_info.m_city_list[0].m_fort_num[i]+=fort_num[i];
				ResourceCost( ResourceReq(m_json_root["game_fort"][i]["r"]["r0"],1),npc_info.m_city_list[0].m_resource_num,fort_num[i]);
				has_changed=true;
			}
		}
	}

	/*
	printf("resource->");
	for(i=0;i<npc_info.m_city_list[0].m_resource_num.size();i++)
	{
		printf("%d:",npc_info.m_city_list[0].m_resource_num[i]);
	}
	printf("\n");
	*/
	if(has_changed)
	{
		return 0;
	}
	else
	{
		return -1;
	}
}

string CNpcUpgrade::IntToString(int i)
{
	char buff[20];
	sprintf(buff,"%d",i);
	string str(buff);
	return str;
}

vector<int> CNpcUpgrade::TrainTroop()
{
	vector<int> list;
	bool can_train=false;
	int max_num;
	int train_num;
	unsigned int i;
	vector<bool> can_train_list;
	for(i=0;i<m_npc_info.m_city_list[0].m_troop_num.size();i++)
	{
		if(i>=m_json_root["game_troop"].size())
		{
			can_train_list.push_back(false);
			continue;
		}
		can_train=CheckReq(m_json_root["game_troop"][i]["r"],1,1);
		can_train_list.push_back(can_train);
	}
	for(i=0;i<m_npc_info.m_city_list[0].m_troop_num.size();i++)
	{
		//printf("%d:%d\n",i,(int)can_train_list[i]);
		if(i>=m_json_root["game_troop"].size())
		{
			continue;
		}
		if(i<8 && i%4==0 && can_train_list[i+4])
		{
			can_train_list[i]=false;
		}
	}
	for(i=0;i<m_npc_info.m_city_list[0].m_troop_num.size();i++)
	{
		//printf("%d:%d\n",i,(int)can_train_list[i]);
		if(!can_train_list[i])
		{
			list.push_back(0);
			continue;
		}
		int max_num_1;
		if(m_npc_info.m_level==0)
		{
			max_num_1=0;
		}
		else
		{
			max_num_1=pow(m_npc_info.m_level-1,3);
		}
		int max_num_2=pow(m_npc_info.m_level,3);
		max_num=max_num_1+rand()%(max_num_2-max_num_1);
		max_num/=2;

		if(i%4==0 && max_num>1000)//goblin
		{
			max_num=1000;
		}

		if(max_num<=m_npc_info.m_city_list[0].m_troop_num[i])
		{
			train_num=0;
		}
		else
		{
			train_num=max_num-m_npc_info.m_city_list[0].m_troop_num[i];
		}
		if(train_num>max_num/7)
		{
			train_num=max_num/7;
		}
		if(train_num<=0 && m_npc_info.m_city_list[0].m_troop_num[i]==0)
		{
			train_num=1;
		}

		list.push_back(train_num);

	}
	return list;
}
vector<int> CNpcUpgrade::TrainFort()
{
	vector<int> list;
	unsigned int i;
    int j;
	int wall_level=GetBuildingLevel(WALL_ID);
	int space;
	bool can_train=false;

	vector<int> tier_space;

	for(j=1;j<4;j++)
	{
		string str=IntToString(j);
		space=m_json_root["game_space"][str].asInt();
		if(0==wall_level)
		{
			space=0;
		}
		else
		{
			space=pow(wall_level,3)*space;
		}
		tier_space.push_back(space);
	}

	for(i=0;i<m_npc_info.m_city_list[0].m_fort_num.size();i++)
	{
		if(i>=m_json_root["game_fort"].size())
		{
			continue;
		}
		can_train=false;
		can_train=CheckReq(m_json_root["game_fort"][i]["r"],1,1);

		if(!can_train)
		{
			list.push_back(0);
			continue;
		}
		if (m_npc_info.m_city_list[0].m_fort_num[i]<=0)
			list.push_back(1);
		else
			list.push_back(0);
	}
	return list;
}

vector<NPC_BUILDING> CNpcUpgrade::GetBuildingToUpgrade()
{
	vector<NPC_BUILDING> upgrade_list;
	int space_status;
	NPC_BUILDING building;
	bool can_upgrade=false;
	for(int i=0;i<SPACE_NUM;i++)
	{
		space_status=SpaceStatus(i);
		if(SPACE_BUILT==space_status)
		{
			building=SpaceInfo(i);
			can_upgrade=CheckReq(m_json_root["game_building"][building.m_id]["r"],building.m_level+1,B_MAX_LEVEL);
			if(can_upgrade)
			{
				for(int j=0;j<g_building_weight[building.m_id];j++)
				{
					upgrade_list.push_back(building);
				}
			}
			continue;
		}
		if(SPACE_LOCK==space_status)
		{
			continue;
		}
		if(SPACE_TO_BUILD==space_status)
		{
			int building_num=m_json_root["game_building"].size();
			int build_multiple;
			int building_level;
			int area;
			int deprecate;
			bool to_build=false;
			for(int j=0;j<building_num;j++)
			{
				to_build=false;
				build_multiple=m_json_root["game_building"][j]["a"]["a1"].asInt();
				area=m_json_root["game_building"][j]["a"]["a3"].asInt();
				deprecate=m_json_root["game_building"][j]["a"]["a5"].asInt();
				if (1==deprecate)
					continue;

                // 暂时不支持card城的相关建筑操作
                if (3 == area || 31 == j)
                    continue;

                
                building_level=GetBuildingLevel(j);
				can_upgrade=CheckReq(m_json_root["game_building"][j]["r"],1,B_MAX_LEVEL);

				//can not build multiple
				if(0==build_multiple)
				{
					//has not built and can upgrade and space is inside city
					if( 0==building_level && can_upgrade && i<OUTSIDE_SPACE_BEGIN && j!=WALL_ID)//wall first level is 0
					{
						to_build=true;
					}
				}
				//can build multiple
				else
				{
					int num=GetBuildingNum(j);
					//inside city
					if(i<OUTSIDE_SPACE_BEGIN && 1==area)
					{
						//build house
						if( (j==HOUSE_ID && num<MAX_HOUSE_NUM) || (j==CAMP_ID && num<MAX_CAMP_NUM) || (j==HOSPITAL_ID && num<MAX_HOSPITAL_NUM))
						{
							to_build=true;
						}
					}
					//outside city
					if(i>=OUTSIDE_SPACE_BEGIN && 0==area)
					{
						if( (j==FARM_ID && num<MAX_FARM_NUM) || (j==MILL_ID && num<MAX_MILL_NUM) || (j==QUARRY_ID && num<MAX_QUARRY_NUM)|| (j==MINE_ID && num<MAX_MINE_NUM))
						{
							to_build=true;
						}
					}
				}
				if(to_build)
				{
					building.m_id=j;
					building.m_pos=i;
					building.m_level=0;
					for(int j=0;j<g_building_weight[building.m_id];j++)
					{
						upgrade_list.push_back(building);
					}
				}
			}
			continue;
		}
	}
	return upgrade_list;
}

vector<NPC_RESEARCH> CNpcUpgrade::GetResearchToUpgrade()
{
	vector<NPC_RESEARCH> upgrade_list;
	bool can_upgrade=false;
	for(unsigned int i=0;i<m_npc_info.m_research_level.size();i++)
	{
		if(i>=m_json_root["game_research"].size())
		{
			continue;
		}

		can_upgrade=CheckReq(m_json_root["game_research"][i]["r"],m_npc_info.m_research_level[i]+1,R_MAX_LEVEL);
		if(can_upgrade)
		{
			NPC_RESEARCH research;
			research.m_id=i;
			research.m_level=m_npc_info.m_research_level[i];
			for(int j=0;j<g_research_weight[research.m_id];j++)
			{
				upgrade_list.push_back(research);
			}
		}
	}
	return upgrade_list;
}

bool CNpcUpgrade::CheckReq(Value json,int target_level,int max_level)
{
	unsigned int i;
	bool req_meeted=true;
	int id=0;
	int curr_level=0;
	int req_level=0;

	if(target_level>max_level)
	{
		return false;
	}

	//check resource,ignor now

	//check buidling
	Value json_build=json["r1"];

	for(i=0;i<json_build.size();i++)
	{
		if(json_build[i].type()==nullValue)
		{
			continue;
		}
		id=json_build[i]["a0"].asInt();
		curr_level=GetBuildingLevel(id);
		req_level=ReqLevel(json_build[i]["a1"].asInt(),json_build[i]["a2"].asInt(),target_level,B_MAX_LEVEL);
		if(req_level>curr_level)
		{
			req_meeted=false;
		}
	}
	if(!req_meeted)
	{
		return false;
	}

	//check research
	Value json_research=json["r2"];
	for(i=0;i<json_research.size();i++)
	{
		if(json_research[i].type()==nullValue)
		{
			continue;
		}
		id=json_research[i]["a0"].asInt();
		curr_level=GetResearchLevel(id);
		req_level=ReqLevel(json_research[i]["a1"].asInt(),json_research[i]["a2"].asInt(),target_level,R_MAX_LEVEL);
		if(req_level>curr_level)
		{
			req_meeted=false;
		}
	}
	if(!req_meeted)
	{
		return false;
	}

	//check player level
	Value json_player_level=json["r3"];
	if(!json_player_level.empty())
	{
		curr_level=m_npc_info.m_level;
		req_level=ReqLevel(json_player_level["a0"].asInt(),json_player_level["a1"].asInt(),target_level,200);
		if(req_level>curr_level)
		{
			req_meeted=false;
		}
	}
	return req_meeted;
}

int CNpcUpgrade::GetResearchLevel(int id)
{
	return m_npc_info.m_research_level[id];
}
int CNpcUpgrade::GetBuildingLevel(int id)
{
	NPC_BUILDING building;
	int max_level=0;
	for(unsigned int i=0;i<m_npc_info.m_city_list[0].m_building_list.size();i++)
	{
		building=m_npc_info.m_city_list[0].m_building_list[i];
		if(building.m_level > max_level && building.m_id==id)
		{
			max_level=building.m_level;
		}
	}
	return max_level;
}

int CNpcUpgrade::SpaceStatus(int space)
{
	int status=SPACE_TO_BUILD;
	NPC_BUILDING building;
	for(unsigned int i=0;i<m_npc_info.m_city_list[0].m_building_list.size();i++)
	{
		building=m_npc_info.m_city_list[0].m_building_list[i];
		if(building.m_pos==space)
		{
			status=SPACE_BUILT;
		}
	}
	//outside city space
	if(space>=OUTSIDE_SPACE_BEGIN)
	{
		int castle_level=GetBuildingLevel(CASTLE_ID);
		int open_num;
		if(0==castle_level)
		{
			open_num=0;
		}
		else
		{
			open_num=13+3*(castle_level-1);
		}
		if(space >= OUTSIDE_SPACE_BEGIN+open_num)
		{
			status=SPACE_LOCK;
		}
	}
	return status;
}

NPC_BUILDING CNpcUpgrade::SpaceInfo(int space)
{
	NPC_BUILDING building;
	for(unsigned int i=0;i<m_npc_info.m_city_list[0].m_building_list.size();i++)
	{
		building=m_npc_info.m_city_list[0].m_building_list[i];
		if(building.m_pos==space)
		{
			break;
		}
	}
	return building;
}

int CNpcUpgrade::GetBuildingNum(int id)
{
	int num=0;
	NPC_BUILDING building;
	for(unsigned int i=0;i<m_npc_info.m_city_list[0].m_building_list.size();i++)
	{
		building=m_npc_info.m_city_list[0].m_building_list[i];
		if(building.m_id==id)
		{
			num++;
		}
	}
	return num;
}

int CNpcUpgrade::BuildingExp(int id,int target_level,int pos)
{
	int base_time=m_json_root["game_building"][id]["r"]["r0"]["a7"].asInt();
	int time=0;
	if(target_level<11)
	{
		time=base_time*pow(2,target_level-1);
	}
	else
	{
		time=base_time*pow(2,10-1);
		time*=pow(1.2,target_level-10);
	}
	int exp=1+time/30;

	//printf("building=%d,pos=%d,level=%d,time=%d,exp=%d\n",id,pos,target_level,time,exp);
	return exp;
}

int CNpcUpgrade::ReqLevel(int a1,int a2,int target_level,int max_level)
{
	int req_level;
	if(1==a1)
	{
		req_level=a2;
	}
	else if (0==a1)
	{
		req_level=target_level+a2;
	}
	else
	{
		req_level=target_level*2-1;
	}
	if(req_level>max_level)
	{
		req_level=max_level;
	}
	return req_level;
}

int CNpcUpgrade::ResearchExp(int id,int target_level)
{
	int base_time=m_json_root["game_research"][id]["r"]["r0"]["a7"].asInt();
	int time=0;
	if(target_level<6)
	{
		time=base_time*pow(2,target_level-1);
	}
	else
	{
		time=base_time*pow(2,5-1);
		time*=pow(1.5,target_level-5);
	}
	int exp=1+time/30;

	//printf("research=%d,level=%d,time=%d,exp=%d\n",id,target_level,time,exp);
	return exp;
}

void CNpcUpgrade::ResourceCost(vector<int> cost,vector<int> &own,int num)
{
	bool lack=false;
	unsigned int i;
	for(i=0;i<cost.size();i++)
	{
		if(cost[i]*num>own[i])
		{
			lack=true;
			break;
		}
	}
	if(!lack)
	{
		for(i=0;i<cost.size();i++)
		{
			own[i]-=(cost[i]*num);
		}
	}
	return;
}
vector<int> CNpcUpgrade::ResourceReq(Value json,int target_level)
{
	vector<int> list;
	int factor=5;
	string str;
	int base;
	int resource;
	for(int i=0;i<5;i++)
	{
		str="a"+IntToString(i);

		base=json[str].asInt();
		resource=base*pow(2,target_level-1);
		list.push_back(resource/factor);
	}
	return list;
}

int CNpcUpgrade::ExpToLevel(int exp)
{
	int level=0;
	for(unsigned int i=0;i<m_json_root["game_exp"].size();i++)
	{
		if(exp<m_json_root["game_exp"][i].asInt())
		{
			break;
		}
		level=i;
	}
	return level;
}
