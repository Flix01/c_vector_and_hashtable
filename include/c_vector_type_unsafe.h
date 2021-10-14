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
/* USAGE: Please see the bundled "c_vector_type_unsafe_main.c". */


/* GLOBAL DEFINITIONS: The following definitions (when used) must be set
   globally (= in the Project Options or in a StdAfx.h file):

   CV_DISABLE_FAKE_MEMBER_FUNCTIONS     // it disables "fake-member-function-syntax" (e.g. v.push_back(&v,item);). Use it to improve performance and reduce memory.
   CV_ENABLE_CLEARING_ITEM_MEMORY       // enable it if you want that, before each item is constructed (and before the user-provided item_ctr function, if present, is called), the item memory is cleared to zero to increase code robustness (but it slows down performance).
   CV_ENABLE_DECLARATION_AND_DEFINITION // when used, C_VECTOR_TYPE_UNSAFE_IMPLEMENTATION must be defined before including this file in a single source (.c) file
                                        // by doing so "c_vector_type_unsafe.h" becomes lighter, when used without its implementation.
   CV_FORCE_MEMCPY_S                    // it enforces the use of memcpy_s(...) and similar functions (but the standard and safer way is to define __STDC_WANT_LIB_EXT1__. Please see: https://en.cppreference.com/w/c/string/byte/memcpy).
   CV_NO_MEMCPY_S                       // it forces the use of memcpy(...) and similar functions, in cases where memcpy_s(...) is used by default (currently only Visual Studio 2005 (8.0) or newer).
   CV_NO_CVH_STRING_T                   // it disables the 'cvh_string_t' struct and functions.
   CV_NO_PLACEMENT_NEW                  // (c++ mode only) it does not define (unused) helper stuff like: CV_PLACEMENT_NEW, cpp_ctr_tu,cpp_dtr_tu,cpp_cpy_tu,cpp_cmp_tu

   CV_MALLOC
   CV_REALLOC
   CV_FREE
   CV_ASSERT
   CV_NO_ASSERT
   CV_NO_STDIO
   CV_NO_STDLIB
   CV_API_INL                           // this simply defines the 'inline' keyword syntax (defaults to __inline)
   CV_API                               // used always when CV_ENABLE_DECLARATION_AND_DEFINITION is not defined and in some global or private functions otherwise
   CV_API_DEC                           // defaults to CV_API, or to 'CV_API_INL extern' if CV_ENABLE_DECLARATION_AND_DEFINITION is defined
   CV_APY_DEF                           // defaults to CV_API, or to nothing if CV_ENABLE_DECLARATION_AND_DEFINITION is defined

*/


#ifndef C_VECTOR_TYPE_UNSAFE_H
#define C_VECTOR_TYPE_UNSAFE_H

#define C_VECTOR_TYPE_UNSAFE_VERSION            "1.09"
#define C_VECTOR_TYPE_UNSAFE_VERSION_NUM        0109

/* HISTORY
   C_VECTOR_TYPE_UNSAFE_VERSION_NUM 109
   -> Removed CV_DISABLE_CLEARING_ITEM_MEMORY (now it's the default).
   -> Added CV_ENABLE_CLEARING_ITEM_MEMORY.
   -> Renamed cvector_create(...) -> cvector_init(...).
   -> Renamed cvector_create_with(...) -> cvector_init_with(...).
   -> Added cvector_create(...) and cvector_create_with(...) that return an initialized cvector by value.
   -> Added serialization/deserialization support:
        -> now cvector_init_with(...) and cvector_create_with(...) take two additional arguments: item_serialization and item_deserialization function pointers.
           If they are NULL, items will be serialized/deserialized using plain memcpy calls (good only for plain item structs without pointers or vectors inside).
        -> a new helper struct cvh_serializer_t must be used to serialize/deserialize cvectors (and items if needed) (please see "c_vector_type_unsafe_main.c").
        -> code near the bottom of this file (search backwards for "cvector_serialize" and "cvector_deserialize")
           can be taken as a reference for implementing item_serialize/item_deserialize funtions.
        -> serialization/deserialization is in binary-mode, it's NOT endian-independent and the size of each type and struct must match in both serialization and deserialization
           (it would be nice to know what common systems are compatible).
        -> The functions cvh_serializer_write(...)/cvh_serializer_read(...) can be used to ease the implementation of item_serialize/item_deserialize funtions.
   -> Added the optional extra struct cvh_string_t (and relative demo test in "c_vector_type_unsafe_main.c"). It can be disabled using the CV_NO_CVH_STRING_T definition.

   C_VECTOR_TYPE_UNSAFE_VERSION_NUM 108
   -> Added code to silence some compiler warnings (see: COMPILER_SUPPORTS_GCC_DIAGNOSTIC definition)

   C_VECTOR_TYPE_UNSAFE_VERSION_NUM 107
   -> added optional stuff for c++ compilation: cpp_cmp_tu (never used nor tested)

   C_VECTOR_TYPE_UNSAFE_VERSION_NUM 106
   -> added optional stuff for c++ compilation: CV_PLACEMENT_NEW,cpp_ctr_tu,cpp_dtr_tu,cpp_cpy_tu

   C_VECTOR_TYPE_UNSAFE_VERSION_NUM 105
   -> added c++ copy ctr, copy assignment and dtr (and move ctr plus move assignment if CV_HAS_MOVE_SEMANTICS is defined),
      to ease porting code from c++ to c a bit more (but 'cvector_create(...)' is still necessary in c++ mode)
   -> removed internal definitions CV_EXTERN_C_START, CV_EXTERN_C_END, CV_DEFAULT_STRUCT_INIT
      Now code is no more 'extern C'

   C_VECTOR_TYPE_UNSAFE_VERSION_NUM 104
   -> renamed CV_VERSION to C_VECTOR_TYPE_UNSAFE_VERSION
   -> renamed CV_VERSION_NUM to C_VECTOR_TYPE_UNSAFE_VERSION_NUM
   -> some internal changes to minimize interference with (the type-safe version) "c_vector.h",
      hoping that they can both cohexist in the same project.
   -> added ctr to ease compilation in c++ mode

   CV_VERSION_NUM   0103
   -> added a specific guard for the implementation for robustness
   -> removed unused macros CV_XSTR(...) CV_STR(...)
   -> renamed some internal functions from 'cvh_xxx(...)' to 'cv_xxx(...)' (for example cv_malloc(...) and similar functions)

   CV_VERSION_NUM   0102
   -> changed the header guard definition to C_VECTOR_TYPE_UNSAFE_H
   -> various changes to make this file compile correctly in c++ mode

   CV_VERSION_NUM   0101
   -> added the definitions CV_ENABLE_DECLARATION_AND_DEFINITION and C_VECTOR_TYPE_UNSAFE_IMPLEMENTATION (see above)

*/

#ifndef COMPILER_SUPPORTS_GCC_DIAGNOSTIC    // We define this
#   if (defined(__GNUC__) || defined(__MINGW__) || defined(__clang__))
#       define COMPILER_SUPPORTS_GCC_DIAGNOSTIC
#   endif
#endif

#ifdef COMPILER_SUPPORTS_GCC_DIAGNOSTIC
#   pragma GCC diagnostic push
#   pragma GCC diagnostic ignored "-Wpragmas"
#   pragma GCC diagnostic ignored "-Wunknown-warning-option"
#   pragma GCC diagnostic ignored "-Wclass-memaccess"
#endif //COMPILER_SUPPORTS_GCC_DIAGNOSTIC

#if __cplusplus>=201103L
#   undef CV_HAS_MOVE_SEMANTICS
#   define CV_HAS_MOVE_SEMANTICS
#endif

#if (defined (NDEBUG) || defined (_NDEBUG))
#   undef CV_NO_ASSERT
#   define CV_NO_ASSERT
#   undef CV_NO_STDIO
#   define CV_NO_STDIO
#   undef CV_NO_STDLIB
#   define CV_NO_STDLIB
#endif

#ifndef CV_MALLOC
#   undef CV_NO_STDLIB  /* stdlib cannot be avoided in this case */
#   include <stdlib.h>	 /* malloc/realloc/free */
#   define CV_MALLOC(X) malloc(X)
#endif

#ifndef CV_FREE
#   define CV_FREE(X) free(X)
#endif

#ifndef CV_REALLOC
#   define CV_REALLOC(x,y) realloc((x),(y))
#endif

#include <stddef.h> /* size_t */

#ifndef CV_ASSERT
#   ifdef CV_NO_ASSERT
#       define CV_ASSERT(X) /*no-op*/
#   else
#       include <assert.h>
#       define CV_ASSERT(X) assert((X))
#   endif
#endif
#ifndef CV_NO_STDIO
#   include <stdio.h> /*fprintf,printf,stderr*/
#endif
#ifndef CV_NO_STDLIB
#   include <stdlib.h> /*exit*/
#endif

#include <string.h> /*memcpy,memmove,memset*/


#ifndef CV_API_INL  /* __inline, _inline or inline (C99) */
#define CV_API_INL __inline
#endif

#ifndef CV_API /* can we remove 'static' here? */
#define CV_API CV_API_INL static
#endif

#ifndef CV_ENABLE_DECLARATION_AND_DEFINITION
#	ifndef CV_API_DEC
#		define CV_API_DEC CV_API
#	endif
#	ifndef CV_API_DEF
#		define CV_API_DEF CV_API
#	endif
#else /* CV_ENABLE_DECLARATION_AND_DEFINITION */
#	ifndef CV_API_DEC
#		define CV_API_DEC CV_API_INL extern
#	endif
#	ifndef CV_API_DEF
#		define CV_API_DEF /* no-op */
#	endif
#endif /* CV_ENABLE_DECLARATION_AND_DEFINITION */

#ifdef __cplusplus
/* helper stuff never used in this file */
#   if (!defined(CV_PLACEMENT_NEW) && !defined(CV_NO_PLACEMENT_NEW))
#       if ((defined(_MSC_VER) && _MSC_VER<=1310) || defined(CV_USE_SIMPLER_NEW_OVERLOAD))
#           define CV_PLACEMENT_NEW(_PTR)  new(_PTR)        /* it might require <new> header inclusion */
#       else
            struct CVectorPlacementNewDummy {};
            inline void* operator new(size_t, CVectorPlacementNewDummy, void* ptr) { return ptr; }
            inline void operator delete(void*, CVectorPlacementNewDummy, void*) {}
#           define CV_PLACEMENT_NEW(_PTR)  new(CVectorPlacementNewDummy() ,_PTR)
#       endif /* _MSC_VER */
#   endif /* CV_PLACEMENT_NEW */
#   if (defined(CV_PLACEMENT_NEW) && !defined(CVH_CPP_TU_GUARD))
#   define CVH_CPP_TU_GUARD
    template<typename T> inline void cpp_ctr_tu(void* vv) {T* v=(T*) vv;CV_PLACEMENT_NEW(v) T();}
    template<typename T> inline void cpp_dtr_tu(void* vv) {T* v=(T*) vv;v->T::~T();}
    template<typename T> inline void cpp_cpy_tu(void* av,const void* bv) {T* a=(T*) av;const T* b=(const T*) bv;*a=*b;}
    template<typename T> inline int cpp_cmp_tu(const void* av,const void* bv) {const T* a=(const T*) av;const T* b=(const T*) bv;return (*a)<(*b)?-1:((*a)>(*b)?1:0);}
#   endif
#endif

/*  To use memcpy_s, memmove_s and memset_s, in your source file(s), please add this line before including this header:
#define __STDC_WANT_LIB_EXT1__ 1
#include "c_vector_type_unsafe.h"
*/
#if (defined(CV_FORCE_MEMCPY_S) && defined(CV_NO_MEMCPY_S))
#   error CV_FORCE_MEMCPY_S and CV_NO_MEMCPY_S cannot be both defined.
#endif
#ifndef CV_MEMCPY
#if (!defined(CV_NO_MEMCPY_S) && (defined(__STDC_LIB_EXT1__) || defined(CV_FORCE_MEMCPY_S) || (defined(_MSC_VER) && _MSC_VER>=1400)))   /* 1400 == Visual Studio 8.0 2005 */
#   define CV_MEMCPY(DST,SRC,SIZE)      memcpy_s((unsigned char*)DST,SIZE,(unsigned char*)SRC,SIZE)
#   define CV_MEMMOVE(DST,SRC,SIZE)     memmove_s((unsigned char*)DST,SIZE,(unsigned char*)SRC,SIZE)
#   define CV_MEMSET(DST,VALUE,SIZE)    memset_s((unsigned char*)DST,SIZE,VALUE,SIZE)
#else
#   define CV_MEMCPY(DST,SRC,SIZE)      memcpy((unsigned char*)DST,(unsigned char*)SRC,SIZE)
#   define CV_MEMMOVE(DST,SRC,SIZE)     memmove((unsigned char*)DST,(unsigned char*)SRC,SIZE)
#   define CV_MEMSET(DST,VALUE,SIZE)    memset((unsigned char*)DST,VALUE,SIZE)
#endif
#endif /*CV_MEMCPY */

#ifndef CV_COMMON_FUNCTIONS_GUARD
#define CV_COMMON_FUNCTIONS_GUARD
/* base memory helpers */
CV_API void* cv_malloc(size_t size) {
    void* p = CV_MALLOC(size);
    if (!p)	{
        CV_ASSERT(0);	/* No more memory error */
#       ifndef CV_NO_STDIO
        fprintf(stderr,"CV_ERROR: cv_malloc(...) failed. Not enough memory.\n");
#       endif
#       ifndef CV_NO_STDLIB
        exit(1);
#       endif
    }
    return p;
}
CV_API void cv_free(void* p)                         {CV_FREE(p);}
CV_API void* cv_safe_realloc(void** const ptr, size_t new_size)  {
    void *ptr2 = CV_REALLOC(*ptr,new_size);
    CV_ASSERT(new_size!=0);    /* undefined behaviour */
    if (ptr2) *ptr=ptr2;
    else {
        CV_FREE(*ptr);*ptr=NULL;
        CV_ASSERT(0);	/* No more memory error */
#       ifndef CV_NO_STDIO
        fprintf(stderr,"CV_ERROR: cv_safe_realloc(...) failed. Not enough memory.\n");
#       endif
#       ifndef CV_NO_STDLIB
        exit(1);
#       endif
    }
    return ptr2;
}
CV_API void cv_convert_bytes(size_t bytes_in,size_t pTGMKB[5])   {
    size_t i;pTGMKB[4] = bytes_in;
    for (i=0;i<4;i++)  {pTGMKB[3-i]=pTGMKB[4-i]/1024;pTGMKB[4-i]%=1024;}
}
#ifndef CV_NO_STDIO
CV_API void cv_display_bytes(size_t bytes_in)   {
    size_t pTGMKB[5],i,cnt=0;
    const char* names[5] = {"TB","GB","MB","KB","Bytes"};
    cv_convert_bytes(bytes_in,pTGMKB);
    for (i=0;i<5;i++)   {
        if (pTGMKB[i]!=0) {
            if (cnt>0) printf(" ");
            printf("%lu %s",pTGMKB[i],names[i]);
            ++cnt;
        }
    }
}
#endif /* CV_NO_STDIO */
#endif /* CV_COMMON_FUNCTIONS_GUARD */

#ifndef CVH_SRIALIZER_GUARD_
/* cvh_serializer_t provides serialization/deserialization support to the cvector struct */
typedef struct cvh_serializer_t {
    unsigned char* v;
    size_t size,capacity;
    /* mutable */ size_t offset;  /* used as read-pointer in deserialization. 'mutable' is not available in plain C (and it's better not to use 'ifdef __cplusplus' here) */
#   ifndef CV_DISABLE_FAKE_MEMBER_FUNCTIONS
    void (*const reserve)(struct cvh_serializer_t* p,size_t new_capacity);
    void (*const free)(struct cvh_serializer_t* p);
    void (*const cpy)(struct cvh_serializer_t* dst,const struct cvh_serializer_t* src);
    void (*const clear)(struct cvh_serializer_t* p);
    int (*const save)(const struct cvh_serializer_t* p,const char* path);
    int (*const load)(struct cvh_serializer_t* p,const char* path);
    void (*const offset_rewind)(const struct cvh_serializer_t* d);
    void (*const offset_set)(const struct cvh_serializer_t* d,size_t offset);
    void (*const offset_advance)(const struct cvh_serializer_t* d,size_t amount);
    void (*const write_size_t)(struct cvh_serializer_t* s,size_t value);
    int (*const read_size_t)(const struct cvh_serializer_t* d,size_t* value);
    void (*const write_unsigned_char)(struct cvh_serializer_t* s,unsigned char value);
    int (*const read_unsigned_char)(const struct cvh_serializer_t* d,unsigned char* value);
    void (*const write_signed_char)(struct cvh_serializer_t* s,signed char value);
    int (*const read_signed_char)(const struct cvh_serializer_t* d,signed char* value);
    void (*const write_unsigned_short)(struct cvh_serializer_t* s,unsigned short value);
    int (*const read_unsigned_short)(const struct cvh_serializer_t* d,unsigned short* value);
    void (*const write_short)(struct cvh_serializer_t* s,short value);
    int (*const read_short)(const struct cvh_serializer_t* d,short* value);
    void (*const write_unsigned_int)(struct cvh_serializer_t* s,unsigned value);
    int (*const read_unsigned_int)(const struct cvh_serializer_t* d,unsigned* value);
    void (*const write_int)(struct cvh_serializer_t* s,int value);
    int (*const read_int)(const struct cvh_serializer_t* d,int* value);
    void (*const write_unsigned_long)(struct cvh_serializer_t* s,unsigned long value);
    int (*const read_unsigned_long)(const struct cvh_serializer_t* d,unsigned long* value);
    void (*const write_long)(struct cvh_serializer_t* s,long value);
    int (*const read_long)(const struct cvh_serializer_t* d,long* value);
    void (*const write_unsigned_long_long)(struct cvh_serializer_t* s,unsigned long long value);
    int (*const read_unsigned_long_long)(const struct cvh_serializer_t* d,unsigned long long* value);
    void (*const write_long_long)(struct cvh_serializer_t* s,long long value);
    int (*const read_long_long)(const struct cvh_serializer_t* d,long long* value);
    void (*const write_float)(struct cvh_serializer_t* s,float value);
    int (*const read_float)(const struct cvh_serializer_t* d,float* value);
    void (*const write_double)(struct cvh_serializer_t* s,double value);
    int (*const read_double)(const struct cvh_serializer_t* d,double* value);
    void (*const write_size_t_using_mipmaps)(struct cvh_serializer_t* s,size_t value);
    int (*const read_size_t_using_mipmaps)(const struct cvh_serializer_t* d,size_t* value);
    void (*const write_string)(struct cvh_serializer_t* s,const char* str_beg,const char* str_end /*=NULL*/);
    int (*const read_string)(const struct cvh_serializer_t* d,char** pstr,void* (*my_realloc)(void*,size_t)/*=NULL*/,void (*my_free)(void*)/*=NULL*/);
    void (*const write_blob)(struct cvh_serializer_t* s,const void* blob,size_t blob_size_in_bytes);
    int (*const read_blob)(const struct cvh_serializer_t* d,void** pblob,size_t* blob_size_out,void* (*my_realloc)(void*,size_t)/*=NULL*/,void (*my_free)(void*)/*=NULL*/);
#   endif /* CV_DISABLE_FAKE_MEMBER_FUNCTIONS */
#   ifdef __cplusplus
    CV_API_INL cvh_serializer_t();
    CV_API_INL cvh_serializer_t(const cvh_serializer_t& o);
    CV_API_INL cvh_serializer_t& operator=(const cvh_serializer_t& o);
    CV_API_INL ~cvh_serializer_t();
#       ifdef CV_HAS_MOVE_SEMANTICS
        CV_API_INL cvh_serializer_t(cvh_serializer_t&& o);
        CV_API_INL cvh_serializer_t& operator=(cvh_serializer_t&& o);
#       endif /* CV_HAS_MOVE_SEMANTICS */
#   endif /*__cplusplus*/
} cvh_serializer_t;
#endif /* CVH_SRIALIZER_GUARD_ */


typedef struct cvector cvector;
struct cvector {
	void * v;
	const size_t size;
	const size_t capacity;
	const size_t item_size_in_bytes;

    int (*const item_cmp)(const void*,const void*);			/* optional (can be NULL) (for sorted vectors only) */
    void (*const item_ctr)(void*);							/* optional (can be NULL) */
    void (*const item_dtr)(void*);							/* optional (can be NULL) */
    void (*const item_cpy)(void*,const void*);				/* optional (can be NULL) */
    void (*const item_serialize)(const void*,cvh_serializer_t*);    /* optional (can be NULL) */
    int  (*const item_deserialize)(void*,const cvh_serializer_t*);  /* optional (can be NULL) */

#   ifndef CV_DISABLE_FAKE_MEMBER_FUNCTIONS  /* must be defined glabally (in the Project Options)) */
    void (* const free)(cvector* v);
    void (* const clear)(cvector* v);
    void (* const shrink_to_fit)(cvector* v);
    void (* const swap)(cvector* a,cvector* b);
    void (* const reserve)(cvector* v,size_t size);
    void (* const resize)(cvector* v,size_t size);
    void (* const resize_with)(cvector* v,size_t size,const void* default_value);
    void (* const push_back)(cvector* v,const void* value);
    void (* const pop_back)(cvector* v);
    size_t (* const linear_search)(const cvector* v,const void* item_to_search,int* match);
    size_t (* const binary_search)(const cvector* v,const void* item_to_search,int* match);
    size_t (* const insert_at)(cvector* v,const void* item_to_insert,size_t position);
    size_t (* const insert_range_at)(cvector* v,const void* items_to_insert,size_t num_items_to_insert,size_t start_position);
    size_t (* const insert_sorted)(cvector* v,const void* item_to_insert,int* match,int insert_even_if_item_match);
    int (* const remove_at)(cvector* v,size_t position);
    int (* const remove_range_at)(cvector* v,size_t start_item_position,size_t num_items_to_remove);
    void (* const cpy)(cvector* a,const cvector* b);
    void (* const dbg_check)(const cvector* v);
    void (* const serialize)(const cvector* v,cvh_serializer_t* serializer);
    int  (* const deserialize)(cvector* v,const cvh_serializer_t* deserializer);
#   endif

#   ifdef __cplusplus
    cvector();
    cvector(const cvector& o);
    cvector& operator=(const cvector& o);

    template<typename T> CV_API_INL T& at(size_t i) {CV_ASSERT(i<size);T* p=(T*)v;return p[i];}
    template<typename T> CV_API_INL const T& at(size_t i) const {CV_ASSERT(i<size);const T* p=(const T*)v;return p[i];}

#   ifdef CV_HAS_MOVE_SEMANTICS
    cvector(cvector&& o);
    cvector& operator=(cvector&& o);
#   endif
    ~cvector();
#   endif
};


/* cv function declarations */ 
CV_API_DEC void cvector_free(cvector* v);
CV_API_DEC void cvector_clear(cvector* v);
CV_API_DEC void cvector_swap(cvector* a,cvector* b);
CV_API_DEC void cvector_reserve(cvector* v,size_t size);
CV_API_DEC void cvector_resize(cvector* v,size_t size);
CV_API_DEC void cvector_resize_with(cvector* v,size_t size,const void* default_value);
CV_API_DEC void cvector_push_back(cvector* v,const void* value);
CV_API_DEC void cvector_pop_back(cvector* v);
CV_API_DEC size_t cvector_linear_search(const cvector* v,const void* item_to_search,int* match);
CV_API_DEC size_t cvector_binary_search(const cvector* v,const void* item_to_search,int* match);
CV_API_DEC size_t cvector_insert_at(cvector* v,const void* item_to_insert,size_t position);
CV_API_DEC size_t cvector_insert_range_at(cvector* v,const void* items_to_insert,size_t num_items_to_insert,size_t start_position);
CV_API_DEC size_t cvector_insert_sorted(cvector* v,const void* item_to_insert,int* match,int insert_even_if_item_match);
CV_API_DEC int cvector_remove_at(cvector* v,size_t position);
CV_API_DEC int cvector_remove_range_at(cvector* v,size_t start_item_position,size_t num_items_to_remove);
CV_API_DEC void cvector_cpy(cvector* a,const cvector* b);
CV_API_DEC void cvector_shrink_to_fit(cvector* v);
CV_API_DEC void cvector_dbg_check(const cvector* v);
CV_API_DEC void cvector_serialize(const cvector* v,cvh_serializer_t* serializer);
CV_API_DEC int  cvector_deserialize(cvector* v,const cvh_serializer_t* deserializer);
CV_API_DEC void cvector_init_with(cvector* v,size_t item_size_in_bytes,int (*item_cmp)(const void*,const void*),void (*item_ctr)(void*),void (*item_dtr)(void*),void (*item_cpy)(void*,const void*),
                                  void (*item_serialize)(const void*,cvh_serializer_t*),int (*item_deserialize)(void*,const cvh_serializer_t*));
CV_API_DEC void cvector_init(cvector* v,size_t item_size_in_bytes,int (*item_cmp)(const void*,const void*));
CV_API_DEC cvector cvector_create_with(size_t item_size_in_bytes,int (*item_cmp)(const void*,const void*),void (*item_ctr)(void*),void (*item_dtr)(void*),void (*item_cpy)(void*,const void*),
                                       void (*item_serialize)(const void*,cvh_serializer_t*),int (*item_deserialize)(void*,const cvh_serializer_t*));
CV_API_DEC cvector cvector_create(size_t item_size_in_bytes,int (*item_cmp)(const void*,const void*));


/* cvh_serializer function declarations */
#ifndef CVH_SRIALIZER_GUARD_
CV_API_DEC void cvh_serializer_reserve(cvh_serializer_t* p,size_t new_capacity);
CV_API_DEC void cvh_serializer_free(cvh_serializer_t* p);
CV_API_DEC void cvh_serializer_cpy(cvh_serializer_t* dst,const cvh_serializer_t* src);
CV_API_DEC void cvh_serializer_clear(cvh_serializer_t* p);
CV_API_DEC int cvh_serializer_save(const cvh_serializer_t* p,const char* path);
CV_API_DEC int cvh_serializer_load(cvh_serializer_t* p,const char* path);
CV_API_DEC void cvh_serializer_offset_rewind(const cvh_serializer_t* d);
CV_API_DEC void cvh_serializer_offset_set(const cvh_serializer_t* d,size_t offset);
CV_API_DEC void cvh_serializer_offset_advance(const cvh_serializer_t* d,size_t amount);
CV_API_DEC void cvh_serializer_write_size_t(cvh_serializer_t* s,size_t value);
CV_API_DEC int cvh_serializer_read_size_t(const cvh_serializer_t* d,size_t* value);
CV_API_DEC void cvh_serializer_write_unsigned_char(cvh_serializer_t* s,unsigned char value);
CV_API_DEC int cvh_serializer_read_unsigned_char(const cvh_serializer_t* d,unsigned char* value);
CV_API_DEC void cvh_serializer_write_signed_char(cvh_serializer_t* s,signed char value);
CV_API_DEC int cvh_serializer_read_signed_char(const cvh_serializer_t* d,signed char* value);
CV_API_DEC void cvh_serializer_write_unsigned_short(cvh_serializer_t* s,unsigned short value);
CV_API_DEC int cvh_serializer_read_unsigned_short(const cvh_serializer_t* d,unsigned short* value);
CV_API_DEC void cvh_serializer_write_short(cvh_serializer_t* s,short value);
CV_API_DEC int cvh_serializer_read_short(const cvh_serializer_t* d,short* value);
CV_API_DEC void cvh_serializer_write_unsigned_int(cvh_serializer_t* s,unsigned value);
CV_API_DEC int cvh_serializer_read_unsigned_int(const cvh_serializer_t* d,unsigned* value);
CV_API_DEC void cvh_serializer_write_int(cvh_serializer_t* s,int value);
CV_API_DEC int cvh_serializer_read_int(const cvh_serializer_t* d,int* value);
CV_API_DEC void cvh_serializer_write_unsigned_long(cvh_serializer_t* s,unsigned long value);
CV_API_DEC int cvh_serializer_read_unsigned_long(const cvh_serializer_t* d,unsigned long* value);
CV_API_DEC void cvh_serializer_write_long(cvh_serializer_t* s,long value);
CV_API_DEC int cvh_serializer_read_long(const cvh_serializer_t* d,long* value);
CV_API_DEC void cvh_serializer_write_unsigned_long_long(cvh_serializer_t* s,unsigned long long value);
CV_API_DEC int cvh_serializer_read_unsigned_long_long(const cvh_serializer_t* d,unsigned long long* value);
CV_API_DEC void cvh_serializer_write_long_long(cvh_serializer_t* s,long long value);
CV_API_DEC int cvh_serializer_read_long_long(const cvh_serializer_t* d,long long* value);
CV_API_DEC void cvh_serializer_write_float(cvh_serializer_t* s,float value);
CV_API_DEC int cvh_serializer_read_float(const cvh_serializer_t* d,float* value);
CV_API_DEC void cvh_serializer_write_double(cvh_serializer_t* s,double value);
CV_API_DEC int cvh_serializer_read_double(const cvh_serializer_t* d,double* value);
CV_API_DEC void cvh_serializer_write_size_t_using_mipmaps(cvh_serializer_t* s,size_t value);
CV_API_DEC int cvh_serializer_read_size_t_using_mipmaps(const cvh_serializer_t* d,size_t* value);
CV_API_DEC void cvh_serializer_write_string(cvh_serializer_t* s,const char* str_beg,const char* str_end /*=NULL*/);
CV_API_DEC int cvh_serializer_read_string(const cvh_serializer_t* d,char** pstr,void* (*my_realloc)(void*,size_t)/*=NULL*/,void (*my_free)(void*)/*=NULL*/);
CV_API_DEC void cvh_serializer_write_blob(cvh_serializer_t* s,const void* blob,size_t blob_size_in_bytes);
CV_API_DEC int cvh_serializer_read_blob(const cvh_serializer_t* d,void** pblob,size_t* blob_size_out,void* (*my_realloc)(void*,size_t)/*=NULL*/,void (*my_free)(void*)/*=NULL*/);
CV_API_DEC void cvh_serializer_init(cvh_serializer_t* p);
CV_API_DEC cvh_serializer_t cvh_serializer_create(void);
#endif /* CVH_SRIALIZER_GUARD_ */


/* cvh_string_t struct and function declarations */
#ifndef CV_NO_CVH_STRING_T
#ifndef CVH_STRING_GUARD_
/* A constant, grow-only string pool. Basically you store the 'size_t' returned by 'cvh_string_push_back(...)' instead of a char* */
typedef struct cvh_string_t {
    char* v;size_t size,capacity;
#   ifndef CV_DISABLE_FAKE_MEMBER_FUNCTIONS
    void (*const reserve)(struct cvh_string_t* p,size_t new_capacity);
    size_t (*const push_back)(struct cvh_string_t* p,const char* str_beg,const char* str_end);
    void (*const free)(struct cvh_string_t* p);
    void (*const clear)(struct cvh_string_t* p);
    void (*const cpy)(struct cvh_string_t* dst,const struct cvh_string_t* src);
    void (*const serialize)(const struct cvh_string_t* p,cvh_serializer_t* s);
    int (*const deserialize)(struct cvh_string_t* p,const cvh_serializer_t* d);
#   endif /* CV_DISABLE_FAKE_MEMBER_FUNCTIONS */
#   ifdef __cplusplus
    CV_API_INL cvh_string_t();
    CV_API_INL cvh_string_t(const cvh_string_t& o);
    CV_API_INL cvh_string_t& operator=(const cvh_string_t& o);
    CV_API_INL const char* operator[](size_t i) const;
    CV_API_INL ~cvh_string_t();
#       ifdef CV_HAS_MOVE_SEMANTICS
        CV_API_INL cvh_string_t(cvh_string_t&& o);
        CV_API_INL cvh_string_t& operator=(cvh_string_t&& o);
#       endif /* CV_HAS_MOVE_SEMANTICS */
#   endif  /*__cplusplus*/
} cvh_string_t;
CV_API_DEC void cvh_string_reserve(cvh_string_t* p,size_t new_capacity);
CV_API_DEC size_t cvh_string_push_back(cvh_string_t* p,const char* str_beg,const char* str_end/*=NULL*/);
CV_API_DEC void cvh_string_free(cvh_string_t* p);
CV_API_DEC void cvh_string_cpy(cvh_string_t* dst,const cvh_string_t* src);
CV_API_DEC void cvh_string_clear(cvh_string_t* p);
CV_API_DEC void cvh_string_serialize(const cvh_string_t* p,cvh_serializer_t* s);
CV_API_DEC int cvh_string_deserialize(cvh_string_t* p,const cvh_serializer_t* d);
CV_API_DEC void cvh_string_init(cvh_string_t* p);
CV_API_DEC cvh_string_t cvh_string_create(void);
#endif /* CVH_STRING_GUARD_ */
#endif /* CV_NO_CVH_STRING_T */

#ifdef COMPILER_SUPPORTS_GCC_DIAGNOSTIC
#   pragma GCC diagnostic pop
#endif //COMPILER_SUPPORTS_GCC_DIAGNOSTIC

#endif /* C_VECTOR_TYPE_UNSAFE_H */



#if (!defined(CV_ENABLE_DECLARATION_AND_DEFINITION) || defined(C_VECTOR_TYPE_UNSAFE_IMPLEMENTATION))
#ifndef C_VECTOR_TYPE_UNSAFE_H_IMPLEMENTATION_GUARD
#define C_VECTOR_TYPE_UNSAFE_H_IMPLEMENTATION_GUARD

#ifdef COMPILER_SUPPORTS_GCC_DIAGNOSTIC
#   pragma GCC diagnostic push
#   pragma GCC diagnostic ignored "-Wpragmas"
#   pragma GCC diagnostic ignored "-Wunknown-warning-option"
#   pragma GCC diagnostic ignored "-Wclass-memaccess"
#   pragma GCC diagnostic ignored "-Wignored-qualifiers"
#endif //COMPILER_SUPPORTS_GCC_DIAGNOSTIC


/* cvh_serializer function dfinition */
#ifndef CVH_SRIALIZER_GUARD_

CV_API_DEF void cvh_serializer_reserve(cvh_serializer_t* p,size_t new_capacity)    {
    if (new_capacity>p->capacity) {
        new_capacity = new_capacity<p->capacity*3/2 ? p->capacity*3/2 : new_capacity; /* grow factor */
        cv_safe_realloc((void**)&p->v,new_capacity);CV_ASSERT(p->v);
        p->capacity = new_capacity;
    }
}
CV_API_DEF void cvh_serializer_free(cvh_serializer_t* p)    {cv_free(p->v);p->v=NULL;p->size=p->capacity=p->offset=0;}
CV_API_DEF void cvh_serializer_cpy(cvh_serializer_t* dst,const cvh_serializer_t* src)   {
    CV_ASSERT(src && dst);
    if (dst->capacity<src->size) cvh_serializer_reserve(dst,src->size);
    CV_MEMCPY(dst->v,src->v,src->size);dst->size = src->size;/*dst->offset = src->offset;*/
}
CV_API_DEF void cvh_serializer_clear(cvh_serializer_t* p)   {p->size=p->offset=0;}
CV_API_DEF int cvh_serializer_save(const cvh_serializer_t* p,const char* path)    {
#   ifndef CV_NO_STDIO
    FILE* f = fopen(path,"wb");if (f) {fwrite(p->v,p->size,1,f);fclose(f);return 1;}
#   else   /*CV_NO_STDIO*/
    CV_ASSERT(0 && "Cannot save files with the CV_NO_STDIO definition enabled");
#   endif    /*CV_NO_STDIO*/
    return 0;
}
CV_API_DEF int cvh_serializer_load(cvh_serializer_t* p,const char* path)    {
#   ifndef CV_NO_STDIO
    FILE* f=fopen(path,"rb");
    if (f) {
        size_t file_size = 0;
        fseek(f,0,SEEK_END);file_size=(size_t) ftell(f);fseek(f,0,SEEK_SET);
        cvh_serializer_free(p);   /* optional */
        cvh_serializer_reserve(p,file_size);
        fread(p->v,file_size,1,f);p->size=file_size;
        fclose(f);
        return 1;
    }
#   else   /*CV_NO_STDIO*/
    CV_ASSERT(0 && "Cannot load files with the CV_NO_STDIO definition enabled");
#   endif   /*CV_NO_STDIO*/
    return 0;
}
/* the following cvh_serializer_xxx(...) functions have never been used (not just tested, USED!) */
CV_API_DEF void cvh_serializer_offset_rewind(const cvh_serializer_t* d)  {*((size_t*)&d->offset)=0;}
CV_API_DEF void cvh_serializer_offset_set(const cvh_serializer_t* d,size_t offset)  {CV_ASSERT(offset<=d->size);*((size_t*)&d->offset)=offset;}
CV_API_DEF void cvh_serializer_offset_advance(const cvh_serializer_t* d,size_t amount)  {CV_ASSERT(d->offset+amount<=d->size);*((size_t*)&d->offset)+=amount;}
#   define CVH_SERIALIZER_WRITE(S,type,value)   {cvh_serializer_reserve(S,S->size + sizeof(type));*((type*) (&S->v[S->size])) = value;S->size+=sizeof(type);}
#   define CVH_DESERIALIZER_READ(D,type,value_ptr)   { \
       int check = (D->offset+sizeof(type)<=D->size);CV_ASSERT(check);if (!check) return 0; \
       *value_ptr = *((type*) &D->v[D->offset]);*((size_t*) &D->offset)+=sizeof(type); return 1;}
CV_API_DEF void cvh_serializer_write_size_t(cvh_serializer_t* s,size_t value) {CVH_SERIALIZER_WRITE(s,size_t,value)}
CV_API_DEF int cvh_serializer_read_size_t(const cvh_serializer_t* d,size_t* value) {CVH_DESERIALIZER_READ(d,size_t,value)}
CV_API_DEF void cvh_serializer_write_unsigned_char(cvh_serializer_t* s,unsigned char value) {CVH_SERIALIZER_WRITE(s,unsigned char,value)}
CV_API_DEF int cvh_serializer_read_unsigned_char(const cvh_serializer_t* d,unsigned char* value) {CVH_DESERIALIZER_READ(d,unsigned char,value)}
CV_API_DEF void cvh_serializer_write_signed_char(cvh_serializer_t* s,signed char value) {CVH_SERIALIZER_WRITE(s,signed char,value)}
CV_API_DEF int cvh_serializer_read_signed_char(const cvh_serializer_t* d,signed char* value) {CVH_DESERIALIZER_READ(d,signed char,value)}
CV_API_DEF void cvh_serializer_write_unsigned_short(cvh_serializer_t* s,unsigned short value) {CVH_SERIALIZER_WRITE(s,unsigned short,value)}
CV_API_DEF int cvh_serializer_read_unsigned_short(const cvh_serializer_t* d,unsigned short* value) {CVH_DESERIALIZER_READ(d,unsigned short,value)}
CV_API_DEF void cvh_serializer_write_short(cvh_serializer_t* s,short value) {CVH_SERIALIZER_WRITE(s,short,value)}
CV_API_DEF int cvh_serializer_read_short(const cvh_serializer_t* d,short* value) {CVH_DESERIALIZER_READ(d,short,value)}
CV_API_DEF void cvh_serializer_write_unsigned_int(cvh_serializer_t* s,unsigned value) {CVH_SERIALIZER_WRITE(s,unsigned,value)}
CV_API_DEF int cvh_serializer_read_unsigned_int(const cvh_serializer_t* d,unsigned* value) {CVH_DESERIALIZER_READ(d,unsigned,value)}
CV_API_DEF void cvh_serializer_write_int(cvh_serializer_t* s,int value) {CVH_SERIALIZER_WRITE(s,int,value)}
CV_API_DEF int cvh_serializer_read_int(const cvh_serializer_t* d,int* value) {CVH_DESERIALIZER_READ(d,int,value)}
CV_API_DEF void cvh_serializer_write_unsigned_long(cvh_serializer_t* s,unsigned long value) {CVH_SERIALIZER_WRITE(s,unsigned long,value)}
CV_API_DEF int cvh_serializer_read_unsigned_long(const cvh_serializer_t* d,unsigned long* value) {CVH_DESERIALIZER_READ(d,unsigned long,value)}
CV_API_DEF void cvh_serializer_write_long(cvh_serializer_t* s,long value) {CVH_SERIALIZER_WRITE(s,long,value)}
CV_API_DEF int cvh_serializer_read_long(const cvh_serializer_t* d,long* value) {CVH_DESERIALIZER_READ(d,long,value)}
CV_API_DEF void cvh_serializer_write_unsigned_long_long(cvh_serializer_t* s,unsigned long long value) {CVH_SERIALIZER_WRITE(s,unsigned long long,value)}
CV_API_DEF int cvh_serializer_read_unsigned_long_long(const cvh_serializer_t* d,unsigned long long* value) {CVH_DESERIALIZER_READ(d,unsigned long long,value)}
CV_API_DEF void cvh_serializer_write_long_long(cvh_serializer_t* s,long long value) {CVH_SERIALIZER_WRITE(s,long long,value)}
CV_API_DEF int cvh_serializer_read_long_long(const cvh_serializer_t* d,long long* value) {CVH_DESERIALIZER_READ(d,long long,value)}
CV_API_DEF void cvh_serializer_write_float(cvh_serializer_t* s,float value) {CVH_SERIALIZER_WRITE(s,float,value)}
CV_API_DEF int cvh_serializer_read_float(const cvh_serializer_t* d,float* value) {CVH_DESERIALIZER_READ(d,float,value)}
CV_API_DEF void cvh_serializer_write_double(cvh_serializer_t* s,double value) {CVH_SERIALIZER_WRITE(s,double,value)}
CV_API_DEF int cvh_serializer_read_double(const cvh_serializer_t* d,double* value) {CVH_DESERIALIZER_READ(d,double,value)}
#   undef CVH_SERIALIZER_WRITE
#   undef CVH_SERIALIZER_READ
CV_API_DEF void cvh_serializer_write_size_t_using_mipmaps(cvh_serializer_t* s,size_t value) {
    /* In most use-cases this should have less memory impact than 'cvh_serializer_write_size_t(...)' */
    int overflow=value>=255;cvh_serializer_write_unsigned_char(s,(unsigned char)(overflow ? 255 : value));if (!overflow) return;
    if (sizeof(unsigned short)==2) {overflow=value>=65535;cvh_serializer_write_unsigned_short(s,(unsigned short)(overflow ? 65535 : value));if (!overflow) return;}
    if (sizeof(unsigned int)==4) {overflow=value>=4294967295;cvh_serializer_write_unsigned_int(s,(unsigned int)(overflow ? 4294967295 : value));if (!overflow) return;}
    cvh_serializer_write_size_t(s,value);
}
CV_API_DEF int cvh_serializer_read_size_t_using_mipmaps(const cvh_serializer_t* d,size_t* value) {
    /* In most use-cases this should have less memory impact than 'cvh_serializer_read_size_t(...)' */
    int check = 0;*value=0;
    {unsigned char v=255,vmax=255;if ((check=cvh_serializer_read_unsigned_char(d,&v)) && v<vmax) {*value=v;return 1;} if (!check) return 0;}
    if (sizeof(unsigned short)==2) {unsigned short v=65535,vmax=65535;if ((check=cvh_serializer_read_unsigned_short(d,&v)) && v<vmax) {*value=v;return 1;} if (!check) return 0;}
    if (sizeof(unsigned int)==4) {unsigned int v=4294967295,vmax=4294967295;if ((check=cvh_serializer_read_unsigned_int(d,&v)) && v<vmax) {*value=v;return 1;} if (!check) return 0;}
    return cvh_serializer_read_size_t(d,value);
}
CV_API_DEF void cvh_serializer_write_string(cvh_serializer_t* s,const char* str_beg,const char* str_end /*=NULL*/)    {
    if (!str_beg) cvh_serializer_write_size_t_using_mipmaps(s,0);   /* we want to preserve NULL strings */
    else {
        const size_t str_len = str_end ? (size_t)(str_end-str_beg) : strlen(str_beg);
        const size_t str_len_plus_trailing_zero = str_len+1;
        cvh_serializer_write_size_t_using_mipmaps(s,str_len_plus_trailing_zero);
        cvh_serializer_reserve(s,s->size + str_len_plus_trailing_zero);
        CV_MEMCPY(&s->v[s->size],str_beg,str_len);s->v[s->size+str_len]='\0';
        s->size+=str_len_plus_trailing_zero;
    }
}
CV_API_DEF int cvh_serializer_read_string(const cvh_serializer_t* d,char** pstr,void* (*my_realloc)(void*,size_t)/*=NULL*/,void (*my_free)(void*)/*=NULL*/)    {
    int check=0;size_t str_len_plus_one = 0;
    CV_ASSERT(d && pstr);
    if (!cvh_serializer_read_size_t_using_mipmaps(d,&str_len_plus_one)) return 0;
    check=d->offset+str_len_plus_one<=d->size;CV_ASSERT(check);if (!check) return 0;
    if (str_len_plus_one==0)    {if (*pstr) {if (my_free) my_free(*pstr);else CV_FREE(*pstr);} return 1;}
    check = d->v[d->offset+str_len_plus_one-1]=='\0';CV_ASSERT(check); /* additional check (should we return 0?) */
    if (!(*pstr) || strlen(*pstr)<str_len_plus_one-1) {
        if (my_realloc) *pstr=(char*)my_realloc(*pstr,str_len_plus_one);
        else            *pstr=(char*)CV_REALLOC(*pstr,str_len_plus_one);
        CV_ASSERT(*pstr);
    }
    CV_MEMCPY(*pstr,&d->v[d->offset],str_len_plus_one);*((size_t*)&d->offset)+=str_len_plus_one;
    return 1;
}
CV_API_DEF void cvh_serializer_write_blob(cvh_serializer_t* s,const void* blob,size_t blob_size_in_bytes)    {
    if (!blob || blob_size_in_bytes==0) cvh_serializer_write_size_t_using_mipmaps(s,0);   /* we want to preserve NULL blobs */
    else {
        cvh_serializer_write_size_t_using_mipmaps(s,blob_size_in_bytes);
        cvh_serializer_reserve(s,s->size + blob_size_in_bytes);
        CV_MEMCPY(&s->v[s->size],blob,blob_size_in_bytes);s->size+=blob_size_in_bytes;
    }
}
CV_API_DEF int cvh_serializer_read_blob(const cvh_serializer_t* d,void** pblob,size_t* blob_size_out,void* (*my_realloc)(void*,size_t)/*=NULL*/,void (*my_free)(void*)/*=NULL*/)    {
    int check=0;size_t blob_size = 0;if (blob_size_out) *blob_size_out=0;
    CV_ASSERT(d && pblob);
    if (!cvh_serializer_read_size_t_using_mipmaps(d,&blob_size)) return 0;
    check=d->offset+blob_size<=d->size;CV_ASSERT(check);if (!check) return 0;
    if (blob_size==0)    {if (*pblob) {if (my_free) my_free(*pblob);else CV_FREE(*pblob);} return 1;}
    if (my_realloc) *pblob=(char*)my_realloc(*pblob,blob_size);
    else            *pblob=(char*)CV_REALLOC(*pblob,blob_size);
    CV_ASSERT(*pblob);
    CV_MEMCPY(*pblob,&d->v[d->offset],blob_size);*((size_t*)&d->offset)+=blob_size;
    if (blob_size_out) *blob_size_out=blob_size;
    return 1;
}
CV_API_DEF void cvh_serializer_init(cvh_serializer_t* p)    {
    CV_MEMSET(p,0,sizeof(*p));
#   ifndef CV_DISABLE_FAKE_MEMBER_FUNCTIONS
    {typedef void (*fnctype)(cvh_serializer_t*,size_t);     *((fnctype*)&p->reserve)=&cvh_serializer_reserve;}  \
    {typedef void (*fnctype)(cvh_serializer_t*);            *((fnctype*)&p->free)=&cvh_serializer_free;}    \
    {typedef void (*fnctype)(cvh_serializer_t*,const cvh_serializer_t*);            *((fnctype*)&p->cpy)=&cvh_serializer_cpy;}    \
    {typedef void (*fnctype)(cvh_serializer_t*);            *((fnctype*)&p->clear)=&cvh_serializer_clear;}    \
    {typedef int (*fnctype)(const cvh_serializer_t*,const char*);  *((fnctype*)&p->save)=&cvh_serializer_save;} \
    {typedef int (*fnctype)(cvh_serializer_t*,const char*);  *((fnctype*)&p->load)=&cvh_serializer_load;}   \
    {typedef void (*fnctype)(const cvh_serializer_t*);  *((fnctype*)&p->offset_rewind)=&cvh_serializer_offset_rewind;}  \
    {typedef void (*fnctype)(const cvh_serializer_t*,size_t);  *((fnctype*)&p->offset_set)=&cvh_serializer_offset_set;} \
    {typedef void (*fnctype)(const cvh_serializer_t*,size_t);  *((fnctype*)&p->offset_advance)=&cvh_serializer_offset_advance;} \
    {typedef void (*fnctype)(cvh_serializer_t*,size_t);  *((fnctype*)&p->write_size_t)=&cvh_serializer_write_size_t;}   \
    {typedef int (*fnctype)(const cvh_serializer_t*,size_t*);  *((fnctype*)&p->read_size_t)=&cvh_serializer_read_size_t;}   \
    {typedef void (*fnctype)(cvh_serializer_t*,unsigned char);  *((fnctype*)&p->write_unsigned_char)=&cvh_serializer_write_unsigned_char;}  \
    {typedef int (*fnctype)(const cvh_serializer_t*,unsigned char*);  *((fnctype*)&p->read_unsigned_char)=&cvh_serializer_read_unsigned_char;}  \
    {typedef void (*fnctype)(cvh_serializer_t*,signed char);  *((fnctype*)&p->write_signed_char)=&cvh_serializer_write_signed_char;}    \
    {typedef int (*fnctype)(const cvh_serializer_t*,signed char*);  *((fnctype*)&p->read_signed_char)=&cvh_serializer_read_signed_char;}    \
    {typedef void (*fnctype)(cvh_serializer_t*,unsigned short);  *((fnctype*)&p->write_unsigned_short)=&cvh_serializer_write_unsigned_short;}   \
    {typedef int (*fnctype)(const cvh_serializer_t*,unsigned short*);  *((fnctype*)&p->read_unsigned_short)=&cvh_serializer_read_unsigned_short;}   \
    {typedef void (*fnctype)(cvh_serializer_t*,short);  *((fnctype*)&p->write_short)=&cvh_serializer_write_short;}  \
    {typedef int (*fnctype)(const cvh_serializer_t*,short*);  *((fnctype*)&p->read_short)=&cvh_serializer_read_short;}  \
    {typedef void (*fnctype)(cvh_serializer_t*,unsigned);  *((fnctype*)&p->write_unsigned_int)=&cvh_serializer_write_unsigned_int;} \
    {typedef int (*fnctype)(const cvh_serializer_t*,unsigned*);  *((fnctype*)&p->read_unsigned_int)=&cvh_serializer_read_unsigned_int;} \
    {typedef void (*fnctype)(cvh_serializer_t*,int);  *((fnctype*)&p->write_int)=&cvh_serializer_write_int;}    \
    {typedef int (*fnctype)(const cvh_serializer_t*,int*);  *((fnctype*)&p->read_int)=&cvh_serializer_read_int;}    \
    {typedef void (*fnctype)(cvh_serializer_t*,unsigned long);  *((fnctype*)&p->write_unsigned_long)=&cvh_serializer_write_unsigned_long;}  \
    {typedef int (*fnctype)(const cvh_serializer_t*,unsigned long*);  *((fnctype*)&p->read_unsigned_long)=&cvh_serializer_read_unsigned_long;}  \
    {typedef void (*fnctype)(cvh_serializer_t*,long);  *((fnctype*)&p->write_long)=&cvh_serializer_write_long;} \
    {typedef int (*fnctype)(const cvh_serializer_t*,long*);  *((fnctype*)&p->read_long)=&cvh_serializer_read_long;} \
    {typedef void (*fnctype)(cvh_serializer_t*,unsigned long long);  *((fnctype*)&p->write_unsigned_long_long)=&cvh_serializer_write_unsigned_long_long;}   \
    {typedef int (*fnctype)(const cvh_serializer_t*,unsigned long long*);  *((fnctype*)&p->read_unsigned_long_long)=&cvh_serializer_read_unsigned_long_long;}   \
    {typedef void (*fnctype)(cvh_serializer_t*,long long);  *((fnctype*)&p->write_long_long)=&cvh_serializer_write_long_long;}  \
    {typedef int (*fnctype)(const cvh_serializer_t*,long long*);  *((fnctype*)&p->read_long_long)=&cvh_serializer_read_long_long;}  \
    {typedef void (*fnctype)(cvh_serializer_t*,float);  *((fnctype*)&p->write_float)=&cvh_serializer_write_float;}  \
    {typedef int (*fnctype)(const cvh_serializer_t*,float*);  *((fnctype*)&p->read_float)=&cvh_serializer_read_float;}  \
    {typedef void (*fnctype)(cvh_serializer_t*,double);  *((fnctype*)&p->write_double)=&cvh_serializer_write_double;}   \
    {typedef int (*fnctype)(const cvh_serializer_t*,double*);  *((fnctype*)&p->read_double)=&cvh_serializer_read_double;}   \
    {typedef void (*fnctype)(cvh_serializer_t*,size_t);  *((fnctype*)&p->write_size_t_using_mipmaps)=&cvh_serializer_write_size_t_using_mipmaps;}   \
    {typedef int (*fnctype)(const cvh_serializer_t*,size_t*);  *((fnctype*)&p->read_size_t_using_mipmaps)=&cvh_serializer_read_size_t_using_mipmaps;}   \
    {typedef void (*fnctype)(cvh_serializer_t*,const char*,const char*);  *((fnctype*)&p->write_string)=&cvh_serializer_write_string;}  \
    {typedef int (*fnctype)(const cvh_serializer_t*,char**,void* (*)(void*,size_t),void (*)(void*));  *((fnctype*)&p->read_string)=&cvh_serializer_read_string;}    \
    {typedef void (*fnctype)(cvh_serializer_t*,const void*,size_t);  *((fnctype*)&p->write_blob)=&cvh_serializer_write_blob;}   \
    {typedef int (*fnctype)(const cvh_serializer_t*,void**,size_t*,void* (*)(void*,size_t),void (*)(void*));  *((fnctype*)&p->read_blob)=&cvh_serializer_read_blob;}
#   endif /* CV_DISABLE_FAKE_MEMBER_FUNCTIONS */
}
CV_API_DEF cvh_serializer_t cvh_serializer_create(void) {cvh_serializer_t p;cvh_serializer_init(&p);return p;}
#   ifdef __cplusplus
#   ifndef CV_DISABLE_FAKE_MEMBER_FUNCTIONS
#       define CV_SERIALIZER_MF_CHUNK0  \
                ,reserve(&cvh_serializer_reserve),free(&cvh_serializer_free),cpy(&cvh_serializer_cpy),clear(&cvh_serializer_clear),save(&cvh_serializer_save),load(&cvh_serializer_load)   \
                ,offset_rewind(&cvh_serializer_offset_rewind),offset_set(&cvh_serializer_offset_set),offset_advance(&cvh_serializer_offset_advance),write_size_t(&cvh_serializer_write_size_t),read_size_t(&cvh_serializer_read_size_t) \
                ,write_unsigned_char(&cvh_serializer_write_unsigned_char),read_unsigned_char(&cvh_serializer_read_unsigned_char),write_signed_char(&cvh_serializer_write_signed_char),read_signed_char(&cvh_serializer_read_signed_char) \
                ,write_unsigned_short(&cvh_serializer_write_unsigned_short),read_unsigned_short(&cvh_serializer_read_unsigned_short),write_short(&cvh_serializer_write_short),read_short(&cvh_serializer_read_short) \
                ,write_unsigned_int(&cvh_serializer_write_unsigned_int),read_unsigned_int(&cvh_serializer_read_unsigned_int),write_int(&cvh_serializer_write_int),read_int(&cvh_serializer_read_int) \
                ,write_unsigned_long(&cvh_serializer_write_unsigned_long),read_unsigned_long(&cvh_serializer_read_unsigned_long),write_long(&cvh_serializer_write_long),read_long(&cvh_serializer_read_long) \
                ,write_unsigned_long_long(&cvh_serializer_write_unsigned_long_long),read_unsigned_long_long(&cvh_serializer_read_unsigned_long_long) \
                ,write_long_long(&cvh_serializer_write_long_long),read_long_long(&cvh_serializer_read_long_long) \
                ,write_float(&cvh_serializer_write_float),read_float(&cvh_serializer_read_float),write_double(&cvh_serializer_write_double),read_double(&cvh_serializer_read_double) \
                ,write_size_t_using_mipmaps(&cvh_serializer_write_size_t_using_mipmaps),read_size_t_using_mipmaps(&cvh_serializer_read_size_t_using_mipmaps) \
                ,write_string(&cvh_serializer_write_string),read_string(&cvh_serializer_read_string),write_blob(&cvh_serializer_write_blob),read_blob(&cvh_serializer_read_blob)
#   else    /*CV_DISABLE_FAKE_MEMBER_FUNCTIONS*/
#       define CV_SERIALIZER_MF_CHUNK0  /* no-op */
#   endif   /*CV_DISABLE_FAKE_MEMBER_FUNCTIONS*/
    CV_API_INL cvh_serializer_t::cvh_serializer_t() : v(NULL),size(0),capacity(0),offset(0) CV_SERIALIZER_MF_CHUNK0 {}
    CV_API_INL cvh_serializer_t::cvh_serializer_t(const cvh_serializer_t& o)  : v(NULL),size(0),capacity(0),offset(0) CV_SERIALIZER_MF_CHUNK0 {cvh_serializer_cpy(this,&o);}
    CV_API_INL cvh_serializer_t& cvh_serializer_t::operator=(const cvh_serializer_t& o) {cvh_serializer_cpy(this,&o);return *this;}
    CV_API_INL cvh_serializer_t::~cvh_serializer_t()    {cvh_serializer_free(this);}
#       ifdef CV_HAS_MOVE_SEMANTICS
        CV_API_INL cvh_serializer_t::cvh_serializer_t(cvh_serializer_t&& o) : v(o.v),size(o.size),capacity(o.capacity),offset(o.offset) CV_SERIALIZER_MF_CHUNK0 {o.v=NULL;*((size_t*)&o.size)=0;*((size_t*)&o.capacity)=0;*((size_t*)&o.offset)=0;}
        CV_API_INL cvh_serializer_t& cvh_serializer_t::operator=(cvh_serializer_t&& o)  {
            if (this != &o) {
                cvh_serializer_free(this);
                v=o.v;
                *((size_t*)&size)=o.size;*((size_t*)&capacity)=o.capacity;*((size_t*)&offset)=o.offset;
                o.v=NULL;*((size_t*)&o.size)=0;*((size_t*)&o.capacity)=0;*((size_t*)&o.offset)=0;
            }
            return *this;
        }
#       endif /* CV_HAS_MOVE_SEMANTICS */
#   endif /*__cplusplus*/
#undef CV_SERIALIZER_MF_CHUNK0

#define CVH_SRIALIZER_GUARD_
#endif /* CVH_SRIALIZER_GUARD_ */

/* cvh_string_t function definitions */
#ifndef CV_NO_CVH_STRING_T
#ifndef CVH_STRING_GUARD_
#define CVH_STRING_GUARD_
CV_API_DEF void cvh_string_reserve(cvh_string_t* p,size_t new_capacity)    {
    if (new_capacity>p->capacity) {
        new_capacity = new_capacity<p->capacity*3/2 ? p->capacity*3/2 : new_capacity; /* grow factor */
        cv_safe_realloc((void**)&p->v,new_capacity);CV_ASSERT(p->v);
        p->capacity = new_capacity;
    }
}
CV_API_DEF size_t cvh_string_push_back(cvh_string_t* p,const char* str_beg,const char* str_end/*=NULL*/)    {
    const size_t old_size = p->size;size_t len;
    CV_ASSERT(str_beg);
    len = (!str_end) ? strlen(str_beg) : (size_t)(str_end-str_beg);
    cvh_string_reserve(p,p->size+len+1);
    CV_MEMCPY(&p->v[p->size],str_beg,len);
    p->v[p->size+len]='\0';
    p->size+=len+1;
    return old_size;
}
CV_API_DEF void cvh_string_free(cvh_string_t* p)    {cv_free(p->v);p->v=NULL;p->size=p->capacity=0;}
CV_API_DEF void cvh_string_cpy(cvh_string_t* dst,const cvh_string_t* src)   {
    CV_ASSERT(src && dst);
    if (dst->capacity<src->size) cvh_string_reserve(dst,src->size);
    CV_MEMCPY(dst->v,src->v,src->size);dst->size = src->size;
}
CV_API_DEF void cvh_string_clear(cvh_string_t* p)   {p->size=0;}
CV_API_DEF void cvh_string_serialize(const cvh_string_t* p,cvh_serializer_t* s)    {
    const size_t size_t_size_in_bytes = sizeof(size_t);
    const size_t p_v_size_in_bytes = p->size;
    CV_ASSERT(p && s);
    /*if (p && p->v) {CV_ASSERT(p->size>0 && p->v[p->size-1]=='\0');}*/
    cvh_serializer_reserve(s,s->size + size_t_size_in_bytes+p_v_size_in_bytes);
    *((size_t*) &s->v[s->size]) = p->size;s->size+=size_t_size_in_bytes;
    CV_MEMCPY(&s->v[s->size],p->v,p_v_size_in_bytes);s->size+=p_v_size_in_bytes;
}
CV_API_DEF int cvh_string_deserialize(cvh_string_t* p,const cvh_serializer_t* d)    {
    const size_t size_t_size_in_bytes = sizeof(size_t);size_t psize=0;
    int check = d->offset+size_t_size_in_bytes<=d->size;CV_ASSERT(check);if (!check) return 0;
    CV_ASSERT(p && d);
    psize = *((const size_t*) &d->v[d->offset]);*((size_t*)&d->offset)+=size_t_size_in_bytes;
    check = d->offset+psize<=d->size;CV_ASSERT(check && "No space to deserialize the content of a cvh_string_t");if (!check) return 0;
    cvh_string_reserve(p,psize);CV_ASSERT(p->v);
    CV_MEMCPY(p->v,&d->v[d->offset],psize);*((size_t*)&d->offset)+=psize;p->size=psize;
    return 1;
}
CV_API_DEF void cvh_string_init(cvh_string_t* p)   {
    CV_MEMSET(p,0,sizeof(*p));
#   ifndef CV_DISABLE_FAKE_MEMBER_FUNCTIONS
    {
    typedef void (*reserve_type)(cvh_string_t*,size_t);
    typedef size_t (* push_back_type)(cvh_string_t*,const char*,const char*);
    typedef void (*clear_free_type)(cvh_string_t*);
    typedef void (*cpy_type)(cvh_string_t*,const cvh_string_t*);
    typedef void (*serialize_type)(const cvh_string_t*,cvh_serializer_t*);
    typedef int (*deserialize_type)(cvh_string_t*,const cvh_serializer_t*);
    *((reserve_type*)&p->reserve)=&cvh_string_reserve;
    *((push_back_type*)&p->push_back)=&cvh_string_push_back;
    *((clear_free_type*)&p->clear)=&cvh_string_clear;
    *((clear_free_type*)&p->free)=&cvh_string_free;
    *((cpy_type*)&p->cpy)=&cvh_string_cpy;
    *((serialize_type*)&p->serialize)=&cvh_string_serialize;
    *((deserialize_type*)&p->deserialize)=&cvh_string_deserialize;
    }
#   endif /* CV_DISABLE_FAKE_MEMBER_FUNCTIONS */
}
CV_API_DEF cvh_string_t cvh_string_create(void)   {cvh_string_t v;cvh_string_init(&v);return v;}
#ifdef __cplusplus
#   ifndef CV_DISABLE_FAKE_MEMBER_FUNCTIONS
#       define CV_CPP_STRINGT_CHUNK0    ,reserve(&cvh_string_reserve),push_back(&cvh_string_push_back),free(&cvh_string_free),clear(&cvh_string_clear),  \
                                        cpy(&cvh_string_cpy),serialize(cvh_string_serialize),deserialize(&cvh_string_deserialize)
#   else /* CV_DISABLE_FAKE_MEMBER_FUNCTIONS */
#       define CV_CPP_STRINGT_CHUNK0    /* no-op */
#   endif /* CV_DISABLE_FAKE_MEMBER_FUNCTIONS */
    CV_API_INL cvh_string_t::cvh_string_t() : v(NULL),size(0),capacity(0) CV_CPP_STRINGT_CHUNK0 {}
    CV_API_INL cvh_string_t::cvh_string_t(const cvh_string_t& o) : v(NULL),size(0),capacity(0) CV_CPP_STRINGT_CHUNK0 {cvh_string_cpy(this,&o);}
    CV_API_INL cvh_string_t& cvh_string_t::operator=(const cvh_string_t& o) {cvh_string_cpy(this,&o);return *this;}
    CV_API_INL const char* cvh_string_t::operator[](size_t i) const {CV_ASSERT(i<size);return &v[i];}
    CV_API_INL cvh_string_t::~cvh_string_t() {cvh_string_free(this);}
#   ifdef CV_HAS_MOVE_SEMANTICS
        CV_API_INL cvh_string_t::cvh_string_t(cvh_string_t&& o) : v(o.v),size(o.size),capacity(o.capacity) CV_CPP_STRINGT_CHUNK0 {o.v=NULL;*((size_t*)&o.size)=0;*((size_t*)&o.capacity)=0;}
        CV_API_INL cvh_string_t& cvh_string_t::operator=(cvh_string_t&& o)    {
            if (this != &o) {
                cvh_string_free(this);
                v=o.v;
                *((size_t*)&size)=o.size;*((size_t*)&capacity)=o.capacity;
                o.v=NULL;*((size_t*)&o.size)=0;*((size_t*)&o.capacity)=0;
            }
            return *this;
        }
#   endif /* CV_HAS_MOVE_SEMANTICS */
#   undef CV_CPP_STRINGT_CHUNK0
#endif /* __cplusplus */
#endif /* CVH_STRING_GUARD_ */
#endif /* CV_NO_CVH_STRING_T */


/* cv implementation */

/* Mandatory call at the end to free memory. The vector can be reused after this call. The function can be safely re-called multiple times  */
CV_API_DEF void cvector_free(cvector* v)	{
	if (v)	{
		if (v->v) {
            if (v->item_dtr)	{
				size_t i;
                for (i=0;i<v->size;i++)	v->item_dtr((unsigned char*)v->v+i*v->item_size_in_bytes);
			}
			cv_free(v->v);v->v=NULL;
		}	
		*((size_t*) &v->size)=0;
		*((size_t*) &v->capacity)=0;
        /* we don't clear the other fields */
	}	
}
/* Same as 'cvector_xxx_free(...)', but it does not free the memory (= the vector capacity)  */
CV_API_DEF void cvector_clear(cvector* v)	{
	if (v)	{
		if (v->v) {
            if (v->item_dtr)	{
				size_t i;
                for (i=0;i<v->size;i++)	v->item_dtr((unsigned char*)v->v+i*v->item_size_in_bytes);
			}
		}	
		*((size_t*) &v->size)=0;
	}	
}
CV_API_DEF void cvector_swap(cvector* a,cvector* b)  {
    /*unsigned char t[sizeof(cvector)];*/
    if (a!=b)   {
    CV_ASSERT(a && b);
        /*CV_MEMCPY(t,a,sizeof(cvector));
        CV_MEMCPY(a,b,sizeof(cvector));
        CV_MEMCPY(b,t,sizeof(cvector));*/
        /* nope, we just swap 3 values */
        {void* tmp=a->v;a->v=b->v;b->v=tmp;}
        {size_t tmp=a->size;*((size_t*)&a->size)=b->size;*((size_t*)&b->size)=tmp;}
        {size_t tmp=a->capacity;*((size_t*)&a->capacity)=b->capacity;*((size_t*)&b->capacity)=tmp;}
    }
}
CV_API_DEF void cvector_reserve(cvector* v,size_t size)	{
	CV_ASSERT(v);    
	/* grows-only! */
    if (size>v->capacity) {		
        const size_t new_capacity = (v->capacity==0 && size>1) ?
                    size :      /* possibly keep initial user-guided 'reserve(...)' */
                    (v->capacity+(size-v->capacity)+(v->capacity)/2);   /* our growing strategy */
        cv_safe_realloc((void** const) &v->v,new_capacity*v->item_size_in_bytes);
        *((size_t*) &v->capacity) = new_capacity;
	}
}
CV_API_DEF void cvector_resize(cvector* v,size_t size)	{
    CV_ASSERT(v);
    if (size>v->capacity) cvector_reserve(v,size);
    if (size<v->size)   {if (v->item_dtr) {size_t i;for (i=size;i<v->size;i++) v->item_dtr((unsigned char*)v->v+i*v->item_size_in_bytes);}}
    else {
#       ifdef CV_ENABLE_CLEARING_ITEM_MEMORY
        if (v->item_ctr || v->item_cpy) CV_MEMSET(v->v+v->size*v->item_size_in_bytes,0,(size-v->size)*v->item_size_in_bytes);
#       endif
        if (v->item_ctr) {size_t i;for (i=v->size;i<size;i++) v->item_ctr((unsigned char*)v->v+i*v->item_size_in_bytes);}
    }
    *((size_t*) &v->size)=size;
}
CV_API_DEF void cvector_resize_with(cvector* v,size_t size,const void* default_value)	{
    CV_ASSERT(v);
    if (!default_value) {cvector_resize(v,size);return;}
    if (size>v->capacity) cvector_reserve(v,size);
    if (size<v->size)   {if (v->item_dtr) {size_t i;for (i=size;i<v->size;i++) v->item_dtr((unsigned char*)v->v+i*v->item_size_in_bytes);}}
    else {
        size_t i;
#       ifdef CV_ENABLE_CLEARING_ITEM_MEMORY
        if (v->item_ctr || v->item_cpy) CV_MEMSET(v->v+v->size*v->item_size_in_bytes,0,(size-v->size)*v->item_size_in_bytes);
#       endif
        if (v->item_cpy)   {
            if (v->item_ctr)    {
                for (i=v->size;i<size;i++) {v->item_ctr((unsigned char*)v->v+i*v->item_size_in_bytes);v->item_cpy((unsigned char*)v->v+i*v->item_size_in_bytes,default_value);}
            }
            else    {for (i=v->size;i<size;i++) {v->item_cpy((unsigned char*)v->v+i*v->item_size_in_bytes,default_value);}}
        }
        else    {
            if (v->item_ctr)    {
                for (i=v->size;i<size;i++) {v->item_ctr((unsigned char*)v->v+i*v->item_size_in_bytes);CV_MEMCPY(v->v+i*v->item_size_in_bytes,default_value,v->item_size_in_bytes);}
            }
            else    {for (i=v->size;i<size;i++) {CV_MEMCPY(v->v+i*v->item_size_in_bytes,default_value,v->item_size_in_bytes);}}
        }
    }
    *((size_t*) &v->size)=size;
}
CV_API_DEF void cvector_push_back(cvector* v,const void* value)  {
    unsigned char *v_val = NULL;const unsigned char* pvalue = (const unsigned char*)value;
    unsigned char* p = (unsigned char*) v->v;
    CV_ASSERT(v);
    if (p && pvalue>=p && pvalue<(p+v->size))  {
        v_val = (unsigned char*) cv_malloc(v->item_size_in_bytes);
#       ifdef CV_ENABLE_CLEARING_ITEM_MEMORY
        if (v->item_ctr || v->item_cpy) CV_MEMSET(v_val,0,v->item_size_in_bytes);
#		endif
        if (v->item_ctr) v->item_ctr(v_val);
        if (v->item_cpy) v->item_cpy(v_val,value);
        else CV_MEMCPY(v_val,value,v->item_size_in_bytes);
        pvalue=v_val;
    }
    if (v->size == v->capacity) {cvector_reserve(v,v->size+1);}
    CV_ASSERT(v->v);
#   ifdef CV_ENABLE_CLEARING_ITEM_MEMORY
    if (v->item_ctr || v->item_cpy) CV_MEMSET(v->v+v->size*v->item_size_in_bytes,0,v->item_size_in_bytes);
#	endif
    if (v->item_ctr) v->item_ctr((unsigned char*)v->v+v->size*v->item_size_in_bytes);
    if (v->item_cpy) v->item_cpy((unsigned char*)v->v+v->size*v->item_size_in_bytes,pvalue);
    else CV_MEMCPY(v->v+v->size*v->item_size_in_bytes,pvalue,v->item_size_in_bytes);

    if (v_val) {
        if (v->item_dtr) v->item_dtr(v_val);
        cv_free(v_val);v_val=NULL;
    }
    *((size_t*) &v->size)=v->size+1;
}
CV_API_DEF void cvector_pop_back(cvector* v)	{
   CV_ASSERT(v && v->size>0);
   if (v->size>0) {*((size_t*) &v->size)=v->size-1;if (v->item_dtr) v->item_dtr((unsigned char*)v->v+v->size*v->item_size_in_bytes);}
}
CV_API_DEF size_t cvector_linear_search(const cvector* v,const void* item_to_search,int* match)  {
    int cmp=0;size_t i;const unsigned char* p;
    int (* const item_cmp)(const void*,const void*) = v->item_cmp;
    CV_ASSERT(v && item_cmp);
    p = (const unsigned char*) v->v;
    if (match) *match=0;
    if (v->size==0) return 0;  /* otherwise match will be 1 */
    for (i = 0; i < v->size; i++) {
        cmp = item_cmp(item_to_search,p+i*v->item_size_in_bytes);
        if (cmp<=0) {
            if (cmp==0 && match) *match=1;
            return i;
        }
    }
    CV_ASSERT(i==v->size);
    return i;
}
CV_API_DEF size_t cvector_binary_search(const cvector* v,const void* item_to_search,int* match)  {
    size_t first=0, last;
    size_t mid;int cmp;const unsigned char* p;
    int (* const item_cmp)(const void*,const void*) = v->item_cmp;
    CV_ASSERT(v && item_cmp);
    p = (const unsigned char*) v->v;
    if (match) *match=0;
    if (v->size==0) return 0;  /* otherwise match will be 1 */
	last=v->size-1;
    while (first <= last) {
        mid = (first + last) / 2;
        cmp = item_cmp(item_to_search,p+mid*v->item_size_in_bytes);
        if (cmp>0) {
            first = mid + 1;
        }
        else if (cmp<0) {
            if (mid==0) return 0;
            last = mid - 1;
        }
        else {if (match) *match=1;CV_ASSERT(mid<v->size); return mid;}
    }
    CV_ASSERT(mid<v->size);
    return cmp>0 ? (mid+1) : mid;
}
CV_API_DEF size_t cvector_insert_range_at(cvector* v,const void* items_to_insert,size_t num_items_to_insert,size_t start_position)  {
    /* position is in [0,v->size] */
    const size_t end_position = start_position+num_items_to_insert;size_t i;
    unsigned char* v_val=NULL;const unsigned char* pitems=(const unsigned char*) items_to_insert;
    unsigned char* p;
    CV_ASSERT(v && start_position<=v->size);
    p = (unsigned char*) v->v;

    if (num_items_to_insert==0) return start_position;
    if (p && (pitems+num_items_to_insert)>=p && pitems<(p+v->size))  {
        v_val = (unsigned char*) cv_malloc(num_items_to_insert*v->item_size_in_bytes);
#		ifdef CV_ENABLE_CLEARING_ITEM_MEMORY
        if (v->item_ctr || v->item_cpy) CV_MEMSET(v_val,0,num_items_to_insert*v->item_size_in_bytes);
#		endif
        if (v->item_cpy)	{
            if (v->item_ctr)	{
                for (i=0;i<num_items_to_insert;i++)   {
                    v->item_ctr(v_val+i*v->item_size_in_bytes);
                    v->item_cpy(v_val+i*v->item_size_in_bytes,pitems+i*v->item_size_in_bytes);
                }
            }
            else	{for (i=0;i<num_items_to_insert;i++)   v->item_cpy(v_val+i*v->item_size_in_bytes,pitems+i*v->item_size_in_bytes);}
        }
        else	{
            if (v->item_ctr)	{for (i=0;i<num_items_to_insert;i++)   v->item_ctr(v_val+i*v->item_size_in_bytes);}
            CV_MEMCPY(v_val,pitems,num_items_to_insert*v->item_size_in_bytes);
        }
        pitems = v_val;
    }
    if (v->size+num_items_to_insert > v->capacity) {
        cvector_reserve(v,v->size+num_items_to_insert);
        p = (unsigned char*) v->v;
    }
    if (start_position<v->size) CV_MEMMOVE(p+end_position*v->item_size_in_bytes,p+start_position*v->item_size_in_bytes,(v->size-start_position)*v->item_size_in_bytes);
#	ifdef CV_ENABLE_CLEARING_ITEM_MEMORY
    if (v->item_ctr || v->item_cpy) CV_MEMSET(p+start_position*v->item_size_in_bytes,0,num_items_to_insert*v->item_size_in_bytes);
#	endif
    if (v->item_cpy)	{
        if (v->item_ctr)	{
            for (i=start_position;i<end_position;i++)   {
                v->item_ctr(p+i*v->item_size_in_bytes);
                v->item_cpy(p+i*v->item_size_in_bytes,pitems+(i-start_position)*v->item_size_in_bytes);
            }
        }
        else	{for (i=start_position;i<end_position;i++)   v->item_cpy(p+i*v->item_size_in_bytes,pitems+(i-start_position)*v->item_size_in_bytes);}
    }
    else	{
        if (v->item_ctr)	{for (i=start_position;i<end_position;i++)   v->item_ctr(p+i*v->item_size_in_bytes);}
        CV_MEMCPY(p+start_position*v->item_size_in_bytes,pitems,num_items_to_insert*v->item_size_in_bytes);
    }
    if (v_val) {
        if (v->item_dtr)	{for (i=0;i<num_items_to_insert;i++)   v->item_dtr(p+i*v->item_size_in_bytes);}
        cv_free(v_val);v_val=NULL;
    }
    *((size_t*) &v->size)=v->size+num_items_to_insert;
    return start_position;
 }	
CV_API_DEF size_t cvector_insert_at(cvector* v,const void* item_to_insert,size_t position)  {return cvector_insert_range_at(v,item_to_insert,1,position);}
CV_API_DEF size_t cvector_insert_sorted(cvector* v,const void* item_to_insert,int* match,int insert_even_if_item_match)  {
    int my_match = 0;size_t position;
    position = cvector_binary_search(v,item_to_insert,&my_match);
    if (match) *match = my_match;
    if (my_match && !insert_even_if_item_match) return position;
    cvector_insert_at(v,item_to_insert,position);
    return position;
}
CV_API_DEF int cvector_remove_at(cvector* v,size_t position)  {
    /* position is in [0,num_items) */
    int removal_ok;
    void (*item_dtr)(void*) = v ? v->item_dtr : NULL;
    CV_ASSERT(v);
	removal_ok = (position<v->size) ? 1 : 0;
    CV_ASSERT(removal_ok);	/* error: position>=v->size */
	if (removal_ok)	{
        if (item_dtr) item_dtr((unsigned char*)v->v+position*v->item_size_in_bytes);
        CV_MEMMOVE(v->v+position*v->item_size_in_bytes,(unsigned char*)v->v+(position+1)*v->item_size_in_bytes,(v->size-position-1)*v->item_size_in_bytes);
        *((size_t*) &v->size)=v->size-1;
    }
    return removal_ok;
}
CV_API_DEF int cvector_remove_range_at(cvector* v,size_t start_item_position,size_t num_items_to_remove)  {
    /* (start_item_position+num_items_to_remove) is <= size */
    const size_t end_item_position = start_item_position+num_items_to_remove;
    int removal_ok;size_t i;
    void (*item_dtr)(void*) = v ? v->item_dtr : NULL;
    CV_ASSERT(v);
    removal_ok = end_item_position<=v->size ? 1 : 0;
    CV_ASSERT(removal_ok);	/* error: start_item_position + num_items_to_remove > v.size */	
    if (removal_ok && num_items_to_remove>0)	{
        if (item_dtr) {for (i=start_item_position;i<end_item_position;i++) item_dtr((unsigned char*)v->v+i*v->item_size_in_bytes);}
        if (end_item_position<v->size) CV_MEMMOVE(v->v+start_item_position*v->item_size_in_bytes,(unsigned char*)v->v+end_item_position*v->item_size_in_bytes,(v->size-end_item_position)*v->item_size_in_bytes);
        *((size_t*) &v->size)=v->size-num_items_to_remove;
    }
    return removal_ok;
}
CV_API_DEF void cvector_cpy(cvector* a,const cvector* b) {
    size_t i;
    /*typedef int (*item_cmp_type)(const void*,const void*);
	typedef void (*item_ctr_dtr_type)(void*);
    typedef void (*item_cpy_type)(void*,const void*);*/
    if (a==b || (a->size==0 && b->size==0)) return;
    CV_ASSERT(a && b);
    /* bad init asserts */
    CV_ASSERT(!(a->v && a->capacity==0));
    CV_ASSERT(!(!a->v && a->capacity>0));
    CV_ASSERT(!(b->v && b->capacity==0));
    CV_ASSERT(!(!b->v && b->capacity>0));
    /*if (a->item_size_in_bytes==0) *((size_t*)&a->item_size_in_bytes)=b->item_size_in_bytes;*/
    /*CV_ASSERT(a->item_size_in_bytes==b->item_size_in_bytes);*/   /* can't cpy two different vectors! */
    /*if (a->item_size_in_bytes!=b->item_size_in_bytes)   {
#       ifndef CV_NO_STDIO
        fprintf(stderr,"[cvector_cpy] Error: two vector with different 'item_size_in_bytes' (%lu != %lu) can't be copied.\n",a->item_size_in_bytes,b->item_size_in_bytes);
#       endif
        return;
    }*/
    /*cv_free(a);*/
    cvector_clear(a);
    /* *((item_cmp_type*)&a->item_cmp)=b->item_cmp;
	*((item_ctr_dtr_type*)&a->item_ctr)=b->item_ctr;
    *((item_ctr_dtr_type*)&a->item_dtr)=b->item_dtr;
    *((item_cpy_type*)&a->item_cpy)=b->item_cpy; */
    CV_ASSERT(a->item_size_in_bytes==b->item_size_in_bytes && a->item_ctr==b->item_ctr && a->item_dtr==b->item_dtr && a->item_cpy==b->item_cpy
              /*&& a->item_serialize==b->item_serialize &&  a->item_deserialize==b->item_deserialize*/
              && "One of the two vectors has not been properly initialized");
    cvector_resize(a,b->size);
    CV_ASSERT(((a->v && b->v) || (!a->v && !b->v)) && a->size==b->size);
    if (!a->item_cpy)   {CV_MEMCPY(a->v,(const unsigned char*)b->v,a->size*a->item_size_in_bytes);}
    else    {for (i=0;i<a->size;i++) a->item_cpy((unsigned char*)a->v+i*a->item_size_in_bytes,(const unsigned char*)b->v+i*b->item_size_in_bytes);}
}
CV_API_DEF void cvector_shrink_to_fit(cvector* v)	{
    if (v)	{
        cvector o = cvector_create_with(v->item_size_in_bytes,v->item_cmp,v->item_ctr,v->item_dtr,v->item_cpy,v->item_serialize,v->item_deserialize);
        cvector_cpy(&o,v); /* now 'o' is 'v' trimmed */
        cvector_free(v);
        cvector_swap(&o,v);
    }
}
CV_API_DEF void cvector_dbg_check(const cvector* v)  {
    size_t j,num_sorting_errors=0;
    const size_t mem_minimal=sizeof(cvector)+v->item_size_in_bytes*v->size;
    const size_t mem_used=sizeof(cvector)+v->item_size_in_bytes*v->capacity;
    const double mem_used_percentage = (double)mem_used*100.0/(double)mem_minimal;
    CV_ASSERT(v);
    /* A potemtial problem here is that sometimes users set a 'v->item_cmp' without using it in a sorted vector...
       So in case of sorting errors, we don't assert, but still display them using fprintf(stderr,...) */
    if (v->item_cmp && v->size)    {
        const unsigned char* last_item = NULL;
        for (j=0;j<v->size;j++)  {
            const unsigned char* item = (const unsigned char*)v->v+j*v->item_size_in_bytes;
            if (last_item) {
                if (v->item_cmp(last_item,item)>0) {
                    /* When this happens, it can be a wrong user 'item_cmp' function (that cannot sort items in a consistent way) */                    
                    ++num_sorting_errors;
#                   ifndef CV_NO_STDIO
                    fprintf(stderr,"[cvector_dbg_check] Sorting Error (%lu): item_cmp(%lu,%lu)>0 [num_items=%lu]\n",num_sorting_errors,j-1,j,v->size);
#                   endif
                }
            }
            last_item=item;
        }
    }
#   ifndef CV_NO_STDIO
    printf("[cvector_dbg_check]:\n");
    printf("\tsize: %lu. capacity: %lu. sizeof(item): %lu\n",v->size,v->capacity,v->item_size_in_bytes);
    if (v->item_cmp && v->size) {
        if (num_sorting_errors==0) printf("\tsorting: OK.\n");
        else printf("\tsorting: NO (%lu sorting errors detected).\n",num_sorting_errors);
    }
    printf("\tmemory_used: ");cv_display_bytes(mem_used);
    printf(". memory_minimal_possible: ");cv_display_bytes(mem_minimal);
    printf(". mem_used_percentage: %1.2f%% (100%% is the best possible result).\n",mem_used_percentage);
#   endif
    /*CV_ASSERT(num_sorting_errors==0);*/ /* When this happens, it can be a wrong user 'itemKey_cmp' function (that cannot sort keys in a consistent way) */
}
CV_API_DEF void cvector_serialize(const cvector* v,cvh_serializer_t* serializer)    {
    const size_t size_t_size_in_bytes = sizeof(size_t);
    CV_ASSERT(v && serializer);
    cvh_serializer_reserve(serializer,serializer->size + size_t_size_in_bytes); /* space reserved for v->size (size_t_size_in_bytes) */
    CV_ASSERT(serializer->v);
    *((size_t*) (&serializer->v[serializer->size])) = v->size;serializer->size+=size_t_size_in_bytes; /* v->size written, now the items: */
    if (v->item_serialize)  {
        size_t i;for(i=0;i<v->size;i++) {
            const unsigned char* item = (const unsigned char*)v->v+i*v->item_size_in_bytes;
            v->item_serialize(item,serializer); /* serializer->size is incremented by 'v->item_serialize' */
        }
    }
    else {
        const size_t v_size_in_bytes = v->size*v->item_size_in_bytes;
        cvh_serializer_reserve(serializer,serializer->size + v_size_in_bytes); /* space reserved for all the items (v_size_in_bytes) */
        CV_ASSERT(serializer->v);
        CV_MEMCPY(&serializer->v[serializer->size],v->v,v_size_in_bytes);serializer->size+=v_size_in_bytes; /* serializer->size must be incremented */
    }
}
CV_API_DEF int  cvector_deserialize(cvector* v,const cvh_serializer_t* deserializer)    {
    /* We must start deserialization from the 'mutable' reader offset: 'deserializer->offset', and then increment it step by step */
    const size_t size_t_size_in_bytes = sizeof(size_t);
    size_t vsize;int check;
    CV_ASSERT(v && deserializer);
    check = (deserializer->offset+size_t_size_in_bytes<=deserializer->size);
    CV_ASSERT(check && "missing space to deserialize 'size_t_size_in_bytes'");  /* otherwise deserialization will fail */
    if (!check) return 0; /* but when we compile with NDEBUG, or CV_NO_ASSERT, the caller must be notified of the failure */
    vsize = *((size_t*) &deserializer->v[deserializer->offset]);*((size_t*) &deserializer->offset)+=size_t_size_in_bytes; /* 'vsize' read, 'deserializer->offset' incremented */
    cvector_resize(v,vsize);/*CV_ASSERT(v->size==vsize);*/ /* now that 'v->size==vsize', we can start deserializing items: */
    CV_ASSERT(v->v);
    if (v->item_deserialize)  {
        size_t i;for(i=0;i<vsize;i++) {
            unsigned char* item = (unsigned char*) v->v+i*v->item_size_in_bytes;
            if (!v->item_deserialize(item,deserializer)) return 0;    /* 'deserializer->offset' is incremented by 'v->item_deserialize' */
        }
    }
    else {
        size_t v_size_in_bytes = vsize*v->item_size_in_bytes;
        check = deserializer->offset+v_size_in_bytes<=deserializer->size;
        CV_ASSERT(check && "missing space to deserialize all items using CV_MEMCPY");  /* otherwise deserialization will fail */
        if (!check) return 0;
        CV_MEMCPY(v->v,&deserializer->v[deserializer->offset],v_size_in_bytes);
        *((size_t*) &deserializer->offset)+=v_size_in_bytes; /* 'deserializer->offset' must be incremented */
    }
    return 1; /* we must return 1 on success */
}

/* create methods */
CV_API_DEF void cvector_init_with(cvector* v,size_t item_size_in_bytes,int (*item_cmp)(const void*,const void*),void (*item_ctr)(void*),void (*item_dtr)(void*),void (*item_cpy)(void*,const void*),
                                  void (*item_serialize)(const void*,cvh_serializer_t*),int (*item_deserialize)(void*,const cvh_serializer_t*))	{
    typedef int (*item_cmp_type)(const void*,const void*);
	typedef void (*item_ctr_dtr_type)(void*);
	typedef void (*item_cpy_type)(void*,const void*);
    typedef void (*item_serialize_type)(const void*,cvh_serializer_t*);
    typedef int (*item_deserialize_type)(void*,const cvh_serializer_t*);
#   ifndef CV_DISABLE_FAKE_MEMBER_FUNCTIONS  /* must be defined glabally (in the Project Options)) */
    typedef void (* free_clear_shrink_to_fit_pop_back_mf)(cvector*);
    typedef void (* swap_mf)(cvector*,cvector*);
    typedef void (* reserve_mf)(cvector*,size_t);
    typedef void (* resize_mf)(cvector*,size_t);
    typedef void (* resize_with_mf)(cvector*,size_t,const void*);
    typedef void (* push_back_mf)(cvector*,const void*);
    typedef size_t (* search_mf)(const cvector*,const void*,int*);
    typedef size_t (* insert_at_mf)(cvector*,const void*,size_t);
    typedef size_t (* insert_range_at_mf)(cvector*,const void*,size_t,size_t);
    typedef size_t (* insert_sorted_mf)(cvector*,const void*,int*,int);
    typedef int (* remove_at_mf)(cvector*,size_t);
    typedef int (* remove_range_at_mf)(cvector*,size_t,size_t);
    typedef void (* cpy_mf)(cvector*,const cvector*);
    typedef void (* dbg_check_mf)(const cvector*);
#   endif
    CV_ASSERT(v);
    CV_ASSERT(item_size_in_bytes>0);
    CV_MEMSET(v,0,sizeof(cvector));
    *((size_t*)&v->item_size_in_bytes)=item_size_in_bytes;
    *((item_cmp_type*)&v->item_cmp)=item_cmp;
	*((item_ctr_dtr_type*)&v->item_ctr)=item_ctr;
	*((item_ctr_dtr_type*)&v->item_dtr)=item_dtr;
	*((item_cpy_type*)&v->item_cpy)=item_cpy; 
    *((item_serialize_type*)&v->item_serialize)=item_serialize;
    *((item_deserialize_type*)&v->item_deserialize)=item_deserialize;
#   ifndef CV_DISABLE_FAKE_MEMBER_FUNCTIONS  /* must be defined glabally (in the Project Options)) */
    *((free_clear_shrink_to_fit_pop_back_mf*)&v->free)=&cvector_free;
    *((free_clear_shrink_to_fit_pop_back_mf*)&v->clear)=&cvector_clear;
    *((free_clear_shrink_to_fit_pop_back_mf*)&v->shrink_to_fit)=&cvector_shrink_to_fit;
    *((swap_mf*)&v->swap)=&cvector_swap;
    *((reserve_mf*)&v->reserve)=&cvector_reserve;
    *((resize_mf*)&v->resize)=&cvector_resize;
    *((resize_with_mf*)&v->resize_with)=&cvector_resize_with;
    *((push_back_mf*)&v->push_back)=&cvector_push_back;
    *((free_clear_shrink_to_fit_pop_back_mf*)&v->pop_back)=&cvector_pop_back;
    *((search_mf*)&v->linear_search)=&cvector_linear_search;
    *((search_mf*)&v->binary_search)=&cvector_binary_search;
    *((insert_at_mf*)&v->insert_at)=&cvector_insert_at;
    *((insert_range_at_mf*)&v->insert_range_at)=&cvector_insert_range_at;
    *((insert_sorted_mf*)&v->insert_sorted)=&cvector_insert_sorted;
    *((remove_at_mf*)&v->remove_at)=&cvector_remove_at;
    *((remove_range_at_mf*)&v->remove_range_at)=&cvector_remove_range_at;
    *((cpy_mf*)&v->cpy)=&cvector_cpy;
    *((dbg_check_mf*)&v->dbg_check)=&cvector_dbg_check;
#   endif
}
CV_API_DEF void cvector_init(cvector* v,size_t item_size_in_bytes,int (*item_cmp)(const void*,const void*))	{cvector_init_with(v,item_size_in_bytes,item_cmp,NULL,NULL,NULL,NULL,NULL);}
CV_API_DEF cvector cvector_create_with(size_t item_size_in_bytes,int (*item_cmp)(const void*,const void*),void (*item_ctr)(void*),void (*item_dtr)(void*),void (*item_cpy)(void*,const void*),
                                       void (*item_serialize)(const void*,cvh_serializer_t*),int (*item_deserialize)(void*,const cvh_serializer_t*))	{cvector v;cvector_init_with(&v,item_size_in_bytes,item_cmp,item_ctr,item_dtr,item_cpy,item_serialize,item_deserialize);return v;}
CV_API_DEF cvector cvector_create(size_t item_size_in_bytes,int (*item_cmp)(const void*,const void*))	{return cvector_create_with(item_size_in_bytes,item_cmp,NULL,NULL,NULL,NULL,NULL);}

#ifdef __cplusplus
    cvector::cvector() :
    v(NULL),size(0),capacity(0),item_size_in_bytes(0),
    item_cmp(NULL),item_ctr(NULL),item_dtr(NULL),item_cpy(NULL),item_serialize(NULL),item_deserialize(NULL),
    free(&cvector_free),clear(&cvector_clear),shrink_to_fit(&cvector_shrink_to_fit),swap(&cvector_swap),
    reserve(&cvector_reserve),resize(&cvector_resize),resize_with(&cvector_resize_with),
    push_back(&cvector_push_back),pop_back(&cvector_pop_back),
    linear_search(&cvector_linear_search),binary_search(&cvector_binary_search),
    insert_at(&cvector_insert_at),insert_range_at(&cvector_insert_range_at),insert_sorted(&cvector_insert_sorted),
    remove_at(&cvector_remove_at),remove_range_at(&cvector_remove_range_at),
    cpy(&cvector_cpy),dbg_check(&cvector_dbg_check),serialize(&cvector_serialize),deserialize(&cvector_deserialize)
    {}

    cvector::cvector(const cvector& o) :
    v(NULL),size(0),capacity(0),item_size_in_bytes(o.item_size_in_bytes),
    item_cmp(o.item_cmp),item_ctr(o.item_ctr),item_dtr(o.item_dtr),item_cpy(o.item_cpy),item_serialize(NULL),item_deserialize(NULL),
    free(&cvector_free),clear(&cvector_clear),shrink_to_fit(&cvector_shrink_to_fit),swap(&cvector_swap),
    reserve(&cvector_reserve),resize(&cvector_resize),resize_with(&cvector_resize_with),
    push_back(&cvector_push_back),pop_back(&cvector_pop_back),
    linear_search(&cvector_linear_search),binary_search(&cvector_binary_search),
    insert_at(&cvector_insert_at),insert_range_at(&cvector_insert_range_at),insert_sorted(&cvector_insert_sorted),
    remove_at(&cvector_remove_at),remove_range_at(&cvector_remove_range_at),
    cpy(&cvector_cpy),dbg_check(&cvector_dbg_check),serialize(&cvector_serialize),deserialize(&cvector_deserialize)
    {
        cvector_cpy(this,&o);
    }

    cvector& cvector::operator=(const cvector& o)    {cvector_cpy(this,&o);return *this;}

#   ifdef CV_HAS_MOVE_SEMANTICS
    cvector::cvector(cvector&& o) :
    v(o.v),size(o.size),capacity(o.capacity),item_size_in_bytes(o.item_size_in_bytes),
    item_cmp(o.item_cmp),item_ctr(o.item_ctr),item_dtr(o.item_dtr),item_cpy(o.item_cpy),item_serialize(o.item_serialize),item_deserialize(o.item_deserialize),
    free(&cvector_free),clear(&cvector_clear),shrink_to_fit(&cvector_shrink_to_fit),swap(&cvector_swap),
    reserve(&cvector_reserve),resize(&cvector_resize),resize_with(&cvector_resize_with),
    push_back(&cvector_push_back),pop_back(&cvector_pop_back),
    linear_search(&cvector_linear_search),binary_search(&cvector_binary_search),
    insert_at(&cvector_insert_at),insert_range_at(&cvector_insert_range_at),insert_sorted(&cvector_insert_sorted),
    remove_at(&cvector_remove_at),remove_range_at(&cvector_remove_range_at),
    cpy(&cvector_cpy),dbg_check(&cvector_dbg_check),serialize(&cvector_serialize),deserialize(&cvector_deserialize)
    {
        o.v=NULL;*((size_t*)&o.size)=0;*((size_t*)&o.capacity)=0;
    }

    cvector& cvector::operator=(cvector&& o)    {
        if (this != &o) {
            cvector_free(this);
            v=o.v;
            *((size_t*)&size)=o.size;*((size_t*)&capacity)=o.capacity;
            o.v=NULL;*((size_t*)&o.size)=0;*((size_t*)&o.capacity)=0;
        }
        return *this;
    }
#   endif

    cvector::~cvector() {cvector_free(this);}
#endif

#ifdef COMPILER_SUPPORTS_GCC_DIAGNOSTIC
#   pragma GCC diagnostic pop
#endif //COMPILER_SUPPORTS_GCC_DIAGNOSTIC

#endif /* (!defined(CV_ENABLE_DECLARATION_AND_DEFINITION) || defined(C_VECTOR_TYPE_UNSAFE_IMPLEMENTATION)) */
#endif /* C_VECTOR_TYPE_UNSAFE_H_IMPLEMENTATION_GUARD */


