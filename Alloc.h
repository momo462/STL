#pragma once

typedef void(*HAV_FUNC)();
//一级空间配置器
template <int inst>
class MallocAllocTemplate{
private:
	static void *OomMalloc(size_t);
	static void *OomRealloc(void *,size_t);
	static void (*MallocAllocOomHandler)();
public:
	// 先malloc成功返回指针，失败调用错误处理函数
	static void* Allocate(size_t n)
	{
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
static MallocAllocTemplate<inst>::MallocAllocOomHandler()=0;


//out-of-memory-malloc函数
//内存分配失败，检查是否设置了MallocAllocOomHandler
//有设置就调用以后在分配，不断重复，直到分配成功
//没有就抛出异常
template <int inst>
static MallocAllocTemplate<inst>::OomMalloc(size_t n)
{
	//当Allocate调用malloc申请分配内存失败的时候调用OomMalloc函数
	//定义一个函数指针
	HAV_FUNC MyMallocHandler;
	//定义一个void*指针保存申请成功之后的地址
	void *result;
	for(;;)
	{
		//MallocAllocOomHander默认值是0，
		//设置由SetMallocHandler决定MallocAllocOomHandler = f;
		MyMallocHandler=MallocAllocOomHander;
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
static MallocAllocTemplate<inst>::OomRealloc(void *p,size_t new_sz)
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
	enum {ALIGN = 8};
	enum {MAXBYTES = 128};
	enum {NFREELISTS = MAXBYTES/ALIGN};
	union obj
	{
		union obj* FreeListLink;
		char client_data[1];
	};
	//自由链表
	static obj* volatile FreeList[NFREELISTS];
	//内存池
	static char * StartFree;
	static char * EndFree;
	static size_t HeapSize;
private:
	static size_t RoundUp(size_t);
	static size_t FreeeListIndex(size_t);
	static void * Refill(size_t);
	static char * ChunkAlloc(size_t,int&);
public:
    static void *allocate(size_t n)
	{}
	static void deallocate(void *p,size_t n)
	{}
	static void *reallocate(void *p,size_t old_sz,size_t new_sz)
	{}

};