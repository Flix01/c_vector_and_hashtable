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

/* SCOPED DEFINITION: The following definitions must be set before each <c_hashtable.h>
   inclusion, and get reset soon after:

   CV_TYPE                      (mandatory)
   CV_USE_VOID_PTRS_IN_CMP_FCT  (optional: if you want to share cmp_fcts with c style functions like qsort)

   GLOBAL DEFINITIONS: The following definitions (when used) must be set
   globally (= in the Project Options or in a StdAfx.h file):

   CV_DISABLE_FAKE_MEMBER_FUNCTIONS
   CV_MALLOC
   CV_REALLOC
   CV_FREE
   CV_ASSERT
   CV_NO_ASSERT
   CV_NO_STDIO
   CV_NO_STDLIB
   CV_API
*/

#ifndef CV_TYPE
#error Please define CV_TYPE
#endif

#ifndef CV_VERSION
#define CV_VERSION               "1.01"
#define CV_VERSION_NUM           0101
#endif

/* TODO:
     -> add a method to debug and trim unused memory
     -> reconsider and reimplement 'clearing memory' before item ctrs
*/
/* HISTORY:
   CH_VERSION_NUM 0101:
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
#ifdef __cplusplus
extern "C"	{
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

#ifndef CV_API
#define CV_API __inline static
#endif

/* the following 4 lines can be removed */
#ifndef CV_XSTR
#define CV_XSTR(s) CV_STR(s)
#define CV_STR(s) #s
#endif

#ifndef CV_CAT 
#	define CV_CAT(x, y) CV_CAT_(x, y)
#	define CV_CAT_(x, y) x ## y
#endif

#ifndef CV_USE_VOID_PTRS_IN_CMP_FCT
#define CV_CMP_TYPE CV_TYPE
#else
#define CV_CMP_TYPE void
#endif

#define CV_VECTOR_TYPE_FCT(name) CV_CAT(CV_VECTOR_TYPE,name)
#define CV_TYPE_FCT(name) CV_CAT(CV_TYPE,name)

#ifndef CV_VECTOR_TYPE
#define CV_VECTOR_(name) CV_CAT(cv_,name)
#define CV_VECTOR_TYPE CV_VECTOR_(CV_TYPE)
typedef struct CV_VECTOR_TYPE CV_VECTOR_TYPE;
struct CV_VECTOR_TYPE {
	CV_TYPE * v;
	const size_t size;
	const size_t capacity;
    void (*const item_ctr)(CV_TYPE*);                     /* optional (can be NULL) */
    void (*const item_dtr)(CV_TYPE*);                     /* optional (can be NULL) */
    void (*const item_cpy)(CV_TYPE*,const CV_TYPE*);		/* optional (can be NULL) */
    int (*const item_cmp)(const CV_CMP_TYPE*,const CV_CMP_TYPE*);		/* optional (can be NULL) (for sorted vectors only) */
#   ifndef CV_DISABLE_FAKE_MEMBER_FUNCTIONS  /* must be defined glabally (in the Project Options)) */
    void (* const free)(CV_VECTOR_TYPE* v);
    void (* const clear)(CV_VECTOR_TYPE* v);
    void (* const swap)(CV_VECTOR_TYPE* a,CV_VECTOR_TYPE* b);
    void (* const reserve)(CV_VECTOR_TYPE* v,size_t size);
    void (* const resize)(CV_VECTOR_TYPE* v,size_t size);
    void (* const resize_with)(CV_VECTOR_TYPE* v,size_t size,const CV_TYPE* default_value);
    void (* const resize_with_by_val)(CV_VECTOR_TYPE* v,size_t size,const CV_TYPE default_value);
    void (* const push_back)(CV_VECTOR_TYPE* v,const CV_TYPE* value);
    void (* const push_back_by_val)(CV_VECTOR_TYPE* v,const CV_TYPE value);
    void (* const pop_back)(CV_VECTOR_TYPE* v);
    size_t (* const linear_search)(const CV_VECTOR_TYPE* v,const CV_TYPE* item_to_search,int* match);
    size_t (* const linear_search_by_val)(const CV_VECTOR_TYPE* v,const CV_TYPE item_to_search,int* match);
    size_t (* const binary_search)(const CV_VECTOR_TYPE* v,const CV_TYPE* item_to_search,int* match);
    size_t (* const binary_search_by_val)(const CV_VECTOR_TYPE* v,const CV_TYPE item_to_search,int* match);
    size_t (* const insert_at)(CV_VECTOR_TYPE* v,const CV_TYPE* item_to_insert,size_t position);
    size_t (* const insert_at_by_val)(CV_VECTOR_TYPE* v,const CV_TYPE item_to_insert,size_t position);
    size_t (* const insert_sorted)(CV_VECTOR_TYPE* v,const CV_TYPE* item_to_insert,int* match,int insert_even_if_item_match);
    size_t (* const insert_sorted_by_val)(CV_VECTOR_TYPE* v,const CV_TYPE item_to_insert,int* match,int insert_even_if_item_match);
    int (* const remove_at)(CV_VECTOR_TYPE* v,size_t position);
    void (* const cpy)(CV_VECTOR_TYPE* a,const CV_VECTOR_TYPE* b);
#   endif
};
#endif /* CV_VECTOR_TYPE */

#ifndef CV_PRIV_FUNCTIONS
#define CV_PRIV_FUNCTIONS
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
#endif /* CV_PRIV_FUNCTIONS */

/* warning: internal function. Implementation can't be modified, because some optimizations bypass it */
/* used only in 'cv_xxx_resize_with(...) and cv_xxx_push_back(...)' */
/* TODO: remove this function (well, this will make code bigger...) */
CV_API void CV_TYPE_FCT(_default_item_cpy)(CV_TYPE* a,const CV_TYPE* b) {memcpy(a,b,sizeof(CV_TYPE));}

/* 'cv_xxx_create(...)' and 'cv_xxx_free(...)' can be thought as the ctr and the dct of the 'cv_xxx' vector struct */

/* Mandatory call at the end to free memory. The vector can be reused after this call. The function can be safely re-called multiple times  */
CV_API void CV_VECTOR_TYPE_FCT(_free)(CV_VECTOR_TYPE* v)	{
	if (v)	{
		if (v->v) {
			if (v->item_dtr)	{			
				size_t i;
				for (i=0;i<v->size;i++)	v->item_dtr(&v->v[i]);
			}
			cv_free(v->v);v->v=NULL;
		}	
		*((size_t*) &v->size)=0;
		*((size_t*) &v->capacity)=0;
        /* we don't clear the other fields */
	}	
}
/* Same as 'cv_xxx_free(...)', but it does not free the memory (= the vector capacity)  */
CV_API void CV_VECTOR_TYPE_FCT(_clear)(CV_VECTOR_TYPE* v)	{
	if (v)	{
		if (v->v) {
			if (v->item_dtr)	{			
				size_t i;
				for (i=0;i<v->size;i++)	v->item_dtr(&v->v[i]);
			}
		}	
		*((size_t*) &v->size)=0;
	}	
}
CV_API void CV_VECTOR_TYPE_FCT(_swap)(CV_VECTOR_TYPE* a,CV_VECTOR_TYPE* b)  {
    unsigned char t[sizeof(CV_VECTOR_TYPE)];
    CV_ASSERT(a && b);
    memcpy(t,&a,sizeof(CV_VECTOR_TYPE));
    memcpy(&a,&b,sizeof(CV_VECTOR_TYPE));
    memcpy(&b,t,sizeof(CV_VECTOR_TYPE));
}
CV_API void CV_VECTOR_TYPE_FCT(_reserve)(CV_VECTOR_TYPE* v,size_t size)	{
	/*printf("ok %s (sizeof(%s)=%lu)\n",CV_XSTR(CV_VECTOR_TYPE_FCT(_reserve)),CV_XSTR(CV_TYPE),sizeof(CV_TYPE));*/
	CV_ASSERT(v);    
	/* grows-only! */
    if (size>v->capacity) {		
        const size_t new_capacity = v->capacity+(size-v->capacity)+(v->capacity)/2;
        cv_safe_realloc((void** const) &v->v,new_capacity*sizeof(CV_TYPE));
        /* we reset to 0 the additional capacity (this helps robustness) */
        memset(&v->v[v->capacity],0,(new_capacity-v->capacity)*sizeof(CV_TYPE));
        *((size_t*) &v->capacity) = new_capacity;
	}
}
CV_API void CV_VECTOR_TYPE_FCT(_resize)(CV_VECTOR_TYPE* v,size_t size)	{
	/*printf("%s\n",CV_XSTR(CV_VECTOR_TYPE_FCT(_resize)));*/
	CV_ASSERT(v);  
    if (size>v->capacity) CV_VECTOR_TYPE_FCT(_reserve)(v,size);
    if (size<v->size)   {if (v->item_dtr) {size_t i;for (i=size;i<v->size;i++) v->item_dtr(&v->v[i]);}}
    else {if (v->item_ctr) {size_t i;for (i=v->size;i<size;i++) v->item_ctr(&v->v[i]);}}
    *((size_t*) &v->size)=size;
}
CV_API void CV_VECTOR_TYPE_FCT(_resize_with)(CV_VECTOR_TYPE* v,size_t size,const CV_TYPE* default_value)	{
	/*printf("%s\n",CV_XSTR(CV_VECTOR_TYPE_FCT(_resize_with)));*/
	CV_ASSERT(v);  
	if (!default_value) {CV_VECTOR_TYPE_FCT(_resize)(v,size);return;}
    if (size>v->capacity) CV_VECTOR_TYPE_FCT(_reserve)(v,size);
    if (size<v->size)   {if (v->item_dtr) {size_t i;for (i=size;i<v->size;i++) v->item_dtr(&v->v[i]);}}
    else {
        size_t i;
        void (* const item_cpy)(CV_TYPE*,const CV_TYPE*) = v->item_cpy ? v->item_cpy : &(CV_TYPE_FCT(_default_item_cpy));
        if (v->item_ctr)    {for (i=v->size;i<size;i++) {v->item_ctr(&v->v[i]);item_cpy(&v->v[i],default_value);}}
        else    {for (i=v->size;i<size;i++) {item_cpy(&v->v[i],default_value);}}
    }
    *((size_t*) &v->size)=size;
}
CV_API void CV_VECTOR_TYPE_FCT(_resize_with_by_val)(CV_VECTOR_TYPE* v,size_t size,const CV_TYPE default_value)	{CV_VECTOR_TYPE_FCT(_resize_with)(v,size,&default_value);}
CV_API void CV_VECTOR_TYPE_FCT(_push_back)(CV_VECTOR_TYPE* v,const CV_TYPE* value)  {
	/*printf("%s\n",CV_XSTR(CV_VECTOR_TYPE_FCT(_push_back)));*/
    void (*item_cpy)(CV_TYPE*,const CV_TYPE*);
    CV_ASSERT(v);
    item_cpy = v->item_cpy ? v->item_cpy : &(CV_TYPE_FCT(_default_item_cpy));
	if (v->size == v->capacity) {			            
		if (v->v && value>=v->v && value<(v->v+v->size))  {
            CV_TYPE v_val =
#           ifdef __cplusplus
            {};
#           else
            {0};
#           endif
            item_cpy(&v_val,value);
            CV_VECTOR_TYPE_FCT(_reserve)(v,v->size+1);
            if (v->item_ctr) v->item_ctr(&v->v[v->size]);
            item_cpy(&v->v[v->size],&v_val);
        }
        else {
            CV_VECTOR_TYPE_FCT(_reserve)(v,v->size+1);
            if (v->item_ctr) v->item_ctr(&v->v[v->size]);
            item_cpy(&v->v[v->size],value);
    	}
    }
    else {
	    if (v->item_ctr) v->item_ctr(&v->v[v->size]);
        item_cpy(&v->v[v->size],value);
	}
	*((size_t*) &v->size)=v->size+1;
}
CV_API void CV_VECTOR_TYPE_FCT(_push_back_by_val)(CV_VECTOR_TYPE* v,const CV_TYPE value)  {CV_VECTOR_TYPE_FCT(_push_back)(v,&value);}
CV_API void CV_VECTOR_TYPE_FCT(_pop_back)(CV_VECTOR_TYPE* v)	{
   printf("%s\n",CV_XSTR(CV_VECTOR_TYPE_FCT(_pop_back)));
   CV_ASSERT(v && v->size>0);
   if (v->size>0) {*((size_t*) &v->size)=v->size-1;if (v->item_dtr) v->item_dtr(&v->v[v->size]);}
}
CV_API size_t CV_VECTOR_TYPE_FCT(_linear_search)(const CV_VECTOR_TYPE* v,const CV_TYPE* item_to_search,int* match)  {
    int cmp=0;size_t i;
    CV_ASSERT(v && v->item_cmp);
    if (match) *match=0;
    if (v->size==0) return 0;  /* otherwise match will be 1 */
    for (i = 0; i < v->size; i++) {
        cmp = v->item_cmp(item_to_search,&v->v[i]);
        if (cmp<=0) {
            if (cmp==0 && match) *match=1;
            return i;
        }
    }
    CV_ASSERT(i==v->size);
    return i;
}
CV_API size_t CV_VECTOR_TYPE_FCT(_linear_search_by_val)(const CV_VECTOR_TYPE* v,const CV_TYPE item_to_search,int* match)  {return CV_VECTOR_TYPE_FCT(_linear_search)(v,&item_to_search,match);}
CV_API size_t CV_VECTOR_TYPE_FCT(_binary_search)(const CV_VECTOR_TYPE* v,const CV_TYPE* item_to_search,int* match)  {
    size_t first=0, last;
    size_t mid;int cmp;
    CV_ASSERT(v && v->item_cmp);
    if (match) *match=0;
    if (v->size==0) return 0;  /* otherwise match will be 1 */
	last=v->size-1;
    while (first <= last) {
        mid = (first + last) / 2;
        cmp = v->item_cmp(item_to_search,&v->v[mid]);
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
CV_API size_t CV_VECTOR_TYPE_FCT(_binary_search_by_val)(const CV_VECTOR_TYPE* v,const CV_TYPE item_to_search,int* match)    {return CV_VECTOR_TYPE_FCT(_binary_search)(v,&item_to_search,match);}
CV_API size_t CV_VECTOR_TYPE_FCT(_insert_at)(CV_VECTOR_TYPE* v,const CV_TYPE* item_to_insert,size_t position)  {
    /* position is in [0,v->size] */
    CV_ASSERT(v && position<=v->size);
    CV_VECTOR_TYPE_FCT(_reserve)(v,v->size+1);    
    if (position<v->size) memmove(&v->v[position+1],&v->v[position],(v->size-position)*sizeof(CV_TYPE));
    memset(&v->v[position],0,sizeof(CV_TYPE));
    if (v->item_ctr) v->item_ctr(&v->v[position]);
    if (!v->item_cpy)   memcpy(&v->v[position],item_to_insert,sizeof(CV_TYPE));
    else    v->item_cpy(&v->v[position],item_to_insert);
	*((size_t*) &v->size)=v->size+1;	
	return position;
}
CV_API size_t CV_VECTOR_TYPE_FCT(_insert_at_by_val)(CV_VECTOR_TYPE* v,const CV_TYPE item_to_insert,size_t position)   {return CV_VECTOR_TYPE_FCT(_insert_at)(v,&item_to_insert,position);}
CV_API size_t CV_VECTOR_TYPE_FCT(_insert_sorted)(CV_VECTOR_TYPE* v,const CV_TYPE* item_to_insert,int* match,int insert_even_if_item_match)  {
    int my_match = 0;size_t position;
	position = CV_VECTOR_TYPE_FCT(_binary_search)(v,item_to_insert,&my_match);
    if (match) *match = my_match;
    if (my_match && !insert_even_if_item_match) return position;
    CV_VECTOR_TYPE_FCT(_insert_at)(v,item_to_insert,position);
    return position;
}
CV_API size_t CV_VECTOR_TYPE_FCT(_insert_sorted_by_val)(CV_VECTOR_TYPE* v,const CV_TYPE item_to_insert,int* match,int insert_even_if_item_match)  {return CV_VECTOR_TYPE_FCT(_insert_sorted)(v,&item_to_insert,match,insert_even_if_item_match);}
CV_API int CV_VECTOR_TYPE_FCT(_remove_at)(CV_VECTOR_TYPE* v,size_t position)  {
    /* position is in [0,num_items) */
    int removal_ok;size_t i;
	CV_ASSERT(v);
	removal_ok = (position<v->size) ? 1 : 0;
    CV_ASSERT(removal_ok);	/* error: position>=v->size */
	if (removal_ok)	{
        if (v->item_dtr) v->item_dtr(&v->v[position]);
        memmove(&v->v[position],&v->v[position+1],(v->size-position-1)*sizeof(CV_TYPE));
        *((size_t*) &v->size)=v->size-1;
    }
    return removal_ok;
}

CV_API void CV_VECTOR_TYPE_FCT(_cpy)(CV_VECTOR_TYPE* a,const CV_VECTOR_TYPE* b) {
    size_t i;
    typedef void (*item_ctr_dtr_type)(CV_TYPE*);
    typedef void (*item_cpy_type)(CV_TYPE*,const CV_TYPE*);
    typedef int (*item_cmp_type)(const CV_CMP_TYPE*,const CV_CMP_TYPE*);
    CV_ASSERT(a && b);
    /* bad init asserts */
    CV_ASSERT(!(a->v && a->capacity==0));
    CV_ASSERT(!(!a->v && a->capacity>0));
    CV_ASSERT(!(b->v && b->capacity==0));
    CV_ASSERT(!(!b->v && b->capacity>0));
    /*CV_VECTOR_TYPE_FCT(_free)(a);*/
    CV_VECTOR_TYPE_FCT(_clear)(a);
    *((item_ctr_dtr_type*)&a->item_ctr)=b->item_ctr;
    *((item_ctr_dtr_type*)&a->item_dtr)=b->item_dtr;
    *((item_cpy_type*)&a->item_cpy)=b->item_cpy;
    *((item_cmp_type*)&a->item_cmp)=b->item_cmp;
    CV_VECTOR_TYPE_FCT(_resize)(a,b->size);
    CV_ASSERT(a->size==b->size);
    if (!a->item_cpy)   {memcpy(&a->v[0],&b->v[0],a->size*sizeof(CV_TYPE));}
    else    {for (i=0;i<a->size;i++) a->item_cpy(&a->v[i],&b->v[i]);}
}

/* create methods */
CV_API void CV_VECTOR_TYPE_FCT(_create_with)(CV_VECTOR_TYPE* v,int (*item_cmp)(const CV_CMP_TYPE*,const CV_CMP_TYPE*),void (*item_ctr)(CV_TYPE*),void (*item_dtr)(CV_TYPE*),void (*item_cpy)(CV_TYPE*,const CV_TYPE*))	{
    typedef void (*item_ctr_dtr_type)(CV_TYPE*);
    typedef void (*item_cpy_type)(CV_TYPE*,const CV_TYPE*);
    typedef int (*item_cmp_type)(const CV_CMP_TYPE*,const CV_CMP_TYPE*);
#   ifndef CV_DISABLE_FAKE_MEMBER_FUNCTIONS  /* must be defined glabally (in the Project Options)) */
    typedef void (* free_clear_pop_back_mf)(CV_VECTOR_TYPE*);
    typedef void (* swap_mf)(CV_VECTOR_TYPE*,CV_VECTOR_TYPE*);
    typedef void (* reserve_mf)(CV_VECTOR_TYPE*,size_t);
    typedef void (* resize_mf)(CV_VECTOR_TYPE*,size_t);
    typedef void (* resize_with_mf)(CV_VECTOR_TYPE*,size_t,const CV_TYPE*);
    typedef void (* resize_with_by_val_mf)(CV_VECTOR_TYPE*,size_t,const CV_TYPE);
    typedef void (* push_back_mf)(CV_VECTOR_TYPE*,const CV_TYPE*);
    typedef void (* push_back_by_val_mf)(CV_VECTOR_TYPE*,const CV_TYPE);
    typedef size_t (* search_mf)(const CV_VECTOR_TYPE*,const CV_TYPE*,int*);
    typedef size_t (* search_by_val_mf)(const CV_VECTOR_TYPE*,const CV_TYPE,int*);
    typedef size_t (* insert_at_mf)(CV_VECTOR_TYPE*,const CV_TYPE*,size_t);
    typedef size_t (* insert_at_by_val_mf)(CV_VECTOR_TYPE*,const CV_TYPE,size_t);
    typedef size_t (* insert_sorted_mf)(CV_VECTOR_TYPE*,const CV_TYPE*,int*,int);
    typedef size_t (* insert_sorted_by_val_mf)(CV_VECTOR_TYPE*,const CV_TYPE,int*,int);
    typedef int (* remove_at_mf)(CV_VECTOR_TYPE*,size_t);
    typedef void (* cpy_mf)(CV_VECTOR_TYPE*,const CV_VECTOR_TYPE*);
#   endif
    CV_ASSERT(v);
    memset(v,0,sizeof(CV_VECTOR_TYPE));
    *((item_ctr_dtr_type*)&v->item_ctr)=item_ctr;
    *((item_ctr_dtr_type*)&v->item_dtr)=item_dtr;
    *((item_cpy_type*)&v->item_cpy)=item_cpy;
    *((item_cmp_type*)&v->item_cmp)=item_cmp;
#   ifndef CV_DISABLE_FAKE_MEMBER_FUNCTIONS  /* must be defined glabally (in the Project Options)) */
    *((free_clear_pop_back_mf*)&v->free)=&CV_VECTOR_TYPE_FCT(_free);
    *((free_clear_pop_back_mf*)&v->clear)=&CV_VECTOR_TYPE_FCT(_clear);
    *((swap_mf*)&v->swap)=&CV_VECTOR_TYPE_FCT(_swap);
    *((reserve_mf*)&v->reserve)=&CV_VECTOR_TYPE_FCT(_reserve);
    *((resize_mf*)&v->resize)=&CV_VECTOR_TYPE_FCT(_resize);
    *((resize_with_mf*)&v->resize_with)=&CV_VECTOR_TYPE_FCT(_resize_with);
    *((resize_with_by_val_mf*)&v->resize_with_by_val)=&CV_VECTOR_TYPE_FCT(_resize_with_by_val);
    *((push_back_mf*)&v->push_back)=&CV_VECTOR_TYPE_FCT(_push_back);
    *((push_back_by_val_mf*)&v->push_back_by_val)=&CV_VECTOR_TYPE_FCT(_push_back_by_val);
    *((free_clear_pop_back_mf*)&v->pop_back)=&CV_VECTOR_TYPE_FCT(_pop_back);
    *((search_mf*)&v->linear_search)=&CV_VECTOR_TYPE_FCT(_linear_search);
    *((search_by_val_mf*)&v->linear_search_by_val)=&CV_VECTOR_TYPE_FCT(_linear_search_by_val);
    *((search_mf*)&v->binary_search)=&CV_VECTOR_TYPE_FCT(_binary_search);
    *((search_by_val_mf*)&v->binary_search_by_val)=&CV_VECTOR_TYPE_FCT(_binary_search_by_val);
    *((insert_at_mf*)&v->insert_at)=&CV_VECTOR_TYPE_FCT(_insert_at);
    *((insert_at_by_val_mf*)&v->insert_at_by_val)=&CV_VECTOR_TYPE_FCT(_insert_at_by_val);
    *((insert_sorted_mf*)&v->insert_sorted)=&CV_VECTOR_TYPE_FCT(_insert_sorted);
    *((insert_sorted_by_val_mf*)&v->insert_sorted_by_val)=&CV_VECTOR_TYPE_FCT(_insert_sorted_by_val);
    *((remove_at_mf*)&v->remove_at)=&CV_VECTOR_TYPE_FCT(_remove_at);
    *((cpy_mf*)&v->cpy)=&CV_VECTOR_TYPE_FCT(_cpy);
#   endif
}
CV_API void CV_VECTOR_TYPE_FCT(_create)(CV_VECTOR_TYPE* v)  {CV_VECTOR_TYPE_FCT(_create_with)(v,NULL,NULL,NULL,NULL);}

#undef CV_VECTOR_TYPE_FCT
#undef CV_TYPE_FCT
#undef CV_CMP_TYPE
#undef CV_USE_VOID_PTRS_IN_CMP_FCT
#undef CV_VECTOR_TYPE
#undef CV_VECTOR_
#ifdef __cplusplus
} /* extern "C" */
#endif
/* ------------------------------------------------- */
#undef CV_TYPE


