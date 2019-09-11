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

   CV_DISABLE_FAKE_MEMBER_FUNCTIONS     // faster with this defined
   CV_DISABLE_CLEARING_ITEM_MEMORY      // faster with this defined
   CV_ENABLE_DECLARATION_AND_DEFINITION // when used, C_VECTOR_TYPE_UNSAFE_IMPLEMENTATION must be
                                        // defined before including this file in a single source (.c) file
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

   HISTORY:

*/


#ifndef C_VECTOR_TYPE_UNSAFE_H
#define C_VECTOR_TYPE_UNSAFE_H

#define C_VECTOR_TYPE_UNSAFE_VERSION            "1.07"
#define C_VECTOR_TYPE_UNSAFE_VERSION_NUM        0107

/* HISTORY
   C_VECTOR_TYPE_UNSAFE_VERSION_NUM 106
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
#ifdef CV_ENABLE_DECLARATION_AND_DEFINITION
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
CV_API_DEC void cvector_create_with(cvector* v,size_t item_size_in_bytes,int (*item_cmp)(const void*,const void*),void (*item_ctr)(void*),void (*item_dtr)(void*),void (*item_cpy)(void*,const void*));
CV_API_DEC void cvector_create(cvector* v,size_t item_size_in_bytes,int (*item_cmp)(const void*,const void*));
#endif /* CV_ENABLE_DECLARATION_AND_DEFINITION */
#endif /* C_VECTOR_TYPE_UNSAFE_H */

#if (!defined(CV_ENABLE_DECLARATION_AND_DEFINITION) || defined(C_VECTOR_TYPE_UNSAFE_IMPLEMENTATION))
#ifndef C_VECTOR_TYPE_UNSAFE_H_IMPLEMENTATION_GUARD
#define C_VECTOR_TYPE_UNSAFE_H_IMPLEMENTATION_GUARD

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
    unsigned char t[sizeof(cvector)];
    if (a!=b)   {
    CV_ASSERT(a && b);
        memcpy(t,a,sizeof(cvector));
        memcpy(a,b,sizeof(cvector));
        memcpy(b,t,sizeof(cvector));
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
#       ifndef CV_DISABLE_CLEARING_ITEM_MEMORY
        if (v->item_ctr || v->item_cpy) memset((unsigned char*)v->v+v->size*v->item_size_in_bytes,0,(size-v->size)*v->item_size_in_bytes);
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
#       ifndef CV_DISABLE_CLEARING_ITEM_MEMORY
        if (v->item_ctr || v->item_cpy) memset((unsigned char*)v->v+v->size*v->item_size_in_bytes,0,(size-v->size)*v->item_size_in_bytes);
#       endif
        if (v->item_cpy)   {
            if (v->item_ctr)    {
                for (i=v->size;i<size;i++) {v->item_ctr((unsigned char*)v->v+i*v->item_size_in_bytes);v->item_cpy((unsigned char*)v->v+i*v->item_size_in_bytes,default_value);}
            }
            else    {for (i=v->size;i<size;i++) {v->item_cpy((unsigned char*)v->v+i*v->item_size_in_bytes,default_value);}}
        }
        else    {
            if (v->item_ctr)    {
                for (i=v->size;i<size;i++) {v->item_ctr((unsigned char*)v->v+i*v->item_size_in_bytes);memcpy((unsigned char*)v->v+i*v->item_size_in_bytes,default_value,v->item_size_in_bytes);}
            }
            else    {for (i=v->size;i<size;i++) {memcpy((unsigned char*)v->v+i*v->item_size_in_bytes,default_value,v->item_size_in_bytes);}}
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
#       ifndef CV_DISABLE_CLEARING_ITEM_MEMORY
        if (v->item_ctr || v->item_cpy) memset(v_val,0,v->item_size_in_bytes);
#		endif
        if (v->item_ctr) v->item_ctr(v_val);
        if (v->item_cpy) v->item_cpy(v_val,value);
        else memcpy(v_val,value,v->item_size_in_bytes);
        pvalue=v_val;
    }
    if (v->size == v->capacity) {cvector_reserve(v,v->size+1);}
#   ifndef CV_DISABLE_CLEARING_ITEM_MEMORY
    if (v->item_ctr || v->item_cpy) memset((unsigned char*)v->v+v->size*v->item_size_in_bytes,0,v->item_size_in_bytes);
#	endif
    if (v->item_ctr) v->item_ctr((unsigned char*)v->v+v->size*v->item_size_in_bytes);
    if (v->item_cpy) v->item_cpy((unsigned char*)v->v+v->size*v->item_size_in_bytes,pvalue);
    else memcpy((unsigned char*)v->v+v->size*v->item_size_in_bytes,pvalue,v->item_size_in_bytes);

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
#		ifndef CV_DISABLE_CLEARING_ITEM_MEMORY
        if (v->item_ctr || v->item_cpy) memset(v_val,0,num_items_to_insert*v->item_size_in_bytes);
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
            memcpy(v_val,pitems,num_items_to_insert*v->item_size_in_bytes);
        }
        pitems = v_val;
    }
    if (v->size+num_items_to_insert > v->capacity) {
        cvector_reserve(v,v->size+num_items_to_insert);
        p = (unsigned char*) v->v;
    }
    if (start_position<v->size) memmove(p+end_position*v->item_size_in_bytes,p+start_position*v->item_size_in_bytes,(v->size-start_position)*v->item_size_in_bytes);
#	ifndef CV_DISABLE_CLEARING_ITEM_MEMORY
    if (v->item_ctr || v->item_cpy) memset(p+start_position*v->item_size_in_bytes,0,num_items_to_insert*v->item_size_in_bytes);
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
        memcpy(p+start_position*v->item_size_in_bytes,pitems,num_items_to_insert*v->item_size_in_bytes);
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
        memmove((unsigned char*)v->v+position*v->item_size_in_bytes,(unsigned char*)v->v+(position+1)*v->item_size_in_bytes,(v->size-position-1)*v->item_size_in_bytes);
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
        if (end_item_position<v->size) memmove((unsigned char*)v->v+start_item_position*v->item_size_in_bytes,(unsigned char*)v->v+end_item_position*v->item_size_in_bytes,(v->size-end_item_position)*v->item_size_in_bytes);
        *((size_t*) &v->size)=v->size-num_items_to_remove;
    }
    return removal_ok;
}
CV_API_DEF void cvector_cpy(cvector* a,const cvector* b) {
    size_t i;
    typedef int (*item_cmp_type)(const void*,const void*);
	typedef void (*item_ctr_dtr_type)(void*);
	typedef void (*item_cpy_type)(void*,const void*);
    if (a==b) return;
    CV_ASSERT(a && b);
    /* bad init asserts */
    CV_ASSERT(!(a->v && a->capacity==0));
    CV_ASSERT(!(!a->v && a->capacity>0));
    CV_ASSERT(!(b->v && b->capacity==0));
    CV_ASSERT(!(!b->v && b->capacity>0));
    if (a->item_size_in_bytes==0) *((size_t*)&a->item_size_in_bytes)=b->item_size_in_bytes;
    CV_ASSERT(a->item_size_in_bytes==b->item_size_in_bytes);   /* can't cpy two different vectors! */
    if (a->item_size_in_bytes!=b->item_size_in_bytes)   {
#       ifndef CV_NO_STDIO
        fprintf(stderr,"[cvector_cpy] Error: two vector with different 'item_size_in_bytes' (%lu != %lu) can't be copied.\n",a->item_size_in_bytes,b->item_size_in_bytes);
#       endif
        return;
    }
    /*cv_free(a);*/
    cvector_clear(a);
    *((item_cmp_type*)&a->item_cmp)=b->item_cmp;
	*((item_ctr_dtr_type*)&a->item_ctr)=b->item_ctr;
    *((item_ctr_dtr_type*)&a->item_dtr)=b->item_dtr;
    *((item_cpy_type*)&a->item_cpy)=b->item_cpy;
    cvector_resize(a,b->size);
    CV_ASSERT(a->size==b->size);
    if (!a->item_cpy)   {memcpy((unsigned char*)a->v,(const unsigned char*)b->v,a->size*a->item_size_in_bytes);}
    else    {for (i=0;i<a->size;i++) a->item_cpy((unsigned char*)a->v+i*a->item_size_in_bytes,(const unsigned char*)b->v+i*b->item_size_in_bytes);}
}
CV_API_DEF void cvector_shrink_to_fit(cvector* v)	{
    if (v)	{
        cvector o;memset(&o,0,sizeof(cvector));
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


/* create methods */
CV_API_DEF void cvector_create_with(cvector* v,size_t item_size_in_bytes,int (*item_cmp)(const void*,const void*),void (*item_ctr)(void*),void (*item_dtr)(void*),void (*item_cpy)(void*,const void*))	{
    typedef int (*item_cmp_type)(const void*,const void*);
	typedef void (*item_ctr_dtr_type)(void*);
	typedef void (*item_cpy_type)(void*,const void*);
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
    memset(v,0,sizeof(cvector));
    *((size_t*)&v->item_size_in_bytes)=item_size_in_bytes;
    *((item_cmp_type*)&v->item_cmp)=item_cmp;
	*((item_ctr_dtr_type*)&v->item_ctr)=item_ctr;
	*((item_ctr_dtr_type*)&v->item_dtr)=item_dtr;
	*((item_cpy_type*)&v->item_cpy)=item_cpy; 
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
CV_API_DEF void cvector_create(cvector* v,size_t item_size_in_bytes,int (*item_cmp)(const void*,const void*))	{cvector_create_with(v,item_size_in_bytes,item_cmp,NULL,NULL,NULL);}

#ifdef __cplusplus
    cvector::cvector() :
    v(NULL),size(0),capacity(0),item_size_in_bytes(0),
    item_cmp(NULL),item_ctr(NULL),item_dtr(NULL),item_cpy(NULL),
    free(&cvector_free),clear(&cvector_clear),shrink_to_fit(&cvector_shrink_to_fit),swap(&cvector_swap),
    reserve(&cvector_reserve),resize(&cvector_resize),resize_with(&cvector_resize_with),
    push_back(&cvector_push_back),pop_back(&cvector_pop_back),
    linear_search(&cvector_linear_search),binary_search(&cvector_binary_search),
    insert_at(&cvector_insert_at),insert_range_at(&cvector_insert_range_at),insert_sorted(&cvector_insert_sorted),
    remove_at(&cvector_remove_at),remove_range_at(&cvector_remove_range_at),
    cpy(&cvector_cpy),dbg_check(&cvector_dbg_check)
    {}

    cvector::cvector(const cvector& o) :
    v(NULL),size(0),capacity(0),item_size_in_bytes(o.item_size_in_bytes),
    item_cmp(o.item_cmp),item_ctr(o.item_ctr),item_dtr(o.item_dtr),item_cpy(o.item_cpy),
    free(&cvector_free),clear(&cvector_clear),shrink_to_fit(&cvector_shrink_to_fit),swap(&cvector_swap),
    reserve(&cvector_reserve),resize(&cvector_resize),resize_with(&cvector_resize_with),
    push_back(&cvector_push_back),pop_back(&cvector_pop_back),
    linear_search(&cvector_linear_search),binary_search(&cvector_binary_search),
    insert_at(&cvector_insert_at),insert_range_at(&cvector_insert_range_at),insert_sorted(&cvector_insert_sorted),
    remove_at(&cvector_remove_at),remove_range_at(&cvector_remove_range_at),
    cpy(&cvector_cpy),dbg_check(&cvector_dbg_check)
    {
        cvector_cpy(this,&o);
    }

    cvector& cvector::operator=(const cvector& o)    {cvector_cpy(this,&o);return *this;}

#   ifdef CV_HAS_MOVE_SEMANTICS
    cvector::cvector(cvector&& o) :
    v(o.v),size(o.size),capacity(o.capacity),item_size_in_bytes(o.item_size_in_bytes),
    item_cmp(o.item_cmp),item_ctr(o.item_ctr),item_dtr(o.item_dtr),item_cpy(o.item_cpy),
    free(&cvector_free),clear(&cvector_clear),shrink_to_fit(&cvector_shrink_to_fit),swap(&cvector_swap),
    reserve(&cvector_reserve),resize(&cvector_resize),resize_with(&cvector_resize_with),
    push_back(&cvector_push_back),pop_back(&cvector_pop_back),
    linear_search(&cvector_linear_search),binary_search(&cvector_binary_search),
    insert_at(&cvector_insert_at),insert_range_at(&cvector_insert_range_at),insert_sorted(&cvector_insert_sorted),
    remove_at(&cvector_remove_at),remove_range_at(&cvector_remove_range_at),
    cpy(&cvector_cpy),dbg_check(&cvector_dbg_check)
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

#endif /* (!defined(CV_ENABLE_DECLARATION_AND_DEFINITION) || defined(C_VECTOR_TYPE_UNSAFE_IMPLEMENTATION)) */
#endif /* C_VECTOR_TYPE_UNSAFE_H_IMPLEMENTATION_GUARD */


