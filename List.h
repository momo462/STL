#pragma  once

template <class T>
struct ListNode
{
	ListNode<T>* _next;
	ListNode<T>* _prev;
	T _data;
};

template<class T>
struct ListIterator
{
	typedef ListIterator<T> Iterator;
	typedef T ValueType;
	typedef T* Pointer;
	typedef T& Reference;
	//???Ϊʲô����LiskNode<T> LinkType
	typedef ListNode<T>* LinkType;

	//LinkNode<T>* _node;
	LinkType _node;

	ListIterator(LinkType node=NULL)
		:_node(node)
	{}
	bool operator==(const Iterator &x)
	{
		return _node==x._node;
	}
	bool operator!=(const Iterator &x)
	{
		return _node!=x._node;
	}
	Reference operator*()const
	{
		return _node->_data;
	}
	Pointer operator->()const
	{
		//????
		//&(_node->data)
		return &(operator*());
	}
	Iterator& operator++()
	{
		_node=_node->_next;
		return *this;
	}
	Iterator& operator++(int)
	{
		Iterator tmp(node);
		++*this;
		return tmp;
	}
	Iterator& operator--()
	{
		node=node->_prev;
		return *this;
	}
	Iterator& operator--(int)
	{
		Iterator tmp(node);
		--*this;
		return tmp;
	}
};


template <class T>
class List
{
public:
	typedef ListIterator<T> Iterator;
	typedef T valueType;
	typedef ListNode<T>* LinkType;
protected:
	//ͷ�ڵ�����ݽڵ�
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
	const Iterator Begin() const
	{
		return _head->_next;
	}
	Iterator End()
	{
		return _head;
	}
	const Iterator End() const
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
		//�½�һ��tmp�ڵ�
		LinkType tmp=new LinkNode(x);
		//�ҵ�posǰ��Ľڵ���Ϊprev
		LinkType prev=pos.node->_prev;
		//   tmp---->pos
		tmp->_next=pos.node;
		//   prev<----tmp
		tmp->_prev=prev;
		//   prev---->tmp
		prev->_next=tmp;
		//   tmp<----pos
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
		Insert(_head,x);
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