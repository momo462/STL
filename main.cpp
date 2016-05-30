#include <vector>
#include <list>
#include <iostream>
#include <assert.h>
using namespace std;
#include "Vector.h"
void PrintVector (vector< int>& v ) 
{
	vector<int >::iterator it = v .begin();
	for(;it!=v.end(); ++it) 
	{
		cout<<*it<<" "; 
	}
	cout<<endl ; 
}
void Test1 () 
{
	vector<int > v1;
	v1.push_back (1);
	v1.push_back (2); 
	v1.push_back (3);
	v1.push_back (4);
	v1.push_back (5);
	v1.push_back (6);
	v1.push_back (7);
	v1.push_back (8);
	PrintVector(v1 );
	// 迭代器失效
	vector<int >::iterator it = v1 .begin(); 
	while(it != v1. end())
	{
		if(*it% 2 == 0)
			//erase之后it指向删除元素的下一个，如果不进行 it= 那么这个迭代器的指针就会丢失
		    //it =
			v1 .erase(it++); 
		else
			++ it; 
	}
	PrintVector(v1 ); 
}
void PrintList (list< int>& l1 ) 
{
	list<int >::iterator it = l1 .begin();
	for(;it!=l1.end(); ++it) 
	{
		cout<<*it<<" ";
	}
	cout<<endl ; 
}
void Test2 () 
{
	list<int > l1;
	l1.push_back (1); 
	l1.push_back (2);
	l1.push_back (3); 
	l1.push_back (4); 
	l1.push_back (5); 
	l1.push_back (6); 
	l1.push_back (7); 
	l1.push_back (8); 
	PrintList(l1 );
	// 迭代器失效
	list<int >::iterator it = l1 .begin(); while(it != l1. end())
	{
		if(*it% 2 == 0)
			//it = 
			l1 .erase(it++);
		else
			++ it; 
	}
	PrintList(l1 ); 
}


int main()
{
	//Test1();
	//Test2();
	testvector();
}
