#include "aws_struct.h"

AwsValue& AwsValue::operator=(const AwsValue& right)
{
	if (this!= &right)
	{
		Release();
		Clone(right);
	}
	return *this;
}

void AwsValue::Dump(ostringstream& oss)
{
	switch(type)
	{
		case AWS_VALUE_NUMBER:
			oss << nValue;
			break;
		case AWS_VALUE_BOOLEAN:
			oss << (bValue ? "true" : "false");
			break;
		case AWS_VALUE_STRING:
			oss << "\"" << sValue << "\"";
			break;
		case AWS_VALUE_STRING_PTR:
			oss << "\"" << *psValue << "\"";
			break;
		case AWS_VALUE_MAP_PTR:
			pmValue->Dump(oss);
			break;
		case AWS_VALUE_LIST_PTR:
			plValue->Dump(oss);
			break;
		default:
			break;
	}
}

void AwsValue::Reset()
{
	type = AWS_VALUE_UNKNOWN;
	flag = false;
	sValue = "";
	nValue = 0; 
}

void AwsValue::Clone(const AwsValue& right)
{
	type = right.type;
	sValue = right.sValue;
	nValue = right.nValue;
	flag = right.flag;
	if (flag) //转移控制权
	{
		const_cast<AwsValue&>(right).Reset();
	}
}

void AwsValue::Release()
{
	if (!flag)
	{
		return;
	}
	//接管psValue/pmValue/plValue的内存释放
	switch(type)
	{
		case AWS_VALUE_STRING_PTR:
			delete psValue;
			break;
		case AWS_VALUE_MAP_PTR:
			delete pmValue;
			break;
		case AWS_VALUE_LIST_PTR:
			delete plValue;
			break;
		default:
			break;
	}
}


int AwsList::SetNumber(TINT64 value)
{
	if (_vec.size() > 0) //确保列表插入的元素都是相同类型
	{
		assert(AWS_VALUE_NUMBER == _vec[0].type);
	}
	_vec.push_back(AwsValue(value));
	return 0;
}

int AwsList::SetBoolean(bool value)
{
	if (_vec.size() > 0) //确保列表插入的元素都是相同类型
	{
		assert(AWS_VALUE_BOOLEAN == _vec[0].type);
	}
	_vec.push_back(AwsValue(value));
	return 0;
}

int AwsList::SetValue(TINT64 value)
{
	ostringstream oss;
	oss << value;
	return SetValue(oss.str());
}

int AwsList::SetValue(bool value)
{
	ostringstream oss;
	oss << (value ? "true" : "false");
	return SetValue(oss.str());
}


int AwsList::SetValue(const string& value)
{
	if (_vec.size() > 0) //确保列表插入的元素都是相同类型
	{
		assert(AWS_VALUE_STRING == _vec[0].type);
	}
	_vec.push_back(AwsValue(value));
	return 0;
}

//flag 标识释放需要接管value的内存释放
int AwsList::SetValue(const string* value, bool flag)
{
	if (_vec.size() > 0) //确保列表插入的元素都是相同类型
	{
		assert(AWS_VALUE_STRING_PTR == _vec[0].type);
	}
	_vec.push_back(AwsValue(value, flag));
	return 0;
}

int AwsList::SetValue(const AwsMap* value, bool flag)
{
	if (_vec.size() > 0) //确保列表插入的元素都是相同类型
	{
		assert(AWS_VALUE_MAP_PTR == _vec[0].type);
	}
	_vec.push_back(AwsValue(value, flag));
	return 0;
}

void AwsList::Dump(ostringstream& oss)
{
	oss << "[";
	for (unsigned int i = 0; i < _vec.size(); ++i)
	{
		_vec[i].Dump(oss);
		if (i < _vec.size()-1)
		{
			oss << ",";
		}
	}
	oss << "]";
}


AwsMap* AwsList::GetAwsMap(unsigned int idx)
{
	assert(idx <= _vec.size());
	if (_vec.size() == idx)
	{
		_vec.push_back(AwsValue(new AwsMap(), true));
	}
	assert(AWS_VALUE_MAP_PTR == _vec[0].type);
	return _vec[idx].pmValue;
}

int AwsList::SetValue(unsigned int idx, const string& value)
{
	assert(idx <= _vec.size());
	if (_vec.size() == idx)
	{
		_vec.push_back(AwsValue(value));
	}
	else
	{
		assert(AWS_VALUE_STRING == _vec[idx].type);
		_vec[idx] = AwsValue(value);
	}
	assert(AWS_VALUE_STRING == _vec[0].type);
	return 0;
}

int AwsList::SetNumber(unsigned int idx, TINT64 value)
{
	assert(idx <= _vec.size());
	if (_vec.size() == idx)
	{
		_vec.push_back(AwsValue(value));
	}
	assert(AWS_VALUE_NUMBER == _vec[0].type);
	return 0;
}

int AwsList::SetBoolean(unsigned int idx, bool value)
{
	assert(idx <= _vec.size());
	if (_vec.size() == idx)
	{
		_vec.push_back(AwsValue(value));
	}
	assert(AWS_VALUE_BOOLEAN == _vec[0].type);
	return 0;
}



int AwsMap::AddValue(const string& path, TINT64 value)
{
	ostringstream oss;
	oss << value;
	return AddValue(path, oss.str());
}

int AwsMap::AddValueWithS(const string& path, const string& value)
{
	return AddValue(path+"/S", value);
}

int AwsMap::AddValueWithN(const string& path, const string& value)
{
	return AddValue(path+"/N",value);
}

int AwsMap::AddValueWithB(const string& path, const string& value)
{
	return AddValue(path+"/B",value);
}



int AwsMap::AddValue(const string& path, const string& value)
{
	//检验path的合法性
	assert(path.size()>2 && '/'==path[0] && '/'!=path[path.size()-1]);
	assert(path.find("//") == string::npos);

	int iLastPos = 0;
	int iPos = 0;
	int iLen = 0;
	char buf[MAX_KEY_LEN] = {0};
	char key[MAX_KEY_LEN]={0};
	unsigned int idx;
	AwsMap *pAwsMap = this; //从根开始
	AwsList *pAwsList;
	while ((iPos = path.find('/', iLastPos+1)) != (int)string::npos)
	{
		iLen = iPos - iLastPos -1;
		assert(iLen>0 && iLen < MAX_KEY_LEN);
		memcpy(buf, path.c_str()+ iLastPos+1, iLen);
		buf[iLen] = '\0';
		if (']' == buf[iLen-1])
		{
			if (sscanf(buf, "%[^[][%u]", key, &idx)!=2)
			{
				assert(0);
				return -1;
			}
			pAwsList = pAwsMap->GetAwsList(key);
			pAwsMap = pAwsList->GetAwsMap(idx);
		}
		else
		{
			pAwsMap = pAwsMap->GetAwsMap(buf);
		}
		iLastPos = iPos;
	}
	iLen = path.size() - iLastPos - 1;
	assert(iLen>0 && iLen < MAX_KEY_LEN);
	memcpy(buf, path.c_str()+ iLastPos+1, iLen);
	buf[iLen] = '\0';
	if (']' == buf[iLen-1])
	{
		if (sscanf(buf, "%[^[][%u]", key, &idx)!=2)
		{
			assert(0);
			return -1;
		}
		pAwsList = pAwsMap->GetAwsList(key);
		pAwsList->SetValue(idx,value);
	}
	else
	{
		pAwsMap->SetValue(buf,value);
	}
	return 0;
}

int AwsMap::AddNumber(const string& path, TINT64 value)
{
	//检验path的合法性
	assert(path.size()>2 && '/'==path[0] && '/'!=path[path.size()-1]);
	assert(path.find("//") == string::npos);

	int iLastPos = 0;
	int iPos = 0;
	int iLen = 0;
	char buf[MAX_KEY_LEN] = {0};
	char key[MAX_KEY_LEN]={0};
	unsigned int idx;
	AwsMap *pAwsMap = this; //从根开始
	AwsList *pAwsList;
	while ((iPos = path.find('/', iLastPos+1)) != (int)string::npos)
	{
		iLen = iPos - iLastPos -1;
		assert(iLen>0 && iLen < MAX_KEY_LEN);
		memcpy(buf, path.c_str()+ iLastPos+1, iLen);
		buf[iLen] = '\0';
		if (']' == buf[iLen-1])
		{
			if (sscanf(buf, "%[^[][%u]", key, &idx)!=2)
			{
				assert(0);
				return -1;
			}
			pAwsList = pAwsMap->GetAwsList(key);
			pAwsMap = pAwsList->GetAwsMap(idx);
		}
		else
		{
			pAwsMap = pAwsMap->GetAwsMap(buf);
		}
		iLastPos = iPos;
	}
	iLen = path.size() - iLastPos - 1;
	assert(iLen>0 && iLen < MAX_KEY_LEN);
	memcpy(buf, path.c_str()+ iLastPos+1, iLen);
	buf[iLen] = '\0';
	if (']' == buf[iLen-1])
	{
		if (sscanf(buf, "%[^[][%u]", key, &idx)!=2)
		{
			assert(0);
			return -1;
		}
		pAwsList = pAwsMap->GetAwsList(key);
		pAwsList->SetNumber(idx,value);
	}
	else
	{
		pAwsMap->SetNumber(buf,value);
	}
	return 0;
}

int AwsMap::AddBoolean(const string& path, bool value)
{
	//检验path的合法性
	assert(path.size()>2 && '/'==path[0] && '/'!=path[path.size()-1]);
	assert(path.find("//") == string::npos);

	int iLastPos = 0;
	int iPos = 0;
	int iLen = 0;
	char buf[MAX_KEY_LEN] = {0};
	char key[MAX_KEY_LEN]={0};
	unsigned int idx;
	AwsMap *pAwsMap = this; //从根开始
	AwsList *pAwsList;
	while ((iPos = path.find('/', iLastPos+1)) != (int)string::npos)
	{
		iLen = iPos - iLastPos -1;
		assert(iLen>0 && iLen < MAX_KEY_LEN);
		memcpy(buf, path.c_str()+ iLastPos+1, iLen);
		buf[iLen] = '\0';
		if (']' == buf[iLen-1])
		{
			if (sscanf(buf, "%[^[][%u]", key, &idx)!=2)
			{
				assert(0);
				return -1;
			}
			pAwsList = pAwsMap->GetAwsList(key);
			pAwsMap = pAwsList->GetAwsMap(idx);
		}
		else
		{
			pAwsMap = pAwsMap->GetAwsMap(buf);
		}
		iLastPos = iPos;
	}
	iLen = path.size() - iLastPos - 1;
	assert(iLen>0 && iLen < MAX_KEY_LEN);
	memcpy(buf, path.c_str()+ iLastPos+1, iLen);
	buf[iLen] = '\0';
	if (']' == buf[iLen-1])
	{
		if (sscanf(buf, "%[^[][%u]", key, &idx)!=2)
		{
			assert(0);
			return -1;
		}
		pAwsList = pAwsMap->GetAwsList(key);
		pAwsList->SetBoolean(idx,value);
	}
	else
	{
		pAwsMap->SetBoolean(buf,value);
	}
	return 0;	
}



AwsMap* AwsMap::GetAwsMap(const string& key)
{
	if (_map.find(key) == _map.end())
	{
		_map[key] = AwsValue(new AwsMap(), true);
	}
	assert(AWS_VALUE_MAP_PTR == _map[key].type);
	return _map[key].pmValue;
}

AwsList* AwsMap::GetAwsList(const string& key)
{
	if (_map.find(key) == _map.end())
	{
		_map[key] = AwsValue(new AwsList(), true);
	}
	assert(AWS_VALUE_LIST_PTR == _map[key].type);
	return _map[key].plValue;
}


int AwsMap::SetNumber(const string& key, TINT64 value)
{
	_map[key] = AwsValue(value);
	return 0;
}

int AwsMap::SetBoolean(const string& key, bool value)
{
	_map[key] = AwsValue(value);
	return 0;
}

int AwsMap::SetValue(const string& key, TINT64 value)
{
	ostringstream oss;
	oss << value;
	return SetValue(key, oss.str());
}

int AwsMap::SetValue(const string& key, bool value)
{
	ostringstream oss;
	oss << (value ? "true" : "false");
	return SetValue(key, oss.str());
}


int AwsMap::SetValue(const string& key, const string& value)
{
	_map[key] = AwsValue(value);
	return 0;
}

//flag 标识释放需要接管value的内存释放
int AwsMap::SetValue(const string& key, const string* value, bool flag)
{
	_map[key] = AwsValue(value, flag);
	return 0;
}

int AwsMap::SetValue(const string& key, const AwsMap* value, bool flag)
{
	_map[key] = AwsValue(value, flag);
	return 0;
}

int AwsMap::SetValue(const string& key, const AwsList* value, bool flag)
{
	_map[key] = AwsValue(value, flag);
	return 0;
}

void AwsMap::Dump(ostringstream& oss)
{
	oss << "{";
	unsigned int i = 0;
	for (map<string, AwsValue>::iterator iter = _map.begin(); iter != _map.end(); ++i, ++iter)
	{
		oss << "\"" << iter->first << "\"" << ":";
		iter->second.Dump(oss);
		if (i < _map.size()-1)
		{
			oss << ",";
		}
	}
	oss << "}";
}

