#pragma once

typedef void(*HAV_FUNC)();
//һ���ռ�������
template <int inst>
class MallocAllocTemplate{
private:
	static void *OomMalloc(size_t);
	static void *OomRealloc(void *,size_t);
	static void (*MallocAllocOomHandler)();
public:
	// ��malloc�ɹ�����ָ�룬ʧ�ܵ��ô�������
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
	//��������
    //static void(*SetMallocHander(void(*f)()))()
	//typedef void(*HAV_FUNC)();----д��ȫ�֣�һ�ỹҪ��
	static HAV_FUNC SetMallocHandler(HAV_FUNC f)
	{
		HAV_FUNC old = MallocAllocOomHandler;
		MallocAllocOomHandler = f;
		return old;
	}

};
//private�ĳ�Ա����ָ��MallocAllocOomHander��ʼΪnull
template <int inst>
static MallocAllocTemplate<inst>::MallocAllocOomHandler()=0;


//out-of-memory-malloc����
//�ڴ����ʧ�ܣ�����Ƿ�������MallocAllocOomHandler
//�����þ͵����Ժ��ڷ��䣬�����ظ���ֱ������ɹ�
//û�о��׳��쳣
template <int inst>
static MallocAllocTemplate<inst>::OomMalloc(size_t n)
{
	//��Allocate����malloc��������ڴ�ʧ�ܵ�ʱ�����OomMalloc����
	//����һ������ָ��
	HAV_FUNC MyMallocHandler;
	//����һ��void*ָ�뱣������ɹ�֮��ĵ�ַ
	void *result;
	for(;;)
	{
		//MallocAllocOomHanderĬ��ֵ��0��
		//������SetMallocHandler����MallocAllocOomHandler = f;
		MyMallocHandler=MallocAllocOomHander;
		if(0 == MyMallocHandler)
		{
		   cerr << "out of memory" << endl;
		   exit(1);
		}
		//����f����--->�ͷ��ڴ�
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


//�����ռ�������
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
	//��������
	static obj* volatile FreeList[NFREELISTS];
	//�ڴ��
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