#pragma once


#define __DEBUG__
static string GetFileName(const string& path)
{
	char ch = '/';
#ifdef _WIN32 
	ch = '\\' ;
#endif
	//rfind �������ch
	size_t pos = path. rfind(ch );
	//npos��ʾ�ַ����в����ڵ�λ�ã�һ��Ĭ��Ϊ-1
	if (pos == string:: npos)
		return path ; 
	else
		return path .substr( pos + 1);
}
// ���ڵ���׷�ݵ�trace log
inline static void __trace_debug (const char* function,
	const char * filename, int line , char* format , ...)
{
#ifdef __DEBUG__
	// ������ú�������Ϣ
	fprintf(stdout , "�� %s:%d��%s" , GetFileName (filename). c_str(), line , function);
	// ����û����trace��Ϣ
	//��������˵������ֻҪ̽�⵽����һ�������ĵ�ַ������֪���������������ͣ�ͨ��ָ����λ���㣬���ܿ���˳�������ҵ����������������

	//char*-->va_list
	va_list args;
	//argsָ��...ǰ��Ĳ���
	va_start (args , format);
	//�͸�ʽ�������һ���� ����argsָ������ݱ�ɺ�formatһ�������ݸ�ʽ���뵽stdout��
	vfprintf (stdout , format, args);
	//args =null
	va_end (args );
#endif
}
#define __TRACE_DEBUG(...) \
	__trace_debug(__FUNCTION__ , __FILE__, __LINE__, __VA_ARGS__);

typedef void(*HAV_FUNC)();
//һ���ռ�������
template <int inst>
class MallocAllocTemplate{
private:
	static void *OomMalloc(size_t);
	static void *OomRealloc(void *,size_t);
	static HAV_FUNC MallocAllocOomHandler;
	// ��malloc�ɹ�����ָ�룬ʧ�ܵ��ô�������
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
HAV_FUNC MallocAllocTemplate<inst>::MallocAllocOomHandler=0;


//out-of-memory-malloc����
//�ڴ����ʧ�ܣ�����Ƿ�������MallocAllocOomHandler
//�����þ͵����Ժ��ڷ��䣬�����ظ���ֱ������ɹ�
//û�о��׳��쳣
template <int inst>
void* MallocAllocTemplate<inst>::OomMalloc(size_t n)
{
	//��Allocate����malloc��������ڴ�ʧ�ܵ�ʱ�����OomMalloc����
	//����һ������ָ��
	HAV_FUNC MyMallocHandler;
	//����һ��void*ָ�뱣������ɹ�֮��ĵ�ַ
	void *result=0;
	for(;;)
	{
		//MallocAllocOomHanderĬ��ֵ��0��
		//������SetMallocHandler����MallocAllocOomHandler = f;
		MyMallocHandler=MallocAllocOomHandler;
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


//�����ռ�������
template<bool Thread,int inst>
class DefaultAllocTemplate
{
private:
	enum {ALIGN = 8};   //����������������
	enum {MAXBYTES = 128}; //��������ڵ���󳤶�
	enum {NFREELISTS = MAXBYTES/ALIGN}; //��������ڵ����
	union obj
	{
		union obj* FreeListLink;
		char client_data[1];
	};
	//��������  ����ָ�����������б����ַ
	static obj* volatile FreeList[NFREELISTS];
	//�ڴ��
	static char * StartFree;
	static char * EndFree;
	static size_t HeapSize;
private:
	//����ȡ��
	static size_t RoundUp(size_t);
	static size_t FreeeListIndex(size_t);
	static void * Refill(size_t n)
	{
		__TRACE_DEBUG("(n:%u)\n",n);
		int nobjs = 20;
		//�����nobjs����������
		//����ڴ�ؿ��Է���nobjs*n��ô����ڴ棬nobjs=20
		//����nobjs�͵��ڿ��Է�����ٸ��ڵ�
		char * chunk = DefaultAllocTemplate::ChunkAlloc(n,nobjs);
		obj * volatile * myfreelist;
		obj * result;
		obj *cur_obj,*next_obj;
		int i;
		//�ڴ����ֻ�ܷ���һ���ڵ�
		if (1==nobjs)
		{
			return chunk;
		}
		//�ҵ������������ڵ�ַ
		//֮ǰ���һ���ڵ��FreeListLink��ַ
		myfreelist=FreeList+FreeeListIndex(n);
		//���ѵõ���chunk�еĸ����ڵ���������
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
		//����ܹ�����nobjs��
		if(leftbytes>totalbytes)
		{
			__TRACE_DEBUG("�ڴ�����ڴ��㹻����%d������\n",nobjs);
			result=StartFree;
			StartFree+=totalbytes;
			return result;
		}
		//����ܹ���������һ��
		else if(leftbytes>=size)
		{
			__TRACE_DEBUG("�ڴ�����ڴ治������%d������,ֻ�ܷ���%d������\n",nobjs,leftbytes/size);
			nobjs=leftbytes/size;
			totalbytes=size*nobjs;
			result=StartFree;
			StartFree+=totalbytes;
			return result;
		}
		else
		{
			//��һ��startfree=0��endfree=0��heapsize=0��
			//bytestogetһ����8�ı���
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
				__TRACE_DEBUG("ϵͳ�Ѻ��������������ڴ�,һ�������������һ������\n");
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
		//MyFreeListΪ��ڵ�ַ
		obj* volatile *MyFreeList;
		//__restrict���ε�result����ָ�����ݵ�Ψһ��ڣ������ݲ���������ָ��ȥ������
		//resultΪ������������ȡ�����ڴ�ĵ�ַ
		obj* __restrict result;
		//���n>128---����һ��������
		if(n>(size_t)MAXBYTES)
		{
			return(MallocAllocTemplate<0>::Allocate(n));
		}
		//�õ���ڵ�ַ
		MyFreeList=FreeList+FreeeListIndex(n);
		//resultָ��������õĵ�һ���ڴ�
		result=*MyFreeList;
		//���û�иÿ��ڴ棬�ʹ��ڴ����ȥ���ڴ棬Ȼ����䵽����������,���ҽ��ҵ��ĵ�һ���ڴ��ַ����
		if( 0==result)
		{
			void *r=Refill(RoundUp(n));
			return r;
		}
		__TRACE_DEBUG("��������ȡ�ڴ�:_freeList[%d]\n",FreeeListIndex(n));
		//����ڵ�ַ���ڵ�һ���ڴ����һ��
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
//����8�ı���������ȡ��
template<bool Thread,int inst>
size_t DefaultAllocTemplate<Thread,inst>::RoundUp(size_t n)
{
	//7     7+7=14--0000 0000 0000 1110&1111 1111 1111 1000--8
	//8     8+7=15--0000 0000 0000 1111&1111 1111 1111 1000--8
	//9     9+7=16--0000 0000 0001 0000&1111 1111 1111 1000--16
	return (((n)+ALIGN-1)&~(ALIGN-1));
}

//�����ж�Ӧ�±�
template<bool Thread,int inst>
size_t DefaultAllocTemplate<Thread,inst>::FreeeListIndex(size_t n)
{
	return ((n+ALIGN-1)/(ALIGN-1));
}

typedef DefaultAllocTemplate<0,0> _alloc;

//��һ���Ͷ������������м򵥵ķ�װ
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
// ���Ե���һ�������������ڴ�
void testalloc1()
{
	/*cout<<" ���Ե���һ�������������ڴ� "<<endl ;
	char*p1 = SimpleAlloc<char>::Allocate (129); 
	SimpleAlloc<char>:: Deallocate(p1 , 129);*/
	//���Ե��ö��������������ڴ�
	cout<<" ���Ե��ö��������������ڴ� "<<endl ;
	char*p2 = SimpleAlloc<char>::Allocate (128); 
	char*p3 = SimpleAlloc<char>::Allocate (128); 
	char*p4 = SimpleAlloc<char>::Allocate (128);
	//// ps:���̻߳�����Ҫ���Ǽ��� size_tindex=FREELIST_INDEX(n);
	////ͷ�����������
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
		printf(" ���Ե�%d�η��� \n", i +1);
		p[i] = SimpleAlloc<char>::Allocate (128);
		//SimpleAlloc<char>:: Deallocate(p5 , 128);
	}
	for (int i = 0; i < 21; ++i )
	{
		//printf(" ���Ե�%d�η��� \n", i +1);
		//p[i] = SimpleAlloc<char>::Allocate (128);
		SimpleAlloc<char>:: Deallocate(p[i] , 128);
	}

}

// �������ⳡ�� 
void Testalloc2 ()
{
cout<<" �����ڴ�ؿռ䲻������ "<<endl ;
// 8*20->8*2->320
char*p1 = SimpleAlloc< char>::Allocate (8);
char*p2 = SimpleAlloc< char>::Allocate (8);
cout<<" �����ڴ�ؿռ䲻��,ϵͳ�ѽ��з��� "<<endl ;
char*p3 = SimpleAlloc< char>::Allocate (12); }
// ����ϵͳ���ڴ�ľ��ĳ��� 
void Testalloc3 ()
{
	cout<<" ����ϵͳ���ڴ�ľ� "<<endl ;
	SimpleAlloc<char>::Allocate (1024*1024*1024);
	SimpleAlloc<char>::Allocate(1024*1024*1024);
	SimpleAlloc<char>::Allocate (1024*1024);
	// ���ò���,˵��ϵͳ����С���ڴ���������Ǻ�ǿ�ġ�
	for (int i = 0; i < 100000; ++i ) 
	{
		char*p1 = SimpleAlloc<char>::Allocate (128);
	}
}