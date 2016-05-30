#pragma once

template <class T>
class Vector
{
public:
	typedef T ValueType;
	typedef ValueType* Pointer;
	/*****迭代器的设计*********/
	typedef ValueType* Iterator;
	typedef  const ValueType* ConstIterator;
	typedef ValueType& Reference;
	typedef const ValueType& ConstReference;
	typedef size_t SizeType;
	typedef ptrdiff_t DifferenceType;
	/*typedef ReverseIterator<ConstIterator> ConstReverseIterator;
	typedef ReverseIterator<Iterator> ReverseIterator;*/
	/**************************/
protected:
	Iterator _start;
	Iterator _finish;
	Iterator _EndOfStorage;
public:
	Iterator Begin()
	{
		return _start;
	}
	ConstIterator Begin() const
	{
		return _start;
	}
	Iterator end()
	{
		return Finish;
	}
	ConstIterator End() const
	{
		return _finish;
	}
	SizeType Size() const
	{
		return _finish-_start;
	}
	SizeType MaxSize() const
	{
		return SizeType(-1)/sizeof(T);
	}
	SizeType Capacity() const
	{
		return _EndOfStorage-_start;
	}
	bool empty()
	{
		return _finish==_start;
	}
	Reference operator[](SizeType n)
	{
		return *(_start+n);
	}
	ConstReference operator[](SizeType n) const
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
	ConstReference Front() const
	{
		return *_start;
	}
	Reference Back()
	{
		return *(_finish-1);
	}
	ConstReference Back() const
	{
		return *(_finish-1);
	}
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