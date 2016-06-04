#pragma once

template <class T>
class Vector
{
public:
	typedef T ValueType;
	typedef ValueType* Pointer;
	typedef ValueType* Iterator;
	typedef ValueType& Reference;
	
protected:
	//[)
	Iterator _start;//vector起始位置
	Iterator _finish;//vector末尾位置的下一位置
	Iterator _EndOfStorage;//容量结束的位置
public:
	Iterator Begin()
	{
		return _start;
	}
	const Iterator Begin() const
	{
		return _start;
	}
	Iterator end()
	{
		return _finish;
	}
	const Iterator End() const
	{
		return _finish;
	}
	size_t Size() const
	{
		return _finish-_start;
	}
	size_t MaxSize() const
	{
		return size_t(-1)/sizeof(T);
	}
	size_t Capacity() const
	{
		return _EndOfStorage-_start;
	}
	bool empty()
	{
		return _finish==_start;
	}
	Reference operator[](size_t n)
	{
		return *(_start+n);
	}
	const Reference operator[](size_t n) const
	{
		return *(_start+n);
	}

	Vector()
		:_start(NULL)
		,_finish(NULL)
		,_EndOfStorage(NULL)
	{}
	Reference Front()
	{
		return *_start;
	}
	const Reference Front() const
	{
		return *_start;
	}
	Reference Back()
	{
		return *(_finish-1);
	}
	const Reference Back() const
	{
		return *(_finish-1);
	}
	//增容
	void _CheckExpand()
	{
		if(_finish==_EndOfStorage)
		{
			size_t size=Size();
			size_t capacity=size*2+3;
			T *temp=new T[capacity]();
			if(_start)
			{
				for(size_t i=0;i<size;i++)
				{
					temp[i]=*(_start+i);
				}
			}
			_start=temp;
			_finish=_start+size;
			_EndOfStorage=_start+capacity;
		}
	}
	void PushBack(const T& x)
	{
		_CheckExpand();
		assert(_finish!=_EndOfStorage);
		*_finish=x;
		++_finish;
	}
	void PopBack()
	{
		_finish--;
	}
	Iterator Erase(Iterator pos)
	{
		assert(pos!=_finish);
		Iterator begin=pos+1;
		while(begin!=_finish)
		{
			*(begin-1)=*(begin);
			begin++;
		}
		_finish--;
		return pos;
	}
};


void testvector()
{
	Vector<int> v1;
	v1.PushBack(1);
	v1.PushBack(2);
	v1.PushBack(3);
	v1.PushBack(4);

	Vector<int>::Iterator it=v1.Begin();
	while(it!=v1.End())
	{
		cout<<*it<<endl;
		it++;
	}
	
	it=v1.Begin();
	while (it!=v1.End())
	{
		if (*it%2==0)
		{
			v1.Erase(it);
		}
		else
			it++;
	}

	it=v1.Begin();
	while(it!=v1.End())
	{
		cout<<*it<<endl;
		it++;
	}
}