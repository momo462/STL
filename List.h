#pragma  once

template <class T>
struct ListNode
{
	ListNode<T>* _next;
	ListNode<T>* _prev;
	T _data;
};

template<class T,class Ref,class Ptr>
struct ListIterator
{
	typedef ListIterator<T,T&,T*> Iterator;
	typedef ListIterator<const T,const T&,const T*> ConstIterator;
	typedef ListIterator<T,Ref,Ptr> Self;
	
	//typedef BidirectionalIteratorTag IteratorCategory;
	typedef T ValueType;
	typedef Ptr Pointer;
	typedef Ref Reference;
	typedef ListNode<T> LinkType;
	typedef size_t SizeType;
	typedef ptrdiff_t DiffrenenceType;

	LinkType node;

	ListIterator(LinkType x)
		:node(x)
	{}
	ListIterator()
	{}

	bool operator==(const Self &x)
	{
		return node==x.node;
	}
	bool operator!=(const Self &x)
	{
		return node!=x.node;
	}
	Reference operator*()const
	{
		return node->_data;
	}
	Pointer operator->()const
	{
		return node;
	}
	Self& operator++()
	{
		node=node->_next;
		return node;
	}
	Self& operator++(int)
	{
		Self tmp(node);
		++*this;
		return tmp;
	}
	Self& operator--()
	{
		node=node->_prev;
	}
	Self& operator--(int)
	{
		Self tmp(node);
		--*this;
		return tmp;
	}
};


template <class T>
class List
{
public:
	typedef ListNode<T> LinkNode;
	typedef T ValueType;
	typedef ValueType* Pointer;
	typedef const Pointer ConstPointer;
	typedef ValueType& Reference ;
	typedef const Reference ConstReference;
	typedef LinkNode* LinkType;
	typedef size_t SizeType;
	typedef ptrdiff_t DiffrenceType;

	typedef ListIterator<T,T&,T*> Iterator;
	typedef ListIterator<const T,const T&,const T *> ConstIterator;
protected:
	//�ڱ��ڵ㣬�����洢������
	LinkType _head;
public:
	List()
	{
		_head->_next=_head;
		_head->_prev=_head;
	}
	~List()
	{
		Clear();
	}
	Iterator Begin()
	{
		return _head->_next;
	}
	ConstIterator Begin() const
	{
		return _head->_next;
	}
	Iterator End()
	{
		return _head;
	}
	ConstIterator End() const
	{
		return _head;
	}
	bool Empty() const
	{
		return _head->_next==_head;
	}
	SizeType Size() const
	{
		SizeType result=0;
		ListType cur=_head->_next;
		while(cur!=_head)
		{
			result++;
		}
		return result;
	}
	//��posǰ����룬���ز���ڵ�λ��
	Iterator Insert(Iterator pos,const T& x)
	{
		LinkType tmp=new LinkNode(x);
		LinkType prev=pos.node->_prev;
		tmp->_next=pos.node;
		tmp->_prev=prev;
		prev->_next=tmp;
		pos.node->_prev=tmp;
		//��Ϊ ListIterator��node x��
		//����tmp��Iterator����ʽת���������������
		//Ҳ���� Iterator(tmp) ��������
		return tmp;
	}
	//ɾ��posλ�õĽڵ㣬����pos��һ���ڵ�
	Iterator Erase(Iterator pos)
	{
		LinkType prev=pos.node->_prev;
		LinkType next=pos.node->_next;
		prev->_next=next;
		next->_prev=prev;
		delete pos.node;
		return next;
	}
	void PushBack(const T& x)
	{
		Insert(_head);
	}
	void PopBack()
	{
		Erase(_head->_prev);
	}
	void PushFront(const T& x)
	{
		Insert(_head->_next);
	}
	void PopFront()
	{
		Erase(_head->_next);
	}
	void Clear()
	{
		Iterator begin=Begin();
		while(begin!=End())
		{
			LinkType del= begin.node;
			begin++;
			delete del;
		}
	}
}

void testlist()
{

}