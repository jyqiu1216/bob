#ifndef _AWS_STRUCT_H_
#define _AWS_STRUCT_H_

#include <string>
#include <map>
#include <vector>
#include <sstream>
#include <assert.h>
#include <iostream>
#include <string.h>
#include <stdio.h>
using namespace std;

#define MAX_KEY_LEN 64

//typedef long long TUINT64;
typedef __int64_t               TINT64;

enum AWS_VALUE_TYPE
{
	AWS_VALUE_UNKNOWN,
	AWS_VALUE_NUMBER, 
	AWS_VALUE_BOOLEAN,
	AWS_VALUE_STRING,
	AWS_VALUE_STRING_PTR,
	AWS_VALUE_MAP_PTR,
	AWS_VALUE_LIST_PTR,
};

class AwsMap;
class AwsList;

class AwsValue
{
	AWS_VALUE_TYPE type;
	string sValue;
	union
	{
		TINT64	nValue;
		bool bValue;
		string* psValue;
		AwsMap* pmValue;
		AwsList*  plValue;
	};
	bool flag; //标识是否需要接管psValue/pmValue/plValue的内存释放
public:
	AwsValue()
	{
		Reset();
	}
	AwsValue(const AwsValue& right)
	{
		Clone(right);
	}
	AwsValue(TINT64 value):type(AWS_VALUE_NUMBER),nValue(value),flag(false)
	{}
	AwsValue(bool value):type(AWS_VALUE_BOOLEAN),bValue(value),flag(false)
	{}
	AwsValue(const string& value):type(AWS_VALUE_STRING),sValue(value),flag(false)
	{}
	AwsValue(const string* value, bool bflag):type(AWS_VALUE_STRING_PTR),psValue(const_cast<string*>(value)),flag(bflag)
	{}
	AwsValue(const AwsMap* value, bool bflag):type(AWS_VALUE_MAP_PTR),pmValue(const_cast<AwsMap*>(value)),flag(bflag)
	{}
	AwsValue(const AwsList* value, bool bflag):type(AWS_VALUE_LIST_PTR),plValue(const_cast<AwsList*>(value)),flag(bflag)
	{}
	~AwsValue()
	{	
		Release();
	}
	AwsValue& operator=(const AwsValue& right);
	void Dump(ostringstream& oss);
	
public:
	friend class AwsMap;
	friend class AwsList;
	
private:
	void Reset();
	void Clone(const AwsValue& right);
	void Release();
	
};

class AwsList
{
	//禁止拷贝构造和复制
	AwsList(const AwsList&);
	AwsList& operator=(const AwsList&);
	
public:
	AwsList(){}
	int SetNumber(TINT64 value);
	int SetBoolean(bool value);
	int SetValue(TINT64 value);
	int SetValue(bool value);
	int SetValue(const string& value);
	//flag 标识释放需要接管value的内存释放
	int SetValue(const string* value, bool flag = false);
	int SetValue(const AwsMap* value, bool flag = false);
	void Dump(ostringstream& oss);

	AwsMap* GetAwsMap(unsigned int idx);

	int SetValue(unsigned int idx, const string& value);

	int SetNumber(unsigned int idx, TINT64 value);
	int SetBoolean(unsigned int idx, bool value);

public:
	friend class AwsMap;
	
private:
	vector<AwsValue> _vec;

};

class AwsMap
{
	//禁止拷贝构造和复制
	AwsMap(const AwsMap&);
	AwsMap& operator=(const AwsMap&);
	
public:
	AwsMap(){}
	int SetNumber(const string& key, TINT64 value);
	int SetBoolean(const string& key, bool value);
	int SetValue(const string& key, TINT64 value);
	int SetValue(const string& key, bool value);
	int SetValue(const string& key, const string& value);
	//flag 标识释是否需要接管value的内存释放
	int SetValue(const string& key, const string* value, bool flag = false);
	int SetValue(const string& key, const AwsMap* value, bool flag = false);
	int SetValue(const string& key, const AwsList* value, bool flag = false);
	void Dump(ostringstream& oss);

public:
	friend class AwsList;

public:
	int AddValue(const string& path, const string& value);
	int AddValueWithS(const string& path, const string& value);
	int AddValueWithN(const string& path, const string& value);
	int AddValueWithB(const string& path, const string& value);
	int AddValue(const string& path,  TINT64 value);
	int AddNumber(const string& path,  TINT64 value);
	int AddBoolean(const string& path, bool value);
	AwsMap* GetAwsMap(const string& key);
	AwsList* GetAwsList(const string& key);

private:

	map<string, AwsValue> _map;
	
};

#endif

