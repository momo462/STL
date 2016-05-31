#pragma once


#define __DEBUG__
static string GetFileName(const string& path)
{
	char ch = '/';
#ifdef _WIN32 
	ch = '\\' ;
#endif
	//rfind 反向查找ch
	size_t pos = path. rfind(ch );
	//npos表示字符串中不存在的位置，一般默认为-1
	if (pos == string:: npos)
		return path ; 
	else
		return path .substr( pos + 1);
}
// 用于调试追溯的trace log
inline static void __trace_debug (const char* function,
	const char * filename, int line , char* format , ...)
{
#ifdef __DEBUG__
	// 输出调用函数的信息
	fprintf(stdout , "【 %s:%d】%s" , GetFileName (filename). c_str(), line , function);
	// 输出用户打的trace信息
	//从理论上说，我们只要探测到任意一个变量的地址，并且知道其他变量的类型，通过指针移位运算，则总可以顺藤摸瓜找到其他的输入变量。

	//char*-->va_list
	va_list args;
	//args指向...前面的参数
	va_start (args , format);
	//送格式化输出到一流中 （将args指向的数据变成和format一样的数据格式输入到stdout）
	vfprintf (stdout , format, args);
	//args =null
	va_end (args );
#endif
}
#define __TRACE_DEBUG(...) \
	__trace_debug(__FUNCTION__ , __FILE__, __LINE__, __VA_ARGS__);

typedef void(*HAV_FUNC)();
//一级空间配置器
template <int inst>
class MallocAllocTemplate{
private:
	static void *OomMalloc(size_t);
	static void *OomRealloc(void *,size_t);
	static HAV_FUNC MallocAllocOomHandler;
	// 先malloc成功返回指针，失败调用错误处理函数
public:
	static void* Allocate(size_t n)
	{
		__TRACE_DEBUG("(n:%u)\n",n);
		void *result =  malloc(n);
		if(0 == result)
		{
			result = OomMalloc(n);
		}
		return result;
	}

	//free
	static void Deallocate(void* p)
	{
		__TRACE_DEBUG("(p:%p)\n",p);
		free(p);
	}
	//realloc
	static void* Reallocate(void *p,size_t new_sz)
	{
		void *result = realloc(p,new_sz);
		if(0 == result)
		{
			result = OomRealloc(p,new_sz);
		}
		return result;
	}
	//错误处理函数
    //static void(*SetMallocHander(void(*f)()))()
	//typedef void(*HAV_FUNC)();----写成全局，一会还要用
	static HAV_FUNC SetMallocHandler(HAV_FUNC f)
	{
		HAV_FUNC old = MallocAllocOomHandler;
		MallocAllocOomHandler = f;
		return old;
	}

};
//private的成员函数指针MallocAllocOomHander初始为null
template <int inst>
HAV_FUNC MallocAllocTemplate<inst>::MallocAllocOomHandler=0;


//out-of-memory-malloc函数
//内存分配失败，检查是否设置了MallocAllocOomHandler
//有设置就调用以后在分配，不断重复，直到分配成功
//没有就抛出异常
template <int inst>
void* MallocAllocTemplate<inst>::OomMalloc(size_t n)
{
	//当Allocate调用malloc申请分配内存失败的时候调用OomMalloc函数
	//定义一个函数指针
	HAV_FUNC MyMallocHandler;
	//定义一个void*指针保存申请成功之后的地址
	void *result=0;
	for(;;)
	{
		//MallocAllocOomHander默认值是0，
		//设置由SetMallocHandler决定MallocAllocOomHandler = f;
		MyMallocHandler=MallocAllocOomHandler;
		if(0 == MyMallocHandler)
		{
		   cerr << "out of memory" << endl;
		   exit(1);
		}
		//调用f函数--->释放内存
		MyMallocHandler();
		result = malloc(n);
		if (result)
		{
			return result;
		}
	}
}
template<int inst>
void* MallocAllocTemplate<inst>::OomRealloc(void *p,size_t new_sz)
{
	HAV_FUNC MyMallocHandler;
	void * result;
	MyMallocHandler=MallocAllocOomHandler();
	if (0==MyMallocHandler)
	{
		cerr << "out of memory" << endl;
		exit(1);
	}
	MyMallocHandler();
	result = realloc(p,n);
	if (result)
	{
		return result;
	}
}


//二级空间配置器
template<bool Thread,int inst>
class DefaultAllocTemplate
{
private:
	enum {ALIGN = 8};   //相邻自由链表相差长度
	enum {MAXBYTES = 128}; //自由链表节点最大长度
	enum {NFREELISTS = MAXBYTES/ALIGN}; //自由链表节点个数
	union obj
	{
		union obj* FreeListLink;
		char client_data[1];
	};
	//自由链表  二级指针自由链表中保存地址
	static obj* volatile FreeList[NFREELISTS];
	//内存池
	static char * StartFree;
	static char * EndFree;
	static size_t HeapSize;
private:
	//向上取整
	static size_t RoundUp(size_t);
	static size_t FreeeListIndex(size_t);
	static void * Refill(size_t n)
	{
		__TRACE_DEBUG("(n:%u)\n",n);
		int nobjs = 20;
		//这里的nobjs传的是引用
		//如果内存池可以分配nobjs*n那么大的内存，nobjs=20
		//否则nobjs就等于可以分配多少个节点
		char * chunk = DefaultAllocTemplate::ChunkAlloc(n,nobjs);
		obj * volatile * myfreelist;
		obj * result;
		obj *cur_obj,*next_obj;
		int i;
		//内存池中只能分配一个节点
		if (1==nobjs)
		{
			return chunk;
		}
		//找到自由链表的入口地址
		//之前最后一个节点的FreeListLink地址
		myfreelist=FreeList+FreeeListIndex(n);
		//将已得到的chunk中的各个节点连接起来
		result=(obj *)chunk;
		*myfreelist=next_obj=(obj *)(chunk+n);
		for(i=2;i<nobjs;i++)
		{
			cur_obj=next_obj;
			next_obj=(obj *)(next_obj+1);
			if(nobjs-1==i)
			{
				cur_obj->FreeListLink=NULL;
				break;
			}
			else
			{
				cur_obj->FreeListLink=next_obj;
			}
			//next_obj->FreeListLink=(obj*)(chunk+i*n);
			//next_obj=next_obj->FreeListLink;
		}
		//next_obj->FreeListLink=NULL;
		return result;
	}
	static char * ChunkAlloc(size_t size,int& nobjs)
	{
		
        __TRACE_DEBUG("(size: %u, nobjs: %d)\n",size,nobjs);
		char *result;
		size_t totalbytes= size*nobjs;
		size_t leftbytes=EndFree-StartFree;
		//如果能够分配nobjs个
		if(leftbytes>totalbytes)
		{
			__TRACE_DEBUG("内存池中内存足够分配%d个对象\n",nobjs);
			result=StartFree;
			StartFree+=totalbytes;
			return result;
		}
		//如果能够分配至少一个
		else if(leftbytes>=size)
		{
			__TRACE_DEBUG("内存池中内存不够分配%d个对象,只能分配%d个对象\n",nobjs,leftbytes/size);
			nobjs=leftbytes/size;
			totalbytes=size*nobjs;
			result=StartFree;
			StartFree+=totalbytes;
			return result;
		}
		else
		{
			//第一次startfree=0，endfree=0；heapsize=0；
			//bytestoget一定是8的倍数
			size_t bytestoget=2*totalbytes+RoundUp(HeapSize);
			//
			if (leftbytes>0)
			{
				obj *volatile * myfreelist= FreeList+FreeeListIndex(leftbytes);
				((obj*)StartFree)->FreeListLink=*myfreelist;
				*myfreelist=((obj*)StartFree);
			}
			StartFree=(char *)malloc(bytestoget);
			if (0==StartFree)
			{
				int i;
				obj *volatile *myfreelist,*p;
				for (i=size;i<=MAXBYTES;i+=ALIGN)
				{
					myfreelist=FreeList+FreeeListIndex(i);
					p=*myfreelist;
					if (0!=p)
					{
						*myfreelist=p->FreeListLink;
						StartFree=(char *)p;
						EndFree=StartFree+i;
						return(ChunkAlloc(size,nobjs));
					}
				}
				EndFree=0;
				__TRACE_DEBUG("系统堆和自由链表都已无内存,一级配置器做最后一根稻草\n");
				StartFree=(char *)MallocAllocTemplate<0>::Allocate(bytestoget);
			}
			HeapSize+=bytestoget;
			EndFree=StartFree+bytestoget;
			return ChunkAlloc(size,nobjs);
		}

	}
public:
    static void *Allocate(size_t n)
	{
		__TRACE_DEBUG("(n: %u)\n",n);
		//MyFreeList为入口地址
		obj* volatile *MyFreeList;
		//__restrict修饰的result是所指向数据的唯一入口，此数据不允许其他指针去访问它
		//result为从自由链表中取出的内存的地址
		obj* __restrict result;
		//如果n>128---调用一级配置器
		if(n>(size_t)MAXBYTES)
		{
			return(MallocAllocTemplate<0>::Allocate(n));
		}
		//得到入口地址
		MyFreeList=FreeList+FreeeListIndex(n);
		//result指向可以利用的第一块内存
		result=*MyFreeList;
		//如果没有该块内存，就从内存池中去找内存，然后填充到自由链表中,并且将找到的第一块内存地址返回
		if( 0==result)
		{
			void *r=Refill(RoundUp(n));
			return r;
		}
		__TRACE_DEBUG("自由链表取内存:_freeList[%d]\n",FreeeListIndex(n));
		//让入口地址等于第一块内存的下一块
		*MyFreeList = result->FreeListLink;
		return result;
	}
	static void Deallocate(void *p,size_t n)
	{
		
        __TRACE_DEBUG("(p:%p, n: %u)\n",p,n);
		obj* volatile *MyFreeList=FreeList+FreeeListIndex(n);
		obj * ret=(obj*)p;
		if(n>(size_t)MAXBYTES)
		{
			return(MallocAllocTemplate<0>::Deallocate(p));
		}
		ret->FreeListLink=*MyFreeList;
		*MyFreeList=ret;
	}
	static void *Reallocate(void *p,size_t old_sz,size_t new_sz)
	{
		obj * ret=(obj *)p;
		if (old_sz>(size_t)MAXBYTES&&new_sz>(size_t)MAXBYTES)
		{
			MallocAllocTemplate::Reallocate(p,new_sz);
		}
		if(RoundUp(old_sz)==RoundUp(new_sz))
		{
			return p;
		}
		result=Allocate(new_sz);
		size_t copy_size=new_sz>old_sz? old_sz:new_sz;
		memcopy(ret,p,copy_size);
		Deallocate(p,old_sz);
		return(result);
	}

};
template<bool Thread,int inst>
char * DefaultAllocTemplate<Thread,inst>::StartFree=0;
template<bool Thread,int inst>
char * DefaultAllocTemplate<Thread,inst>::EndFree=0;
template<bool Thread,int inst>
size_t DefaultAllocTemplate<Thread,inst>::HeapSize=0;
template<bool Thread,int inst>
typename DefaultAllocTemplate<Thread,inst>:: obj* volatile DefaultAllocTemplate<Thread,inst>:: FreeList[DefaultAllocTemplate<Thread,inst>:: NFREELISTS]={0};
//按照8的倍数，向上取整
template<bool Thread,int inst>
size_t DefaultAllocTemplate<Thread,inst>::RoundUp(size_t n)
{
	//7     7+7=14--0000 0000 0000 1110&1111 1111 1111 1000--8
	//8     8+7=15--0000 0000 0000 1111&1111 1111 1111 1000--8
	//9     9+7=16--0000 0000 0001 0000&1111 1111 1111 1000--16
	return (((n)+ALIGN-1)&~(ALIGN-1));
}

//链表中对应下标
template<bool Thread,int inst>
size_t DefaultAllocTemplate<Thread,inst>::FreeeListIndex(size_t n)
{
	return ((n+ALIGN-1)/(ALIGN-1));
}

typedef DefaultAllocTemplate<0,0> _alloc;

//将一级和二级配置器进行简单的封装
template <class T,class Alloc=_alloc>
class SimpleAlloc
{
public:
	static T* Allocate(size_t size)
	{
		return (T*)Alloc::Allocate(size*sizeof(T));
	}
	static void Deallocate(void *p,size_t n)
	{
		return Alloc::Deallocate(p,n*sizeof(T));
	}
};
// 测试调用一级配置器分配内存
void testalloc1()
{
	/*cout<<" 测试调用一级配置器分配内存 "<<endl ;
	char*p1 = SimpleAlloc<char>::Allocate (129); 
	SimpleAlloc<char>:: Deallocate(p1 , 129);*/
	//测试调用二级配置器分配内存
	cout<<" 测试调用二级配置器分配内存 "<<endl ;
	char*p2 = SimpleAlloc<char>::Allocate (128); 
	char*p3 = SimpleAlloc<char>::Allocate (128); 
	char*p4 = SimpleAlloc<char>::Allocate (128);
	//// ps:多线程环境需要考虑加锁 size_tindex=FREELIST_INDEX(n);
	////头插回自由链表
	///*Obj* tmp= (Obj*) p;
	//tmp->_freeListLink=_freeList[index]; 
	//_freeList[index] =tmp;*/
	char*p5 = SimpleAlloc< char>::Allocate (128); 
	SimpleAlloc<char>:: Deallocate(p2 , 128); 
	SimpleAlloc<char>:: Deallocate(p3 , 128);
	SimpleAlloc<char>:: Deallocate(p4 , 128); 
	SimpleAlloc<char>:: Deallocate(p5 , 128);
    char *p[21];
	for (int i = 0; i < 21; ++i )
	{
		printf(" 测试第%d次分配 \n", i +1);
		p[i] = SimpleAlloc<char>::Allocate (128);
		//SimpleAlloc<char>:: Deallocate(p5 , 128);
	}
	for (int i = 0; i < 21; ++i )
	{
		//printf(" 测试第%d次分配 \n", i +1);
		//p[i] = SimpleAlloc<char>::Allocate (128);
		SimpleAlloc<char>:: Deallocate(p[i] , 128);
	}

}

// 测试特殊场景 
void Testalloc2 ()
{
cout<<" 测试内存池空间不足分配个 "<<endl ;
// 8*20->8*2->320
char*p1 = SimpleAlloc< char>::Allocate (8);
char*p2 = SimpleAlloc< char>::Allocate (8);
cout<<" 测试内存池空间不足,系统堆进行分配 "<<endl ;
char*p3 = SimpleAlloc< char>::Allocate (12); }
// 测试系统堆内存耗尽的场景 
void Testalloc3 ()
{
	cout<<" 测试系统堆内存耗尽 "<<endl ;
	SimpleAlloc<char>::Allocate (1024*1024*1024);
	SimpleAlloc<char>::Allocate(1024*1024*1024);
	SimpleAlloc<char>::Allocate (1024*1024);
	// 不好测试,说明系统管理小块内存的能力还是很强的。
	for (int i = 0; i < 100000; ++i ) 
	{
		char*p1 = SimpleAlloc<char>::Allocate (128);
	}
}