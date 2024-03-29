/* https://github.com/Flix01/c_vector_and_hashtable */
/*==================================================================================*/
/* Plain C implementation of std::unordered_map */
/*==================================================================================*/
/*zlib License

Copyright (c) 2019 Flix

This software is provided 'as-is', without any express or implied
warranty.  In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not
   claim that you wrote the original software. If you use this software
   in a product, an acknowledgment in the product documentation would be
   appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must not be
   misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.
*/
/* USAGE: Please see the bundled "c_hashtable_main.c". */

/* SCOPED DEFINITION: The following definitions must be set before each <c_hashtable.h>
   inclusion, and get reset soon after:

   CH_KEY_TYPE              (mandatory)
   CH_VALUE_TYPE            (mandatory)
   CH_NUM_BUCKETS           (mandatory: and must be copied to some 'type-safe definition')
							// in [1,CH_MAX_NUM_BUCKETS] (defaults to 256). USING POWERS OF TWO ALLOW AVOIDING MODULO IN YOUR HASH FUNCTION (i.e. hash % CH_NUM_BUCKETS) BY USING										
							//		'xor-folding' or 'fibonacci-folding' (please search this file for: 'xor-folding' or 'fibonacci-folding' for further info)
   CH_USE_VOID_PTRS_IN_CMP_FCT  (optional: if you want to share cmp_fcts with c style functions like qsort)
   CHV_KEY_SUPPORT_EQUALITY_CMP_IN_UNSORTED_SEARCH (optional, but affects only unsorted buckets, which we should never use)
   C_HASHTABLE_IMPLEMENTATION   	(optional: it needs CH_ENABLE_DECLARATION_AND_DEFINITION. See history for version 1.04)
   C_HASHTABLE_FORCE_DECLARATION  	(optional: it needs CH_ENABLE_DECLARATION_AND_DEFINITION. See history for version 1.04)

   GLOBAL DEFINITIONS: The following definitions (when used) must be set
   globally (= in the Project Options or in a StdAfx.h file):

   CH_MAX_NUM_BUCKETS                   // must be 256, 65536 or 2147483648 and defines ch_hash_uint as unsigned char, unsigned short or unsigned int
   CH_DISABLE_FAKE_MEMBER_FUNCTIONS     // faster with this defined
   CH_DISABLE_CLEARING_ITEM_MEMORY      // faster with this defined
   CH_ENABLE_DECLARATION_AND_DEFINITION // slower with this defined (but saves memory)
   CH_NO_PLACEMENT_NEW                  // (c++ mode only) it does not define (unused) helper stuff like: CH_PLACEMENT_NEW, cpp_ctr,cpp_dtr,cpp_cpy,cpp_cmp

   CH_MALLOC
   CH_REALLOC
   CH_FREE
   CH_ASSERT
   CH_NO_ASSERT
   CH_NO_STDIO
   CH_NO_STDLIB
   CH_API_INL                           // this simply defines the 'inline' keyword syntax (defaults to __inline)
   CH_API                               // used always when CH_ENABLE_DECLARATION_AND_DEFINITION is not defined and in some global or private functions otherwise
   CH_API_DEC                           // defaults to CH_API, or to 'CH_API_INL extern' if CH_ENABLE_DECLARATION_AND_DEFINITION is defined
   CH_APY_DEF                           // defaults to CH_API, or to nothing if CH_ENABLE_DECLARATION_AND_DEFINITION is defined

*/

#ifndef CH_KEY_TYPE
#error Please define CH_KEY_TYPE
#endif
#ifndef CH_VALUE_TYPE
#error Please define CH_VALUE_TYPE
#endif

#ifndef C_HASHTABLE_VERSION
#define C_HASHTABLE_VERSION         "1.10 rev2"
#define C_HASHTABLE_VERSION_NUM     0110
#endif


/* HISTORY:
   C_HASHTABLE_VERSION_NUM 0110 rev2:
   -> added the CV_SIZE_T_FORMATTING definition (internal usage).

   C_HASHTABLE_VERSION_NUM 0110:
   -> Renamed 'ch_hash_murmur3(...)' -> 'ch_hash32_murmur3(...)'
   -> Added the helper functions 'ch_hash32_murmur3_str(...)', 'ch_hash32_FNV1a(...)', 'ch_hash32_FNV1a_str(...)'

   C_HASHTABLE_VERSION_NUM 0108:
   -> Fixed wrong detection of sorting errors in 'ch_xxx_xxx_dbg_check(...)'

   C_HASHTABLE_VERSION_NUM 0108:
   -> added c++ copy ctr, copy assignment and dtr (and move ctr plus move assignment if CH_HAS_MOVE_SEMANTICS is defined),
      to ease porting code from c++ to c a bit more (but 'ch_xxx_xxx_create(...)' is still necessary in c++ mode)
   -> removed internal definitions CH_EXTERN_C_START, CH_EXTERN_C_END, CH_DEFAULT_STRUCT_INIT
      Now code is no more 'extern C'
   -> added optional stuff for c++ compilation: CH_PLACEMENT_NEW,cpp_ctr,cpp_dtr,cpp_cpy,cpp_cmp

   C_HASHTABLE_VERSION_NUM 0107:
   -> renamed CH_VERSION to C_HASHTABLE_VERSION
   -> renamed CH_VERSION_NUM to C_HASHTABLE_VERSION_NUM
   -> some internal changes to minimize interference with (the type-unsafe version) "c_hashtable_type_unsafe.h",
      hoping that they can both cohexist in the same project.
   -> removed some commented out code
   -> made some changes to allow compilation in c++ mode
   -> added a hashtable ctr to ease compilation in c++ mode

   CH_VERSION_NUM 0106:
   -> Fixed a potential duplicate call to key_ctr when resizing a bucket's vector.

   CH_VERSION_NUM 0105:
   -> added CH_API_INL to specify the preferred 'inline' syntax
   -> fixed detection of sorting errors in 'ch_xxx_dbg_check(...)'

   CH_VERSION_NUM 0104:
   -> added the global definition CH_ENABLE_DECLARATION_AND_DEFINITION (disabled by default, because it's faster and easier not to use it)
      Normally we generate ch_mykey_myvalue (== the type std::unordered_map<mykey,myvalue>), this way:
          #ifndef C_HASHTABLE_mykey_myvalue_H
          #define C_HASHTABLE_mykey_myvalue_H
          #    define CH_KEY_TYPE mykey
          #    define CH_VALUE_TYPE myvalue
          #    define CH_NUM_BUCKETS_mykey_myvalue 256
          #    define CH_NUM_BUCKETS CH_NUM_BUCKETS_mykey_myvalue
          #    include <c_hashtable.h>
          #endif
      When CH_ENABLE_DECLARATION_AND_DEFINITION is defined (and it must be defined globally == in the Project Options),
      then the code above is still valid (and much lighter), but we need to include its implementation (once per item type)
      in a (single) .c file this way:
          #ifdef CH_ENABLE_DECLARATION_AND_DEFINITION
          #	   define C_HASHTABLE_IMPLEMENTATION
          #    ifndef C_HASHTABLE_mykey_myvalue_H
          #        define C_HASHTABLE_FORCE_DECLARATION
          #    endif
          #    define CH_KEY_TYPE mykey
          #    define CH_VALUE_TYPE myvalue
          #    define CH_NUM_BUCKETS CH_NUM_BUCKETS_mykey_myvalue   // or just set the same number directly
          #    include <c_hashtable.h>
          #endif
      Please note that C_HASHTABLE_IMPLEMENTATION and C_HASHTABLE_FORCE_DECLARATION are 'scoped definitions' (== get undefined
      at the end of <c_hashtable.h>), and of course CH_KEY_TYPE, CH_VALUE_TYPE and CH_NUM_BUCKETS too:
      so they must be redefined by the user every time they are needed.

   CH_VERSION_NUM 0103:
   -> fixed 'ch_xxx_swap(...)'.
   -> added 'ch_xxx_shrink_to_fit(...)'
   -> CH_MAX_NUM_BUCKETS now can be set as big as 2147483648 (max 32-bit unsigned int plus one)
   -> added 'ch_hash_murmur3(...)' (for little-endian CPUs only). From https://en.wikipedia.org/wiki/MurmurHash
   -> in (nested) struct chv_xxx (the 'bucket' type):
       -> now the first call to 'reserve(&v,new_cap)' (with 'new_cap'>1 to exclude fist indirect call by 'push_back(...)')
          should set the vector capacity exactly to 'new_cap'. This includes the first call to 'resize(&v,new_size)', with 'new_size'>1
          This was done to mitigate a bit the difference between our 'reserve' implementation and others, even if our code is perfectly legal
          [From the C++ reference: reserve][Increase the capacity of the vector to a value that's 'greater or equal' to 'new_cap'. If 'new_cap' is greater than the current capacity(), new storage is allocated, otherwise the method does nothing.]
          So it seems it's O.K. to grow capacity inside 'reserve' (although most implementations do that inside 'resize']
       -> revisited the 'clear item memory before item_ctr()' (where 'item' is our 'key_value pair') concept a bit,
          and added the optional definition CH_DISABLE_CLEARING_ITEM_MEMORY


   CH_VERSION_NUM 0102:
   -> added used memory info to 'ch_xxx_dbg_check(...)'.

   CH_VERSION_NUM 0101:
   -> allowed unsorted buckets (when key_cmp==NULL). Not very robust: they should never be used.
   -> added 'fake member function calls' syntax like:
        ht_mykey_myvalue ht;	// no C init here for fake member functions
        ht_mykey_myvalue_create(&ht,...);	// this inits fake members functions too now

        ht.get_or_insert(&ht,...);	// fake member function calls ('ht' appears twice)
        ht.get(&ht,...);

        ht_mykey_myvalue_free(&ht);
      It can be disabled globally (= in the Project Options) by defining CH_DISABLE_FAKE_MEMBER_FUNCTIONS.
      'Fake member functions' can slow down performance, because the functions can't be inlined anymore.
*/

/* ------------------------------------------------- */
#if __cplusplus>=201103L
#   undef CH_HAS_MOVE_SEMANTICS
#   define CH_HAS_MOVE_SEMANTICS
#endif

#ifndef CV_SIZE_T_FORMATTING
#   if ((defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L) || (defined(_MSC_VER) && _MSC_VER>1800))    /* 1800 -> Visual C++ 2013; 1900 ->  Visual C++ 2015 */
       /* Problem: I'm not sure which Visual C++ version started supporting the C99 printf "%zu" (or at least "%llu") formatting syntax.
          Visual C++ 2013 finally added support for various C99 features in its C mode (including designated initializers, compound literals, and the _Bool type), though it was still not complete.
          Visual C++ 2015 further improved the C99 support, with full support of the C99 Standard Library, except for features that require C99 language features not yet supported by the compiler */
#      define CV_SIZE_T_FORMATTING "zu"
#   else
#      define CV_SIZE_T_FORMATTING "lu"    /* C89 fallback (loses precision and there might be a compiler warning) */
#   endif
#endif /* CV_SIZE_T_FORMATTING */

#if (defined (NDEBUG) || defined (_NDEBUG))
#   undef CH_NO_ASSERT
#   define CH_NO_ASSERT
#   undef CH_NO_STDIO
#   define CH_NO_STDIO
#   undef CH_NO_STDLIB
#   define CH_NO_STDLIB
#endif

#ifndef CH_MALLOC
#   undef CH_NO_STDLIB  /* stdlib cannot be avoided in this case */
#   include <stdlib.h>	 /* malloc/realloc/free */
#   define CH_MALLOC(X) malloc(X)
#endif

#ifndef CH_FREE
#   define CH_FREE(X) free(X)
#endif

#ifndef CH_REALLOC
#   define CH_REALLOC(x,y) realloc((x),(y))
#endif

#include <stddef.h> /* size_t */

#ifndef CH_ASSERT
#   ifdef CH_NO_ASSERT
#       define CH_ASSERT(X) /*no-op*/
#   else
#       include <assert.h>
#       define CH_ASSERT(X) assert((X))
#   endif
#endif
#ifndef CH_NO_STDIO
#   include <stdio.h> /*fprintf,printf,stderr*/
#endif
#ifndef CH_NO_STDLIB
#   include <stdlib.h> /*exit*/
#endif

#include <string.h> /*memcpy,memmove,memset*/

#ifndef CH_API_INL  /* __inline, _inline or inline (C99) */
#define CH_API_INL __inline
#endif

#ifndef CH_API /* can we remove 'static' here? */
#define CH_API CH_API_INL static
#endif

#ifndef CH_ENABLE_DECLARATION_AND_DEFINITION
#	ifndef CH_API_DEC
#		define CH_API_DEC CH_API
#	endif
#	ifndef CH_API_DEF
#		define CH_API_DEF CH_API
#	endif
#else /* CH_ENABLE_DECLARATION_AND_DEFINITION */
#	ifndef CH_API_DEC
#		define CH_API_DEC CH_API_INL extern
#	endif
#	ifndef CH_API_DEF
#		define CH_API_DEF /* no-op */
#	endif
#endif /* CH_ENABLE_DECLARATION_AND_DEFINITION */

#ifdef __cplusplus
/* helper stuff never used in this file */
#   if (!defined(CH_PLACEMENT_NEW) && !defined(CH_NO_PLACEMENT_NEW))
#       if ((defined(_MSC_VER) && _MSC_VER<=1310) || defined(CH_USE_SIMPLER_NEW_OVERLOAD))
#           define CH_PLACEMENT_NEW(_PTR)  new(_PTR)        /* it might require <new> header inclusion */
#       else
            struct CHashtablePlacementNewDummy {};
            inline void* operator new(size_t, CHashtablePlacementNewDummy, void* ptr) { return ptr; }
            inline void operator delete(void*, CHashtablePlacementNewDummy, void*) {}
#           define CH_PLACEMENT_NEW(_PTR)  new(CHashtablePlacementNewDummy() ,_PTR)
#       endif /* _MSC_VER */
#   endif /* CH_PLACEMENT_NEW */
#   if (defined(CH_PLACEMENT_NEW) && !defined(CVH_CPP_GUARD))
#   define CVH_CPP_GUARD
    template<typename T> inline void cpp_ctr(T* v) {CH_PLACEMENT_NEW(v) T();}
    template<typename T> inline void cpp_dtr(T* v) {v->T::~T();}
    template<typename T> inline void cpp_cpy(T* a,const T* b) {*a=*b;}
    template<typename T> inline int cpp_cmp(const T* a,const T* b) {return (*a)<(*b)?-1:((*a)>(*b)?1:0);}
#   endif
#endif

#ifndef CH_XSTR
#define CH_XSTR(s) CH_STR(s)
#define CH_STR(s) #s
#endif

#ifndef CH_CAT 
#	define CH_CAT(x, y) CH_CAT_(x, y)
#	define CH_CAT_(x, y) x ## y
#endif

#ifndef CH_USE_VOID_PTRS_IN_CMP_FCT
#define CH_CMP_TYPE CH_KEY_TYPE
#else
#define CH_CMP_TYPE void
#endif

#define CH_HASHTABLE_TYPE_FCT(name) CH_CAT(CH_HASHTABLE_TYPE,name)
#define CH_KEY_TYPE_FCT(name) CH_CAT(CH_KEY_TYPE,name)
#define CH_VALUE_TYPE_FCT(name) CH_CAT(CH_VALUE_TYPE,name)

#ifndef CH_MAX_NUM_BUCKETS  /* This MUST be 256, 65536 or 2147483648 and can be set only in the project options (not at each header inclusion) */
#   define CH_MAX_NUM_BUCKETS 256
#endif



#ifndef CH_HASH_TYPE_DEFINED    /* internal usage */
#   define CH_HASH_TYPE_DEFINED
#   if CH_MAX_NUM_BUCKETS<=0
#       error CH_MAX_NUM_BUCKETS must be positive.
#   elif CH_MAX_NUM_BUCKETS<=256
#       undef CH_MAX_NUM_BUCKETS
#       define CH_MAX_NUM_BUCKETS 256
        typedef unsigned char ch_hash_uint;
#   elif CH_MAX_NUM_BUCKETS<=65536
#       undef CH_MAX_NUM_BUCKETS
#       define CH_MAX_NUM_BUCKETS 65536
        typedef unsigned short ch_hash_uint;
#   elif CH_MAX_NUM_BUCKETS<=2147483648
#       undef CH_MAX_NUM_BUCKETS
#       define CH_MAX_NUM_BUCKETS 2147483648
        typedef unsigned int ch_hash_uint;
#   else
#       error CH_MAX_NUM_BUCKETS cannot be bigger than 2147483648.
#   endif
#endif

#ifndef CH_NUM_BUCKETS
#   define CH_NUM_BUCKETS CH_MAX_NUM_BUCKETS
#endif

#if CH_NUM_BUCKETS<=0
#   error CH_NUM_BUCKETS must be positive.
#elif CH_NUM_BUCKETS>CH_MAX_NUM_BUCKETS
#   if CH_NUM_BUCKETS<=65536
#       error CH_MAX_NUM_BUCKETS must be set to 65536 globally in the Project Options to allow CH_NUM_BUCKETS>256.
#   elif CH_NUM_BUCKETS<=2147483648
#       error CH_MAX_NUM_BUCKETS must be set to 2147483648 globally in the Project Options to allow CH_NUM_BUCKETS>65536.
#   else
#       error CH_NUM_BUCKETS cannot be bigger than 2147483648.
#   endif
#endif

#define CH_HASHTABLE_ITEM_TYPE_TMP CH_CAT(CH_KEY_TYPE,_) 
#define CH_HASHTABLE_ITEM_TYPE CH_CAT(CH_HASHTABLE_ITEM_TYPE_TMP,CH_VALUE_TYPE) 
#define CH_HASHTABLE_TYPE CH_CAT(ch_,CH_HASHTABLE_ITEM_TYPE)
#define CH_VECTOR_TYPE CH_CAT(chv_,CH_HASHTABLE_ITEM_TYPE)
#define CH_VECTORS_TYPE CH_CAT(chvs_,CH_HASHTABLE_ITEM_TYPE)
#define CH_VECTOR_TYPE_FCT(name) CH_CAT(CH_VECTOR_TYPE,name)

#if (!defined(CH_ENABLE_DECLARATION_AND_DEFINITION) || !defined(C_HASHTABLE_IMPLEMENTATION) || defined(C_HASHTABLE_FORCE_DECLARATION))
typedef struct {
	CH_KEY_TYPE k;
	CH_VALUE_TYPE v;	
} CH_HASHTABLE_ITEM_TYPE;
typedef struct CH_HASHTABLE_TYPE CH_HASHTABLE_TYPE;
struct CH_HASHTABLE_TYPE {
    /* key callbacks */
    void (*const key_ctr)(CH_KEY_TYPE*);                     /* optional (can be NULL) */
    void (*const key_dtr)(CH_KEY_TYPE*);                     /* optional (can be NULL) */
    void (*const key_cpy)(CH_KEY_TYPE*,const CH_KEY_TYPE*);		/* optional (can be NULL) */
    int (*const key_cmp)(const CH_CMP_TYPE*,const CH_CMP_TYPE*);/* optional (can be NULL) (for sorted vectors only) */
    ch_hash_uint (*const key_hash)(const CH_KEY_TYPE*);
    /* value callbacks */
    void (*const value_ctr)(CH_VALUE_TYPE*);                     /* optional (can be NULL) */
    void (*const value_dtr)(CH_VALUE_TYPE*);                     /* optional (can be NULL) */
    void (*const value_cpy)(CH_VALUE_TYPE*,const CH_VALUE_TYPE*);/* optional (can be NULL) */

    /* CH_NUM_BUCKETS sorted buckets */
    const size_t initial_bucket_capacity;
    struct CH_VECTOR_TYPE {
		CH_HASHTABLE_ITEM_TYPE * v;
		const size_t size;
		const size_t capacity;	
#       ifdef __cplusplus
        CH_VECTOR_TYPE() : v(NULL),size(0),capacity(0) {}
        inline const CH_HASHTABLE_ITEM_TYPE& operator[](size_t i) const {CH_ASSERT(i<size);return v[i];}
        inline CH_HASHTABLE_ITEM_TYPE& operator[](size_t i) {CH_ASSERT(i<size);return v[i];}
#       endif
    } buckets[CH_NUM_BUCKETS];

#   ifndef CH_DISABLE_FAKE_MEMBER_FUNCTIONS  /* must be defined glabally (in the Project Options)) */
    void (* const clear)(CH_HASHTABLE_TYPE* ht);
    void (* const free)(CH_HASHTABLE_TYPE* ht);
    void (* const shrink_to_fit)(CH_HASHTABLE_TYPE* ht);
    CH_VALUE_TYPE* (* const get_or_insert)(CH_HASHTABLE_TYPE* ht,const CH_KEY_TYPE* key,int* match);
    CH_VALUE_TYPE* (* const get_or_insert_by_val)(CH_HASHTABLE_TYPE* ht,const CH_KEY_TYPE key,int* match);
    CH_VALUE_TYPE* (* const get)(CH_HASHTABLE_TYPE* ht,const CH_KEY_TYPE* key);
    CH_VALUE_TYPE* (* const get_by_val)(CH_HASHTABLE_TYPE* ht,const CH_KEY_TYPE key);
    const CH_VALUE_TYPE* (* const get_const)(const CH_HASHTABLE_TYPE* ht,const CH_KEY_TYPE* key);
    const CH_VALUE_TYPE* (* const get_const_by_val)(const CH_HASHTABLE_TYPE* ht,const CH_KEY_TYPE key);
    int (* const remove)(CH_HASHTABLE_TYPE* ht,const CH_KEY_TYPE* key);
    int (* const remove_by_val)(CH_HASHTABLE_TYPE* ht,const CH_KEY_TYPE key);
    size_t (* const get_num_items)(const CH_HASHTABLE_TYPE* ht);
    int (* const dbg_check)(const CH_HASHTABLE_TYPE* ht);
    void (* const swap)(CH_HASHTABLE_TYPE* a,CH_HASHTABLE_TYPE* b);
    void (* const cpy)(CH_HASHTABLE_TYPE* a,const CH_HASHTABLE_TYPE* b);
#   endif
#   ifdef __cplusplus
    CH_HASHTABLE_TYPE();
    CH_HASHTABLE_TYPE(const CH_HASHTABLE_TYPE& o);
    CH_HASHTABLE_TYPE& operator=(const CH_HASHTABLE_TYPE& o);

#   ifdef CH_HAS_MOVE_SEMANTICS
    CH_HASHTABLE_TYPE(CH_HASHTABLE_TYPE&& o);
    CH_HASHTABLE_TYPE& operator=(CH_HASHTABLE_TYPE&& o);
#   endif
    ~CH_HASHTABLE_TYPE();
#   endif
};
#ifdef __cplusplus
typedef CH_HASHTABLE_TYPE::CH_VECTOR_TYPE CH_VECTOR_TYPE;
#else
typedef struct CH_VECTOR_TYPE CH_VECTOR_TYPE;
#endif
typedef CH_VECTOR_TYPE CH_VECTORS_TYPE[CH_NUM_BUCKETS];


#ifdef CH_ENABLE_DECLARATION_AND_DEFINITION
/* function declarations */
CH_API_DEC void CH_HASHTABLE_TYPE_FCT(_free)(CH_HASHTABLE_TYPE* ht);
CH_API_DEC void CH_HASHTABLE_TYPE_FCT(_clear)(CH_HASHTABLE_TYPE* ht);
CH_API_DEC CH_VALUE_TYPE* CH_HASHTABLE_TYPE_FCT(_get_or_insert)(CH_HASHTABLE_TYPE* ht,const CH_KEY_TYPE* key,int* match);
CH_API_DEC CH_VALUE_TYPE* CH_HASHTABLE_TYPE_FCT(_get_or_insert_by_val)(CH_HASHTABLE_TYPE* ht,const CH_KEY_TYPE key,int* match);
CH_API_DEC CH_VALUE_TYPE* CH_HASHTABLE_TYPE_FCT(_get)(CH_HASHTABLE_TYPE* ht,const CH_KEY_TYPE* key);
CH_API_DEC CH_VALUE_TYPE* CH_HASHTABLE_TYPE_FCT(_get_by_val)(CH_HASHTABLE_TYPE* ht,const CH_KEY_TYPE key);
CH_API_DEC const CH_VALUE_TYPE* CH_HASHTABLE_TYPE_FCT(_get_const)(const CH_HASHTABLE_TYPE* ht,const CH_KEY_TYPE* key);
CH_API_DEC const CH_VALUE_TYPE* CH_HASHTABLE_TYPE_FCT(_get_const_by_val)(const CH_HASHTABLE_TYPE* ht,const CH_KEY_TYPE key);
CH_API_DEC int CH_HASHTABLE_TYPE_FCT(_remove)(CH_HASHTABLE_TYPE* ht,const CH_KEY_TYPE* key);
CH_API_DEC int CH_HASHTABLE_TYPE_FCT(_remove_by_val)(CH_HASHTABLE_TYPE* ht,const CH_KEY_TYPE key);
CH_API_DEC size_t CH_HASHTABLE_TYPE_FCT(_get_num_items)(const CH_HASHTABLE_TYPE* ht);
CH_API_DEC int CH_HASHTABLE_TYPE_FCT(_dbg_check)(const CH_HASHTABLE_TYPE* ht);
CH_API_DEC void CH_HASHTABLE_TYPE_FCT(_swap)(CH_HASHTABLE_TYPE* a,CH_HASHTABLE_TYPE* b);
CH_API_DEC void CH_HASHTABLE_TYPE_FCT(_cpy)(CH_HASHTABLE_TYPE* a,const CH_HASHTABLE_TYPE* b);
CH_API_DEC void CH_HASHTABLE_TYPE_FCT(_shrink_to_fit)(CH_HASHTABLE_TYPE* ht);
CH_API_DEC void CH_HASHTABLE_TYPE_FCT(_create_with)(
        CH_HASHTABLE_TYPE* ht,
        ch_hash_uint (*key_hash)(const CH_KEY_TYPE*),
        int (*key_cmp) (const CH_CMP_TYPE*,const CH_CMP_TYPE*),
        void (*key_ctr)(CH_KEY_TYPE*),void (*key_dtr)(CH_KEY_TYPE*),void (*key_cpy)(CH_KEY_TYPE*,const CH_KEY_TYPE*),
        void (*value_ctr)(CH_VALUE_TYPE*),void (*value_dtr)(CH_VALUE_TYPE*),void (*value_cpy)(CH_VALUE_TYPE*,const CH_VALUE_TYPE*),
        size_t initial_bucket_capacity);
CH_API_DEC void CH_HASHTABLE_TYPE_FCT(_create)(CH_HASHTABLE_TYPE* ht,ch_hash_uint (*key_hash)(const CH_KEY_TYPE*),int (*key_cmp) (const CH_CMP_TYPE*,const CH_CMP_TYPE*),size_t initial_bucket_capacity);
#endif /* CH_ENABLE_DECLARATION_AND_DEFINITION */
#endif /*  (!defined(CH_ENABLE_DECLARATION_AND_DEFINITION) || !defined(C_HASHTABLE_IMPLEMENTATION) || defined(C_HASHTABLE_FORCE_DECLARATION)) */

#ifndef CH_COMMON_FUNCTIONS_GUARD
#define CH_COMMON_FUNCTIONS_GUARD
/* base memory helpers */
CH_API void* ch_malloc(size_t size) {
    void* p = CH_MALLOC(size);
    if (!p)	{
        CH_ASSERT(0);	/* No more memory error */
#       ifndef CH_NO_STDIO
        fprintf(stderr,"CH_ERROR: ch_malloc(...) failed. Not enough memory.\n");
#       endif
#       ifndef CH_NO_STDLIB
        exit(1);
#       endif
    }
    return p;
}
CH_API void ch_free(void* p)                         {CH_FREE(p);}
CH_API void* ch_safe_realloc(void** const ptr, size_t new_size)  {
    void *ptr2 = CH_REALLOC(*ptr,new_size);
    CH_ASSERT(new_size!=0);    /* undefined behaviour */
    if (ptr2) *ptr=ptr2;
    else {
        CH_FREE(*ptr);*ptr=NULL;
        CH_ASSERT(0);	/* No more memory error */
#       ifndef CH_NO_STDIO
        fprintf(stderr,"CH_ERROR: ch_safe_realloc(...) failed. Not enough memory.\n");
#       endif
#       ifndef CH_NO_STDLIB
        exit(1);
#       endif
    }
    return ptr2;
}
CH_API void ch_convert_bytes(size_t bytes_in,size_t pTGMKB[5])   {
    size_t i;pTGMKB[4] = bytes_in;
    for (i=0;i<4;i++)  {pTGMKB[3-i]=pTGMKB[4-i]/1024;pTGMKB[4-i]%=1024;}
}
#ifndef CH_NO_STDIO
CH_API void ch_display_bytes(size_t bytes_in)   {
    size_t pTGMKB[5],i,cnt=0;
    const char* names[5] = {"TB","GB","MB","KB","Bytes"};
    ch_convert_bytes(bytes_in,pTGMKB);
    for (i=0;i<5;i++)   {
        if (pTGMKB[i]!=0) {
            if (cnt>0) printf(" ");
            printf("%" CV_SIZE_T_FORMATTING " %s",pTGMKB[i],names[i]);
            ++cnt;
        }
    }
}
#endif /* CH_NO_STDIO */
/* from https://en.wikipedia.org/wiki/MurmurHash
   Warning: it returns unsigned int and works for little-endian CPUs only
*/
CH_API unsigned ch_hash32_murmur3(const unsigned char* key, size_t len, unsigned seed)    {
    unsigned h = seed;	/* seed, an arbitrary (one per application, or one per hashtable): ex: 0xBAADF00D */
    if (len > 3) {
        size_t i = len >> 2;
        do {
            unsigned k;memcpy(&k, key, sizeof(unsigned));key += sizeof(unsigned);
			/* each 4-byte-chunk of 'key' is processed in 'k' this way: */
            k *= 0xcc9e2d51;
            k = (k << 15) | (k >> 17);
            k *= 0x1b873593;
            h ^= k;
            h = (h << 13) | (h >> 19);
            h = h * 5 + 0xe6546b64;
        } while (--i);
    }
    if (len & 3) {
        size_t i = len & 3;
        unsigned k = 0;
        do {
            k <<= 8;
            k |= key[i - 1];
        } while (--i);
		/* Note: Endian swapping of k is necessary on big-endian machines here. */
		/* any remaining-byte of 'key' is processed in 'k' this way: */        
		k *= 0xcc9e2d51;
        k = (k << 15) | (k >> 17);
        k *= 0x1b873593;
        h ^= k;
    }
    h ^= len;
    h ^= h >> 16;
    h *= 0x85ebca6b;
    h ^= h >> 13;
    h *= 0xc2b2ae35;
    h ^= h >> 16;
    return h;
}
CH_API unsigned ch_hash32_murmur3_str(const char* text, unsigned seed)    {
    unsigned h = seed;	/* seed, an arbitrary (one per application, or one per hashtable): ex: 0xBAADF00D */
	const unsigned char* str = (const unsigned char*) text;	
	const unsigned char* key = str;
	size_t len = 0,tot_len = 0;	/* we use'len' always in [0,4] here */
	while (*key++) {
		++len;
		if (len==4)	{
            unsigned k;memcpy(&k, str, sizeof(unsigned));str += sizeof(unsigned);
			/* each 4-byte-chunk of 'key' is processed in 'k' this way: */
            k *= 0xcc9e2d51;
            k = (k << 15) | (k >> 17);
            k *= 0x1b873593;
            h ^= k;
            h = (h << 13) | (h >> 19);
            h = h * 5 + 0xe6546b64;	
			tot_len+=len;len = 0; key = str;		
		}	
	}
	tot_len+=len;
    if (len) {
		/* 'len' should be in [1,3] here */
        unsigned k = 0;
        do {
            k <<= 8;
            k |= str[len - 1];
        } while (--len);
		/* Note: Endian swapping of k is necessary on big-endian machines here (skipped). */
		/* any remaining-byte of 'key' is processed in 'k' this way: */        
		k *= 0xcc9e2d51;
        k = (k << 15) | (k >> 17);
        k *= 0x1b873593;
        h ^= k;
    }
    h ^= tot_len;
    h ^= h >> 16;
    h *= 0x85ebca6b;
    h ^= h >> 13;
    h *= 0xc2b2ae35;
    h ^= h >> 16;
    return h;
}
/* based on http://www.isthe.com/chongo/tech/comp/fnv/index.html */
CH_API unsigned ch_hash32_FNV1a(const unsigned char* key, size_t len)	{
    const unsigned char *bp = (unsigned char *)key;	
    const unsigned char *be = bp + len;
    unsigned hash = 2166136261;	/* offset_basis for 32 bit hash */
	while (bp < be) {
		hash ^= (unsigned)*bp++;    
#		if (defined(__GCC__) || defined(CH_FNV_GCC_OPTIMIZATION))
		hash += (hash<<1) + (hash<<4) + (hash<<7) + (hash<<8) + (hash<<24);
#		else
		hash = hash * 16777619;	/* prime for 32 bit hash */
#		endif
	}
    return hash;
}
CH_API unsigned ch_hash32_FNV1a_str(const char* text)	{
	unsigned hash = 2166136261;	/* offset_basis for 32 bit hash */
	const unsigned char* str = (const unsigned char*) text;	
	while (*str)	{
		hash ^= (unsigned) *str++;
#		if (defined(__GCC__) || defined(CH_FNV_GCC_OPTIMIZATION))
		hash += (hash<<1) + (hash<<4) + (hash<<7) + (hash<<8) + (hash<<24);
#		else
		hash = hash * 16777619;	/* prime for 32 bit hash */
#		endif
	}
	return hash;
}
/* 	[I don't know much about hashing theory... so please don't trust the following info too much!]
	
	All the helper hash functions above return 32-bit unsigned integers: this type does not always match 'ch_hash_uint',
	that is required by our signature. To convert hash32 (the 32-bit value returned by these helper hash functions) to 'ch_hash_uint':

	->	The type match when CH_NUM_BUCKETS==2147483648(==CH_MAX_NUM_BUCKETS) and: 'ch_hash_uint' == unsigned
		ch_hash_uint hash == hash32; // cast to 'ch_hash_uint' is not necessary

	-> Otherwise we can use the 32-bit hash functions and then fold values in our range.
		To do so:
		-> if CH_NUM_BUCKETS is not a power of two we must use mod:
			ch_hash_uint hash = (ch_hash_uint)(hash32%CH_NUM_BUCKETS);	// a bit slow
		-> otherwise CH_NUM_BUCKETS == 2^X == pow(2,X). In this case we can use mod too, but also:
			-> 'xor-folding' (see http://www.isthe.com/chongo/tech/comp/fnv/index.html):
				ch_hash_uint hashX = (ch_hash_uint) ((hash32>>X) ^ (hash32 & (((unsigned)1<<(x))-1));
			-> 'fibonacci-folding' (see https://programmingpraxis.com/2018/06/19/fibonacci-hash):
				ch_hash_uint hashX = (ch_hash_uint) ((hash32*2654435769U) >> (32-X));

	-> In the default case: CH_NUM_BUCKETS == 256 (== CH_MAX_NUM_BUCKETS): we could do nothing and just cast 
	   to 'ch_hash_uint', but since unsigned char is not a 32-bit value, we can do better by using a folding technique 
       and see if it works better:
		-> ch_hash_uint hash8 = (ch_hash_uint) ((hash32>>8) ^ (hash32 & ((unsigned)0xFF)));		// 'xor-folding' or
		-> ch_hash_uint hash8 = (ch_hash_uint) ((hash32*2654435769U) >> 24);					// 'fibonacci-folding'
*/
/* if CH_NUM_BUCKETS == pow(2,num_buckets_pot_exponent), with num_buckets_pot_exponent<32, then we can 'convert' 32-bit hash values this way: */
#	define CH_HASH_FROM_HASH32_USING_XORFOLDING(hash32,num_buckets_pot_exponent)	( (hash32>>(num_buckets_pot_exponent)) ^ ( hash32 & (((unsigned)1<<(num_buckets_pot_exponent))-1) ) )
#	define CH_HASH_FROM_HASH32_USING_FIBFOLDING(hash32,num_buckets_pot_exponent)	((hash32*2654435769U) >> (32-(num_buckets_pot_exponent)))
#endif /* CH_COMMON_FUNCTIONS_GUARD */


#if (!defined(CH_ENABLE_DECLARATION_AND_DEFINITION) || defined(C_HASHTABLE_IMPLEMENTATION))
/* --- PRIVATE FUNCTIONS START -------------------------------------------------- */
CH_API void CH_VECTOR_TYPE_FCT(_clear)(CH_VECTOR_TYPE* v,const CH_HASHTABLE_TYPE* ht)	{
    CH_ASSERT(v && ht);
    if (v->v) {
        if (ht->key_dtr || ht->value_dtr)   {
            size_t j;
            for (j=0;j<v->size;j++) {
                CH_HASHTABLE_ITEM_TYPE* item = &v->v[j];
                if (ht->key_dtr)    ht->key_dtr(&item->k);
                if (ht->value_dtr)  ht->value_dtr(&item->v);
            }
        }
    }
    *((size_t*) &v->size)=0;
}
CH_API void CH_VECTOR_TYPE_FCT(_reserve)(CH_VECTOR_TYPE* v,size_t size,const CH_HASHTABLE_TYPE* ht)	{
    CH_ASSERT(v && ht);
    /* grows-only! */
    if (size>v->capacity) {
        const size_t new_capacity = (v->capacity==0 && size>1) ?
                    size :      /* possibly keep initial user-guided 'reserve(...)' */
                    (v->capacity+(size-v->capacity)+(v->capacity)/2);   /* our growing strategy */
        ch_safe_realloc((void** const) &v->v,new_capacity*sizeof(CH_HASHTABLE_ITEM_TYPE));
        *((size_t*) &v->capacity) = new_capacity;
    }
}
CH_API void CH_VECTOR_TYPE_FCT(_resize)(CH_VECTOR_TYPE* v,size_t size,const CH_HASHTABLE_TYPE* ht)	{
    CH_ASSERT(v && ht);
    if (size>v->capacity) CH_VECTOR_TYPE_FCT(_reserve)(v,size,ht);
    if (size<v->size)   {
        size_t i;
        if (ht->key_dtr && ht->value_dtr) {for (i=size;i<v->size;i++) {ht->key_dtr(&v->v[i].k);ht->value_dtr(&v->v[i].v);}}
        else if (ht->key_dtr)   {for (i=size;i<v->size;i++) ht->key_dtr(&v->v[i].k);}
        else if (ht->value_dtr) {for (i=size;i<v->size;i++) ht->value_dtr(&v->v[i].v);}
    }
    else {
        size_t i;
#       ifndef CH_DISABLE_CLEARING_ITEM_MEMORY
        if (ht->key_ctr || ht->key_cpy || ht->value_ctr || ht->value_cpy) memset(&v->v[v->size],0,(size-v->size)*sizeof(CH_HASHTABLE_ITEM_TYPE));
#       endif
        if (ht->key_ctr && ht->value_ctr)   {for (i=v->size;i<size;i++) {ht->key_ctr(&v->v[i].k);ht->value_ctr(&v->v[i].v);}}
        else if (ht->key_ctr)        {for (i=v->size;i<size;i++) ht->key_ctr(&v->v[i].k);}
        else if (ht->value_ctr) {for (i=v->size;i<size;i++) ht->value_ctr(&v->v[i].v);}
    }
    *((size_t*) &v->size)=size;
}
CH_API size_t CH_VECTOR_TYPE_FCT(_unsorted_search)(const CH_VECTOR_TYPE* v,const CH_KEY_TYPE* key,int* match,const CH_HASHTABLE_TYPE* ht)  {
    size_t i;
    CH_ASSERT(v && ht);
    if (match) *match=0;
    if (v->size==0) return 0;  /* otherwise match will be 1 */
    if (!ht->key_cmp)   {
        int cmp_ok=0;
        for (i = 0; i < v->size; i++) {
#           ifndef CHV_KEY_SUPPORT_EQUALITY_CMP_IN_UNSORTED_SEARCH  /* this is reset after header inclusion */
            cmp_ok = (memcmp(key,&v->v[i].k,sizeof(CH_KEY_TYPE))==0)?1:0;
#           else
            cmp_ok = (*key==v->v[i].k)?1:0;
#           endif
            if (cmp_ok) {if (match) {*match=1;} return i;}}
    }
    else    {
        for (i = 0; i < v->size; i++) {if (ht->key_cmp(key,&v->v[i].k)==0) {if (match) {*match=1;}return i;}}
    }
    CH_ASSERT(i==v->size);
    return i;
}
CH_API size_t CH_VECTOR_TYPE_FCT(_linear_search)(const CH_VECTOR_TYPE* v,const CH_KEY_TYPE* key,int* match,const CH_HASHTABLE_TYPE* ht)  {
    int cmp=0;size_t i;
    CH_ASSERT(v && ht && ht->key_cmp);
    if (match) *match=0;
    if (v->size==0) return 0;  /* otherwise match will be 1 */
    for (i = 0; i < v->size; i++) {
        cmp = ht->key_cmp(key,&v->v[i].k);
        if (cmp<=0) {
            if (cmp==0 && match) *match=1;
            return i;
        }
    }
    CH_ASSERT(i==v->size);
    return i;
}
CH_API size_t CH_VECTOR_TYPE_FCT(_binary_search)(const CH_VECTOR_TYPE* v,const CH_KEY_TYPE* key,int* match,const CH_HASHTABLE_TYPE* ht)  {
    size_t first=0, last;
    size_t mid;int cmp;
    CH_ASSERT(v && ht && ht->key_cmp);
    if (match) *match=0;
    if (v->size==0) return 0;  /* otherwise match will be 1 */
    last=v->size-1;
    while (first <= last) {
        mid = (first + last) / 2;
        cmp = ht->key_cmp(key,&v->v[mid].k);
        if (cmp>0) {
            first = mid + 1;
        }
        else if (cmp<0) {
            if (mid==0) return 0;
            last = mid - 1;
        }
        else {if (match) *match=1;CH_ASSERT(mid<v->size); return mid;}
    }
    CH_ASSERT(mid<v->size);
    return cmp>0 ? (mid+1) : mid;
}
CH_API size_t CH_VECTOR_TYPE_FCT(_insert_key_at)(CH_VECTOR_TYPE* v,const CH_KEY_TYPE* key_to_insert,size_t position,const CH_HASHTABLE_TYPE* ht)  {
    /* position is in [0,v->size] */
    /* warning: this code does NOT support passing pointers to keys already present in this hashtable */
    CH_ASSERT(v && ht && key_to_insert && position<=v->size);
    CH_VECTOR_TYPE_FCT(_reserve)(v,v->size+1,ht);
    if (position<v->size) memmove(&v->v[position+1],&v->v[position],(v->size-position)*sizeof(CH_HASHTABLE_ITEM_TYPE));
#   ifndef CH_DISABLE_CLEARING_ITEM_MEMORY
    if (ht->key_ctr || ht->key_cpy || ht->value_ctr || ht->value_cpy) memset(&v->v[position],0,sizeof(CH_HASHTABLE_ITEM_TYPE));
#   endif
    if (ht->key_ctr)    ht->key_ctr(&v->v[position].k);
    if (ht->value_ctr)  ht->value_ctr(&v->v[position].v);
    if (!ht->key_cpy)   memcpy(&v->v[position].k,key_to_insert,sizeof(CH_KEY_TYPE));
    else ht->key_cpy(&v->v[position].k,key_to_insert);
    *((size_t*) &v->size)=v->size+1;
    return position;
}
CH_API int CH_VECTOR_TYPE_FCT(_remove_at)(CH_VECTOR_TYPE* v,size_t position,const CH_HASHTABLE_TYPE* ht)  {
    /* position is in [0,num_items) */
    int removal_ok;
    CH_ASSERT(v && ht);
    removal_ok = (position<v->size) ? 1 : 0;
    CH_ASSERT(removal_ok);	/* error: position>=v->size */
    if (removal_ok)	{
        if (ht->key_dtr)    ht->key_dtr(&v->v[position].k);
        if (ht->value_dtr)  ht->value_dtr(&v->v[position].v);
        memmove(&v->v[position],&v->v[position+1],(v->size-position-1)*sizeof(CH_HASHTABLE_ITEM_TYPE));
        *((size_t*) &v->size)=v->size-1;
    }
    return removal_ok;
}
/* --- PRIVATE FUNCTIONS END -------------------------------------------------- */

CH_API_DEF void CH_HASHTABLE_TYPE_FCT(_free)(CH_HASHTABLE_TYPE* ht)    {
    if (ht) {
        const unsigned short max_value = (CH_NUM_BUCKETS-1);
        unsigned short i=0;
        do    {
            CH_VECTOR_TYPE* b = &ht->buckets[i];
            CH_VECTOR_TYPE_FCT(_clear)(b,ht);
            if (b->v) {ch_free(b->v);ht->buckets[i].v=NULL;}
            *((size_t*)&b->capacity)=0;
        }
        while (i++!=max_value);
    }
}
CH_API_DEF void CH_HASHTABLE_TYPE_FCT(_clear)(CH_HASHTABLE_TYPE* ht)    {
    if (ht) {
        const unsigned short max_value = (CH_NUM_BUCKETS-1);
        unsigned short i=0;
        do    {CH_VECTOR_TYPE_FCT(_clear)(&ht->buckets[i],ht);}
        while (i++!=max_value);
    }
}

/* Warning: 'ch_xxx_get_or_insert(...)' and 'ch_xxx_get(...)', and their overloads,
   return pointer to 'value' items inside the hashtable, that are invalidated when
   the hashtable inserts or removes other items. User should copy the result for longer storage.
*/
CH_API_DEF CH_VALUE_TYPE* CH_HASHTABLE_TYPE_FCT(_get_or_insert)(CH_HASHTABLE_TYPE* ht,const CH_KEY_TYPE* key,int* match) {
    CH_VECTOR_TYPE* v = NULL;
    size_t position;ch_hash_uint hash;int match2;
    CH_ASSERT(ht && ht->key_hash);
    hash = ht->key_hash(key);
#   if CH_NUM_BUCKETS!=CH_MAX_NUM_BUCKETS
    CH_ASSERT(hash<CH_NUM_BUCKETS);    /* user 'key_hash' should return values in [0,CH_NUM_BUCKETS). Please use: return somevalue%CH_NUM_BUCKETS */
#   endif
    v = &ht->buckets[hash];
    if (!v->v)  {
        v->v = (CH_HASHTABLE_ITEM_TYPE*) ch_malloc(ht->initial_bucket_capacity*sizeof(CH_HASHTABLE_ITEM_TYPE));
        *((size_t*)&v->capacity) = ht->initial_bucket_capacity;
    }

    if (v->size==0)    {position=0;match2=0;}
    else if (ht->key_cmp)   {
        /* slightly faster */
        position =  v->size>2 ? CH_VECTOR_TYPE_FCT(_binary_search)(v,key,&match2,ht) :
                    CH_VECTOR_TYPE_FCT(_linear_search)(v,key,&match2,ht);
        /* slightly slower */
        /* position = CH_VECTOR_TYPE_FCT(_binary_search)(v,key,&match2,ht); */
    }
    else    {
        /* '_unsorted_search' uses memcmp(...) (when ht->key_cmp==NULL) */
        position = CH_VECTOR_TYPE_FCT(_unsorted_search)(v,key,&match2,ht);
    }
    if (match) *match=match2;

    if (match2) return &v->v[position].v;

    /* we must insert an item at 'position' */
    CH_VECTOR_TYPE_FCT(_insert_key_at)(v,key,position,ht);
    return &v->v[position].v;
}
CH_API_DEF CH_VALUE_TYPE* CH_HASHTABLE_TYPE_FCT(_get_or_insert_by_val)(CH_HASHTABLE_TYPE* ht,const CH_KEY_TYPE key,int* match) {return CH_HASHTABLE_TYPE_FCT(_get_or_insert)(ht,&key,match);}
CH_API_DEF CH_VALUE_TYPE* CH_HASHTABLE_TYPE_FCT(_get)(CH_HASHTABLE_TYPE* ht,const CH_KEY_TYPE* key) {
    CH_VECTOR_TYPE* v = NULL;
    size_t position;ch_hash_uint hash;int match=0;
    CH_ASSERT(ht && ht->key_hash);
    hash = ht->key_hash(key);
#   if CH_NUM_BUCKETS!=CH_MAX_NUM_BUCKETS
    CH_ASSERT(hash<CH_NUM_BUCKETS);    /* user 'key_hash' should return values in [0,CH_NUM_BUCKETS). Please use: return somevalue%CH_NUM_BUCKETS */
#   endif
    v = &ht->buckets[hash];
    if (!v->v || v->size==0)  return NULL;

    if (ht->key_cmp)    {
        /* slightly faster */
        position =  v->size>2 ? CH_VECTOR_TYPE_FCT(_binary_search)(v,key,&match,ht) :
                    CH_VECTOR_TYPE_FCT(_linear_search)(v,key,&match,ht);
        /* slightly slower */
        /* position = CH_VECTOR_TYPE_FCT(_binary_search)(v,key,&match,ht); */
    }
    else    {
        /* '_unsorted_search' uses memcmp(...) (when ht->key_cmp==NULL) */
        position = CH_VECTOR_TYPE_FCT(_unsorted_search)(v,key,&match,ht);
    }

    return (match ? &v->v[position].v : NULL);
}
CH_API_DEF CH_VALUE_TYPE* CH_HASHTABLE_TYPE_FCT(_get_by_val)(CH_HASHTABLE_TYPE* ht,const CH_KEY_TYPE key) {return CH_HASHTABLE_TYPE_FCT(_get)(ht,&key);}
CH_API_DEF const CH_VALUE_TYPE* CH_HASHTABLE_TYPE_FCT(_get_const)(const CH_HASHTABLE_TYPE* ht,const CH_KEY_TYPE* key) {return (const CH_VALUE_TYPE*) CH_HASHTABLE_TYPE_FCT(_get)((CH_HASHTABLE_TYPE*)ht,key);}
CH_API_DEF const CH_VALUE_TYPE* CH_HASHTABLE_TYPE_FCT(_get_const_by_val)(const CH_HASHTABLE_TYPE* ht,const CH_KEY_TYPE key) {return (const CH_VALUE_TYPE*) CH_HASHTABLE_TYPE_FCT(_get)((CH_HASHTABLE_TYPE*)ht,&key);}

CH_API_DEF int CH_HASHTABLE_TYPE_FCT(_remove)(CH_HASHTABLE_TYPE* ht,const CH_KEY_TYPE* key) {
    CH_VECTOR_TYPE* v = NULL;
    size_t position;ch_hash_uint hash;int match = 0;
    CH_ASSERT(ht && ht->key_hash);
    hash = ht->key_hash(key);
#   if CH_NUM_BUCKETS!=CH_MAX_NUM_BUCKETS
    CH_ASSERT(hash<CH_NUM_BUCKETS);    /* user 'key_hash' should return values in [0,CH_NUM_BUCKETS). Please use: return somevalue%CH_NUM_BUCKETS */
#   endif
    v = &ht->buckets[hash];
    if (!v->v)  return 0;
    if (ht->key_cmp)    {
        /* slightly faster */
        position =  v->size>2 ? CH_VECTOR_TYPE_FCT(_binary_search)(v,key,&match,ht) :
                    CH_VECTOR_TYPE_FCT(_linear_search)(v,key,&match,ht);
        /* slightly slower */
        /* position = CH_VECTOR_TYPE_FCT(_binary_search)(v,key,&match,ht); */
    }
    else    {
        /* '_unsorted_search' uses memcmp(...) (when ht->key_cmp==NULL) */
        position = CH_VECTOR_TYPE_FCT(_unsorted_search)(v,key,&match,ht);
    }
    if (match) {
        CH_VECTOR_TYPE_FCT(_remove_at)(v,position,ht);
        return 1;
    }
    return 0;
}
CH_API_DEF int CH_HASHTABLE_TYPE_FCT(_remove_by_val)(CH_HASHTABLE_TYPE* ht,const CH_KEY_TYPE key) {return CH_HASHTABLE_TYPE_FCT(_remove)(ht,&key);}
CH_API_DEF size_t CH_HASHTABLE_TYPE_FCT(_get_num_items)(const CH_HASHTABLE_TYPE* ht) {
    size_t i,sum=0;CH_ASSERT(ht);
    for (i=0;i<CH_NUM_BUCKETS;i++) sum+=ht->buckets[i].size;
    return sum;
}
CH_API_DEF int CH_HASHTABLE_TYPE_FCT(_dbg_check)(const CH_HASHTABLE_TYPE* ht) {
    size_t i,j,num_total_items=0,num_total_capacity=0,num_sorting_errors=0,min_num_bucket_items=(size_t)-1,max_num_bucket_items=0,min_cnt=0,max_cnt=0,avg_cnt=0,avg_round=0;
    double avg_num_bucket_items=0.0,std_deviation=0.0;
    size_t mem_minimal=sizeof(CH_HASHTABLE_TYPE),mem_used=sizeof(CH_HASHTABLE_TYPE);
    double mem_used_percentage = 100;
    const CH_HASHTABLE_ITEM_TYPE* last_item = NULL;
    CH_ASSERT(ht);
    for (i=0;i<CH_NUM_BUCKETS;i++) {
        const CH_VECTOR_TYPE* bck = &ht->buckets[i];
        num_total_items+=bck->size;
        num_total_capacity+=bck->capacity;
        if (min_num_bucket_items>bck->size) min_num_bucket_items=bck->size;
        if (max_num_bucket_items<bck->size) max_num_bucket_items=bck->size;
        if (bck->v) {
            mem_minimal += sizeof(CH_HASHTABLE_ITEM_TYPE)*bck->size;
            mem_used += sizeof(CH_HASHTABLE_ITEM_TYPE)*bck->capacity;
            if (ht->key_cmp && bck->size)    {
                last_item = NULL;
                for (j=0;j<bck->size;j++)  {
                    const CH_HASHTABLE_ITEM_TYPE* item = &bck->v[j];
                    if (last_item) {
                        if (ht->key_cmp(&last_item->k,&item->k)>0) {
                            /* When this happens, it can be a wrong user 'key_cmp' function (that cannot sort keys in a consistent way) */
                            ++num_sorting_errors;
#                       ifndef CH_NO_STDIO
                            fprintf(stderr,"[%s] Sorting Error (%" CV_SIZE_T_FORMATTING "): in bucket[%" CV_SIZE_T_FORMATTING "]: key_cmp(%" CV_SIZE_T_FORMATTING ",%" CV_SIZE_T_FORMATTING ")>0 [num_items=%" CV_SIZE_T_FORMATTING " (in bucket)]\n",CH_XSTR(CH_HASHTABLE_TYPE_FCT(_dbg_check)),num_sorting_errors,i,j-1,j,bck->size);
#                       endif
                        }
                    }
                    last_item=item;
                }
            }
        }
    }
    avg_num_bucket_items = (double)num_total_items/(double) CH_NUM_BUCKETS;
    mem_used_percentage = (double)mem_used*100.0/(double)mem_minimal;
    if (CH_NUM_BUCKETS<2) {std_deviation=0.;avg_cnt=min_cnt=max_cnt=1;avg_round=(min_num_bucket_items+max_num_bucket_items)/2;}
    else {
        const double dec = avg_num_bucket_items-(double)((size_t)avg_num_bucket_items); /* in (0,1] */
        avg_round = (size_t)avg_num_bucket_items;
        if (dec>=0.5) avg_round+=1;
        for (i=0;i<CH_NUM_BUCKETS;i++) {
            const CH_VECTOR_TYPE* bck = &ht->buckets[i];
            double tmp = bck->size-avg_num_bucket_items;
            std_deviation+=tmp*tmp;
            if (bck->size==min_num_bucket_items) ++min_cnt;
            if (bck->size==max_num_bucket_items) ++max_cnt;
            if (bck->size==avg_round) ++avg_cnt;
        }
        std_deviation/=(double)(CH_NUM_BUCKETS-1); /* this is the variance */
        /* we must calculate its square root now without depending on <math.h>. Code based on:
           https://stackoverflow.com/questions/29018864/any-way-to-obtain-square-root-of-a-number-without-using-math-h-and-sqrt
        */
        {
        /*#define SQRT_MINDIFF 2.2250738585072014e-308    smallest positive double*/
#       define SQRT_MINDIFF 2.25e-308                   /* use for convergence check */
        double root=std_deviation/3, last, diff=1;
        if (std_deviation > 0) {
            do {
                last = root;
                root = (root + std_deviation / root) / 2;
                diff = root - last;
            } while (diff > SQRT_MINDIFF || diff < -SQRT_MINDIFF);
            std_deviation = root;
        }
#       undef SQRT_MINDIFF
        }
    }
#   ifndef CH_NO_STDIO    
    printf("[%s]:\n",CH_XSTR(CH_HASHTABLE_TYPE_FCT(_dbg_check)));
    printf("\tnum_total_items=%" CV_SIZE_T_FORMATTING " (num_total_capacity=%" CV_SIZE_T_FORMATTING ") in %d buckets [items per bucket: mean=%1.3f std_deviation=%1.3f min=%" CV_SIZE_T_FORMATTING " (in %" CV_SIZE_T_FORMATTING "/%d) avg=%" CV_SIZE_T_FORMATTING " (in %" CV_SIZE_T_FORMATTING "/%d) max=%" CV_SIZE_T_FORMATTING " (in %" CV_SIZE_T_FORMATTING "/%d)].\n",num_total_items,num_total_capacity,CH_NUM_BUCKETS,avg_num_bucket_items,std_deviation,min_num_bucket_items,min_cnt,CH_NUM_BUCKETS,avg_round,avg_cnt,CH_NUM_BUCKETS,max_num_bucket_items,max_cnt,CH_NUM_BUCKETS);
    printf("\tmemory_used: ");ch_display_bytes(mem_used);
    printf(". memory_minimal_possible: ");ch_display_bytes(mem_minimal);
    printf(". mem_used_percentage: %1.2f%% (100%% is the best possible result).\n",mem_used_percentage);
#   endif
    CH_ASSERT(num_sorting_errors==0); /* When this happens, it can be a wrong user 'itemKey_cmp' function (that cannot sort keys in a consistent way) */
    return (int) num_total_items;
}

CH_API_DEF void CH_HASHTABLE_TYPE_FCT(_swap)(CH_HASHTABLE_TYPE* a,CH_HASHTABLE_TYPE* b)  {
    unsigned char t[sizeof(CH_HASHTABLE_TYPE)];
    if (a!=b)   {
        CH_ASSERT(a && b);
        memcpy(&t,a,sizeof(CH_HASHTABLE_TYPE));
        memcpy(a,b,sizeof(CH_HASHTABLE_TYPE));
        memcpy(b,&t,sizeof(CH_HASHTABLE_TYPE));
    }
}
CH_API_DEF void CH_HASHTABLE_TYPE_FCT(_cpy)(CH_HASHTABLE_TYPE* a,const CH_HASHTABLE_TYPE* b) {
    size_t i;
    typedef ch_hash_uint (*key_hash_type)(const CH_KEY_TYPE*);
    typedef int (*key_cmp_type)(const CH_CMP_TYPE*,const CH_CMP_TYPE*);
    typedef void (*key_ctr_dtr_type)(CH_KEY_TYPE*);
    typedef void (*key_cpy_type)(CH_KEY_TYPE*,const CH_KEY_TYPE*);
    typedef void (*value_ctr_dtr_type)(CH_VALUE_TYPE*);
    typedef void (*value_cpy_type)(CH_VALUE_TYPE*,const CH_VALUE_TYPE*);
    if (a==b) return;
    CH_ASSERT(a && b);
    *((key_hash_type*)&a->key_hash) = b->key_hash;
    *((key_cmp_type*)&a->key_cmp) = b->key_cmp;
    *((key_ctr_dtr_type*)&a->key_ctr) = b->key_ctr;
    *((key_ctr_dtr_type*)&a->key_dtr) = b->key_dtr;
    *((key_cpy_type*)&a->key_cpy) = b->key_cpy;
    *((value_ctr_dtr_type*)&a->value_ctr) = b->value_ctr;
    *((value_ctr_dtr_type*)&a->value_dtr) = b->value_dtr;
    *((value_cpy_type*)&a->value_cpy) = b->value_cpy;
    *((size_t*)&a->initial_bucket_capacity) = b->initial_bucket_capacity;
    for (i=0;i<CH_NUM_BUCKETS;i++)  {
        CH_VECTOR_TYPE* A = &a->buckets[i];
        const CH_VECTOR_TYPE* B = &b->buckets[i];
        /* bad init asserts */
        CH_ASSERT(!(A->v && A->capacity==0));
        CH_ASSERT(!(!A->v && A->capacity>0));
        CH_ASSERT(!(B->v && B->capacity==0));
        CH_ASSERT(!(!B->v && B->capacity>0));
        /*CH_VECTOR_TYPE_FCT(_free)(A,a);*/
        CH_VECTOR_TYPE_FCT(_clear)(A,a);
        CH_VECTOR_TYPE_FCT(_resize)(A,B->size,a);
        CH_ASSERT(A->size==B->size);
        if (!a->key_cpy && !a->value_cpy)   {memcpy(&A->v[0],&B->v[0],A->size*sizeof(CH_HASHTABLE_ITEM_TYPE));}
        else if(a->key_cpy)     {for (i=0;i<A->size;i++) {a->key_cpy(&A->v[i].k,&B->v[i].k);memcpy(&A->v[i].v,&B->v[i].v,sizeof(CH_VALUE_TYPE));}}
        else if(a->value_cpy)   {for (i=0;i<A->size;i++) {memcpy(&A->v[i].k,&B->v[i].k,sizeof(CH_KEY_TYPE));a->value_cpy(&A->v[i].v,&B->v[i].v);}}
    }
}
CH_API_DEF void CH_HASHTABLE_TYPE_FCT(_shrink_to_fit)(CH_HASHTABLE_TYPE* ht)   {
    if (ht)	{
        CH_HASHTABLE_TYPE o;
        memset(&o,0,sizeof(CH_HASHTABLE_TYPE));
        CH_HASHTABLE_TYPE_FCT(_cpy)(&o,ht); /* now 'o' is 'v' trimmed */
        CH_HASHTABLE_TYPE_FCT(_free)(ht);
        CH_HASHTABLE_TYPE_FCT(_swap)(&o,ht);
    }
}

CH_API_DEF void CH_HASHTABLE_TYPE_FCT(_create_with)(
        CH_HASHTABLE_TYPE* ht,
        ch_hash_uint (*key_hash)(const CH_KEY_TYPE*),
        int (*key_cmp) (const CH_CMP_TYPE*,const CH_CMP_TYPE*),
        void (*key_ctr)(CH_KEY_TYPE*),void (*key_dtr)(CH_KEY_TYPE*),void (*key_cpy)(CH_KEY_TYPE*,const CH_KEY_TYPE*),
        void (*value_ctr)(CH_VALUE_TYPE*),void (*value_dtr)(CH_VALUE_TYPE*),void (*value_cpy)(CH_VALUE_TYPE*,const CH_VALUE_TYPE*),
        size_t initial_bucket_capacity)   {
    typedef ch_hash_uint (*key_hash_type)(const CH_KEY_TYPE*);
    typedef int (*key_cmp_type)(const CH_CMP_TYPE*,const CH_CMP_TYPE*);
    typedef void (*key_ctr_dtr_type)(CH_KEY_TYPE*);
    typedef void (*key_cpy_type)(CH_KEY_TYPE*,const CH_KEY_TYPE*);
    typedef void (*value_ctr_dtr_type)(CH_VALUE_TYPE*);
    typedef void (*value_cpy_type)(CH_VALUE_TYPE*,const CH_VALUE_TYPE*);
#   ifndef CH_DISABLE_FAKE_MEMBER_FUNCTIONS
    typedef void (* clear_free_shrink_to_fit_mf)(CH_HASHTABLE_TYPE*);
    typedef CH_VALUE_TYPE* (* get_or_insert_mf)(CH_HASHTABLE_TYPE*,const CH_KEY_TYPE*,int*);
    typedef CH_VALUE_TYPE* (* get_or_insert_by_val_mf)(CH_HASHTABLE_TYPE*,const CH_KEY_TYPE,int*);
    typedef CH_VALUE_TYPE* (* get_mf)(CH_HASHTABLE_TYPE*,const CH_KEY_TYPE*);
    typedef CH_VALUE_TYPE* (* get_by_val_mf)(CH_HASHTABLE_TYPE*,const CH_KEY_TYPE);
    typedef const CH_VALUE_TYPE* (* get_const_mf)(const CH_HASHTABLE_TYPE*,const CH_KEY_TYPE*);
    typedef const CH_VALUE_TYPE* (* get_const_by_val_mf)(const CH_HASHTABLE_TYPE*,const CH_KEY_TYPE);
    typedef int (* remove_mf)(CH_HASHTABLE_TYPE*,const CH_KEY_TYPE*);
    typedef int (* remove_by_val_mf)(CH_HASHTABLE_TYPE*,const CH_KEY_TYPE);
    typedef size_t (* get_num_items_mf)(const CH_HASHTABLE_TYPE* ht);
    typedef int (* dbg_check_mf)(const CH_HASHTABLE_TYPE*);
    typedef void (* swap_mf)(CH_HASHTABLE_TYPE* a,CH_HASHTABLE_TYPE* b);
    typedef void (* cpy_mf)(CH_HASHTABLE_TYPE* a,const CH_HASHTABLE_TYPE* b);
#   endif
    CH_ASSERT(ht);
    memset(ht,0,sizeof(CH_HASHTABLE_TYPE));
    *((key_hash_type*)&ht->key_hash) = key_hash;
    *((key_cmp_type*)&ht->key_cmp) = key_cmp;
    *((key_ctr_dtr_type*)&ht->key_ctr) = key_ctr;
    *((key_ctr_dtr_type*)&ht->key_dtr) = key_dtr;
    *((key_cpy_type*)&ht->key_cpy) = key_cpy;
    *((value_ctr_dtr_type*)&ht->value_ctr) = value_ctr;
    *((value_ctr_dtr_type*)&ht->value_dtr) = value_dtr;
    *((value_cpy_type*)&ht->value_cpy) = value_cpy;
    *((size_t*)&ht->initial_bucket_capacity) = initial_bucket_capacity>1 ? initial_bucket_capacity : 1;
    CH_ASSERT(ht->key_hash);
    /*memset(ht->buckets,0,CH_NUM_BUCKETS*sizeof(CH_VECTOR_TYPE));*/
#   ifndef CH_DISABLE_FAKE_MEMBER_FUNCTIONS
    *((clear_free_shrink_to_fit_mf*)&ht->clear) = &CH_HASHTABLE_TYPE_FCT(_clear);
    *((clear_free_shrink_to_fit_mf*)&ht->free) = &CH_HASHTABLE_TYPE_FCT(_free);
    *((clear_free_shrink_to_fit_mf*)&ht->shrink_to_fit) = &CH_HASHTABLE_TYPE_FCT(_shrink_to_fit);
    *((get_or_insert_mf*)&ht->get_or_insert) = &CH_HASHTABLE_TYPE_FCT(_get_or_insert);
    *((get_or_insert_by_val_mf*)&ht->get_or_insert_by_val) = &CH_HASHTABLE_TYPE_FCT(_get_or_insert_by_val);
    *((get_mf*)&ht->get) = &CH_HASHTABLE_TYPE_FCT(_get);
    *((get_by_val_mf*)&ht->get_by_val) = &CH_HASHTABLE_TYPE_FCT(_get_by_val);
    *((get_const_mf*)&ht->get_const) = &CH_HASHTABLE_TYPE_FCT(_get_const);
    *((get_const_by_val_mf*)&ht->get_const_by_val) = &CH_HASHTABLE_TYPE_FCT(_get_const_by_val);
    *((remove_mf*)&ht->remove) = &CH_HASHTABLE_TYPE_FCT(_remove);
    *((remove_by_val_mf*)&ht->remove_by_val) = &CH_HASHTABLE_TYPE_FCT(_remove_by_val);
    *((get_num_items_mf*)&ht->get_num_items) = &CH_HASHTABLE_TYPE_FCT(_get_num_items);
    *((dbg_check_mf*)&ht->dbg_check) = &CH_HASHTABLE_TYPE_FCT(_dbg_check);
    *((swap_mf*)&ht->swap) = &CH_HASHTABLE_TYPE_FCT(_swap);
    *((cpy_mf*)&ht->cpy) = &CH_HASHTABLE_TYPE_FCT(_cpy);
#   endif
}
CH_API_DEF void CH_HASHTABLE_TYPE_FCT(_create)(CH_HASHTABLE_TYPE* ht,ch_hash_uint (*key_hash)(const CH_KEY_TYPE*),int (*key_cmp) (const CH_CMP_TYPE*,const CH_CMP_TYPE*),size_t initial_bucket_capacity)    {
    CH_HASHTABLE_TYPE_FCT(_create_with)(ht,key_hash,key_cmp,NULL,NULL,NULL,NULL,NULL,NULL,initial_bucket_capacity);
}

#ifdef __cplusplus
    CH_HASHTABLE_TYPE::CH_HASHTABLE_TYPE() :
        key_ctr(NULL),key_dtr(NULL),key_cpy(NULL),key_cmp(NULL),key_hash(NULL),
        value_ctr(NULL),value_dtr(NULL),value_cpy(NULL),
        initial_bucket_capacity(0),
        clear(&CH_HASHTABLE_TYPE_FCT(_clear)),free(&CH_HASHTABLE_TYPE_FCT(_free)),shrink_to_fit(&CH_HASHTABLE_TYPE_FCT(_shrink_to_fit)),
        get_or_insert(&CH_HASHTABLE_TYPE_FCT(_get_or_insert)),get_or_insert_by_val(&CH_HASHTABLE_TYPE_FCT(_get_or_insert_by_val)),
        get(&CH_HASHTABLE_TYPE_FCT(_get)),get_by_val(&CH_HASHTABLE_TYPE_FCT(_get_by_val)),
        get_const(&CH_HASHTABLE_TYPE_FCT(_get_const)),get_const_by_val(&CH_HASHTABLE_TYPE_FCT(_get_const_by_val)),
        remove(&CH_HASHTABLE_TYPE_FCT(_remove)),remove_by_val(&CH_HASHTABLE_TYPE_FCT(_remove_by_val)),
        get_num_items(&CH_HASHTABLE_TYPE_FCT(_get_num_items)),
        dbg_check(&CH_HASHTABLE_TYPE_FCT(_dbg_check)),
        swap(&CH_HASHTABLE_TYPE_FCT(_swap)),cpy(&CH_HASHTABLE_TYPE_FCT(_cpy))
    {}
    CH_HASHTABLE_TYPE::CH_HASHTABLE_TYPE(const CH_HASHTABLE_TYPE& o) :
        key_ctr(o.key_ctr),key_dtr(o.key_dtr),key_cpy(o.key_cpy),key_cmp(o.key_cmp),key_hash(o.key_hash),
        value_ctr(o.value_ctr),value_dtr(o.value_dtr),value_cpy(o.value_cpy),initial_bucket_capacity(o.initial_bucket_capacity),
        clear(&CH_HASHTABLE_TYPE_FCT(_clear)),free(&CH_HASHTABLE_TYPE_FCT(_free)),shrink_to_fit(&CH_HASHTABLE_TYPE_FCT(_shrink_to_fit)),
        get_or_insert(&CH_HASHTABLE_TYPE_FCT(_get_or_insert)),get_or_insert_by_val(&CH_HASHTABLE_TYPE_FCT(_get_or_insert_by_val)),
        get(&CH_HASHTABLE_TYPE_FCT(_get)),get_by_val(&CH_HASHTABLE_TYPE_FCT(_get_by_val)),
        get_const(&CH_HASHTABLE_TYPE_FCT(_get_const)),get_const_by_val(&CH_HASHTABLE_TYPE_FCT(_get_const_by_val)),
        remove(&CH_HASHTABLE_TYPE_FCT(_remove)),remove_by_val(&CH_HASHTABLE_TYPE_FCT(_remove_by_val)),
        get_num_items(&CH_HASHTABLE_TYPE_FCT(_get_num_items)),
        dbg_check(&CH_HASHTABLE_TYPE_FCT(_dbg_check)),
        swap(&CH_HASHTABLE_TYPE_FCT(_swap)),cpy(&CH_HASHTABLE_TYPE_FCT(_cpy))
    {
        CH_HASHTABLE_TYPE_FCT(_cpy)(this,&o);
    }

    CH_HASHTABLE_TYPE& CH_HASHTABLE_TYPE::operator=(const CH_HASHTABLE_TYPE& o)    {CH_HASHTABLE_TYPE_FCT(_cpy)(this,&o);return *this;}

#   ifdef CH_HAS_MOVE_SEMANTICS
    CH_HASHTABLE_TYPE::CH_HASHTABLE_TYPE(CH_HASHTABLE_TYPE&& o) :
        key_ctr(o.key_ctr),key_dtr(o.key_dtr),key_cpy(o.key_cpy),key_cmp(o.key_cmp),key_hash(o.key_hash),
        value_ctr(o.value_ctr),value_dtr(o.value_dtr),value_cpy(o.value_cpy),initial_bucket_capacity(o.initial_bucket_capacity),
        clear(&CH_HASHTABLE_TYPE_FCT(_clear)),free(&CH_HASHTABLE_TYPE_FCT(_free)),shrink_to_fit(&CH_HASHTABLE_TYPE_FCT(_shrink_to_fit)),
        get_or_insert(&CH_HASHTABLE_TYPE_FCT(_get_or_insert)),get_or_insert_by_val(&CH_HASHTABLE_TYPE_FCT(_get_or_insert_by_val)),
        get(&CH_HASHTABLE_TYPE_FCT(_get)),get_by_val(&CH_HASHTABLE_TYPE_FCT(_get_by_val)),
        get_const(&CH_HASHTABLE_TYPE_FCT(_get_const)),get_const_by_val(&CH_HASHTABLE_TYPE_FCT(_get_const_by_val)),
        remove(&CH_HASHTABLE_TYPE_FCT(_remove)),remove_by_val(&CH_HASHTABLE_TYPE_FCT(_remove_by_val)),
        get_num_items(&CH_HASHTABLE_TYPE_FCT(_get_num_items)),
        dbg_check(&CH_HASHTABLE_TYPE_FCT(_dbg_check)),
        swap(&CH_HASHTABLE_TYPE_FCT(_swap)),cpy(&CH_HASHTABLE_TYPE_FCT(_cpy))
    {
        size_t i;
        for (i=0;i<CH_NUM_BUCKETS;i++) {
            CH_VECTOR_TYPE& d = buckets[i];
            CH_VECTOR_TYPE& s = o.buckets[i];
            d.v=s.v;
            *((size_t*)&d.size)=s.size;*((size_t*)&d.capacity)=s.capacity;
            s.v=NULL;
            *((size_t*)&s.size)=0;*((size_t*)&s.capacity)=0;
        }
    }

    CH_HASHTABLE_TYPE& CH_HASHTABLE_TYPE::operator=(CH_HASHTABLE_TYPE&& o)    {
        if (this != &o) {
            size_t i;
            CH_HASHTABLE_TYPE_FCT(_free)(this);
            for (i=0;i<CH_NUM_BUCKETS;i++) {
                CH_VECTOR_TYPE& d = buckets[i];
                CH_VECTOR_TYPE& s = o.buckets[i];
                d.v=s.v;
                *((size_t*)&d.size)=s.size;*((size_t*)&d.capacity)=s.capacity;
                s.v=NULL;
                *((size_t*)&s.size)=0;*((size_t*)&s.capacity)=0;
            }
        }
        return *this;
    }
#   endif

    CH_HASHTABLE_TYPE::~CH_HASHTABLE_TYPE() {CH_HASHTABLE_TYPE_FCT(_free)(this);}
#endif

#endif /* (!defined(CH_ENABLE_DECLARATION_AND_DEFINITION) || defined(C_HASHTABLE_IMPLEMENTATION)) */


#undef CH_HASHTABLE_TYPE_FCT
#undef CH_KEY_TYPE_FCT
#undef CH_VECTOR_TYPE_FCT
#undef CH_CMP_TYPE
#undef CH_USE_VOID_PTRS_IN_CMP_FCT
#undef CH_HASHTABLE_TYPE
#undef CH_VECTOR_TYPE
#undef CH_VECTORS_TYPE
#undef CHV_KEY_SUPPORT_EQUALITY_CMP_IN_UNSORTED_SEARCH
#undef CH_HASHTABLE_ITEM_TYPE_TMP
#undef CH_HASHTABLE_ITEM_TYPE
#undef CH_LAST_INCLUDED_NUM_BUCKETS
#define CH_LAST_INCLUDED_NUM_BUCKETS CH_NUM_BUCKETS
#undef CH_NUM_BUCKETS
#undef C_HASHTABLE_FORCE_DECLARATION
#undef C_HASHTABLE_IMPLEMENTATION
/* ------------------------------------------------- */
#undef CH_KEY_TYPE
#undef CH_VALUE_TYPE

