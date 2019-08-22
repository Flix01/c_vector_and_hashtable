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
   inclusion, and gets reset soon after:

   CH_KEY_TYPE              (mandatory)
   CH_VALUE_TYPE            (mandatory)
   CH_NUM_BUCKETS           (mandatory: and must be copied to some 'type-safe definition')
   CH_USE_VOID_PTRS_IN_CMP_FCT  (optional: if you want to share cmp_fcts with c style functions like qsort)
   CHV_KEY_SUPPORT_EQUALITY_CMP_IN_UNSORTED_SEARCH (optional, but affects only unsorted buckets, which we should never use)

   GLOBAL DEFINITIONS: The following definitions (when used) must be set
   globally (= in the Project Options or in a StdAfx.h file):

   CH_MAX_NUM_BUCKETS                       256 or 65536
   CH_DISABLE_FAKE_MEMBER_FUNCTIONS
   CH_MALLOC
   CH_REALLOC
   CH_FREE
   CH_ASSERT
   CH_NO_ASSERT
   CH_NO_STDIO
   CH_NO_STDLIB
   CH_API
*/

#ifndef CH_KEY_TYPE
#error Please define CH_KEY_TYPE
#endif
#ifndef CH_VALUE_TYPE
#error Please define CH_VALUE_TYPE
#endif

#ifndef CH_VERSION
#define CH_VERSION               "1.01"
#define CH_VERSION_NUM           0101
#endif

/* TODO:
     -> add memory usage info in 'ch_xxx_dbg_check'
     -> reconsider and reimplement 'clearing memory' before item ctrs
*/
/* HISTORY:
   CH_VERSION_NUM 0101:
   -> added unsorted buckets (when key_cmp==NULL). Not very robust: they should never be used.
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
#ifdef __cplusplus
extern "C"	{
#endif

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

#ifndef CH_API
#define CH_API __inline static
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

#ifndef CH_MAX_NUM_BUCKETS  /* This MUST be 256 or 65536 and can be set only in the project options (not at each header inclusion) */
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
#   else
#       error CH_MAX_NUM_BUCKETS cannot be bigger than 65536.
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
#   else
#       error CH_NUM_BUCKETS cannot be bigger than 65536.
#   endif
#endif


#ifndef CH_HASHTABLE_TYPE
#define CH_HASHTABLE_ITEM_TYPE_TMP CH_CAT(CH_KEY_TYPE,_) 
#define CH_HASHTABLE_ITEM_TYPE CH_CAT(CH_HASHTABLE_ITEM_TYPE_TMP,CH_VALUE_TYPE) 
#define CH_HASHTABLE_TYPE CH_CAT(ch_,CH_HASHTABLE_ITEM_TYPE)
#define CH_VECTOR_TYPE CH_CAT(chv_,CH_HASHTABLE_ITEM_TYPE)
#define CH_VECTOR_TYPE_FCT(name) CH_CAT(CH_VECTOR_TYPE,name)
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
    } buckets[CH_NUM_BUCKETS];

#   ifndef CH_DISABLE_FAKE_MEMBER_FUNCTIONS  /* must be defined glabally (in the Project Options)) */
    void (* const clear)(CH_HASHTABLE_TYPE* ht);
    void (* const free)(CH_HASHTABLE_TYPE* ht);
    CH_VALUE_TYPE* (* const get_or_insert)(CH_HASHTABLE_TYPE* ht,const CH_KEY_TYPE* key,int* match);
    CH_VALUE_TYPE* (* const get_or_insert_by_val)(CH_HASHTABLE_TYPE* ht,const CH_KEY_TYPE key,int* match);
    CH_VALUE_TYPE* (* const get)(CH_HASHTABLE_TYPE* ht,const CH_KEY_TYPE* key);
    CH_VALUE_TYPE* (* const get_by_val)(CH_HASHTABLE_TYPE* ht,const CH_KEY_TYPE key);
    const CH_VALUE_TYPE* (* const get_const)(const CH_HASHTABLE_TYPE* ht,const CH_KEY_TYPE* key);
    const CH_VALUE_TYPE* (* const get_const_by_val)(const CH_HASHTABLE_TYPE* ht,const CH_KEY_TYPE key);
    int (* const remove)(CH_HASHTABLE_TYPE* ht,const CH_KEY_TYPE* key);
    int (* const remove_by_val)(CH_HASHTABLE_TYPE* ht,const CH_KEY_TYPE key);
    size_t (* const get_num_items)(const CH_HASHTABLE_TYPE* ht);
    void (* const dbg_check)(const CH_HASHTABLE_TYPE* ht);
    void (* const swap)(CH_HASHTABLE_TYPE* a,CH_HASHTABLE_TYPE* b);
    void (* const cpy)(CH_HASHTABLE_TYPE* a,const CH_HASHTABLE_TYPE* b);
#   endif
};
typedef struct CH_VECTOR_TYPE CH_VECTOR_TYPE;
#endif /* CH_HASHTABLE_TYPE */

#ifndef CH_PRIV_FUNCTIONS
#define CH_PRIV_FUNCTIONS
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
#endif /* CH_PRIV_FUNCTIONS */


/* --- PRIVATE FUNCTIONS START ------------------------------------------------ */
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
    /*printf("ok %s (sizeof(%s)=%lu)\n",CH_XSTR(CH_HASHTABLE_TYPE_FCT(_reserve)),CH_XSTR(CH_KEY_TYPE),sizeof(CH_KEY_TYPE));*/
    CH_ASSERT(v && ht);
    /* grows-only! */
    if (size>v->capacity) {
        const size_t new_capacity = v->capacity+(size-v->capacity)+(v->capacity)/2;
        ch_safe_realloc((void** const) &v->v,new_capacity*sizeof(CH_HASHTABLE_ITEM_TYPE));
        /* we reset to 0 the additional capacity (this helps robustness) */
        memset(&v->v[v->capacity],0,(new_capacity-v->capacity)*sizeof(CH_HASHTABLE_ITEM_TYPE));
        *((size_t*) &v->capacity) = new_capacity;
    }
}
CH_API void CH_VECTOR_TYPE_FCT(_resize)(CH_VECTOR_TYPE* v,size_t size,const CH_HASHTABLE_TYPE* ht)	{
    /*printf("%s\n",CH_XSTR(CH_VECTOR_TYPE_FCT(_resize)));*/
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
        if (ht->key_ctr && ht->value_ctr)   {for (i=v->size;i<size;i++) {ht->key_ctr(&v->v[i].k);ht->value_ctr(&v->v[i].v);}}
        if (ht->key_ctr)        {for (i=v->size;i<size;i++) ht->key_ctr(&v->v[i].k);}
        else if (ht->value_ctr) {for (i=v->size;i<size;i++) ht->value_ctr(&v->v[i].v);}
    }
    *((size_t*) &v->size)=size;
}
CH_API size_t CH_VECTOR_TYPE_FCT(_unsorted_search)(CH_VECTOR_TYPE* v,const CH_KEY_TYPE* key,int* match,const CH_HASHTABLE_TYPE* ht)  {
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
CH_API size_t CH_VECTOR_TYPE_FCT(_linear_search)(CH_VECTOR_TYPE* v,const CH_KEY_TYPE* key,int* match,const CH_HASHTABLE_TYPE* ht)  {
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
CH_API size_t CH_VECTOR_TYPE_FCT(_binary_search)(CH_VECTOR_TYPE* v,const CH_KEY_TYPE* key,int* match,const CH_HASHTABLE_TYPE* ht)  {
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
#if 0
CH_API size_t CH_VECTOR_TYPE_FCT(_insert_at)(CH_VECTOR_TYPE* v,const CH_HASHTABLE_ITEM_TYPE* item_to_insert,size_t position,const CH_HASHTABLE_TYPE* ht)  {
    /* position is in [0,v->size] */
    CH_ASSERT(v && ht && item_to_insert && position<=v->size);
    CH_VECTOR_TYPE_FCT(_reserve)(v,v->size+1,ht);
    if (position<v->size) memmove(&v->v[position+1],&v->v[position],(v->size-position)*sizeof(CH_HASHTABLE_ITEM_TYPE));
    memset(&v->v[position],0,sizeof(CH_HASHTABLE_ITEM_TYPE));
    if (ht->key_ctr)    ht->key_ctr(&v->v[position].k);
    if (ht->value_ctr)  ht->value_ctr(&v->v[position].v);

    if (!ht->key_cpy && !ht->value_cpy) memcpy(&v->v[position],item_to_insert,sizeof(CH_HASHTABLE_ITEM_TYPE));
    else if (ht->key_cpy)   {
        ht->key_cpy(&v->v[position].k,&item_to_insert->k);
        if (ht->value_cpy) ht->value_cpy(&v->v[position].v,&item_to_insert->v);
        else memcpy(&v->v[position].v,&item_to_insert->v,sizeof(CH_VALUE_TYPE));
    }
    else if (ht->value_cpy) {
        memcpy(&v->v[position].k,&item_to_insert->k,sizeof(CH_KEY_TYPE));
        ht->value_cpy(&v->v[position].v,&item_to_insert->v);
    }
    *((size_t*) &v->size)=v->size+1;
    return position;
}
#endif /* 0 */
CH_API size_t CH_VECTOR_TYPE_FCT(_insert_key_at)(CH_VECTOR_TYPE* v,const CH_KEY_TYPE* key_to_insert,size_t position,const CH_HASHTABLE_TYPE* ht)  {
    /* position is in [0,v->size] */
    CH_ASSERT(v && ht && key_to_insert && position<=v->size);
    CH_VECTOR_TYPE_FCT(_reserve)(v,v->size+1,ht);
    if (position<v->size) memmove(&v->v[position+1],&v->v[position],(v->size-position)*sizeof(CH_HASHTABLE_ITEM_TYPE));
    memset(&v->v[position],0,sizeof(CH_HASHTABLE_ITEM_TYPE));
    if (ht->key_ctr)    ht->key_ctr(&v->v[position].k);
    if (ht->value_ctr)  ht->value_ctr(&v->v[position].v);
    if (!ht->key_cpy)   memcpy(&v->v[position].k,key_to_insert,sizeof(CH_KEY_TYPE));
    else ht->key_cpy(&v->v[position].k,key_to_insert);
    *((size_t*) &v->size)=v->size+1;
    return position;
}
CH_API int CH_VECTOR_TYPE_FCT(_remove_at)(CH_VECTOR_TYPE* v,size_t position,const CH_HASHTABLE_TYPE* ht)  {
    /* position is in [0,num_items) */
    int removal_ok;size_t i;
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

CH_API void CH_HASHTABLE_TYPE_FCT(_free)(CH_HASHTABLE_TYPE* ht)    {
    if (ht) {
        const unsigned short max_value = (CH_NUM_BUCKETS-1);
        unsigned short i=0;
        do    {
            CH_VECTOR_TYPE* b = &ht->buckets[i];
            CH_VECTOR_TYPE_FCT(_clear)(b,ht);
            if (b->v) {ch_free(b->v);b->v=NULL;}
            *((size_t*)&b->capacity)=0;
        }
        while (i++!=max_value);
    }
}
CH_API void CH_HASHTABLE_TYPE_FCT(_clear)(CH_HASHTABLE_TYPE* ht)    {
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
CH_API CH_VALUE_TYPE* CH_HASHTABLE_TYPE_FCT(_get_or_insert)(CH_HASHTABLE_TYPE* ht,const CH_KEY_TYPE* key,int* match) {
    CH_VECTOR_TYPE* v = NULL;
    size_t position;ch_hash_uint hash;int match2;
    CH_ASSERT(ht && ht->key_hash);
    hash = ht->key_hash(key);
#   if CH_NUM_BUCKETS!=CH_MAX_NUM_BUCKETS
    CH_ASSERT(hash<CH_NUM_BUCKETS);    /* user 'key_hash' should return values in [0,CH_NUM_BUCKETS). Please use: return somevalue%CH_NUM_BUCKETS */
#   endif
    v = &ht->buckets[hash];
    if (!v->v)  {
        v->v = ch_malloc(ht->initial_bucket_capacity*sizeof(CH_HASHTABLE_ITEM_TYPE));
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
#   if 0
    {
        CH_HASHTABLE_ITEM_TYPE item;
        if (ht->key_ctr) ht->key_ctr(&item.k);
        if (ht->value_ctr) ht->value_ctr(&item.v);        
        if (ht->key_cpy) ht->key_cpy(&item.k,key);
        else memcpy(&item.k,key,sizeof(CH_KEY_TYPE));
        /*if (ht->value_cpy) ht->value_cpy(&item.v,value);
        else memcpy(&item.v,value,sizeof(CH_VALUE_TYPE));
        */
        /* Warning: see the code of 'insert_at': can't we create 'item' by just emplacing it in v? */
        /* Or just take both 'key' and 'value' as arguments in both '_get_or_insert' and '_insert_at' */
        CH_VECTOR_TYPE_FCT(_insert_at)(v,&item,position,ht);
        if (ht->key_dtr) ht->key_dtr(&item.k);
        if (ht->value_dtr) ht->value_dtr(&item.v);
    }
#   else
        CH_VECTOR_TYPE_FCT(_insert_key_at)(v,key,position,ht);
#   endif
    return &v->v[position].v;
}
CH_API CH_VALUE_TYPE* CH_HASHTABLE_TYPE_FCT(_get_or_insert_by_val)(CH_HASHTABLE_TYPE* ht,const CH_KEY_TYPE key,int* match) {return CH_HASHTABLE_TYPE_FCT(_get_or_insert)(ht,&key,match);}
CH_API CH_VALUE_TYPE* CH_HASHTABLE_TYPE_FCT(_get)(CH_HASHTABLE_TYPE* ht,const CH_KEY_TYPE* key) {
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
CH_API CH_VALUE_TYPE* CH_HASHTABLE_TYPE_FCT(_get_by_val)(CH_HASHTABLE_TYPE* ht,const CH_KEY_TYPE key) {return CH_HASHTABLE_TYPE_FCT(_get)(ht,&key);}
CH_API const CH_VALUE_TYPE* CH_HASHTABLE_TYPE_FCT(_get_const)(const CH_HASHTABLE_TYPE* ht,const CH_KEY_TYPE* key) {return (const CH_VALUE_TYPE*) CH_HASHTABLE_TYPE_FCT(_get)((CH_HASHTABLE_TYPE*)ht,key);}
CH_API const CH_VALUE_TYPE* CH_HASHTABLE_TYPE_FCT(_get_const_by_val)(const CH_HASHTABLE_TYPE* ht,const CH_KEY_TYPE key) {return (const CH_VALUE_TYPE*) CH_HASHTABLE_TYPE_FCT(_get)((CH_HASHTABLE_TYPE*)ht,&key);}

CH_API int CH_HASHTABLE_TYPE_FCT(_remove)(CH_HASHTABLE_TYPE* ht,const CH_KEY_TYPE* key) {
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
CH_API int CH_HASHTABLE_TYPE_FCT(_remove_by_val)(CH_HASHTABLE_TYPE* ht,const CH_KEY_TYPE key) {return CH_HASHTABLE_TYPE_FCT(_remove)(ht,&key);}
CH_API size_t CH_HASHTABLE_TYPE_FCT(_get_num_items)(const CH_HASHTABLE_TYPE* ht) {
    size_t i,sum=0;CH_ASSERT(ht);
    for (i=0;i<CH_NUM_BUCKETS;i++) sum+=ht->buckets[i].size;
    return sum;
}
CH_API int CH_HASHTABLE_TYPE_FCT(_dbg_check)(const CH_HASHTABLE_TYPE* ht) {
    size_t i,j,num_total_items=0,num_sorting_errors=0,min_num_bucket_items=(size_t)-1,max_num_bucket_items=0,min_cnt=0,max_cnt=0,avg_cnt=0,avg_round=0;
    double avg_num_bucket_items=0.0,std_deviation=0.0;
    const CH_HASHTABLE_ITEM_TYPE* last_item = NULL;
    CH_ASSERT(ht);
    for (i=0;i<CH_NUM_BUCKETS;i++) {
        const CH_VECTOR_TYPE* bck = &ht->buckets[i];
        num_total_items+=bck->size;
        if (min_num_bucket_items>bck->size) min_num_bucket_items=bck->size;
        if (max_num_bucket_items<bck->size) max_num_bucket_items=bck->size;
        if (ht->key_cmp && bck->v && bck->size) {
            last_item = NULL;
            for (j=0;j<bck->size;j++)  {
                const CH_HASHTABLE_ITEM_TYPE* item = &bck->v[j];
                if (last_item) {
                    if (ht->key_cmp(&last_item->k,&item->k)>=0) {
                        /* When this happens, it can be a wrong user 'key_cmp' function (that cannot sort keys in a consistent way) */
                        ++num_sorting_errors;
#                       ifndef CH_NO_STDIO
                        printf("[%s] Error: in bucket[%lu]: key_cmp(%lu,%lu)<=0 [num_items=%lu (in bucket)]\n",CH_XSTR(CH_HASHTABLE_TYPE_FCT(_dbg_check)),i,j-1,j,bck->size);
#                       endif
                    }
                }
                last_item=item;
            }
        }
    }
    avg_num_bucket_items = (double)num_total_items/(double) CH_NUM_BUCKETS;
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
    printf("[%s] num_total_items=%lu in %d buckets [items per bucket: mean=%1.3f std_deviation=%1.3f min=%lu (in %lu/%d) avg=%lu (in %lu/%d) max=%lu (in %lu/%d)]\n",CH_XSTR(CH_HASHTABLE_TYPE_FCT(_dbg_check)),num_total_items,CH_NUM_BUCKETS,avg_num_bucket_items,std_deviation,min_num_bucket_items,min_cnt,CH_NUM_BUCKETS,avg_round,avg_cnt,CH_NUM_BUCKETS,max_num_bucket_items,max_cnt,CH_NUM_BUCKETS);
#   endif
    CH_ASSERT(num_sorting_errors==0); /* When this happens, it can be a wrong user 'itemKey_cmp' function (that cannot sort keys in a consistent way) */
    return num_total_items;
}

/* two untested functions */
CH_API void CH_HASHTABLE_TYPE_FCT(_swap)(CH_HASHTABLE_TYPE* a,CH_HASHTABLE_TYPE* b)  {
    CH_HASHTABLE_TYPE t;
    CH_ASSERT(a && b);
    memcpy(&t,&a,sizeof(CH_HASHTABLE_TYPE));
    memcpy(&a,&b,sizeof(CH_HASHTABLE_TYPE));
    memcpy(&b,&t,sizeof(CH_HASHTABLE_TYPE));
}
CH_API void CH_HASHTABLE_TYPE_FCT(_cpy)(CH_HASHTABLE_TYPE* a,const CH_HASHTABLE_TYPE* b) {
    size_t i;
    typedef ch_hash_uint (*key_hash_type)(const CH_KEY_TYPE*);
    typedef int (*key_cmp_type)(const CH_CMP_TYPE*,const CH_CMP_TYPE*);
    typedef void (*key_ctr_dtr_type)(CH_KEY_TYPE*);
    typedef void (*key_cpy_type)(CH_KEY_TYPE*,const CH_KEY_TYPE*);
    typedef void (*value_ctr_dtr_type)(CH_VALUE_TYPE*);
    typedef void (*value_cpy_type)(CH_VALUE_TYPE*,const CH_VALUE_TYPE*);
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

CH_API void CH_HASHTABLE_TYPE_FCT(_create_with)(
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
    typedef void (* clear_free_mf)(CH_HASHTABLE_TYPE*);
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
    *((clear_free_mf*)&ht->clear) = &CH_HASHTABLE_TYPE_FCT(_clear);
    *((clear_free_mf*)&ht->free) = &CH_HASHTABLE_TYPE_FCT(_free);
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
CH_API void CH_HASHTABLE_TYPE_FCT(_create)(CH_HASHTABLE_TYPE* ht,ch_hash_uint (*key_hash)(const CH_KEY_TYPE*),int (*key_cmp) (const CH_CMP_TYPE*,const CH_CMP_TYPE*),size_t initial_bucket_capacity)    {
    CH_HASHTABLE_TYPE_FCT(_create_with)(ht,key_hash,key_cmp,NULL,NULL,NULL,NULL,NULL,NULL,initial_bucket_capacity);
}


#undef CH_HASHTABLE_TYPE_FCT
#undef CH_KEY_TYPE_FCT
#undef CH_VECTOR_TYPE_FCT
#undef CH_CMP_TYPE
#undef CH_USE_VOID_PTRS_IN_CMP_FCT
#undef CH_HASHTABLE_TYPE
#undef CH_VECTOR_TYPE
#undef CHV_KEY_SUPPORT_EQUALITY_CMP_IN_UNSORTED_SEARCH
#undef CH_HASHTABLE_ITEM_TYPE_TMP
#undef CH_HASHTABLE_ITEM_TYPE
#undef CH_LAST_INCLUDED_NUM_BUCKETS
#define CH_LAST_INCLUDED_NUM_BUCKETS CH_NUM_BUCKETS
#undef CH_NUM_BUCKETS
#ifdef __cplusplus
} /* extern "C" */
#endif
/* ------------------------------------------------- */
#undef CH_KEY_TYPE
#undef CH_VALUE_TYPE

