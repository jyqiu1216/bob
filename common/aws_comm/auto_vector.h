#include <vector>
#include "assert.h"
using namespace std;

template <class T>
class AutoVector
{
	//禁止拷贝构造和复制
	AutoVector(const AutoVector&);
	AutoVector& operator=(const AutoVector&);
	
public:
	AutoVector()
	{
		_vec.clear();
	}
	~AutoVector()
	{
		clear();
	}
	void push_back(T *pT)
	{
		_vec.push_back(pT);
	}
	unsigned int size()
	{
		return _vec.size();
	}
	void clear()
	{
		for (unsigned int i = 0; i < _vec.size(); ++i)
		{
			delete _vec[i];
		}
		_vec.clear();
	}
	T* operator[](unsigned int i)
	{
		assert(i < _vec.size());
		return _vec[i];
	}
private:
	vector<T*> _vec;
};



