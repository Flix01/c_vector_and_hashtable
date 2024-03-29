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

/*
   GLOBAL DEFINITIONS: The following definitions (when used) must be set
   globally (= in the Project Options or in a StdAfx.h file):

   CH_NUM_USED_BUCKETS                  // in [1,2147483648] (defaults to 256). USING POWERS OF TWO ALLOW AVOIDING MODULO IN YOUR HASH FUNCTION (i.e. hash % CH_NUM_USED_BUCKETS) BY USING										
										//		'xor-folding' or 'fibonacci-folding' (please search this file for: 'xor-folding' or 'fibonacci-folding' for further info)
   CH_MAX_POSSIBLE_NUM_BUCKETS          // [READ-ONLY definition] It can be 256, 65536 or 2147483648 and defines 'chtu_hash_uint' as 'unsigned char', 'unsigned short' or 'unsigned int'
                                        // (while it could seem redundant, it is handy to use it inside hash functions to see if we need a mod (%CH_NUM_USED_BUCKETS) or not)
   CH_DISABLE_FAKE_MEMBER_FUNCTIONS     // faster with this defined
   CH_DISABLE_CLEARING_ITEM_MEMORY      // faster with this defined
   CH_ENABLE_DECLARATION_AND_DEFINITION // when used, C_HASHTABLE_TYPE_UNSAFE_IMPLEMENTATION must be
                                        // defined before including this file in a single source (.c) file
   CH_NO_PLACEMENT_NEW                  // (c++ mode only) it does not define (unused) helper stuff like: CH_PLACEMENT_NEW, cpp_ctr_tu,cpp_dtr_tu,cpp_cpy_tu,cpp_cmp_tu

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

#ifndef C_HASHTABLE_TYPE_UNSAFE_H
#define C_HASHTABLE_TYPE_UNSAFE_H

#define C_HASHTABLE_TYPE_UNSAFE_VERSION         "1.06 rev2"
#define C_HASHTABLE_TYPE_UNSAFE_VERSION_NUM     0106

/* HISTORY:
   C_HASHTABLE_TYPE_UNSAFE_VERSION_NUM 106 rev2
   -> added the CV_SIZE_T_FORMATTING definition (internal usage).

   C_HASHTABLE_TYPE_UNSAFE_VERSION_NUM 106
   -> Added code to silence some compiler warnings (see: COMPILER_SUPPORTS_GCC_DIAGNOSTIC definition)
	
   C_HASHTABLE_TYPE_UNSAFE_VERSION_NUM 105
   -> Renamed 'ch_hash_murmur3(...)' -> 'ch_hash32_murmur3(...)'
   -> Added the helper functions 'ch_hash32_murmur3_str(...)', 'ch_hash32_FNV1a(...)', 'ch_hash32_FNV1a_str(...)'

   C_HASHTABLE_TYPE_UNSAFE_VERSION_NUM 104
   -> Fixed wrong detection of sorting errors in 'chashtable_dbg_check(...)'

   C_HASHTABLE_TYPE_UNSAFE_VERSION_NUM  103
   -> renamed 'ch_hash_uint' to 'chtu_hash_uint' (where 'tu' stands for 'type unsafe'). This is necessary
      to avoid a possible conflicting declaration of 'ch_hash_uint' in same configurations where
      chashtable.h and chashtable_type_unsafe.h are used together

   C_HASHTABLE_TYPE_UNSAFE_VERSION_NUM  102
   -> added c++ copy ctr, copy assignment and dtr (and move ctr plus move assignment if CH_HAS_MOVE_SEMANTICS is defined),
      to ease porting code from c++ to c a bit more (but 'chashtable_create(...)' is still necessary in c++ mode)
   -> removed internal definitions CH_EXTERN_C_START, CH_EXTERN_C_END, CH_DEFAULT_STRUCT_INIT
      Now code is no more 'extern C'
   -> added optional stuff for c++ compilation: CH_PLACEMENT_NEW,cpp_ctr_tu,cpp_dtr_tu,cpp_cpy_tu,cpp_cmp_tu

   C_HASHTABLE_TYPE_UNSAFE_VERSION_NUM  101
   -> renamed CH_VERSION to C_HASHTABLE_TYPE_UNSAFE_VERSION
   -> renamed CH_VERSION_NUM to C_HASHTABLE_TYPE_UNSAFE_VERSION_NUM
   -> some internal changes to minimize interference with (the type-safe version) "c_hashtable.h",
      hoping that they can both cohexist in the same project.
   -> renamed CH_NUM_BUCKETS to CH_NUM_USED_BUCKETS
   -> renamed CH_MAX_NUM_BUCKETS to CH_MAX_POSSIBLE_NUM_BUCKETS [and now it's READ-ONLY!]
   -> removed some commented out code
   -> made some changes to allow compilation in c++ mode
   -> added a hashtable ctr to ease compilation in c++ mode
*/

#ifndef COMPILER_SUPPORTS_GCC_DIAGNOSTIC    // We define this
#   if (defined(__GNUC__) || defined(__MINGW__) || defined(__clang__))
#       define COMPILER_SUPPORTS_GCC_DIAGNOSTIC
#   endif
#endif

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
#   if (defined(CH_PLACEMENT_NEW) && !defined(CVH_CPP_TU_GUARD))
#   define CVH_CPP_TU_GUARD
    template<typename T> inline void cpp_ctr_tu(void* vv) {T* v=(T*) vv;CH_PLACEMENT_NEW(v) T();}
    template<typename T> inline void cpp_dtr_tu(void* vv) {T* v=(T*) vv;v->T::~T();}
    template<typename T> inline void cpp_cpy_tu(void* av,const void* bv) {T* a=(T*) av;const T* b=(const T*) bv;*a=*b;}
    template<typename T> inline int cpp_cmp_tu(const void* av,const void* bv) {const T* a=(const T*) av;const T* b=(const T*) bv;return (*a)<(*b)?-1:((*a)>(*b)?1:0);}
#   endif
#endif


#ifndef CH_COMMON_FUNCTIONS_GUARD
#define CH_COMMON_FUNCTIONS_GUARD
/* base memory helpers */
CH_API void* ch_malloc(size_t size) {
    void* p = CH_MALLOC(size);
    if (!p)	{
        CH_ASSERT(0);	/* No more memory error */
#       ifndef CH_NO_STDIO
        fprintf(stderr,"CH_ERROR: cvh_malloc(...) failed. Not enough memory.\n");
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
        fprintf(stderr,"CH_ERROR: cvh_safe_realloc(...) failed. Not enough memory.\n");
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
   Warning: it returns unsigned int (32-bit) and works for little-endian CPUs only
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
	
	All the helper hash functions above return 32-bit unsigned integers: this type does not always match 'chtu_hash_uint',
	that is required by our signature. To convert hash32 (the 32-bit value returned by these helper hash functions) to 'chtu_hash_uint':

	->	The type match when CH_NUM_USED_BUCKETS==2147483648 and: 'chtu_hash_uint' == unsigned
		chtu_hash_uint hash == hash32; // cast to 'chtu_hash_uint' is not necessary

	-> Otherwise we can use the 32-bit hash functions and then fold values in our range.
		To do so:
		-> if CH_NUM_USED_BUCKETS is not a power of two we must use mod:
			chtu_hash_uint hash = (chtu_hash_uint)(hash32%CH_NUM_USED_BUCKETS);	// a bit slow
		-> otherwise CH_NUM_USED_BUCKETS == 2^X == pow(2,X). In this case we can use mod too, but also:
			-> 'xor-folding' (see http://www.isthe.com/chongo/tech/comp/fnv/index.html):
				chtu_hash_uint hashX = (chtu_hash_uint) ((hash32>>X) ^ (hash32 & (((unsigned)1<<(x))-1));
			-> 'fibonacci-folding' (see https://programmingpraxis.com/2018/06/19/fibonacci-hash):
				chtu_hash_uint hashX = (chtu_hash_uint) ((hash32*2654435769U) >> (32-X));

	-> In the default case: CH_NUM_USED_BUCKETS == 256 (== CH_MAX_POSSIBLE_NUM_BUCKETS): we could do nothing and just cast 
	   to 'chtu_hash_uint', but since unsigned char is not a 32-bit value, we can do better by using a folding technique 
       and see if it works better:
		-> chtu_hash_uint hash8 = (chtu_hash_uint) ((hash32>>8) ^ (hash32 & ((unsigned)0xFF)));		// 'xor-folding' or
		-> chtu_hash_uint hash8 = (chtu_hash_uint) ((hash32*2654435769U) >> 24);					// 'fibonacci-folding'
*/
/* if CH_NUM_USED_BUCKETS == pow(2,num_buckets_pot_exponent), with num_buckets_pot_exponent<32, then we can 'convert' 32-bit hash values this way: */
#	define CH_HASH_FROM_HASH32_USING_XORFOLDING(hash32,num_buckets_pot_exponent)	( (hash32>>(num_buckets_pot_exponent)) ^ ( hash32 & ((1U<<(num_buckets_pot_exponent))-1) ) )
#	define CH_HASH_FROM_HASH32_USING_FIBFOLDING(hash32,num_buckets_pot_exponent)	((hash32*2654435769U) >> (32-(num_buckets_pot_exponent)))
#endif /* CH_COMMON_FUNCTIONS_GUARD */

#ifndef CH_NUM_USED_BUCKETS
#   define CH_NUM_USED_BUCKETS 256
#endif
#if CH_NUM_USED_BUCKETS<0
#   undef CH_NUM_USED_BUCKETS
#   define CH_NUM_USED_BUCKETS 256
#elif CH_NUM_USED_BUCKETS>2147483648
#   undef CH_NUM_USED_BUCKETS
#   define CH_NUM_USED_BUCKETS 2147483648
#endif

/* Here we set CH_MAX_POSSIBLE_NUM_BUCKETS. It can be only be 256, 65536 or 2147483648 and determinates 'chtu_hash_uint' */
#undef CH_MAX_POSSIBLE_NUM_BUCKETS
#if CH_NUM_USED_BUCKETS<=256
#   define CH_MAX_POSSIBLE_NUM_BUCKETS 256
    typedef unsigned char chtu_hash_uint;
#elif CH_NUM_USED_BUCKETS<=65536
#   define CH_MAX_POSSIBLE_NUM_BUCKETS 65536
    typedef unsigned short chtu_hash_uint;
#elif CH_NUM_USED_BUCKETS<=2147483648
#   define CH_MAX_POSSIBLE_NUM_BUCKETS 2147483648
    typedef unsigned int chtu_hash_uint;
#   else
#   error CH_NUM_USED_BUCKETS cannot be bigger than 2147483648.
#   endif


typedef struct chashtable chashtable;
struct chashtable {
	const size_t key_size_in_bytes;
	const size_t value_size_in_bytes;

    /* key callbacks */
    void (*const key_ctr)(void*);                     /* optional (can be NULL) */
    void (*const key_dtr)(void*);                     /* optional (can be NULL) */
    void (*const key_cpy)(void*,const void*);		/* optional (can be NULL) */
    int (*const key_cmp)(const void*,const void*);/* optional (can be NULL) (for sorted vectors only) */
    chtu_hash_uint (*const key_hash)(const void*);
    /* value callbacks */
    void (*const value_ctr)(void*);                     /* optional (can be NULL) */
    void (*const value_dtr)(void*);                     /* optional (can be NULL) */
    void (*const value_cpy)(void*,const void*);/* optional (can be NULL) */

    /* CH_NUM_USED_BUCKETS sorted buckets */
    const size_t initial_bucket_capacity;
    struct chvector {
		void* k;
		void* v;
		const size_t size;
        const size_t capacity;
#       ifdef __cplusplus
        chvector() : k(NULL),v(NULL),size(0),capacity(0) {}
        template<typename K> inline const K& key_at(size_t i) const {CH_ASSERT(i<size);const K* p=(const K*) k;return p[i];}
        template<typename V> inline V& value_at(size_t i) {CH_ASSERT(i<size);V* p=(V*) v;return p[i];}
        template<typename V> inline const V& value_at(size_t i) const {CH_ASSERT(i<size);const V* p=(const V*) v;return p[i];}
#       endif
    } buckets[CH_NUM_USED_BUCKETS];

#   ifndef CH_DISABLE_FAKE_MEMBER_FUNCTIONS  /* must be defined glabally (in the Project Options)) */
    void (* const clear)(chashtable* ht);
    void (* const free)(chashtable* ht);
    void (* const shrink_to_fit)(chashtable* ht);
    void* (* const get_or_insert)(chashtable* ht,const void* key,int* match);
    void* (* const get)(chashtable* ht,const void* key);
    const void* (* const get_const)(const chashtable* ht,const void* key);
    int (* const remove)(chashtable* ht,const void* key);
    size_t (* const get_num_items)(const chashtable* ht);
    int (* const dbg_check)(const chashtable* ht);
    void (* const swap)(chashtable* a,chashtable* b);
    void (* const cpy)(chashtable* a,const chashtable* b);
#   endif

#   ifdef __cplusplus
    chashtable();
    chashtable(const chashtable& o);
    chashtable& operator=(const chashtable& o);

#   ifdef CH_HAS_MOVE_SEMANTICS
    chashtable(chashtable&& o);
    chashtable& operator=(chashtable&& o);
#   endif
    ~chashtable();
#   endif

};
#ifdef __cplusplus
typedef chashtable::chvector chvector;
#else
typedef struct chvector chvector;
#endif
typedef chvector chvectors[CH_NUM_USED_BUCKETS];


#ifdef CH_ENABLE_DECLARATION_AND_DEFINITION
/* function declarations */
CH_API_DEC void chashtable_free(chashtable* ht);
CH_API_DEC void chashtable_clear(chashtable* ht);
CH_API_DEC void* chashtable_get_or_insert(chashtable* ht,const void* key,int* match);
CH_API_DEC void* chashtable_get(chashtable* ht,const void* key);
CH_API_DEC const void* chashtable_get_const(const chashtable* ht,const void* key);
CH_API_DEC int chashtable_remove(chashtable* ht,const void* key);
CH_API_DEC size_t chashtable_get_num_items(const chashtable* ht);
CH_API_DEC int chashtable_dbg_check(const chashtable* ht);
CH_API_DEC void chashtable_swap(chashtable* a,chashtable* b);
CH_API_DEC void chashtable_cpy(chashtable* a,const chashtable* b);
CH_API_DEC void chashtable_shrink_to_fit(chashtable* ht);
CH_API_DEC void chashtable_create_with(
        chashtable* ht,
		size_t key_size_in_bytes,size_t value_size_in_bytes,
        chtu_hash_uint (*key_hash)(const void*),
        int (*key_cmp) (const void*,const void*),
        void (*key_ctr)(void*),void (*key_dtr)(void*),void (*key_cpy)(void*,const void*),
        void (*value_ctr)(void*),void (*value_dtr)(void*),void (*value_cpy)(void*,const void*),
        size_t initial_bucket_capacity);
CH_API_DEC void chashtable_create)(chashtable* ht,size_t key_size_in_bytes,size_t value_size_in_bytes,chtu_hash_uint (*key_hash)(const void*),int (*key_cmp) (const void*,const void*),size_t initial_bucket_capacity);
#endif /* CH_ENABLE_DECLARATION_AND_DEFINITION */

#endif /* C_HASHTABLE_TYPE_UNSAFE_H */




#if (!defined(CH_ENABLE_DECLARATION_AND_DEFINITION) || defined(C_HASHTABLE_TYPE_UNSAFE_IMPLEMENTATION))
#ifndef C_HASHTABLE_TYPE_UNSAFE_H_IMPLEMENTATION_GUARD
#define C_HASHTABLE_TYPE_UNSAFE_H_IMPLEMENTATION_GUARD

#ifdef COMPILER_SUPPORTS_GCC_DIAGNOSTIC
#   pragma GCC diagnostic push
#   pragma GCC diagnostic ignored "-Wpragmas"
#   pragma GCC diagnostic ignored "-Wunknown-warning-option"
#   pragma GCC diagnostic ignored "-Wignored-qualifiers"
#   pragma GCC diagnostic ignored "-Wclass-memaccess"
#endif //COMPILER_SUPPORTS_GCC_DIAGNOSTIC

/* ch implementation */

/* --- PRIVATE FUNCTIONS START -------------------------------------------------- */
CH_API void chvector_clear(chvector* v,const chashtable* ht)	{
    CH_ASSERT(v && ht);
    if (v->k) {
		size_t j;            
        if (ht->key_dtr)   {
			if (ht->value_dtr)	{
            	for (j=0;j<v->size;j++) {
                    ht->key_dtr((unsigned char*)v->k+j*ht->key_size_in_bytes);
                    ht->value_dtr((unsigned char*)v->v+j*ht->value_size_in_bytes);
            	}
			}
            else  {for (j=0;j<v->size;j++) ht->key_dtr((unsigned char*)v->k+j*ht->key_size_in_bytes);}
        }
        else if (ht->value_dtr) {for (j=0;j<v->size;j++) ht->value_dtr((unsigned char*)v->v+j*ht->value_size_in_bytes);}
    }
    *((size_t*) &v->size)=0;
}
CH_API void chvector_reserve(chvector* v,size_t size,const chashtable* ht)	{
    CH_ASSERT(v && ht);
    /* grows-only! */
    if (size>v->capacity) {
        const size_t new_capacity = (v->capacity==0 && size>1) ?
                    size :      /* possibly keep initial user-guided 'reserve(...)' */
                    (v->capacity+(size-v->capacity)+(v->capacity)/2);   /* our growing strategy */
        ch_safe_realloc((void** const) &v->k,new_capacity*(ht->key_size_in_bytes+ht->value_size_in_bytes));
		/* |k0|k1|k2| | |v0|v1|v2| | |      size=3 capacity=5     new_capacity=7 
			we must memmove	(capacity*ht->key_size_in_bytes) to (new_capacity*ht->key_size_in_bytes) a number of bytes (size*	ht->value_size_in_bytes)*/		
        v->v = (unsigned char*)v->k + new_capacity*ht->key_size_in_bytes;
        memmove((unsigned char*) v->v,(const unsigned char*) v->k+v->capacity*ht->key_size_in_bytes,v->size*ht->value_size_in_bytes);
		*((size_t*) &v->capacity) = new_capacity;
    }
}
CH_API void chvector_resize(chvector* v,size_t size,const chashtable* ht)	{
    CH_ASSERT(v && ht);
    if (size>v->capacity) chvector_reserve(v,size,ht);
    if (size<v->size)   {
        size_t i;
        if (ht->key_dtr && ht->value_dtr) {for (i=size;i<v->size;i++) {ht->key_dtr((unsigned char*)v->k+i*ht->key_size_in_bytes);ht->value_dtr((unsigned char*)v->v+i*ht->value_size_in_bytes);}}
        else if (ht->key_dtr)   {for (i=size;i<v->size;i++) ht->key_dtr((unsigned char*)v->k+i*ht->key_size_in_bytes);}
        else if (ht->value_dtr) {for (i=size;i<v->size;i++) ht->value_dtr((unsigned char*)v->v+i*ht->value_size_in_bytes);}
    }
    else {
        size_t i;
#       ifndef CH_DISABLE_CLEARING_ITEM_MEMORY
        if (ht->key_ctr || ht->key_cpy) 	memset((unsigned char*)v->k+v->size*ht->key_size_in_bytes,0,(size-v->size)*ht->key_size_in_bytes);
		if (ht->value_ctr || ht->value_cpy) memset((unsigned char*)v->v+v->size*ht->value_size_in_bytes,0,(size-v->size)*ht->value_size_in_bytes);
#       endif
        if (ht->key_ctr && ht->value_ctr)   {for (i=v->size;i<size;i++) {ht->key_ctr((unsigned char*)v->k+i*ht->key_size_in_bytes);ht->value_ctr((unsigned char*)v->v+i*ht->value_size_in_bytes);}}
        else if (ht->key_ctr)        {for (i=v->size;i<size;i++) ht->key_ctr((unsigned char*)v->k+i*ht->key_size_in_bytes);}
        else if (ht->value_ctr) {for (i=v->size;i<size;i++) ht->value_ctr((unsigned char*)v->v+i*ht->value_size_in_bytes);}
    }
    *((size_t*) &v->size)=size;
}
CH_API size_t chvector_unsorted_search(const chvector* v,const void* key,int* match,const chashtable* ht)  {
    size_t i;
    CH_ASSERT(v && ht);
    if (match) *match=0;
    if (v->size==0) return 0;  /* otherwise match will be 1 */
    if (!ht->key_cmp)   {
        int cmp_ok=0;
        for (i = 0; i < v->size; i++) {
            cmp_ok = (memcmp(key,(const unsigned char*)v->k+i*ht->key_size_in_bytes,ht->key_size_in_bytes)==0)?1:0;
            if (cmp_ok) {if (match) {*match=1;} return i;}}
    }
    else    {
        for (i = 0; i < v->size; i++) {if (ht->key_cmp(key,(const unsigned char*)v->k+i*ht->key_size_in_bytes)==0) {if (match) {*match=1;}return i;}}
    }
    CH_ASSERT(i==v->size);
    return i;
}
CH_API size_t chvector_linear_search(const chvector* v,const void* key,int* match,const chashtable* ht)  {
    int cmp=0;size_t i;
    CH_ASSERT(v && ht && ht->key_cmp);
    if (match) *match=0;
    if (v->size==0) return 0;  /* otherwise match will be 1 */
    for (i = 0; i < v->size; i++) {
        cmp = ht->key_cmp(key,(const unsigned char*)v->k+i*ht->key_size_in_bytes);
        if (cmp<=0) {
            if (cmp==0 && match) *match=1;
            return i;
        }
    }
    CH_ASSERT(i==v->size);
    return i;
}
CH_API size_t chvector_binary_search(const chvector* v,const void* key,int* match,const chashtable* ht)  {
    size_t first=0, last;
    size_t mid;int cmp;
    CH_ASSERT(v && ht && ht->key_cmp);
    if (match) *match=0;
    if (v->size==0) return 0;  /* otherwise match will be 1 */
    last=v->size-1;
    while (first <= last) {
        mid = (first + last) / 2;
        cmp = ht->key_cmp(key,(const unsigned char*)v->k+mid*ht->key_size_in_bytes);
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
CH_API size_t chvector_insert_key_at(chvector* v,const void* key_to_insert,size_t position,const chashtable* ht)  {
    /* position is in [0,v->size] */
    /* warning: this code does NOT support passing pointers to keys already present in this hashtable */
    CH_ASSERT(v && ht && key_to_insert && position<=v->size);
    chvector_reserve(v,v->size+1,ht);
    if (position<v->size) {
        memmove((unsigned char*)v->k+(position+1)*ht->key_size_in_bytes,  (const unsigned char*) v->k+position*ht->key_size_in_bytes,  (v->size-position)*ht->key_size_in_bytes);
        memmove((unsigned char*)v->v+(position+1)*ht->value_size_in_bytes,(const unsigned char*) v->v+position*ht->value_size_in_bytes,(v->size-position)*ht->value_size_in_bytes);
	}
#   ifndef CH_DISABLE_CLEARING_ITEM_MEMORY
    if (ht->key_ctr || ht->key_cpy) 	memset((unsigned char*)v->k+position*ht->key_size_in_bytes,0,ht->key_size_in_bytes);
    if (ht->value_ctr || ht->value_cpy) memset((unsigned char*)v->v+position*ht->value_size_in_bytes,0,ht->value_size_in_bytes);
#   endif
    if (ht->key_ctr)    ht->key_ctr((unsigned char*)v->k+position*ht->key_size_in_bytes);
    if (ht->value_ctr)  ht->value_ctr((unsigned char*)v->v+position*ht->value_size_in_bytes);
    if (!ht->key_cpy)   memcpy((unsigned char*)v->k+position*ht->key_size_in_bytes,key_to_insert,ht->key_size_in_bytes);
    else ht->key_cpy((unsigned char*)v->k+position*ht->key_size_in_bytes,key_to_insert);
    *((size_t*) &v->size)=v->size+1;
    return position;
}
CH_API int chvector_remove_at(chvector* v,size_t position,const chashtable* ht)  {
    /* position is in [0,num_items) */
    int removal_ok;
    CH_ASSERT(v && ht);
    removal_ok = (position<v->size) ? 1 : 0;
    CH_ASSERT(removal_ok);	/* error: position>=v->size */
    if (removal_ok)	{
        if (ht->key_dtr)    ht->key_dtr((unsigned char*)v->k+position*ht->key_size_in_bytes);
        if (ht->value_dtr)  ht->value_dtr((unsigned char*)v->v+position*ht->value_size_in_bytes);
        memmove((unsigned char*)v->k+position*ht->key_size_in_bytes,(unsigned char*)v->k+(position+1)*ht->key_size_in_bytes,(v->size-position-1)*ht->key_size_in_bytes);
        memmove((unsigned char*)v->v+position*ht->value_size_in_bytes,(unsigned char*)v->v+(position+1)*ht->value_size_in_bytes,(v->size-position-1)*ht->value_size_in_bytes);
        *((size_t*) &v->size)=v->size-1;
    }
    return removal_ok;
}
/* --- PRIVATE FUNCTIONS END -------------------------------------------------- */

CH_API_DEF void chashtable_free(chashtable* ht)    {
    if (ht) {
        const unsigned short max_value = (CH_NUM_USED_BUCKETS-1);
        unsigned short i=0;
        do    {
            chvector* b = &ht->buckets[i];
            chvector_clear(b,ht);
            if (b->k) {ch_free(b->k);ht->buckets[i].k=NULL;ht->buckets[i].v=NULL;}
            *((size_t*)&b->capacity)=0;
        }
        while (i++!=max_value);
    }
}
CH_API_DEF void chashtable_clear(chashtable* ht)    {
    if (ht) {
        const unsigned short max_value = (CH_NUM_USED_BUCKETS-1);
        unsigned short i=0;
        do    {chvector_clear(&ht->buckets[i],ht);}
        while (i++!=max_value);
    }
}

/* Warning: 'ch_xxx_get_or_insert(...)' and 'ch_xxx_get(...)', and their overloads,
   return pointer to 'value' items inside the hashtable, that are invalidated when
   the hashtable inserts or removes other items. User should copy the result for longer storage.
*/
CH_API_DEF void* chashtable_get_or_insert(chashtable* ht,const void* key,int* match) {
    chvector* v = NULL;
    size_t position;chtu_hash_uint hash;int match2;
    CH_ASSERT(ht && ht->key_hash);
    hash = ht->key_hash(key);
#   if CH_NUM_USED_BUCKETS!=CH_MAX_POSSIBLE_NUM_BUCKETS
    CH_ASSERT(hash<CH_NUM_USED_BUCKETS);    /* user 'key_hash' should return values in [0,CH_NUM_USED_BUCKETS). Please use: return somevalue%CH_NUM_USED_BUCKETS */
#   endif
    v = &ht->buckets[hash];
    if (!v->k)  {
        v->k = ch_malloc(ht->initial_bucket_capacity*(ht->key_size_in_bytes+ht->value_size_in_bytes));
		v->v = (unsigned char*) v->k+ht->initial_bucket_capacity*ht->key_size_in_bytes;
        *((size_t*)&v->capacity) = ht->initial_bucket_capacity;
    }

    if (v->size==0)    {position=0;match2=0;}
    else if (ht->key_cmp)   {
        /* slightly faster */
        position =  v->size>2 ? chvector_binary_search(v,key,&match2,ht) :
                    chvector_linear_search(v,key,&match2,ht);
        /* slightly slower */
        /* position = chvector_binary_search(v,key,&match2,ht); */
    }
    else    {
        /* '_unsorted_search' uses memcmp(...) (when ht->key_cmp==NULL) */
        position = chvector_unsorted_search(v,key,&match2,ht);
    }
    if (match) *match=match2;

    if (match2) return (unsigned char*)v->v+position*ht->value_size_in_bytes;

    /* we must insert an item at 'position' */
     chvector_insert_key_at(v,key,position,ht);
    return (unsigned char*)v->v+position*ht->value_size_in_bytes;
}
CH_API_DEF void* chashtable_get(chashtable* ht,const void* key) {
    chvector* v = NULL;
    size_t position;chtu_hash_uint hash;int match=0;
    CH_ASSERT(ht && ht->key_hash);
    hash = ht->key_hash(key);
#   if CH_NUM_USED_BUCKETS!=CH_MAX_POSSIBLE_NUM_BUCKETS
    CH_ASSERT(hash<CH_NUM_USED_BUCKETS);    /* user 'key_hash' should return values in [0,CH_NUM_USED_BUCKETS). Please use: return somevalue%CH_NUM_USED_BUCKETS */
#   endif
    v = &ht->buckets[hash];
    if (!v->k || v->size==0)  return NULL;

    if (ht->key_cmp)    {
        /* slightly faster */
        position =  v->size>2 ? chvector_binary_search(v,key,&match,ht) :
                    chvector_linear_search(v,key,&match,ht);
        /* slightly slower */
        /* position = chvector_binary_search(v,key,&match,ht); */
    }
    else    {
        /* '_unsorted_search' uses memcmp(...) (when ht->key_cmp==NULL) */
        position = chvector_unsorted_search(v,key,&match,ht);
    }

    return (match ? ((unsigned char*)v->v+position*ht->value_size_in_bytes) : NULL);
}
CH_API_DEF const void* chashtable_get_const(const chashtable* ht,const void* key) {return (const void*) chashtable_get((chashtable*)ht,key);}

CH_API_DEF int chashtable_remove(chashtable* ht,const void* key) {
    chvector* v = NULL;
    size_t position;chtu_hash_uint hash;int match = 0;
    CH_ASSERT(ht && ht->key_hash);
    hash = ht->key_hash(key);
#   if CH_NUM_USED_BUCKETS!=CH_MAX_POSSIBLE_NUM_BUCKETS
    CH_ASSERT(hash<CH_NUM_USED_BUCKETS);    /* user 'key_hash' should return values in [0,CH_NUM_USED_BUCKETS). Please use: return somevalue%CH_NUM_USED_BUCKETS */
#   endif
    v = &ht->buckets[hash];
    if (!v->k)  return 0;
    if (ht->key_cmp)    {
        /* slightly faster */
        position =  v->size>2 ? chvector_binary_search(v,key,&match,ht) :
                    chvector_linear_search(v,key,&match,ht);
        /* slightly slower */
        /* position = chvector_binary_search(v,key,&match,ht); */
    }
    else    {
        /* '_unsorted_search' uses memcmp(...) (when ht->key_cmp==NULL) */
        position = chvector_unsorted_search(v,key,&match,ht);
    }
    if (match) {
        chvector_remove_at(v,position,ht);
        return 1;
    }
    return 0;
}
CH_API_DEF size_t chashtable_get_num_items(const chashtable* ht) {
    size_t i,sum=0;CH_ASSERT(ht);
    for (i=0;i<CH_NUM_USED_BUCKETS;i++) sum+=ht->buckets[i].size;
    return sum;
}
CH_API_DEF int chashtable_dbg_check(const chashtable* ht) {
    size_t i,j,num_total_items=0,num_total_capacity=0,num_sorting_errors=0,min_num_bucket_items=(size_t)-1,max_num_bucket_items=0,min_cnt=0,max_cnt=0,avg_cnt=0,avg_round=0;
    double avg_num_bucket_items=0.0,std_deviation=0.0;
    size_t mem_minimal=sizeof(chashtable),mem_used=sizeof(chashtable);
    double mem_used_percentage = 100;
    const unsigned char* last_key = NULL;
    CH_ASSERT(ht);
    for (i=0;i<CH_NUM_USED_BUCKETS;i++) {
        const chvector* bck = &ht->buckets[i];
        num_total_items+=bck->size;
        num_total_capacity+=bck->capacity;
        if (min_num_bucket_items>bck->size) min_num_bucket_items=bck->size;
        if (max_num_bucket_items<bck->size) max_num_bucket_items=bck->size;
        if (bck->v) {
            mem_minimal += (ht->key_size_in_bytes+ht->value_size_in_bytes)*bck->size;
            mem_used += (ht->key_size_in_bytes+ht->value_size_in_bytes)*bck->capacity;
            if (ht->key_cmp && bck->size)    {
                last_key = NULL;
                for (j=0;j<bck->size;j++)  {
                    const unsigned char* key = (const unsigned char*) bck->k+j*ht->key_size_in_bytes;
                    if (last_key) {
                        if (ht->key_cmp(last_key,key)>0) {
                            /* When this happens, it can be a wrong user 'key_cmp' function (that cannot sort keys in a consistent way) */
                            ++num_sorting_errors;
#                       ifndef CH_NO_STDIO
                            fprintf(stderr,"[chashtable_dbg_check] Sorting Error (%" CV_SIZE_T_FORMATTING "): in bucket[%" CV_SIZE_T_FORMATTING "]: key_cmp(%" CV_SIZE_T_FORMATTING ",%" CV_SIZE_T_FORMATTING ")>0 [num_items=%" CV_SIZE_T_FORMATTING " (in bucket)]\n",num_sorting_errors,i,j-1,j,bck->size);
#                       endif
                        }
                    }
                    last_key=key;
                }
            }
        }
    }
    avg_num_bucket_items = (double)num_total_items/(double) CH_NUM_USED_BUCKETS;
    mem_used_percentage = (double)mem_used*100.0/(double)mem_minimal;
    if (CH_NUM_USED_BUCKETS<2) {std_deviation=0.;avg_cnt=min_cnt=max_cnt=1;avg_round=(min_num_bucket_items+max_num_bucket_items)/2;}
    else {
        const double dec = avg_num_bucket_items-(double)((size_t)avg_num_bucket_items); /* in (0,1] */
        avg_round = (size_t)avg_num_bucket_items;
        if (dec>=0.5) avg_round+=1;
        for (i=0;i<CH_NUM_USED_BUCKETS;i++) {
            const chvector* bck = &ht->buckets[i];
            double tmp = bck->size-avg_num_bucket_items;
            std_deviation+=tmp*tmp;
            if (bck->size==min_num_bucket_items) ++min_cnt;
            if (bck->size==max_num_bucket_items) ++max_cnt;
            if (bck->size==avg_round) ++avg_cnt;
        }
        std_deviation/=(double)(CH_NUM_USED_BUCKETS-1); /* this is the variance */
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
    printf("[chashtable_dbg_check]:\n");
    printf("\tnum_total_items=%" CV_SIZE_T_FORMATTING " (num_total_capacity=%" CV_SIZE_T_FORMATTING ") in %d buckets [items per bucket: mean=%1.3f std_deviation=%1.3f min=%" CV_SIZE_T_FORMATTING " (in %" CV_SIZE_T_FORMATTING "/%d) avg=%" CV_SIZE_T_FORMATTING " (in %" CV_SIZE_T_FORMATTING "/%d) max=%" CV_SIZE_T_FORMATTING " (in %" CV_SIZE_T_FORMATTING "/%d)].\n",num_total_items,num_total_capacity,CH_NUM_USED_BUCKETS,avg_num_bucket_items,std_deviation,min_num_bucket_items,min_cnt,CH_NUM_USED_BUCKETS,avg_round,avg_cnt,CH_NUM_USED_BUCKETS,max_num_bucket_items,max_cnt,CH_NUM_USED_BUCKETS);
    printf("\tmemory_used: ");ch_display_bytes(mem_used);
    printf(". memory_minimal_possible: ");ch_display_bytes(mem_minimal);
    printf(". mem_used_percentage: %1.2f%% (100%% is the best possible result).\n",mem_used_percentage);
#   endif
    CH_ASSERT(num_sorting_errors==0); /* When this happens, it can be a wrong user 'itemKey_cmp' function (that cannot sort keys in a consistent way) */
    return (int) num_total_items;
}

CH_API_DEF void chashtable_swap(chashtable* a,chashtable* b)  {
    unsigned char t[sizeof(chashtable)];
    if (a!=b)   {
        CH_ASSERT(a && b);
        memcpy(&t,a,sizeof(chashtable));
        memcpy(a,b,sizeof(chashtable));
        memcpy(b,&t,sizeof(chashtable));
    }
}
CH_API_DEF void chashtable_cpy(chashtable* a,const chashtable* b) {
    size_t i;
    typedef chtu_hash_uint (*key_hash_type)(const void*);
    typedef int (*key_cmp_type)(const void*,const void*);
    typedef void (*key_ctr_dtr_type)(void*);
    typedef void (*key_cpy_type)(void*,const void*);
    typedef void (*value_ctr_dtr_type)(void*);
    typedef void (*value_cpy_type)(void*,const void*);
    if (a==b) return;
    CH_ASSERT(a && b);
	if (a->key_size_in_bytes==0 && a->value_size_in_bytes==0)	{
        *((size_t*)&a->key_size_in_bytes)  =b->key_size_in_bytes;
        *((size_t*)&a->value_size_in_bytes)=b->value_size_in_bytes;
	}
	CH_ASSERT(a->key_size_in_bytes==b->key_size_in_bytes);
	CH_ASSERT(a->value_size_in_bytes==b->value_size_in_bytes);
    if (a->key_size_in_bytes!=b->key_size_in_bytes || a->value_size_in_bytes!=b->value_size_in_bytes)   {
#       ifndef CH_NO_STDIO
        fprintf(stderr,"[chashtable_cpy] Error: two hashtables with different 'key_size_in_bytes' (%" CV_SIZE_T_FORMATTING " and %" CV_SIZE_T_FORMATTING ") or 'value_size_in_bytes' (%" CV_SIZE_T_FORMATTING " and %" CV_SIZE_T_FORMATTING ") can't be copied.\n",a->key_size_in_bytes,b->key_size_in_bytes,a->value_size_in_bytes,b->value_size_in_bytes);
#       endif
        return;
    }	
    *((key_hash_type*)&a->key_hash) = b->key_hash;
    *((key_cmp_type*)&a->key_cmp) = b->key_cmp;
    *((key_ctr_dtr_type*)&a->key_ctr) = b->key_ctr;
    *((key_ctr_dtr_type*)&a->key_dtr) = b->key_dtr;
    *((key_cpy_type*)&a->key_cpy) = b->key_cpy;
    *((value_ctr_dtr_type*)&a->value_ctr) = b->value_ctr;
    *((value_ctr_dtr_type*)&a->value_dtr) = b->value_dtr;
    *((value_cpy_type*)&a->value_cpy) = b->value_cpy;
    *((size_t*)&a->initial_bucket_capacity) = b->initial_bucket_capacity;
    for (i=0;i<CH_NUM_USED_BUCKETS;i++)  {
        chvector* A = &a->buckets[i];
        const chvector* B = &b->buckets[i];
        /* bad init asserts */
        CH_ASSERT(!(A->v && A->capacity==0));
        CH_ASSERT(!(!A->v && A->capacity>0));
        CH_ASSERT(!(B->v && B->capacity==0));
        CH_ASSERT(!(!B->v && B->capacity>0));
        /*chvector_free(A,a);*/
        chvector_clear(A,a);
        chvector_resize(A,B->size,a);
        CH_ASSERT(A->size==B->size);
        if (a->key_cpy)	{for (i=0;i<A->size;i++) {a->key_cpy((unsigned char*)A->k+i*a->key_size_in_bytes,(const unsigned char*)B->k+i*b->key_size_in_bytes);}}
        else   memcpy((unsigned char*)A->k,(const unsigned char*)B->k,A->size*a->key_size_in_bytes);
        if (a->value_cpy)	{for (i=0;i<A->size;i++) {a->value_cpy((unsigned char*)A->v+i*a->value_size_in_bytes,(const unsigned char*)B->v+i*b->value_size_in_bytes);}}
        else   memcpy((unsigned char*)A->v,(const unsigned char*)B->v,A->size*a->value_size_in_bytes);
    }
}
CH_API_DEF void chashtable_shrink_to_fit(chashtable* ht)   {
    if (ht)	{
        chashtable o;
        memset(&o,0,sizeof(chashtable));
        chashtable_cpy(&o,ht); /* now 'o' is 'v' trimmed */
        chashtable_free(ht);
        chashtable_swap(&o,ht);
    }
}

CH_API_DEF void chashtable_create_with(
        chashtable* ht,
		size_t key_size_in_bytes,size_t value_size_in_bytes,
        chtu_hash_uint (*key_hash)(const void*),
        int (*key_cmp) (const void*,const void*),
        void (*key_ctr)(void*),void (*key_dtr)(void*),void (*key_cpy)(void*,const void*),
        void (*value_ctr)(void*),void (*value_dtr)(void*),void (*value_cpy)(void*,const void*),
        size_t initial_bucket_capacity)   {
    typedef chtu_hash_uint (*key_hash_type)(const void*);
    typedef int (*key_cmp_type)(const void*,const void*);
    typedef void (*key_ctr_dtr_type)(void*);
    typedef void (*key_cpy_type)(void*,const void*);
    typedef void (*value_ctr_dtr_type)(void*);
    typedef void (*value_cpy_type)(void*,const void*);
#   ifndef CH_DISABLE_FAKE_MEMBER_FUNCTIONS
    typedef void (* clear_free_shrink_to_fit_mf)(chashtable*);
    typedef void* (* get_or_insert_mf)(chashtable*,const void*,int*);
    typedef void* (* get_mf)(chashtable*,const void*);
    typedef const void* (* get_const_mf)(const chashtable*,const void*);
    typedef int (* remove_mf)(chashtable*,const void*);
    typedef size_t (* get_num_items_mf)(const chashtable* ht);
    typedef int (* dbg_check_mf)(const chashtable*);
    typedef void (* swap_mf)(chashtable* a,chashtable* b);
    typedef void (* cpy_mf)(chashtable* a,const chashtable* b);
#   endif
    CH_ASSERT(ht);
    memset(ht,0,sizeof(chashtable));
	*((size_t*) &ht->key_size_in_bytes)=key_size_in_bytes;
    *((size_t*) &ht->value_size_in_bytes)=value_size_in_bytes;
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
    /*memset(ht->buckets,0,CH_NUM_USED_BUCKETS*sizeof(chvector));*/
#   ifndef CH_DISABLE_FAKE_MEMBER_FUNCTIONS
    *((clear_free_shrink_to_fit_mf*)&ht->clear) = &chashtable_clear;
    *((clear_free_shrink_to_fit_mf*)&ht->free) = &chashtable_free;
    *((clear_free_shrink_to_fit_mf*)&ht->shrink_to_fit) = &chashtable_shrink_to_fit;
    *((get_or_insert_mf*)&ht->get_or_insert) = &chashtable_get_or_insert;
    *((get_mf*)&ht->get) = &chashtable_get;
    *((get_const_mf*)&ht->get_const) = &chashtable_get_const;
    *((remove_mf*)&ht->remove) = &chashtable_remove;
    *((get_num_items_mf*)&ht->get_num_items) = &chashtable_get_num_items;
    *((dbg_check_mf*)&ht->dbg_check) = &chashtable_dbg_check;
    *((swap_mf*)&ht->swap) = &chashtable_swap;
    *((cpy_mf*)&ht->cpy) = &chashtable_cpy;
#   endif
}
CH_API_DEF void chashtable_create(chashtable* ht,size_t key_size_in_bytes,size_t value_size_in_bytes,chtu_hash_uint (*key_hash)(const void*),int (*key_cmp) (const void*,const void*),size_t initial_bucket_capacity)    {
    chashtable_create_with(ht,key_size_in_bytes,value_size_in_bytes,key_hash,key_cmp,NULL,NULL,NULL,NULL,NULL,NULL,initial_bucket_capacity);
}

#ifdef __cplusplus
    chashtable::chashtable() :
        key_size_in_bytes(0),value_size_in_bytes(0),
        key_ctr(NULL),key_dtr(NULL),key_cpy(NULL),key_cmp(NULL),key_hash(NULL),
        value_ctr(NULL),value_dtr(NULL),value_cpy(NULL),initial_bucket_capacity(1),
        clear(&chashtable_clear),free(&chashtable_free),shrink_to_fit(&chashtable_shrink_to_fit),
        get_or_insert(&chashtable_get_or_insert),get(&chashtable_get),get_const(&chashtable_get_const),
        remove(&chashtable_remove),get_num_items(&chashtable_get_num_items),
        dbg_check(&chashtable_dbg_check),swap(&chashtable_swap),cpy(&chashtable_cpy)
    {}

    chashtable::chashtable(const chashtable& o) :
    key_size_in_bytes(o.key_size_in_bytes),value_size_in_bytes(o.value_size_in_bytes),
    key_ctr(o.key_ctr),key_dtr(o.key_dtr),key_cpy(o.key_cpy),key_cmp(o.key_cmp),key_hash(o.key_hash),
    value_ctr(o.value_ctr),value_dtr(o.value_dtr),value_cpy(o.value_cpy),initial_bucket_capacity(o.initial_bucket_capacity),
    clear(&chashtable_clear),free(&chashtable_free),shrink_to_fit(&chashtable_shrink_to_fit),
    get_or_insert(&chashtable_get_or_insert),get(&chashtable_get),get_const(&chashtable_get_const),
    remove(&chashtable_remove),get_num_items(&chashtable_get_num_items),
    dbg_check(&chashtable_dbg_check),swap(&chashtable_swap),cpy(&chashtable_cpy)
    {
        chashtable_cpy(this,&o);
    }

    chashtable& chashtable::operator=(const chashtable& o)    {chashtable_cpy(this,&o);return *this;}

#   ifdef CH_HAS_MOVE_SEMANTICS
    chashtable::chashtable(chashtable&& o) :
    key_size_in_bytes(o.key_size_in_bytes),value_size_in_bytes(o.value_size_in_bytes),
    key_ctr(o.key_ctr),key_dtr(o.key_dtr),key_cpy(o.key_cpy),key_cmp(o.key_cmp),key_hash(o.key_hash),
    value_ctr(o.value_ctr),value_dtr(o.value_dtr),value_cpy(o.value_cpy),initial_bucket_capacity(o.initial_bucket_capacity),
    clear(&chashtable_clear),free(&chashtable_free),shrink_to_fit(&chashtable_shrink_to_fit),
    get_or_insert(&chashtable_get_or_insert),get(&chashtable_get),get_const(&chashtable_get_const),
    remove(&chashtable_remove),get_num_items(&chashtable_get_num_items),
    dbg_check(&chashtable_dbg_check),swap(&chashtable_swap),cpy(&chashtable_cpy)
    {
        size_t i;
        for (i=0;i<CH_NUM_USED_BUCKETS;i++) {
            chvector& d = buckets[i];
            chvector& s = o.buckets[i];
            d.k=s.k;d.v=s.v;
            *((size_t*)&d.size)=s.size;*((size_t*)&d.capacity)=s.capacity;
            s.k=NULL;s.v=NULL;
            *((size_t*)&s.size)=0;*((size_t*)&s.capacity)=0;
        }
    }

    chashtable& chashtable::operator=(chashtable&& o)    {
        if (this != &o) {
            size_t i;
            chashtable_free(this);
            for (i=0;i<CH_NUM_USED_BUCKETS;i++) {
                chvector& d = buckets[i];
                chvector& s = o.buckets[i];
                d.k=s.k;d.v=s.v;
                *((size_t*)&d.size)=s.size;*((size_t*)&d.capacity)=s.capacity;
                s.k=NULL;s.v=NULL;
                *((size_t*)&s.size)=0;*((size_t*)&s.capacity)=0;
            }
        }
        return *this;
    }
#   endif

    chashtable::~chashtable() {chashtable_free(this);}
#endif

#ifdef COMPILER_SUPPORTS_GCC_DIAGNOSTIC
#   pragma GCC diagnostic pop
#endif //COMPILER_SUPPORTS_GCC_DIAGNOSTIC

#endif /* C_HASHTABLE_TYPE_UNSAFE_H_IMPLEMENTATION_GUARD */
#endif /* (!defined(CH_ENABLE_DECLARATION_AND_DEFINITION) || defined(C_HASHTABLE_TYPE_UNSAFE_IMPLEMENTATION)) */



