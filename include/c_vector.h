/* https://github.com/Flix01/c_vector_and_hashtable */
/*==================================================================================*/
/* Plain C implementation of std::vector */
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

/* USAGE: Please see the bundled "c_vector_main.c". */

/*
   GLOBAL DEFINITIONS: The following definitions (when used) must be set
   globally (= in the Project Options or in a StdAfx.h file):

   CV_DISABLE_FAKE_MEMBER_FUNCTIONS     // it disables "fake-member-function-syntax" (e.g. v.push_back(&v,item);). Use it to improve performance and reduce memory.
   CV_ENABLE_CLEARING_ITEM_MEMORY       // enable it if you want that, before each item is constructed (and before the user-provided item_ctr function, if present, is called), the item memory is cleared to zero to increase code robustness (but it slows down performance).
   CV_USE_VOID_PTRS_IN_CMP_FCT          // define this if you want to share vector item compare functions (when used) with c style functions like qsort. Basically you need to use const void* pointers as arguments.
   CV_FORCE_MEMCPY_S                    // it enforces the use of memcpy_s(...) and similar functions (but the standard and safer way is to define __STDC_WANT_LIB_EXT1__. Please see: https://en.cppreference.com/w/c/string/byte/memcpy).
   CV_NO_MEMCPY_S                       // it forces the use of memcpy(...) and similar functions, in cases where memcpy_s(...) is used by default (currently only Visual Studio 2005 (8.0) or newer).
   CV_NO_CVH_STRING_T                   // it disables the 'cvh_string_t' struct and functions.
   CV_NO_PLACEMENT_NEW                  // (c++ mode only) it does not define helper stuff like: CV_PLACEMENT_NEW, cpp_ctr,cpp_dtr,cpp_cpy,cpp_cmp.
   CV_MALLOC
   CV_REALLOC
   CV_FREE
   CV_ASSERT
   CV_NO_ASSERT
   CV_NO_STDIO
   CV_NO_STDLIB
   CV_API_INL                           // this simply defines the 'inline' keyword syntax (defaults to __inline).
   CV_API                               // used always when CV_ENABLE_DECLARATION_AND_DEFINITION is not defined and in some global or private functions otherwise.
   CV_API_DEC                           // defaults to CV_API, or to 'CV_API_INL extern' if CV_ENABLE_DECLARATION_AND_DEFINITION is defined.
   CV_APY_DEF                           // defaults to CV_API, or to nothing if CV_ENABLE_DECLARATION_AND_DEFINITION is defined.
*/

#ifndef C_VECTOR_H_
#define C_VECTOR_H_


#ifndef C_VECTOR_VERSION
#define C_VECTOR_VERSION        "1.15 rev4"
#define C_VECTOR_VERSION_NUM    0115
#endif


/* HISTORY:
   C_VECTOR_VERSION_NUM 115 rev3
   -> added the CV_SIZE_T_FORMATTING definition (internal usage).

   C_VECTOR_VERSION_NUM 115 rev3
   -> replaced a few "naked" asserts with CV_ASSERT.
   -> added some additional guards in some parts of the code
      (so that we can mix "c_vector.h" and "c_vector_type_unsafe.h" together... at least I hope so)

   C_VECTOR_VERSION_NUM 115 rev2
   -> fixed compilation with Visual C++ 7.1 2003 by removing two semicolons after macro calls.

   C_VECTOR_VERSION_NUM 115
   -> removed definition CV_ENABLE_UNTESTED_FEATURES, enabling everything by default (although it still lacks some testing).
   -> removed definitions CV_ENABLE_EXTRA_FEATURES.
   -> added definition CV_NO_CVH_STRING_T.
   -> added an example that uses 'cvh_string_t'.
   -> renamed cv_xxx_create(...) -> cv_xxx_init(...), and cv_xxx_create_with(...) -> cv_xxx_init_with(...).
   -> added cv_xxx_create(...) and cv_xxx_create_with(...) that RETURN BY VALUE an INITED cv_xxx.
   -> renamed cvh_serializer_destroy(...) -> cvh_serializer_free(...).
   -> added cvh_serializer_init(...),cvh_serializer_create(...) and fake member functions support for cvh_serializer_t.
   -> renamed cvh_string_destroy(...) -> cvh_string_free(...).
   -> added cvh_string_init(...),cvh_string_create(...) and fake member functions support for cvh_string_t.
   -> remove the CV_ZERO_INIT macro. Good choice! Now all the cv_xxx (and cvh_xxx) structs MUST be explicitly initialized
      using cv_xxx_init(...) or cv_xxx_create(...). This choice enforces a general good and precise programming practice.
   -> removed definition CV_DISABLE_CLEARING_ITEM_MEMORY. Now it's hard-coded by default.
   -> added definition CV_ENABLE_CLEARING_ITEM_MEMORY instead.
   -> removed definition CV_ZERO_INIT. It can be readded with the CV_ADD_ZERO_INIT_DEFINITION definition.

   C_VECTOR_VERSION_NUM 114 rev2
   -> Fixed cvh_serialization_write_string(...)/cvh_serialization_read_string(...)
   -> "c_vector_main.c": when CV_ENABLE_UNTESTED_FEATURES is defined, cvh_serialization_write_xxx(...)/cvh_serialization_read_xxx(...) are used.

   C_VECTOR_VERSION_NUM 114
   -> restored definition CV_DISABLE_FAKE_MEMBER_FUNCTIONS
   -> added serialization/deserialization support:
        -> now cv_xxx_create_with(...) takes two additional arguments: item_serialization and item_deserialization function pointers.
           If they are NULL, items will be serialized/deserialized using plain memcpy calls (good only for plain item structs without pointers or vectors inside).
        -> a new helper struct cvh_serializer_t must be used to serialize/deserialize vectors (and items if needed) (please see "c_vector_main.c").
        -> code near the bottom of this file (search backwards for "(CV_TYPE,_serialize)" and "(CV_TYPE,_deserialize)")
           can be taken as a reference for implementing item_serialize/item_deserialize funtions.
        -> serialization/deserialization is in binary-mode, it's NOT endian-independent and the size of each type and struct must match in both serialization and deserialization
           (it would be nice to know what common systems are compatible).
        -> IMPORTANT: all the functions cvh_serializer_write(...)/cvh_serializer_read(...) are UNTESTED!
           They are currently disabled by default, and can be enabled with the CV_ENABLE_UNTESTED_FEATURES definition.
        -> added the undocumented, untested and probably useless struct 'cvh_string_t'. To enable it,
           CV_ENABLE_EXTRA_FEATURES must be defined together with CV_ENABLE_UNTESTED_FEATURES.

   C_VECTOR_VERSION_NUM 113
   -> restored syntax used in version 1.05 (*), so that now:
        -> this header must be included once (per file)
        -> the definition C_VECTOR_IMPLEMENTATION is no more necessary.
        -> CV_DECLARE_AND_DEFINE(base_type) (or CV_DECLARE(base_type) and/or CV_DEFINE(base_type)) must be used.
   -> removed definition CV_ENABLE_DECLARATION_AND_DEFINITION (no more useful).
   -> removed definition CV_DISABLE_FAKE_MEMBER_FUNCTIONS (simply don't use the
        'fake member function syntax' to gain more performance).
   -> added support for replacing memcpy, memmove and memset calls with memcpy_s, memmove_s and memset_s. This is a
      no-op, only useful to silence some compiler or static analyzer warning. It's disabled by default, and it can
      be enabled in the standard way one enables bounds-checking functions in C (if it is available).
      #define __STDC_WANT_LIB_EXT1__ 1
      #include "c_vector.h"
      Tip: if you're sure your compiler (or libc) supports memcpy_s, etc., you can just define CV_FORCE_MEMCPY_S before including "c_vector.h".
   -> silenced all compiler and clang static analyzer warnings I could find (except the warning to use memcpy_s,etc. when the compiler,
      or the default libc it uses, does not seem to support it. Basically what I've learnt is that using memcpy_s, etc. makes your code
      non-portable, but removes many compiler or static analyzer warnings,,, unbelievable!).
   (*) - this is a BAD decision IMHO, because now code is more difficult to debug, and only one line per vector declaration
         is saved in user code. It was done just because many users prefer having to define a vector type using a macro,
         rather than including the same header multiple times.

   C_VECTOR_VERSION_NUM 112
   -> added optional stuff for c++ compilation: cpp_cmp (never used nor tested)

   C_VECTOR_VERSION_NUM 111
   -> added optional stuff for c++ compilation: CV_PLACEMENT_NEW,cpp_ctr,cpp_dtr,cpp_cpy

   C_VECTOR_VERSION_NUM 110
   -> added c++ copy ctr, copy assignment and dtr (and move ctr plus move assignment if CV_HAS_MOVE_SEMANTICS is defined),
      to ease porting code from c++ to c a bit more
   -> removed internal definitions CV_EXTERN_C_START, CV_EXTERN_C_END, CV_DEFAULT_STRUCT_INIT
      Now code is no more 'extern C'

   CV_VERSION_NUM 0108:
   -> fixed an error in 'cv_xxx_insert_range_at(...)

   CV_VERSION_NUM 0107:
   -> added CV_API_INL to specify the preferred 'inline' syntax
   -> small fixed to 'cv_xxx_dbg_check(...)', in the part that detects sorting errors

   CV_VERSION_NUM 0106: restored syntax in version 1.03
   -> restored syntax in version 1.03, without macros and with multiple inclusions of <c_vector.h> (once per item type):
    It's simply the way of doing it. Breakpoints and asserts didn't work with the 'macro syntax' in my IDE. 
    Rolling back is the best choice for both developer and users.
   -> added the global definition CV_ENABLE_DECLARATION_AND_DEFINITION (disabled by default, because it's faster and easier not to use it)
      Normally we generate cv_mystuct (== the type std::vector<mystruct>), this way:
          #ifndef C_VECTOR_mystruct_H
          #define C_VECTOR_mystruct_H
          #    define CV_TYPE mystruct
          #    include <c_vector.h>
          #endif
      When CV_ENABLE_DECLARATION_AND_DEFINITION is defined (and it must be defined globally == in the Project Options),
      then the code above is still valid (and much lighter), but we need to include its implementation (once per item type)
      in a (single) .c file this way:
          #ifdef CV_ENABLE_DECLARATION_AND_DEFINITION
          #	   define C_VECTOR_IMPLEMENTATION
          #    ifndef C_VECTOR_mystruct_H
          #        define C_VECTOR_FORCE_DECLARATION
          #    endif
          #    define CV_TYPE mystruct
          #    include <c_vector.h>
          #endif
      Please note that C_VECTOR_IMPLEMENTATION and C_VECTOR_FORCE_DECLARATION are 'scoped definitions' (== get undefined
      at the end of <c_vector.h>), and of course CV_TYPE too: so they must be redefined by the user every time they are needed.

   CV_VERSION_NUM 0105:
   -> renamed 'CV_DECLARATION(...)' to 'CV_DECLARE(...)'
   -> renamed 'CV_DEFINITION(...)' to 'CV_DEFINE(...)'
   -> renamed 'CV_DECLARATION_AND_DEFINITION(...)' to 'CV_DECLARE_AND_DEFINE(...)'
   -> some fixes (that's what I hope) in 'push_back(...)' and 'insert_at(...)' when re-inserting
      existing items in the same vector.
   -> added 'cv_xxx_insert_range_at(...)' and 'cv_xxx_remove_range_at(...)'

   CV_VERSION_NUM 0104: regression version [= last version had more features]
   -> after having seen the vector implementation at https://gist.github.com/zrbecker/3087880,
      I've wrapped (most of) the code into three macros:
      CV_DECLARATION(CV_TYPE)
      CV_DEFINITION(CV_TYPE)
      CV_DECLARATION_AND_DEFINITION(CV_TYPE)
      and now this header can be included just once like every other header... but:
   -> scoped definitions are gone
   -> it's not possible to use CV_DISABLE_FAKE_MEMBER_FUNCTIONS anymore
   -> CV_DISABLE_CLEARING_ITEM_MEMORY and CV_USE_VOID_PTRS_IN_CMP_FCT now are global definitions
   -> performance was faster before
   -> now most of this code is a huge macro, MUCH more difficult to mantain (and breakpoints stop working in my IDE)
   -> now it's more possible that in some IDEs code completion stops working (and this makes a very big difference indeed!)
   -> UNTESTED CONFIGURATIONS: I've never tried to set a CV_DECLARATION(CV_TYPE) somewhere (e.g. in a .h file), and to set a CV_DEFINITION(CV_TYPE) once in a .c file, and see if this works or not.

   CV_VERSION_NUM 0103:
   -> renamed 'cv_xxx_trim(...)' to 'cv_xxx_shrink_to_fit(...)'
   -> now the first call to 'reserve(&v,new_cap)' (with 'new_cap'>1 to exclude fist indirect call by 'push_back(...)')
      should set the vector capacity exactly to 'new_cap'. This includes the first call to 'resize(&v,new_size)', with 'new_size'>1
      This was done to mitigate a bit the difference between our 'reserve' implementation and others, even if our code is perfectly legal
      [From the C++ reference: reserve][Increase the capacity of the vector to a value that's 'greater or equal' to 'new_cap'. If 'new_cap' is greater than the current capacity(), new storage is allocated, otherwise the method does nothing.]
      So it seems it's O.K. to grow capacity inside 'reserve' (although most implementations do that inside 'resize']
   -> revisited the 'clear item memory before item_ctr()' concept a bit, and added the optional definition CV_DISABLE_CLEARING_ITEM_MEMORY

   CV_VERSION_NUM 0102:
   -> fixed 'cv_xxx_swap(...)'
   -> added a 'cv_xxx_dbg_check(...)'
   -> added 'cv_xxx_trim(...)' to optimize memory usage for current storage

   CV_VERSION_NUM 0101:
   -> added 'fake member function calls' syntax like:
        cv_mystruct v;	// no C init here for fake member functions
        cv_mystruct_create_with(&v,...);	// this inits fake members functions too now

        v.reserve(&v,10);	// fake member function calls ('v' appears twice)
        v.push_back(&v,...);

        cv_mystruct_free(&v);
      It can be disabled globally (= in the Project Options) by defining CV_DISABLE_FAKE_MEMBER_FUNCTIONS.
      'Fake member functions' can slow down performance, because the functions can't be inlined anymore.
*/

/* ------------------------------------------------- */
#if __cplusplus>=201103L
#   undef CV_HAS_MOVE_SEMANTICS
#   define CV_HAS_MOVE_SEMANTICS
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
#   undef CV_NO_ASSERT
#   define CV_NO_ASSERT
/*#   undef CV_NO_STDIO
#   define CV_NO_STDIO*/
/*#   undef CV_NO_STDLIB
#   define CV_NO_STDLIB*/ /* no exit(1) on bad malloc, realloc ? */
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

#if (defined(CV_ADD_ZERO_INIT_DEFINITION) && !defined(CV_ZERO_INIT))
#   ifndef __cplusplus
#       define CV_ZERO_INIT {0}
#   else
#       define CV_ZERO_INIT {}
#   endif
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
#   if (defined(CV_PLACEMENT_NEW) && !defined(CVH_CPP_GUARD))
#   define CVH_CPP_GUARD
    template <typename T> inline void cpp_ctr(T* v) {CV_PLACEMENT_NEW(v) T();}
    template <typename T> inline void cpp_dtr(T* v) {v->T::~T();}
    template <typename T> inline void cpp_cpy(T* a,const T* b) {*a=*b;}
    template <typename T> inline int cpp_cmp(const T* a,const T* b) {return (*a)<(*b)?-1:((*a)>(*b)?1:0);}
#   endif
#endif

#ifndef CV_XSTR
#define CV_XSTR(s) CV_STR(s)
#define CV_STR(s) #s
#endif

#ifndef CV_CAT 
#	define CV_CAT(x, y) CV_CAT_(x, y)
#	define CV_CAT_(x, y) x ## y
#endif

#define CV_VECTOR_TYPE_FCT(CV_TYPE,name) CV_CAT(CV_VECTOR_TYPE(CV_TYPE),name)
#define CV_TYPE_FCT(CV_TYPE,name) CV_CAT(CV_TYPE,name)
#define CV_VECTOR_(name) CV_CAT(cv_,name)
#define CV_VECTOR_TYPE(CV_TYPE) CV_VECTOR_(CV_TYPE)
#ifdef CV_USE_VOID_PTRS_IN_CMP_FCT
#   define CV_CMP_TYPE(CV_TYPE) void
#else
#   define CV_CMP_TYPE(CV_TYPE) CV_TYPE
#endif


#ifndef CVH_SRIALIZER_GUARD_
/* cvh_serializer_t provides serialization/deserialization support for all the vector macros that follow */
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

#ifdef __cplusplus
#   define CV_CPP_DECLARATION_CHUNK0(CV_TYPE)    \
        CV_VECTOR_TYPE(CV_TYPE)();   \
        CV_VECTOR_TYPE(CV_TYPE)(const CV_VECTOR_TYPE(CV_TYPE)& o);    \
        CV_VECTOR_TYPE(CV_TYPE)& operator=(const CV_VECTOR_TYPE(CV_TYPE)& o); \
        CV_API_INL CV_TYPE& operator[](size_t i) {CV_ASSERT(i<size);return v[i];}   \
        CV_API_INL const CV_TYPE& operator[](size_t i) const {CV_ASSERT(i<size);return v[i];}   \
        ~CV_VECTOR_TYPE(CV_TYPE)();
#   ifdef CV_HAS_MOVE_SEMANTICS
#       define CV_CPP_DECLARATION_CHUNK1(CV_TYPE)   \
            CV_VECTOR_TYPE(CV_TYPE)(CV_VECTOR_TYPE(CV_TYPE)&& o); \
            CV_VECTOR_TYPE(CV_TYPE)& operator=(CV_VECTOR_TYPE(CV_TYPE)&& o);  
#   else /*CV_HAS_MOVE_SEMANTICS*/ 
#       define CV_CPP_DECLARATION_CHUNK1(CV_TYPE)   /*no-op*/
#   endif /*CV_HAS_MOVE_SEMANTICS*/
#else  /*__cplusplus*/
#   define CV_CPP_DECLARATION_CHUNK0(CV_TYPE)   /*no-op*/
#   define CV_CPP_DECLARATION_CHUNK1(CV_TYPE)   /*no-op*/
#endif /*__cplusplus*/

#ifndef CV_DISABLE_FAKE_MEMBER_FUNCTIONS
#   define CV_FAKE_MEMBER_FUNCTIONS_DECL_CHUNK(CV_TYPE)  \
    void (* const free)(CV_VECTOR_TYPE(CV_TYPE)* v); \
    void (* const clear)(CV_VECTOR_TYPE(CV_TYPE)* v);    \
    void (* const shrink_to_fit)(CV_VECTOR_TYPE(CV_TYPE)* v);    \
    void (* const swap)(CV_VECTOR_TYPE(CV_TYPE)* a,CV_VECTOR_TYPE(CV_TYPE)* b);   \
    void (* const reserve)(CV_VECTOR_TYPE(CV_TYPE)* v,size_t size);  \
    void (* const resize)(CV_VECTOR_TYPE(CV_TYPE)* v,size_t size);   \
    void (* const resize_with)(CV_VECTOR_TYPE(CV_TYPE)* v,size_t size,const CV_TYPE* default_value); \
    void (* const resize_with_by_val)(CV_VECTOR_TYPE(CV_TYPE)* v,size_t size,const CV_TYPE default_value);   \
    void (* const push_back)(CV_VECTOR_TYPE(CV_TYPE)* v,const CV_TYPE* value);   \
    void (* const push_back_by_val)(CV_VECTOR_TYPE(CV_TYPE)* v,const CV_TYPE value); \
    void (* const pop_back)(CV_VECTOR_TYPE(CV_TYPE)* v); \
    size_t (* const linear_search)(const CV_VECTOR_TYPE(CV_TYPE)* v,const CV_TYPE* item_to_search,int* match);   \
    size_t (* const linear_search_by_val)(const CV_VECTOR_TYPE(CV_TYPE)* v,const CV_TYPE item_to_search,int* match); \
    size_t (* const binary_search)(const CV_VECTOR_TYPE(CV_TYPE)* v,const CV_TYPE* item_to_search,int* match);   \
    size_t (* const binary_search_by_val)(const CV_VECTOR_TYPE(CV_TYPE)* v,const CV_TYPE item_to_search,int* match); \
    size_t (* const insert_at)(CV_VECTOR_TYPE(CV_TYPE)* v,const CV_TYPE* item_to_insert,size_t position);    \
    size_t (* const insert_at_by_val)(CV_VECTOR_TYPE(CV_TYPE)* v,const CV_TYPE item_to_insert,size_t position);  \
    size_t (* const insert_range_at)(CV_VECTOR_TYPE(CV_TYPE)* v,const CV_TYPE* items_to_insert,size_t num_items_to_insert,size_t start_position);    \
    size_t (* const insert_sorted)(CV_VECTOR_TYPE(CV_TYPE)* v,const CV_TYPE* item_to_insert,int* match,int insert_even_if_item_match);   \
    size_t (* const insert_sorted_by_val)(CV_VECTOR_TYPE(CV_TYPE)* v,const CV_TYPE item_to_insert,int* match,int insert_even_if_item_match); \
    int (* const remove_at)(CV_VECTOR_TYPE(CV_TYPE)* v,size_t position); \
    int (* const remove_range_at)(CV_VECTOR_TYPE(CV_TYPE)* v,size_t start_item_position,size_t num_items_to_remove); \
    void (* const cpy)(CV_VECTOR_TYPE(CV_TYPE)* a,const CV_VECTOR_TYPE(CV_TYPE)* b);  \
    void (* const dbg_check)(const CV_VECTOR_TYPE(CV_TYPE)* v); \
    void (* const serialize)(const CV_VECTOR_TYPE(CV_TYPE)* v,cvh_serializer_t* serializer);  \
    int (* const deserialize)(CV_VECTOR_TYPE(CV_TYPE)* v,const cvh_serializer_t* deserializer);
#else /*CV_DISABLE_FAKE_MEMBER_FUNCTIONS*/
#   define CV_FAKE_MEMBER_FUNCTIONS_DECL_CHUNK(CV_TYPE) /*no-op*/
#endif /*CV_DISABLE_FAKE_MEMBER_FUNCTIONS*/


#define CV_DECLARE(CV_TYPE)										\
typedef struct CV_VECTOR_TYPE(CV_TYPE) CV_VECTOR_TYPE(CV_TYPE);       \
struct CV_VECTOR_TYPE(CV_TYPE) {         \
    CV_TYPE * v;    \
	const size_t size;  \
	const size_t capacity;  \
    void (*const item_ctr)(CV_TYPE*);                     /* optional (can be NULL) */  \
    void (*const item_dtr)(CV_TYPE*);                     /* optional (can be NULL) */  \
    void (*const item_cpy)(CV_TYPE*,const CV_TYPE*);		/* optional (can be NULL) */    \
    int (*const item_cmp)(const CV_CMP_TYPE(CV_TYPE)*,const CV_CMP_TYPE(CV_TYPE)*);		/* optional (can be NULL) (for sorted vectors only) */  \
    void (*const item_serialize)(const CV_TYPE*,cvh_serializer_t*);   \
    int (*const item_deserialize)(CV_TYPE*,const cvh_serializer_t*);   \
    CV_FAKE_MEMBER_FUNCTIONS_DECL_CHUNK(CV_TYPE)    \
    CV_CPP_DECLARATION_CHUNK0(CV_TYPE)  \
    CV_CPP_DECLARATION_CHUNK1(CV_TYPE)  \
};  \
/* function declarations */ \
CV_API_DEC void CV_VECTOR_TYPE_FCT(CV_TYPE,_free)(CV_VECTOR_TYPE(CV_TYPE)* v);   \
CV_API_DEC void CV_VECTOR_TYPE_FCT(CV_TYPE,_clear)(CV_VECTOR_TYPE(CV_TYPE)* v);  \
CV_API_DEC void CV_VECTOR_TYPE_FCT(CV_TYPE,_swap)(CV_VECTOR_TYPE(CV_TYPE)* a,CV_VECTOR_TYPE(CV_TYPE)* b); \
CV_API_DEC void CV_VECTOR_TYPE_FCT(CV_TYPE,_reserve)(CV_VECTOR_TYPE(CV_TYPE)* v,size_t size);    \
CV_API_DEC void CV_VECTOR_TYPE_FCT(CV_TYPE,_resize)(CV_VECTOR_TYPE(CV_TYPE)* v,size_t size); \
CV_API_DEC void CV_VECTOR_TYPE_FCT(CV_TYPE,_resize_with)(CV_VECTOR_TYPE(CV_TYPE)* v,size_t size,const CV_TYPE* default_value);   \
CV_API_DEC void CV_VECTOR_TYPE_FCT(CV_TYPE,_resize_with_by_val)(CV_VECTOR_TYPE(CV_TYPE)* v,size_t size,const CV_TYPE default_value); \
CV_API_DEC void CV_VECTOR_TYPE_FCT(CV_TYPE,_push_back)(CV_VECTOR_TYPE(CV_TYPE)* v,const CV_TYPE* value); \
CV_API_DEC void CV_VECTOR_TYPE_FCT(CV_TYPE,_push_back_by_val)(CV_VECTOR_TYPE(CV_TYPE)* v,const CV_TYPE value);   \
CV_API_DEC void CV_VECTOR_TYPE_FCT(CV_TYPE,_pop_back)(CV_VECTOR_TYPE(CV_TYPE)* v);   \
CV_API_DEC size_t CV_VECTOR_TYPE_FCT(CV_TYPE,_linear_search)(const CV_VECTOR_TYPE(CV_TYPE)* v,const CV_TYPE* item_to_search,int* match); \
CV_API_DEC size_t CV_VECTOR_TYPE_FCT(CV_TYPE,_linear_search_by_val)(const CV_VECTOR_TYPE(CV_TYPE)* v,const CV_TYPE item_to_search,int* match);   \
CV_API_DEC size_t CV_VECTOR_TYPE_FCT(CV_TYPE,_binary_search)(const CV_VECTOR_TYPE(CV_TYPE)* v,const CV_TYPE* item_to_search,int* match); \
CV_API_DEC size_t CV_VECTOR_TYPE_FCT(CV_TYPE,_binary_search_by_val)(const CV_VECTOR_TYPE(CV_TYPE)* v,const CV_TYPE item_to_search,int* match);   \
CV_API_DEC size_t CV_VECTOR_TYPE_FCT(CV_TYPE,_insert_at)(CV_VECTOR_TYPE(CV_TYPE)* v,const CV_TYPE* item_to_insert,size_t position);  \
CV_API_DEC size_t CV_VECTOR_TYPE_FCT(CV_TYPE,_insert_at_by_val)(CV_VECTOR_TYPE(CV_TYPE)* v,const CV_TYPE item_to_insert,size_t position);    \
CV_API_DEC size_t CV_VECTOR_TYPE_FCT(CV_TYPE,_insert_range_at)(CV_VECTOR_TYPE(CV_TYPE)* v,const CV_TYPE* items_to_insert,size_t num_items_to_insert,size_t start_position);  \
CV_API_DEC size_t CV_VECTOR_TYPE_FCT(CV_TYPE,_insert_sorted)(CV_VECTOR_TYPE(CV_TYPE)* v,const CV_TYPE* item_to_insert,int* match,int insert_even_if_item_match); \
CV_API_DEC size_t CV_VECTOR_TYPE_FCT(CV_TYPE,_insert_sorted_by_val)(CV_VECTOR_TYPE(CV_TYPE)* v,const CV_TYPE item_to_insert,int* match,int insert_even_if_item_match);   \
CV_API_DEC int CV_VECTOR_TYPE_FCT(CV_TYPE,_remove_at)(CV_VECTOR_TYPE(CV_TYPE)* v,size_t position);   \
CV_API_DEC int CV_VECTOR_TYPE_FCT(CV_TYPE,_remove_range_at)(CV_VECTOR_TYPE(CV_TYPE)* v,size_t start_item_position,size_t num_items_to_remove);   \
CV_API_DEC void CV_VECTOR_TYPE_FCT(CV_TYPE,_cpy)(CV_VECTOR_TYPE(CV_TYPE)* a,const CV_VECTOR_TYPE(CV_TYPE)* b);    \
CV_API_DEC void CV_VECTOR_TYPE_FCT(CV_TYPE,_shrink_to_fit)(CV_VECTOR_TYPE(CV_TYPE)* v);  \
CV_API_DEC void CV_VECTOR_TYPE_FCT(CV_TYPE,_dbg_check)(const CV_VECTOR_TYPE(CV_TYPE)* v);    \
CV_API_DEC void CV_VECTOR_TYPE_FCT(CV_TYPE,_serialize)(const CV_VECTOR_TYPE(CV_TYPE)* v,cvh_serializer_t* serializer);    \
CV_API_DEC int CV_VECTOR_TYPE_FCT(CV_TYPE,_deserialize)(CV_VECTOR_TYPE(CV_TYPE)* v,const cvh_serializer_t* deserializer);    \
CV_API_DEC void CV_VECTOR_TYPE_FCT(CV_TYPE,_init_with)(CV_VECTOR_TYPE(CV_TYPE)* v,int (*item_cmp)(const CV_CMP_TYPE(CV_TYPE)*,const CV_CMP_TYPE(CV_TYPE)*),void (*item_ctr)(CV_TYPE*),void (*item_dtr)(CV_TYPE*),void (*item_cpy)(CV_TYPE*,const CV_TYPE*),void (*item_serialize)(const CV_TYPE*,cvh_serializer_t*),int (*item_deserialize)(CV_TYPE*,const cvh_serializer_t*)); \
CV_API_DEC void CV_VECTOR_TYPE_FCT(CV_TYPE,_init)(CV_VECTOR_TYPE(CV_TYPE)* v,int (*item_cmp)(const CV_CMP_TYPE(CV_TYPE)*,const CV_CMP_TYPE(CV_TYPE)*)); \
CV_API_DEC CV_VECTOR_TYPE(CV_TYPE) CV_VECTOR_TYPE_FCT(CV_TYPE,_create_with)(int (*item_cmp)(const CV_CMP_TYPE(CV_TYPE)*,const CV_CMP_TYPE(CV_TYPE)*),void (*item_ctr)(CV_TYPE*),void (*item_dtr)(CV_TYPE*),void (*item_cpy)(CV_TYPE*,const CV_TYPE*),void (*item_serialize)(const CV_TYPE*,cvh_serializer_t*),int (*item_deserialize)(CV_TYPE*,const cvh_serializer_t*));   \
CV_API_DEC CV_VECTOR_TYPE(CV_TYPE) CV_VECTOR_TYPE_FCT(CV_TYPE,_create)(int (*item_cmp)(const CV_CMP_TYPE(CV_TYPE)*,const CV_CMP_TYPE(CV_TYPE)*));


/*  To use memcpy_s, memmove_s and memset_s, in your source file(s), please add this line before including this header:
#define __STDC_WANT_LIB_EXT1__ 1
#include "c_vector.h"
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
            printf("%" CV_SIZE_T_FORMATTING " %s",pTGMKB[i],names[i]);
            ++cnt;
        }
    }
}
#endif /* CV_NO_STDIO */

/* cvh_serializer_t functions */
#ifndef CVH_SRIALIZER_GUARD_
CV_API void cvh_serializer_reserve(cvh_serializer_t* p,size_t new_capacity)    {
    if (new_capacity>p->capacity) {
        new_capacity = new_capacity<p->capacity*3/2 ? p->capacity*3/2 : new_capacity; /* grow factor */
        cv_safe_realloc((void**)&p->v,new_capacity);CV_ASSERT(p->v);
        p->capacity = new_capacity;
    }
}
CV_API void cvh_serializer_free(cvh_serializer_t* p)    {cv_free(p->v);p->v=NULL;p->size=p->capacity=p->offset=0;}
CV_API void cvh_serializer_cpy(cvh_serializer_t* dst,const cvh_serializer_t* src)   {
    CV_ASSERT(src && dst);
    if (dst->capacity<src->size) cvh_serializer_reserve(dst,src->size);
    CV_MEMCPY(dst->v,src->v,src->size);dst->size = src->size;/*dst->offset = src->offset;*/
}
CV_API void cvh_serializer_clear(cvh_serializer_t* p)   {p->size=p->offset=0;}
CV_API int cvh_serializer_save(const cvh_serializer_t* p,const char* path)    {
#   ifndef CV_NO_STDIO
    FILE* f = fopen(path,"wb");if (f) {fwrite(p->v,p->size,1,f);fclose(f);return 1;}
#   else   /*CV_NO_STDIO*/
    CV_ASSERT(0 && "Cannot save files with the CV_NO_STDIO definition enabled");
#   endif    /*CV_NO_STDIO*/
    return 0;
}
CV_API int cvh_serializer_load(cvh_serializer_t* p,const char* path)    {
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
CV_API void cvh_serializer_offset_rewind(const cvh_serializer_t* d)  {*((size_t*)&d->offset)=0;}
CV_API void cvh_serializer_offset_set(const cvh_serializer_t* d,size_t offset)  {CV_ASSERT(offset<=d->size);*((size_t*)&d->offset)=offset;}
CV_API void cvh_serializer_offset_advance(const cvh_serializer_t* d,size_t amount)  {CV_ASSERT(d->offset+amount<=d->size);*((size_t*)&d->offset)+=amount;}
#   define CVH_SERIALIZER_WRITE(S,type,value)   {cvh_serializer_reserve(S,S->size + sizeof(type));*((type*) (&S->v[S->size])) = value;S->size+=sizeof(type);}
#   define CVH_DESERIALIZER_READ(D,type,value_ptr)   { \
       int check = (D->offset+sizeof(type)<=D->size);CV_ASSERT(check);if (!check) return 0; \
       *value_ptr = *((type*) &D->v[D->offset]);*((size_t*) &D->offset)+=sizeof(type); return 1;}
CV_API void cvh_serializer_write_size_t(cvh_serializer_t* s,size_t value) {CVH_SERIALIZER_WRITE(s,size_t,value)}
CV_API int cvh_serializer_read_size_t(const cvh_serializer_t* d,size_t* value) {CVH_DESERIALIZER_READ(d,size_t,value)}
CV_API void cvh_serializer_write_unsigned_char(cvh_serializer_t* s,unsigned char value) {CVH_SERIALIZER_WRITE(s,unsigned char,value)}
CV_API int cvh_serializer_read_unsigned_char(const cvh_serializer_t* d,unsigned char* value) {CVH_DESERIALIZER_READ(d,unsigned char,value)}
CV_API void cvh_serializer_write_signed_char(cvh_serializer_t* s,signed char value) {CVH_SERIALIZER_WRITE(s,signed char,value)}
CV_API int cvh_serializer_read_signed_char(const cvh_serializer_t* d,signed char* value) {CVH_DESERIALIZER_READ(d,signed char,value)}
CV_API void cvh_serializer_write_unsigned_short(cvh_serializer_t* s,unsigned short value) {CVH_SERIALIZER_WRITE(s,unsigned short,value)}
CV_API int cvh_serializer_read_unsigned_short(const cvh_serializer_t* d,unsigned short* value) {CVH_DESERIALIZER_READ(d,unsigned short,value)}
CV_API void cvh_serializer_write_short(cvh_serializer_t* s,short value) {CVH_SERIALIZER_WRITE(s,short,value)}
CV_API int cvh_serializer_read_short(const cvh_serializer_t* d,short* value) {CVH_DESERIALIZER_READ(d,short,value)}
CV_API void cvh_serializer_write_unsigned_int(cvh_serializer_t* s,unsigned value) {CVH_SERIALIZER_WRITE(s,unsigned,value)}
CV_API int cvh_serializer_read_unsigned_int(const cvh_serializer_t* d,unsigned* value) {CVH_DESERIALIZER_READ(d,unsigned,value)}
CV_API void cvh_serializer_write_int(cvh_serializer_t* s,int value) {CVH_SERIALIZER_WRITE(s,int,value)}
CV_API int cvh_serializer_read_int(const cvh_serializer_t* d,int* value) {CVH_DESERIALIZER_READ(d,int,value)}
CV_API void cvh_serializer_write_unsigned_long(cvh_serializer_t* s,unsigned long value) {CVH_SERIALIZER_WRITE(s,unsigned long,value)}
CV_API int cvh_serializer_read_unsigned_long(const cvh_serializer_t* d,unsigned long* value) {CVH_DESERIALIZER_READ(d,unsigned long,value)}
CV_API void cvh_serializer_write_long(cvh_serializer_t* s,long value) {CVH_SERIALIZER_WRITE(s,long,value)}
CV_API int cvh_serializer_read_long(const cvh_serializer_t* d,long* value) {CVH_DESERIALIZER_READ(d,long,value)}
CV_API void cvh_serializer_write_unsigned_long_long(cvh_serializer_t* s,unsigned long long value) {CVH_SERIALIZER_WRITE(s,unsigned long long,value)}
CV_API int cvh_serializer_read_unsigned_long_long(const cvh_serializer_t* d,unsigned long long* value) {CVH_DESERIALIZER_READ(d,unsigned long long,value)}
CV_API void cvh_serializer_write_long_long(cvh_serializer_t* s,long long value) {CVH_SERIALIZER_WRITE(s,long long,value)}
CV_API int cvh_serializer_read_long_long(const cvh_serializer_t* d,long long* value) {CVH_DESERIALIZER_READ(d,long long,value)}
CV_API void cvh_serializer_write_float(cvh_serializer_t* s,float value) {CVH_SERIALIZER_WRITE(s,float,value)}
CV_API int cvh_serializer_read_float(const cvh_serializer_t* d,float* value) {CVH_DESERIALIZER_READ(d,float,value)}
CV_API void cvh_serializer_write_double(cvh_serializer_t* s,double value) {CVH_SERIALIZER_WRITE(s,double,value)}
CV_API int cvh_serializer_read_double(const cvh_serializer_t* d,double* value) {CVH_DESERIALIZER_READ(d,double,value)}
#   undef CVH_SERIALIZER_WRITE
#   undef CVH_SERIALIZER_READ
CV_API void cvh_serializer_write_size_t_using_mipmaps(cvh_serializer_t* s,size_t value) {
    /* In most use-cases this should have less memory impact than 'cvh_serializer_write_size_t(...)' */
    int overflow=value>=255;cvh_serializer_write_unsigned_char(s,(unsigned char)(overflow ? 255 : value));if (!overflow) return;
    if (sizeof(unsigned short)==2) {overflow=value>=65535;cvh_serializer_write_unsigned_short(s,(unsigned short)(overflow ? 65535 : value));if (!overflow) return;}
    if (sizeof(unsigned int)==4) {overflow=value>=4294967295;cvh_serializer_write_unsigned_int(s,(unsigned int)(overflow ? 4294967295 : value));if (!overflow) return;}
    cvh_serializer_write_size_t(s,value);
}
CV_API int cvh_serializer_read_size_t_using_mipmaps(const cvh_serializer_t* d,size_t* value) {
    /* In most use-cases this should have less memory impact than 'cvh_serializer_read_size_t(...)' */
    int check = 0;*value=0;
    {unsigned char v=255,vmax=255;if ((check=cvh_serializer_read_unsigned_char(d,&v)) && v<vmax) {*value=v;return 1;} if (!check) return 0;}
    if (sizeof(unsigned short)==2) {unsigned short v=65535,vmax=65535;if ((check=cvh_serializer_read_unsigned_short(d,&v)) && v<vmax) {*value=v;return 1;} if (!check) return 0;}
    if (sizeof(unsigned int)==4) {unsigned int v=4294967295,vmax=4294967295;if ((check=cvh_serializer_read_unsigned_int(d,&v)) && v<vmax) {*value=v;return 1;} if (!check) return 0;}
    return cvh_serializer_read_size_t(d,value);
}
CV_API void cvh_serializer_write_string(cvh_serializer_t* s,const char* str_beg,const char* str_end /*=NULL*/)    {
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
CV_API int cvh_serializer_read_string(const cvh_serializer_t* d,char** pstr,void* (*my_realloc)(void*,size_t)/*=NULL*/,void (*my_free)(void*)/*=NULL*/)    {
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
CV_API void cvh_serializer_write_blob(cvh_serializer_t* s,const void* blob,size_t blob_size_in_bytes)    {
    if (!blob || blob_size_in_bytes==0) cvh_serializer_write_size_t_using_mipmaps(s,0);   /* we want to preserve NULL blobs */
    else {
        cvh_serializer_write_size_t_using_mipmaps(s,blob_size_in_bytes);
        cvh_serializer_reserve(s,s->size + blob_size_in_bytes);
        CV_MEMCPY(&s->v[s->size],blob,blob_size_in_bytes);s->size+=blob_size_in_bytes;
    }
}
CV_API int cvh_serializer_read_blob(const cvh_serializer_t* d,void** pblob,size_t* blob_size_out,void* (*my_realloc)(void*,size_t)/*=NULL*/,void (*my_free)(void*)/*=NULL*/)    {
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
CV_API void cvh_serializer_init(cvh_serializer_t* p)    {
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
CV_API cvh_serializer_t cvh_serializer_create(void) {cvh_serializer_t p;cvh_serializer_init(&p);return p;}
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


#ifndef CV_NO_CVH_STRING_T
#ifndef CVH_STRING_GUARD_
#define CVH_STRING_GUARD_
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
CV_API void cvh_string_reserve(cvh_string_t* p,size_t new_capacity)    {
    if (new_capacity>p->capacity) {
        new_capacity = new_capacity<p->capacity*3/2 ? p->capacity*3/2 : new_capacity; /* grow factor */
        cv_safe_realloc((void**)&p->v,new_capacity);CV_ASSERT(p->v);
        p->capacity = new_capacity;
    }
}
CV_API size_t cvh_string_push_back(cvh_string_t* p,const char* str_beg,const char* str_end/*=NULL*/)    {
    const size_t old_size = p->size;size_t len;
    CV_ASSERT(str_beg);
    len = (!str_end) ? strlen(str_beg) : (size_t)(str_end-str_beg);
    cvh_string_reserve(p,p->size+len+1);
    CV_MEMCPY(&p->v[p->size],str_beg,len);
    p->v[p->size+len]='\0';
    p->size+=len+1;
    return old_size;
}
CV_API void cvh_string_free(cvh_string_t* p)    {cv_free(p->v);p->v=NULL;p->size=p->capacity=0;}
CV_API void cvh_string_cpy(cvh_string_t* dst,const cvh_string_t* src)   {
    CV_ASSERT(src && dst);
    if (dst->capacity<src->size) cvh_string_reserve(dst,src->size);
    CV_MEMCPY(dst->v,src->v,src->size);dst->size = src->size;
}
CV_API void cvh_string_clear(cvh_string_t* p)   {p->size=0;}
CV_API void cvh_string_serialize(const cvh_string_t* p,cvh_serializer_t* s)    {
    const size_t size_t_size_in_bytes = sizeof(size_t);
    const size_t p_v_size_in_bytes = p->size;
    CV_ASSERT(p && s);
    /*if (p && p->v) {CV_ASSERT(p->size>0 && p->v[p->size-1]=='\0');}*/
    cvh_serializer_reserve(s,s->size + size_t_size_in_bytes+p_v_size_in_bytes);
    *((size_t*) &s->v[s->size]) = p->size;s->size+=size_t_size_in_bytes;
    CV_MEMCPY(&s->v[s->size],p->v,p_v_size_in_bytes);s->size+=p_v_size_in_bytes;
}
CV_API int cvh_string_deserialize(cvh_string_t* p,const cvh_serializer_t* d)    {
    const size_t size_t_size_in_bytes = sizeof(size_t);size_t psize=0;
    int check = d->offset+size_t_size_in_bytes<=d->size;CV_ASSERT(check);if (!check) return 0;
    CV_ASSERT(p && d);
    psize = *((const size_t*) &d->v[d->offset]);*((size_t*)&d->offset)+=size_t_size_in_bytes;
    check = d->offset+psize<=d->size;CV_ASSERT(check && "No space to deserialize the content of a cvh_string_t");if (!check) return 0;
    cvh_string_reserve(p,psize);CV_ASSERT(p->v);
    CV_MEMCPY(p->v,&d->v[d->offset],psize);*((size_t*)&d->offset)+=psize;p->size=psize;
    return 1;
}
CV_API void cvh_string_init(cvh_string_t* p)   {
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
CV_API cvh_string_t cvh_string_create(void)   {cvh_string_t v;cvh_string_init(&v);return v;}
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

#endif /* CV_COMMON_FUNCTIONS_GUARD */


#ifndef CV_NO_STDIO
#   define CV_CHUNK_NO_STDIO_0(CV_TYPE) \
        fprintf(stderr,"[%s] Sorting Error (%" CV_SIZE_T_FORMATTING "): item_cmp(%" CV_SIZE_T_FORMATTING ",%" CV_SIZE_T_FORMATTING ")>0 [num_items=%" CV_SIZE_T_FORMATTING "]\n",CV_XSTR(CV_VECTOR_TYPE_FCT(CV_TYPE,_dbg_check)),num_sorting_errors,j-1,j,v->size);
#   define CV_CHUNK_NO_STDIO_1(CV_TYPE) \
    printf("[%s]:\n",CV_XSTR(CV_VECTOR_TYPE_FCT(CV_TYPE,_dbg_check)));  \
    printf("\tsize: %" CV_SIZE_T_FORMATTING ". capacity: %" CV_SIZE_T_FORMATTING ". sizeof(%s): ",v->size,v->capacity,CV_XSTR(CV_TYPE));cv_display_bytes(sizeof(CV_TYPE));printf(".\n");    \
    if (v->item_cmp && v->size) {   \
        if (num_sorting_errors==0) printf("\tsorting: OK.\n");  \
        else printf("\tsorting: NO (%" CV_SIZE_T_FORMATTING " sorting errors detected).\n",num_sorting_errors);   \
    }   \
    printf("\tmemory_used: ");cv_display_bytes(mem_used);   \
    printf(". memory_minimal_possible: ");cv_display_bytes(mem_minimal);    \
    printf(". mem_used_percentage: %1.2f%% (100%% is the best possible result).\n",mem_used_percentage);
#else /*CV_NO_STDIO*/
#   define CV_CHUNK_NO_STDIO_0(CV_TYPE) /*no-op*/
#   define CV_CHUNK_NO_STDIO_1(CV_TYPE) /*no-op*/
#endif /*CV_NO_STDIO*/


#ifndef CV_DISABLE_FAKE_MEMBER_FUNCTIONS
#   define CV_FAKE_MEMBER_FUNCTIONS_DEF_CHUNK0(CV_TYPE)  \
        typedef void (* free_clear_shrink_to_fit_pop_back_mf)(CV_VECTOR_TYPE(CV_TYPE)*); \
        typedef void (* swap_mf)(CV_VECTOR_TYPE(CV_TYPE)*,CV_VECTOR_TYPE(CV_TYPE)*);  \
        typedef void (* reserve_mf)(CV_VECTOR_TYPE(CV_TYPE)*,size_t);    \
        typedef void (* resize_mf)(CV_VECTOR_TYPE(CV_TYPE)*,size_t); \
        typedef void (* resize_with_mf)(CV_VECTOR_TYPE(CV_TYPE)*,size_t,const CV_TYPE*); \
        typedef void (* resize_with_by_val_mf)(CV_VECTOR_TYPE(CV_TYPE)*,size_t,const CV_TYPE);   \
        typedef void (* push_back_mf)(CV_VECTOR_TYPE(CV_TYPE)*,const CV_TYPE*);  \
        typedef void (* push_back_by_val_mf)(CV_VECTOR_TYPE(CV_TYPE)*,const CV_TYPE);    \
        typedef size_t (* search_mf)(const CV_VECTOR_TYPE(CV_TYPE)*,const CV_TYPE*,int*);    \
        typedef size_t (* search_by_val_mf)(const CV_VECTOR_TYPE(CV_TYPE)*,const CV_TYPE,int*);  \
        typedef size_t (* insert_at_mf)(CV_VECTOR_TYPE(CV_TYPE)*,const CV_TYPE*,size_t); \
        typedef size_t (* insert_at_by_val_mf)(CV_VECTOR_TYPE(CV_TYPE)*,const CV_TYPE,size_t);   \
        typedef size_t (* insert_range_at_mf)(CV_VECTOR_TYPE(CV_TYPE)*,const CV_TYPE*,size_t,size_t);    \
        typedef size_t (* insert_sorted_mf)(CV_VECTOR_TYPE(CV_TYPE)*,const CV_TYPE*,int*,int);   \
        typedef size_t (* insert_sorted_by_val_mf)(CV_VECTOR_TYPE(CV_TYPE)*,const CV_TYPE,int*,int); \
        typedef int (* remove_at_mf)(CV_VECTOR_TYPE(CV_TYPE)*,size_t);   \
        typedef int (* remove_range_at_mf)(CV_VECTOR_TYPE(CV_TYPE)*,size_t,size_t);  \
        typedef void (* cpy_mf)(CV_VECTOR_TYPE(CV_TYPE)*,const CV_VECTOR_TYPE(CV_TYPE)*); \
        typedef void (* dbg_check_mf)(const CV_VECTOR_TYPE(CV_TYPE)*);  \
        typedef void (* serialize_mf)(const CV_VECTOR_TYPE(CV_TYPE)*,cvh_serializer_t*);  \
        typedef int (* deserialize_mf)(CV_VECTOR_TYPE(CV_TYPE)*,const cvh_serializer_t*);
#   define CV_FAKE_MEMBER_FUNCTIONS_DEF_CHUNK1(CV_TYPE)  \
        *((free_clear_shrink_to_fit_pop_back_mf*)&v->free)=&CV_VECTOR_TYPE_FCT(CV_TYPE,_free);  \
        *((free_clear_shrink_to_fit_pop_back_mf*)&v->clear)=&CV_VECTOR_TYPE_FCT(CV_TYPE,_clear);    \
        *((free_clear_shrink_to_fit_pop_back_mf*)&v->shrink_to_fit)=&CV_VECTOR_TYPE_FCT(CV_TYPE,_shrink_to_fit);    \
        *((swap_mf*)&v->swap)=&CV_VECTOR_TYPE_FCT(CV_TYPE,_swap);   \
        *((reserve_mf*)&v->reserve)=&CV_VECTOR_TYPE_FCT(CV_TYPE,_reserve);  \
        *((resize_mf*)&v->resize)=&CV_VECTOR_TYPE_FCT(CV_TYPE,_resize); \
        *((resize_with_mf*)&v->resize_with)=&CV_VECTOR_TYPE_FCT(CV_TYPE,_resize_with);  \
        *((resize_with_by_val_mf*)&v->resize_with_by_val)=&CV_VECTOR_TYPE_FCT(CV_TYPE,_resize_with_by_val); \
        *((push_back_mf*)&v->push_back)=&CV_VECTOR_TYPE_FCT(CV_TYPE,_push_back);    \
        *((push_back_by_val_mf*)&v->push_back_by_val)=&CV_VECTOR_TYPE_FCT(CV_TYPE,_push_back_by_val);   \
        *((free_clear_shrink_to_fit_pop_back_mf*)&v->pop_back)=&CV_VECTOR_TYPE_FCT(CV_TYPE,_pop_back);  \
        *((search_mf*)&v->linear_search)=&CV_VECTOR_TYPE_FCT(CV_TYPE,_linear_search);   \
        *((search_by_val_mf*)&v->linear_search_by_val)=&CV_VECTOR_TYPE_FCT(CV_TYPE,_linear_search_by_val);  \
        *((search_mf*)&v->binary_search)=&CV_VECTOR_TYPE_FCT(CV_TYPE,_binary_search);   \
        *((search_by_val_mf*)&v->binary_search_by_val)=&CV_VECTOR_TYPE_FCT(CV_TYPE,_binary_search_by_val);  \
        *((insert_at_mf*)&v->insert_at)=&CV_VECTOR_TYPE_FCT(CV_TYPE,_insert_at);    \
        *((insert_at_by_val_mf*)&v->insert_at_by_val)=&CV_VECTOR_TYPE_FCT(CV_TYPE,_insert_at_by_val);   \
        *((insert_range_at_mf*)&v->insert_range_at)=&CV_VECTOR_TYPE_FCT(CV_TYPE,_insert_range_at);  \
        *((insert_sorted_mf*)&v->insert_sorted)=&CV_VECTOR_TYPE_FCT(CV_TYPE,_insert_sorted);    \
        *((insert_sorted_by_val_mf*)&v->insert_sorted_by_val)=&CV_VECTOR_TYPE_FCT(CV_TYPE,_insert_sorted_by_val);   \
        *((remove_at_mf*)&v->remove_at)=&CV_VECTOR_TYPE_FCT(CV_TYPE,_remove_at);    \
        *((remove_range_at_mf*)&v->remove_range_at)=&CV_VECTOR_TYPE_FCT(CV_TYPE,_remove_range_at);  \
        *((cpy_mf*)&v->cpy)=&CV_VECTOR_TYPE_FCT(CV_TYPE,_cpy);  \
        *((dbg_check_mf*)&v->dbg_check)=&CV_VECTOR_TYPE_FCT(CV_TYPE,_dbg_check);    \
        *((serialize_mf*)&v->serialize)=&CV_VECTOR_TYPE_FCT(CV_TYPE,_serialize); \
        *((deserialize_mf*)&v->deserialize)=&CV_VECTOR_TYPE_FCT(CV_TYPE,_deserialize);
#   ifdef __cplusplus
#       define CV_FAKE_MEMBER_FUNCTIONS_DEF_CPP_CHUNK(CV_TYPE)  \
            ,free(&CV_VECTOR_TYPE_FCT(CV_TYPE,_free)),clear(&CV_VECTOR_TYPE_FCT(CV_TYPE,_clear)),shrink_to_fit(&CV_VECTOR_TYPE_FCT(CV_TYPE,_shrink_to_fit)), \
            swap(&CV_VECTOR_TYPE_FCT(CV_TYPE,_swap)),reserve(&CV_VECTOR_TYPE_FCT(CV_TYPE,_reserve)),    \
            resize(&CV_VECTOR_TYPE_FCT(CV_TYPE,_resize)),resize_with(&CV_VECTOR_TYPE_FCT(CV_TYPE,_resize_with)),resize_with_by_val(&CV_VECTOR_TYPE_FCT(CV_TYPE,_resize_with_by_val)),   \
            push_back(&CV_VECTOR_TYPE_FCT(CV_TYPE,_push_back)),push_back_by_val(&CV_VECTOR_TYPE_FCT(CV_TYPE,_push_back_by_val)),pop_back(&CV_VECTOR_TYPE_FCT(CV_TYPE,_pop_back)),   \
            linear_search(&CV_VECTOR_TYPE_FCT(CV_TYPE,_linear_search)),linear_search_by_val(&CV_VECTOR_TYPE_FCT(CV_TYPE,_linear_search_by_val)),    \
            binary_search(&CV_VECTOR_TYPE_FCT(CV_TYPE,_binary_search)),binary_search_by_val(&CV_VECTOR_TYPE_FCT(CV_TYPE,_binary_search_by_val)),    \
            insert_at(&CV_VECTOR_TYPE_FCT(CV_TYPE,_insert_at)),insert_at_by_val(&CV_VECTOR_TYPE_FCT(CV_TYPE,_insert_at_by_val)),    \
            insert_range_at(&CV_VECTOR_TYPE_FCT(CV_TYPE,_insert_range_at)), \
            insert_sorted(&CV_VECTOR_TYPE_FCT(CV_TYPE,_insert_sorted)),insert_sorted_by_val(&CV_VECTOR_TYPE_FCT(CV_TYPE,_insert_sorted_by_val)),    \
            remove_at(&CV_VECTOR_TYPE_FCT(CV_TYPE,_remove_at)),remove_range_at(&CV_VECTOR_TYPE_FCT(CV_TYPE,_remove_range_at)),  \
            cpy(&CV_VECTOR_TYPE_FCT(CV_TYPE,_cpy)),dbg_check(&CV_VECTOR_TYPE_FCT(CV_TYPE,_dbg_check)),  \
            serialize(&CV_VECTOR_TYPE_FCT(CV_TYPE,_serialize)),deserialize(&CV_VECTOR_TYPE_FCT(CV_TYPE,_deserialize))
#   else /*__cplusplus*/
#       define CV_FAKE_MEMBER_FUNCTIONS_DEF_CPP_CHUNK(CV_TYPE)  /*no-op*/
#   endif /*__cplusplus*/
#else /*CV_DISABLE_FAKE_MEMBER_FUNCTIONS*/
#   define CV_FAKE_MEMBER_FUNCTIONS_DEF_CHUNK0(CV_TYPE)  /*no-op*/
#   define CV_FAKE_MEMBER_FUNCTIONS_DEF_CHUNK1(CV_TYPE)  /*no-op*/
#   define CV_FAKE_MEMBER_FUNCTIONS_DEF_CPP_CHUNK(CV_TYPE)  /*no-op*/
#endif /*CV_DISABLE_FAKE_MEMBER_FUNCTIONS*/


#ifdef __cplusplus
#   define CV_CPP_DEFINITION_CHUNK0(CV_TYPE)    \
        CV_VECTOR_TYPE(CV_TYPE)::CV_VECTOR_TYPE(CV_TYPE)() :  \
            v(NULL),size(0),capacity(0),    \
            item_ctr(NULL),item_dtr(NULL),item_cpy(NULL),item_cmp(NULL),item_serialize(NULL),item_deserialize(NULL)    \
            CV_FAKE_MEMBER_FUNCTIONS_DEF_CPP_CHUNK(CV_TYPE)   \
        {}  \
            \
        CV_VECTOR_TYPE(CV_TYPE)::CV_VECTOR_TYPE(CV_TYPE)(const CV_VECTOR_TYPE(CV_TYPE)& o) :   \
            v(NULL),size(0),capacity(0),    \
            item_ctr(o.item_ctr),item_dtr(o.item_dtr),item_cpy(o.item_cpy),item_cmp(o.item_cmp),item_serialize(o.item_serialize),item_deserialize(o.item_deserialize)    \
            CV_FAKE_MEMBER_FUNCTIONS_DEF_CPP_CHUNK(CV_TYPE)   \
        {   \
            CV_VECTOR_TYPE_FCT(CV_TYPE,_cpy)(this,&o);  \
        }   \
        \
        CV_VECTOR_TYPE(CV_TYPE)& CV_VECTOR_TYPE(CV_TYPE)::operator=(const CV_VECTOR_TYPE(CV_TYPE)& o)    {CV_VECTOR_TYPE_FCT(CV_TYPE,_cpy)(this,&o);return *this;} \
        \
        CV_VECTOR_TYPE(CV_TYPE)::~CV_VECTOR_TYPE(CV_TYPE)() {CV_VECTOR_TYPE_FCT(CV_TYPE,_free)(this);}
#       ifdef CV_HAS_MOVE_SEMANTICS
#           define CV_CPP_DEFINITION_CHUNK1(CV_TYPE)    \
                CV_VECTOR_TYPE(CV_TYPE)::CV_VECTOR_TYPE(CV_TYPE)(CV_VECTOR_TYPE(CV_TYPE)&& o) :    \
                    v(o.v),size(o.size),capacity(o.capacity),   \
                    item_ctr(o.item_ctr),item_dtr(o.item_dtr),item_cpy(o.item_cpy),item_cmp(o.item_cmp),item_serialize(o.item_serialize),item_deserialize(o.item_deserialize)    \
                    CV_FAKE_MEMBER_FUNCTIONS_DEF_CPP_CHUNK(CV_TYPE)   \
                {   \
                    o.v=NULL;*((size_t*)&o.size)=0;*((size_t*)&o.capacity)=0;   \
                }   \
                \
                CV_VECTOR_TYPE(CV_TYPE)& CV_VECTOR_TYPE(CV_TYPE)::operator=(CV_VECTOR_TYPE(CV_TYPE)&& o)    {  \
                    if (this != &o) {   \
                        CV_VECTOR_TYPE_FCT(CV_TYPE,_free)(this);    \
                        v=o.v;  \
                        *((size_t*)&size)=o.size;*((size_t*)&capacity)=o.capacity;  \
                        o.v=NULL;*((size_t*)&o.size)=0;*((size_t*)&o.capacity)=0;   \
                    }   \
                    return *this;   \
                }
#       else /*CV_HAS_MOVE_SEMANTICS*/ 
#           define CV_CPP_DEFINITION_CHUNK1(CV_TYPE)    /*no-op*/     
#       endif /*CV_HAS_MOVE_SEMANTICS*/
#else /*__cplusplus*/
#   define CV_CPP_DEFINITION_CHUNK0(CV_TYPE)   /*no-op*/
#   define CV_CPP_DEFINITION_CHUNK1(CV_TYPE)   /*no-op*/
#endif /*__cplusplus*/


#ifdef CV_ENABLE_CLEARING_ITEM_MEMORY
#   define CV_CLEARING_ITEM_MEMORY_CHUNK0(CV_TYPE)  \
        if (v->item_ctr || v->item_cpy) {CV_ASSERT(v->v);CV_MEMSET(&v->v[v->size],0,(size-v->size)*sizeof(CV_TYPE));}
#   define CV_CLEARING_ITEM_MEMORY_CHUNK1(CV_TYPE)  \
        if (v->item_ctr || v->item_cpy) CV_MEMSET(&v->v[v->size],0,sizeof(CV_TYPE));
#   define CV_CLEARING_ITEM_MEMORY_CHUNK2(CV_TYPE)  \
        if (v->item_ctr || v->item_cpy) CV_MEMSET(&v->v[position],0,sizeof(CV_TYPE));
#   define CV_CLEARING_ITEM_MEMORY_CHUNK3(CV_TYPE)  \
        if (v->item_ctr || v->item_cpy) CV_MEMSET(v_val,0,num_items_to_insert*sizeof(CV_TYPE));
#   define CV_CLEARING_ITEM_MEMORY_CHUNK4(CV_TYPE)  \
        if (v->item_ctr || v->item_cpy) CV_MEMSET(&v->v[start_position],0,num_items_to_insert*sizeof(CV_TYPE));
#else /*CV_ENABLE_CLEARING_ITEM_MEMORY*/
#   define CV_CLEARING_ITEM_MEMORY_CHUNK0(CV_TYPE)   /*no-op*/
#   define CV_CLEARING_ITEM_MEMORY_CHUNK1(CV_TYPE)   /*no-op*/
#   define CV_CLEARING_ITEM_MEMORY_CHUNK2(CV_TYPE)   /*no-op*/
#   define CV_CLEARING_ITEM_MEMORY_CHUNK3(CV_TYPE)   /*no-op*/
#   define CV_CLEARING_ITEM_MEMORY_CHUNK4(CV_TYPE)   /*no-op*/
#endif /*CV_ENABLE_CLEARING_ITEM_MEMORY*/



#define CV_DEFINE(CV_TYPE)	\
CV_API void CV_TYPE_FCT(CV_TYPE,_default_item_cpy)(CV_TYPE* a,const CV_TYPE* b) {   \
    /*CV_ASSERT(a);CV_ASSERT(b);*/  \
    CV_MEMCPY(a,b,sizeof(CV_TYPE));    \
}  \
/* 'cv_xxx_init(...)' and 'cv_xxx_free(...)' can be thought as the ctr and the dct of the 'cv_xxx' vector struct */   \
/* Mandatory call at the end to free memory. The vector can be reused after this call. The function can be safely re-called multiple times  */  \
CV_API_DEF void CV_VECTOR_TYPE_FCT(CV_TYPE,_free)(CV_VECTOR_TYPE(CV_TYPE)* v)	{   \
	if (v)	{   \
		if (v->v) { \
			if (v->item_dtr)	{			\
				size_t i;   \
				for (i=0;i<v->size;i++)	v->item_dtr(&v->v[i]);  \
			}   \
			cv_free(v->v);v->v=NULL;    \
		}	\
		*((size_t*) &v->size)=0;    \
		*((size_t*) &v->capacity)=0;    \
        /* we don't clear the other fields */   \
	}	\
    CV_ASSERT(!v->v && v->size==0 && v->capacity==0);  \
}   \
/* Same as 'cv_xxx_free(...)', but it does not free the memory (= the vector capacity)  */  \
CV_API_DEF void CV_VECTOR_TYPE_FCT(CV_TYPE,_clear)(CV_VECTOR_TYPE(CV_TYPE)* v)	{   \
	if (v)	{   \
		if (v->v) { \
			if (v->item_dtr)	{			\
				size_t i;   \
				for (i=0;i<v->size;i++)	v->item_dtr(&v->v[i]);  \
			}   \
		}	\
		*((size_t*) &v->size)=0;    \
	}	\
}   \
CV_API_DEF void CV_VECTOR_TYPE_FCT(CV_TYPE,_swap)(CV_VECTOR_TYPE(CV_TYPE)* a,CV_VECTOR_TYPE(CV_TYPE)* b)  {   \
    /*unsigned char t[sizeof(CV_VECTOR_TYPE(CV_TYPE))];*/    \
    if (a!=b)   {   \
        CV_ASSERT(a && b);  \
        {   \
            /* general swap code here */    \
            /*CV_MEMCPY(t,a,sizeof(CV_VECTOR_TYPE(CV_TYPE)));*/ \
            /*CV_MEMCPY(a,b,sizeof(CV_VECTOR_TYPE(CV_TYPE)));*/ \
            /*CV_MEMCPY(b,t,sizeof(CV_VECTOR_TYPE(CV_TYPE)));*/ \
            /* nope, we just swap 3 values */   \
            {CV_TYPE* tmp=a->v;a->v=b->v;b->v=tmp;} \
            {size_t tmp=a->size;*((size_t*)&a->size)=b->size;*((size_t*)&b->size)=tmp;}   \
            {size_t tmp=a->capacity;*((size_t*)&a->capacity)=b->capacity;*((size_t*)&b->capacity)=tmp;}   \
        }   \
    }   \
}   \
CV_API_DEF void CV_VECTOR_TYPE_FCT(CV_TYPE,_reserve)(CV_VECTOR_TYPE(CV_TYPE)* v,size_t size)	{   \
    /*printf("ok %s (sizeof(%s)=%" CV_SIZE_T_FORMATTING ")\n",CV_XSTR(CV_VECTOR_TYPE_FCT(CV_TYPE,_reserve)),CV_XSTR(CV_TYPE),sizeof(CV_TYPE));*/  \
	CV_ASSERT(v);    \
	/* grows-only! */   \
    if (size>v->capacity) {		    \
        const size_t new_capacity = (v->capacity==0) ?    \
                    size :      /* possibly keep initial user-guided 'reserve(...)' */  \
                    (v->capacity+(size-v->capacity)+(v->capacity)/2);   /* our growing strategy */  \
        cv_safe_realloc((void**) &v->v,new_capacity*sizeof(CV_TYPE)); \
        *((size_t*) &v->capacity) = new_capacity;   \
	}   \
}   \
CV_API_DEF void CV_VECTOR_TYPE_FCT(CV_TYPE,_resize)(CV_VECTOR_TYPE(CV_TYPE)* v,size_t size)	{   \
	/*printf("%s\n",CV_XSTR(CV_VECTOR_TYPE_FCT(CV_TYPE,_resize)));*/    \
	CV_ASSERT(v);  \
    if (size>v->capacity) CV_VECTOR_TYPE_FCT(CV_TYPE,_reserve)(v,size); \
    if (size<v->size)   {if (v->item_dtr) {size_t i;for (i=size;i<v->size;i++) v->item_dtr(&v->v[i]);}} \
    else {  \
        CV_CLEARING_ITEM_MEMORY_CHUNK0(CV_TYPE) \
        if (v->item_ctr) {size_t i;for (i=v->size;i<size;i++) v->item_ctr(&v->v[i]);}   \
    }   \
    *((size_t*) &v->size)=size; \
}   \
CV_API_DEF void CV_VECTOR_TYPE_FCT(CV_TYPE,_resize_with)(CV_VECTOR_TYPE(CV_TYPE)* v,size_t size,const CV_TYPE* default_value)	{   \
	CV_ASSERT(v);   \
	if (!default_value) {CV_VECTOR_TYPE_FCT(CV_TYPE,_resize)(v,size);return;}   \
    if (size>v->capacity) CV_VECTOR_TYPE_FCT(CV_TYPE,_reserve)(v,size); \
    if (size<v->size)   {if (v->item_dtr) {size_t i;for (i=size;i<v->size;i++) v->item_dtr(&v->v[i]);}} \
    else {  \
        size_t i;   \
        void (* const item_cpy)(CV_TYPE*,const CV_TYPE*) = v->item_cpy ? v->item_cpy : &(CV_TYPE_FCT(CV_TYPE,_default_item_cpy));   \
        CV_CLEARING_ITEM_MEMORY_CHUNK0(CV_TYPE) \
        if (v->item_ctr)    {   \
            for (i=v->size;i<size;i++) {v->item_ctr(&v->v[i]);item_cpy(&v->v[i],default_value);}    \
        }   \
        else    {for (i=v->size;i<size;i++) {item_cpy(&v->v[i],default_value);}}    \
    }   \
    *((size_t*) &v->size)=size; \
}   \
CV_API_DEF void CV_VECTOR_TYPE_FCT(CV_TYPE,_resize_with_by_val)(CV_VECTOR_TYPE(CV_TYPE)* v,size_t size,const CV_TYPE default_value)	{CV_VECTOR_TYPE_FCT(CV_TYPE,_resize_with)(v,size,&default_value);}  \
CV_API_DEF void CV_VECTOR_TYPE_FCT(CV_TYPE,_push_back)(CV_VECTOR_TYPE(CV_TYPE)* v,const CV_TYPE* value)  {   \
	void (*item_cpy)(CV_TYPE*,const CV_TYPE*);  \
    CV_ASSERT(v);   \
    item_cpy = v->item_cpy ? v->item_cpy : &(CV_TYPE_FCT(CV_TYPE,_default_item_cpy));   \
        if (v->v && value>=v->v && value<(v->v+v->size))  { \
            /* value ia a pointer to another vector item here */    \
            CV_TYPE v_val;CV_MEMSET(&v_val,0,sizeof(CV_TYPE)); \
            if (v->item_ctr) v->item_ctr(&v_val);   \
            item_cpy(&v_val,value); \
            if (v->size == v->capacity) {CV_VECTOR_TYPE_FCT(CV_TYPE,_reserve)(v,v->size+1);}    \
            CV_CLEARING_ITEM_MEMORY_CHUNK1(CV_TYPE) \
            if (v->item_ctr) v->item_ctr(&v->v[v->size]);   \
            item_cpy(&v->v[v->size],&v_val);    \
            if (v->item_dtr) v->item_dtr(&v_val);   \
        }   \
        else {  \
            CV_ASSERT(v->v || v->size == v->capacity); /* to silence a clang static analyzer warning */   \
            if (v->size == v->capacity) {CV_VECTOR_TYPE_FCT(CV_TYPE,_reserve)(v,v->size+1);}    \
            CV_ASSERT(v->v && v->size<=v->capacity); /* to silence a clang static analyzer warning */  \
            CV_CLEARING_ITEM_MEMORY_CHUNK1(CV_TYPE) \
            if (v->item_ctr) v->item_ctr(&v->v[v->size]);   \
            item_cpy(&v->v[v->size],value); /*Null pointer passed tp 1st parameter expecting 'nonnull' [clang-analyzer-core.NonNullParamChecker] */ \
        }   \
	*((size_t*) &v->size)=v->size+1;    \
}   \
CV_API_DEF void CV_VECTOR_TYPE_FCT(CV_TYPE,_push_back_by_val)(CV_VECTOR_TYPE(CV_TYPE)* v,const CV_TYPE value)  {CV_VECTOR_TYPE_FCT(CV_TYPE,_push_back)(v,&value);}   \
CV_API_DEF void CV_VECTOR_TYPE_FCT(CV_TYPE,_pop_back)(CV_VECTOR_TYPE(CV_TYPE)* v)	{   \
   CV_ASSERT(v && v->size>0);   \
   if (v->size>0) {*((size_t*) &v->size)=v->size-1;if (v->item_dtr) v->item_dtr(&v->v[v->size]);}   \
}   \
CV_API_DEF size_t CV_VECTOR_TYPE_FCT(CV_TYPE,_linear_search)(const CV_VECTOR_TYPE(CV_TYPE)* v,const CV_TYPE* item_to_search,int* match)  {   \
    int cmp=0;size_t i; \
    CV_ASSERT(v && v->item_cmp);    \
    if (match) *match=0;    \
    if (v->size==0) return 0;  /* otherwise match will be 1 */  \
    for (i = 0; i < v->size; i++) { \
        cmp = v->item_cmp(item_to_search,&v->v[i]); \
        if (cmp<=0) {   \
            if (cmp==0 && match) *match=1;  \
            return i;   \
        }   \
    }   \
    CV_ASSERT(i==v->size);  \
    return i;   \
}   \
CV_API_DEF size_t CV_VECTOR_TYPE_FCT(CV_TYPE,_linear_search_by_val)(const CV_VECTOR_TYPE(CV_TYPE)* v,const CV_TYPE item_to_search,int* match)  {return CV_VECTOR_TYPE_FCT(CV_TYPE,_linear_search)(v,&item_to_search,match);} \
CV_API_DEF size_t CV_VECTOR_TYPE_FCT(CV_TYPE,_binary_search)(const CV_VECTOR_TYPE(CV_TYPE)* v,const CV_TYPE* item_to_search,int* match)  {   \
    size_t first=0, last;   \
    size_t mid;int cmp; \
    CV_ASSERT(v && v->item_cmp);    \
    if (match) *match=0;    \
    if (v->size==0) return 0;  /* otherwise match will be 1 */  \
	last=v->size-1; \
    while (first <= last) { \
        mid = (first + last) / 2;   \
        cmp = v->item_cmp(item_to_search,&v->v[mid]);   \
        if (cmp>0) {    \
            first = mid + 1;    \
        }   \
        else if (cmp<0) {   \
            if (mid==0) return 0;   \
            last = mid - 1; \
        }   \
        else {if (match) *match=1;CV_ASSERT(mid<v->size); return mid;}  \
    }   \
    CV_ASSERT(mid<v->size); \
    return cmp>0 ? (mid+1) : mid;   \
}   \
CV_API_DEF size_t CV_VECTOR_TYPE_FCT(CV_TYPE,_binary_search_by_val)(const CV_VECTOR_TYPE(CV_TYPE)* v,const CV_TYPE item_to_search,int* match)    {return CV_VECTOR_TYPE_FCT(CV_TYPE,_binary_search)(v,&item_to_search,match);}   \
CV_API_DEF size_t CV_VECTOR_TYPE_FCT(CV_TYPE,_insert_at)(CV_VECTOR_TYPE(CV_TYPE)* v,const CV_TYPE* item_to_insert,size_t position)  {    \
    /* position is in [0,v->size] */    \
    void (*item_cpy)(CV_TYPE*,const CV_TYPE*);  \
    CV_ASSERT(v && position<=v->size);  \
    item_cpy = v->item_cpy ? v->item_cpy : &(CV_TYPE_FCT(CV_TYPE,_default_item_cpy));   \
    if (v->v && item_to_insert>=v->v && item_to_insert<(v->v+v->size))  {   \
        CV_TYPE v_val;CV_MEMSET(&v_val,0,sizeof(CV_TYPE)); \
    \
        if (v->item_ctr) v->item_ctr(&v_val);   \
        item_cpy(&v_val,item_to_insert);    \
    \
        if (v->size == v->capacity) {CV_VECTOR_TYPE_FCT(CV_TYPE,_reserve)(v,v->size+1);}    \
        if (position<v->size) CV_MEMMOVE(&v->v[position+1],&v->v[position],(v->size-position)*sizeof(CV_TYPE));    \
        CV_CLEARING_ITEM_MEMORY_CHUNK2(CV_TYPE) \
        if (v->item_ctr) v->item_ctr(&v->v[position]);  \
        item_cpy(&v->v[position],&v_val);   \
        if (v->item_dtr) v->item_dtr(&v_val);   \
    }   \
    else    {   \
        CV_ASSERT(v->v || v->size == v->capacity); /* to silence a clang static analyzer warning */   \
        if (v->size == v->capacity) {CV_VECTOR_TYPE_FCT(CV_TYPE,_reserve)(v,v->size+1);}    \
        CV_ASSERT(v->v && v->size<=v->capacity); /* to silence a clang static analyzer warning */  \
        if (position<v->size) CV_MEMMOVE(&v->v[position+1],&v->v[position],(v->size-position)*sizeof(CV_TYPE));    \
        CV_CLEARING_ITEM_MEMORY_CHUNK2(CV_TYPE) \
        if (v->item_ctr) v->item_ctr(&v->v[position]);  \
        item_cpy(&v->v[position],item_to_insert);   \
    }   \
    *((size_t*) &v->size)=v->size+1;    \
	return position;    \
}   \
CV_API_DEF size_t CV_VECTOR_TYPE_FCT(CV_TYPE,_insert_at_by_val)(CV_VECTOR_TYPE(CV_TYPE)* v,const CV_TYPE item_to_insert,size_t position)   {return CV_VECTOR_TYPE_FCT(CV_TYPE,_insert_at)(v,&item_to_insert,position);}  \
CV_API_DEF size_t CV_VECTOR_TYPE_FCT(CV_TYPE,_insert_range_at)(CV_VECTOR_TYPE(CV_TYPE)* v,const CV_TYPE* items_to_insert,size_t num_items_to_insert,size_t start_position)  {    \
    /* position is in [0,v->size] */    \
    const size_t end_position = start_position+num_items_to_insert;size_t i;    \
    CV_TYPE* v_val=NULL;const CV_TYPE* pitems=items_to_insert;  \
    CV_ASSERT(v && start_position<=v->size);    \
    if (num_items_to_insert==0) return start_position;  \
    if (v->v && (items_to_insert+num_items_to_insert)>=v->v && items_to_insert<(v->v+v->size))  {   \
        v_val = (CV_TYPE*) cv_malloc(num_items_to_insert*sizeof(CV_TYPE));  \
        CV_CLEARING_ITEM_MEMORY_CHUNK3(CV_TYPE) \
        if (v->item_cpy)	{   \
            if (v->item_ctr)	{   \
                for (i=0;i<num_items_to_insert;i++)   { \
                    v->item_ctr(&v_val[i]); \
                    v->item_cpy(&v_val[i],&items_to_insert[i]); \
                }   \
            }   \
            else	{for (i=0;i<num_items_to_insert;i++)   v->item_cpy(&v_val[i],&items_to_insert[i]);} \
        }   \
        else	{   \
            if (v->item_ctr)	{for (i=0;i<num_items_to_insert;i++)   v->item_ctr(&v_val[i]);} \
            CV_MEMCPY(&v_val[0],&items_to_insert[0],num_items_to_insert*sizeof(CV_TYPE));  \
        }   \
        pitems = v_val; \
    }   \
    if (v->size+num_items_to_insert > v->capacity) {    \
        CV_VECTOR_TYPE_FCT(CV_TYPE,_reserve)(v,v->size+num_items_to_insert);    \
    }   \
    CV_ASSERT(v->v);    \
    if (start_position<v->size) CV_MEMMOVE(&v->v[end_position],&v->v[start_position],(v->size-start_position)*sizeof(CV_TYPE));    \
    CV_CLEARING_ITEM_MEMORY_CHUNK4(CV_TYPE) \
    if (v->item_cpy)	{   \
        if (v->item_ctr)	{   \
            for (i=start_position;i<end_position;i++)   {   \
                v->item_ctr(&v->v[i]);  \
                v->item_cpy(&v->v[i],&pitems[i-start_position]);    \
            }   \
        }   \
        else	{for (i=start_position;i<end_position;i++)   v->item_cpy(&v->v[i],&pitems[i-start_position]);}  \
    }   \
    else	{   \
        if (v->item_ctr)	{for (i=start_position;i<end_position;i++)   v->item_ctr(&v->v[i]);}    \
        CV_MEMCPY(&v->v[start_position],&pitems[0],num_items_to_insert*sizeof(CV_TYPE));   \
    }   \
    if (v_val) {    \
        if (v->item_dtr)	{for (i=0;i<num_items_to_insert;i++)   v->item_dtr(&v_val[i]);} \
        cv_free(v_val);v_val=NULL;  \
    }   \
    *((size_t*) &v->size)=v->size+num_items_to_insert;  \
    return start_position;  \
 }	\
CV_API_DEF size_t CV_VECTOR_TYPE_FCT(CV_TYPE,_insert_sorted)(CV_VECTOR_TYPE(CV_TYPE)* v,const CV_TYPE* item_to_insert,int* match,int insert_even_if_item_match)  {   \
    int my_match = 0;size_t position;   \
	position = CV_VECTOR_TYPE_FCT(CV_TYPE,_binary_search)(v,item_to_insert,&my_match);  \
    if (match) *match = my_match;   \
    if (my_match && !insert_even_if_item_match) return position;    \
    CV_VECTOR_TYPE_FCT(CV_TYPE,_insert_at)(v,item_to_insert,position);  \
    return position;    \
}   \
CV_API_DEF size_t CV_VECTOR_TYPE_FCT(CV_TYPE,_insert_sorted_by_val)(CV_VECTOR_TYPE(CV_TYPE)* v,const CV_TYPE item_to_insert,int* match,int insert_even_if_item_match)  {return CV_VECTOR_TYPE_FCT(CV_TYPE,_insert_sorted)(v,&item_to_insert,match,insert_even_if_item_match);}   \
CV_API_DEF int CV_VECTOR_TYPE_FCT(CV_TYPE,_remove_at)(CV_VECTOR_TYPE(CV_TYPE)* v,size_t position)  { \
    /* position is in [0,num_items) */  \
    int removal_ok; \
	CV_ASSERT(v);   \
	removal_ok = (position<v->size) ? 1 : 0;    \
    CV_ASSERT(removal_ok);	/* error: position>=v->size */  \
	if (removal_ok)	{   \
        if (v->item_dtr) v->item_dtr(&v->v[position]);  \
        CV_MEMMOVE(&v->v[position],&v->v[position+1],(v->size-position-1)*sizeof(CV_TYPE));    \
        *((size_t*) &v->size)=v->size-1;    \
    }   \
    return removal_ok;  \
}   \
CV_API_DEF int CV_VECTOR_TYPE_FCT(CV_TYPE,_remove_range_at)(CV_VECTOR_TYPE(CV_TYPE)* v,size_t start_item_position,size_t num_items_to_remove)  { \
    /* (start_item_position+num_items_to_remove) is <= size */  \
    const size_t end_item_position = start_item_position+num_items_to_remove;   \
    int removal_ok;size_t i;    \
    CV_ASSERT(v);   \
    removal_ok = end_item_position<=v->size ? 1 : 0;    \
    CV_ASSERT(removal_ok);	/* error: start_item_position + num_items_to_remove > v.size */	    \
    if (removal_ok && num_items_to_remove>0)	{   \
        if (v->item_dtr) {for (i=start_item_position;i<end_item_position;i++) v->item_dtr(&v->v[i]);}   \
        if (end_item_position<v->size) CV_MEMMOVE(&v->v[start_item_position],&v->v[end_item_position],(v->size-end_item_position)*sizeof(CV_TYPE));    \
        *((size_t*) &v->size)=v->size-num_items_to_remove;  \
    }   \
    return removal_ok;  \
}   \
CV_API_DEF void CV_VECTOR_TYPE_FCT(CV_TYPE,_cpy)(CV_VECTOR_TYPE(CV_TYPE)* a,const CV_VECTOR_TYPE(CV_TYPE)* b) {   \
    size_t i;   \
    /*typedef void (*item_ctr_dtr_type)(CV_TYPE*);*/    \
    /*typedef void (*item_cpy_type)(CV_TYPE*,const CV_TYPE*);*/ \
    /*typedef int (*item_cmp_type)(const CV_CMP_TYPE(CV_TYPE)*,const CV_CMP_TYPE(CV_TYPE)*);*/    \
    /*typedef void (*item_serialize_type)(const CV_TYPE*,cvh_serializer_t*);*/ \
    /*typedef int (*item_deserialize_type)(CV_TYPE*,const cvh_serializer_t*);*/ \
    if (a==b || (a->size==0 && b->size==0)) return;   \
    CV_ASSERT(a && b);  \
    /* bad init asserts */  \
    CV_ASSERT(!(a->v && a->capacity==0));   \
    CV_ASSERT(!(!a->v && a->capacity>0));   \
    CV_ASSERT(!(b->v && b->capacity==0));   \
    CV_ASSERT(!(!b->v && b->capacity>0));   \
    /*CV_VECTOR_TYPE_FCT(CV_TYPE,_free)(a);*/   \
    CV_VECTOR_TYPE_FCT(CV_TYPE,_clear)(a);  \
    /**((item_ctr_dtr_type*)&a->item_ctr)=b->item_ctr;*/ \
    /**((item_ctr_dtr_type*)&a->item_dtr)=b->item_dtr;*/    \
    /**((item_cpy_type*)&a->item_cpy)=b->item_cpy;*/    \
    /**((item_cmp_type*)&a->item_cmp)=b->item_cmp;*/    \
    /**((item_serialize_type*)&a->item_serialize)=b->item_serialize;*/   \
    /**((item_deserialize_type*)&a->item_deserialize)=b->item_deserialize;*/   \
    CV_ASSERT(a->item_ctr==b->item_ctr && a->item_dtr==b->item_dtr && a->item_cpy==b->item_cpy &&   \
              a->item_serialize==b->item_serialize &&  a->item_deserialize==b->item_deserialize && \
              "One of the two vectors has not been properly initialized");    \
    CV_VECTOR_TYPE_FCT(CV_TYPE,_resize)(a,b->size); \
    CV_ASSERT(((a->v && b->v) || (!a->v && !b->v)) && a->size==b->size);  \
    if (!a->item_cpy)   {CV_MEMCPY(&a->v[0],&b->v[0],a->size*sizeof(CV_TYPE));}    \
    else    {for (i=0;i<a->size;i++) a->item_cpy(&a->v[i],&b->v[i]);}   \
}   \
CV_API_DEF void CV_VECTOR_TYPE_FCT(CV_TYPE,_shrink_to_fit)(CV_VECTOR_TYPE(CV_TYPE)* v)	{   \
    if (v)	{   \
        CV_VECTOR_TYPE(CV_TYPE) o = CV_VECTOR_TYPE_FCT(CV_TYPE,_create_with)(v->item_cmp,v->item_ctr,v->item_dtr,v->item_cpy,v->item_serialize,v->item_deserialize);  \
        CV_VECTOR_TYPE_FCT(CV_TYPE,_cpy)(&o,v); /* now 'o' is 'v' trimmed */    \
        CV_VECTOR_TYPE_FCT(CV_TYPE,_free)(v);   \
        CV_VECTOR_TYPE_FCT(CV_TYPE,_swap)(&o,v);    \
    }   \
}   \
CV_API_DEF void CV_VECTOR_TYPE_FCT(CV_TYPE,_dbg_check)(const CV_VECTOR_TYPE(CV_TYPE)* v)  {  \
    size_t j,num_sorting_errors=0;  \
    const size_t mem_minimal=sizeof(CV_VECTOR_TYPE(CV_TYPE))+sizeof(CV_TYPE)*v->size;    \
    const size_t mem_used=sizeof(CV_VECTOR_TYPE(CV_TYPE))+sizeof(CV_TYPE)*v->capacity;   \
    const double mem_used_percentage = (double)mem_used*100.0/(double)mem_minimal;  \
    CV_ASSERT(v);   \
    /* A potemtial problem here is that sometimes users set a 'v->item_cmp' without using it in a sorted vector...  \
       So in case of sorting errors, we don't assert, but still display them using fprintf(stderr,...) */   \
    if (v->item_cmp && v->size)    {    \
        const CV_TYPE* last_item = NULL;    \
        for (j=0;j<v->size;j++)  {  \
            const CV_TYPE* item = &v->v[j]; \
            if (last_item) {    \
                if (v->item_cmp(last_item,item)>0) {    \
                    /* When this happens, it can be a wrong user 'item_cmp' function (that cannot sort items in a consistent way) */                        \
                    ++num_sorting_errors;   \
                    CV_CHUNK_NO_STDIO_0(CV_TYPE)    \
                }   \
            }   \
            last_item=item; \
        }   \
    }   \
    CV_CHUNK_NO_STDIO_1(CV_TYPE)    \
    /*CV_ASSERT(num_sorting_errors==0);*/ /* When this happens, it can be a wrong user 'itemKey_cmp' function (that cannot sort keys in a consistent way) */    \
}   \
CV_API_DEF void CV_VECTOR_TYPE_FCT(CV_TYPE,_serialize)(const CV_VECTOR_TYPE(CV_TYPE)* v,cvh_serializer_t* serializer)  {    \
    const size_t size_t_size_in_bytes = sizeof(size_t); \
    CV_ASSERT(v && serializer);  \
    cvh_serializer_reserve(serializer,serializer->size + size_t_size_in_bytes); /* space reserved for v->size (size_t_size_in_bytes) */  \
    CV_ASSERT(serializer->v);   \
    *((size_t*) (&serializer->v[serializer->size])) = v->size;serializer->size+=size_t_size_in_bytes; /* v->size written, now the items: */   \
    if (v->item_serialize)  {size_t i;for(i=0;i<v->size;i++) v->item_serialize(&v->v[i],serializer);} /* serializer->size is incremented by 'v->item_serialize' */ \
    else {  \
        const size_t v_size_in_bytes = v->size*sizeof(CV_TYPE);   \
        cvh_serializer_reserve(serializer,serializer->size + v_size_in_bytes); /* space reserved for all the items (v_size_in_bytes) */  \
        CV_ASSERT(serializer->v);   \
        CV_MEMCPY(&serializer->v[serializer->size],v->v,v_size_in_bytes);serializer->size+=v_size_in_bytes; /* serializer->size must be incremented */ \
    } \
}   \
CV_API_DEF int CV_VECTOR_TYPE_FCT(CV_TYPE,_deserialize)(CV_VECTOR_TYPE(CV_TYPE)* v,const cvh_serializer_t* deserializer)  {    \
    /* We must start deserialization from the 'mutable' reader offset: 'deserializer->offset', and then increment it step by step */    \
    const size_t size_t_size_in_bytes = sizeof(size_t); \
    size_t vsize;int check;   \
    CV_ASSERT(v && deserializer);  \
    check = (deserializer->offset+size_t_size_in_bytes<=deserializer->size);    \
    CV_ASSERT(check && "missing space to deserialize 'size_t_size_in_bytes'");  /* otherwise deserialization will fail */ \
    if (!check) return 0; /* but when we compile with NDEBUG, or CV_NO_ASSERT, the caller must be notified of the failure */  \
    vsize = *((size_t*) &deserializer->v[deserializer->offset]);*((size_t*) &deserializer->offset)+=size_t_size_in_bytes; /* 'vsize' read, 'deserializer->offset' incremented */ \
    CV_VECTOR_TYPE_FCT(CV_TYPE,_resize)(v,vsize);/*CV_ASSERT(v->size==vsize);*/ /* now that 'v->size==vsize', we can start deserializing items: */   \
    CV_ASSERT(v->v);\
    if (v->item_deserialize)  {size_t i;for(i=0;i<vsize;i++) {if (!v->item_deserialize(&v->v[i],deserializer)) return 0;}} /* 'deserializer->offset' is incremented by 'v->item_deserialize' */ \
    else {  \
        size_t v_size_in_bytes = vsize*sizeof(CV_TYPE);   \
        check = deserializer->offset+v_size_in_bytes<=deserializer->size;   \
        CV_ASSERT(check && "missing space to deserialize all items using CV_MEMCPY");  /* otherwise deserialization will fail */   \
        if (!check) return 0;\
        CV_MEMCPY(v->v,&deserializer->v[deserializer->offset],v_size_in_bytes); \
        *((size_t*) &deserializer->offset)+=v_size_in_bytes; /* 'deserializer->offset' must be incremented */ \
    } \
    return 1; /* we must return 1 on success */  \
}   \
    \
    \
/* create methods */    \
CV_API_DEF void CV_VECTOR_TYPE_FCT(CV_TYPE,_init_with)(CV_VECTOR_TYPE(CV_TYPE)* v,int (*item_cmp)(const CV_CMP_TYPE(CV_TYPE)*,const CV_CMP_TYPE(CV_TYPE)*),void (*item_ctr)(CV_TYPE*),void (*item_dtr)(CV_TYPE*),void (*item_cpy)(CV_TYPE*,const CV_TYPE*)    \
                                                        ,void (*item_serialize)(const CV_TYPE*,cvh_serializer_t*),int (*item_deserialize)(CV_TYPE*,const cvh_serializer_t*))	{   \
    typedef void (*item_ctr_dtr_type)(CV_TYPE*);    \
    typedef void (*item_cpy_type)(CV_TYPE*,const CV_TYPE*); \
    typedef int (*item_cmp_type)(const CV_CMP_TYPE(CV_TYPE)*,const CV_CMP_TYPE(CV_TYPE)*);    \
    typedef void (*item_serialize_type)(const CV_TYPE*,cvh_serializer_t*); \
    typedef int (*item_deserialize_type)(CV_TYPE*,const cvh_serializer_t*); \
    CV_FAKE_MEMBER_FUNCTIONS_DEF_CHUNK0(CV_TYPE)    \
    CV_ASSERT(v);   \
    CV_MEMSET(v,0,sizeof(CV_VECTOR_TYPE(CV_TYPE))); \
    *((item_ctr_dtr_type*)&v->item_ctr)=item_ctr;   \
    *((item_ctr_dtr_type*)&v->item_dtr)=item_dtr;   \
    *((item_cpy_type*)&v->item_cpy)=item_cpy;   \
    *((item_cmp_type*)&v->item_cmp)=item_cmp;   \
    *((item_serialize_type*)&v->item_serialize)=item_serialize;   \
    *((item_deserialize_type*)&v->item_deserialize)=item_deserialize;   \
    CV_FAKE_MEMBER_FUNCTIONS_DEF_CHUNK1(CV_TYPE)   \
}   \
CV_API_DEF void CV_VECTOR_TYPE_FCT(CV_TYPE,_init)(CV_VECTOR_TYPE(CV_TYPE)* v,int (*item_cmp)(const CV_CMP_TYPE(CV_TYPE)*,const CV_CMP_TYPE(CV_TYPE)*))  {CV_VECTOR_TYPE_FCT(CV_TYPE,_init_with)(v,item_cmp,NULL,NULL,NULL,NULL,NULL);}  \
CV_API_DEF CV_VECTOR_TYPE(CV_TYPE) CV_VECTOR_TYPE_FCT(CV_TYPE,_create_with)(int (*item_cmp)(const CV_CMP_TYPE(CV_TYPE)*,const CV_CMP_TYPE(CV_TYPE)*),void (*item_ctr)(CV_TYPE*),void (*item_dtr)(CV_TYPE*),void (*item_cpy)(CV_TYPE*,const CV_TYPE*),void (*item_serialize)(const CV_TYPE*,cvh_serializer_t*),int (*item_deserialize)(CV_TYPE*,const cvh_serializer_t*))    {   \
    CV_VECTOR_TYPE(CV_TYPE) v; /* = CV_ZERO_INIT; */  \
    CV_VECTOR_TYPE_FCT(CV_TYPE,_init_with)(&v,item_cmp,item_ctr,item_dtr,item_cpy,item_serialize,item_deserialize);return v;}   \
CV_API_DEF CV_VECTOR_TYPE(CV_TYPE) CV_VECTOR_TYPE_FCT(CV_TYPE,_create)(int (*item_cmp)(const CV_CMP_TYPE(CV_TYPE)*,const CV_CMP_TYPE(CV_TYPE)*))    {   \
    CV_VECTOR_TYPE(CV_TYPE) v; /* = CV_ZERO_INIT; */  \
    CV_VECTOR_TYPE_FCT(CV_TYPE,_init)(&v,item_cmp);return v;}   \
    CV_CPP_DEFINITION_CHUNK0(CV_TYPE)   \
    CV_CPP_DEFINITION_CHUNK1(CV_TYPE)


#define CV_DECLARE_AND_DEFINE(CV_TYPE)               \
    CV_DECLARE(CV_TYPE)                                  \
    CV_DEFINE(CV_TYPE)


/* ------------------------------------------------- */
#endif /* C_VECTOR_H_ */


