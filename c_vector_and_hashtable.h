//==================================================================================
// Plain C implementation of vector and hashtable
//==================================================================================
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

#ifndef C_VECTOR_AND_HASHTABLE_H
#define C_VECTOR_AND_HASHTABLE_H

#ifdef __cplusplus
extern "C" {
#endif

#ifndef CVH_MALLOC
#include <stdlib.h>	// malloc/realloc/free
#define CVH_MALLOC(X) malloc(X)
#endif

#ifndef CVH_FREE
#define CVH_FREE(X) free(X)
#endif

#ifndef CVH_REALLOC
#define CVH_REALLOC(x,y) realloc((x),(y))
#endif

#define CVH_API_PRIV static
#define CVH_API_IMPL /*no-op*/
#define CVH_API_INL inline

#include <stddef.h> // size_t

#ifndef CVH_ASSERT
#include <assert.h>
#define CVH_ASSERT(X) assert((X))
#endif

#include <stdio.h> //fprintf,printf,stderr  TODO: make it optional

#include <string.h> //memcpy,memmove

/* base memory helpers */
CVH_API_IMPL void* cvh_malloc(size_t size) {
    void* p = CVH_MALLOC(size);
    if (!p)	{
        CVH_ASSERT(0);	// No more memory error
        fprintf(stderr,"CVH_ERROR: cvh_malloc(...) failed. Not enough memory.\n");
        exit(1);
    }
    return p;
}
CVH_API_IMPL void cvh_free(void* p)                         {CVH_FREE(p);}
//CVH_API_PRIV void* cvh_realloc(void *ptr, size_t new_size)  {return CVH_REALLOC(ptr,new_size);}
CVH_API_PRIV void* cvh_safe_realloc(void** const ptr, size_t new_size)  {
    void *ptr2 = CVH_REALLOC(*ptr,new_size);
    CVH_ASSERT(new_size!=0);    /* undefined behaviour */
    if (ptr2) *ptr=ptr2;
    else {
        CVH_FREE(*ptr);*ptr=NULL;
        CVH_ASSERT(0);	// No more memory error
        fprintf(stderr,"CVH_ERROR: cvh_safe_realloc(...) failed. Not enough memory.\n");
        exit(1);
    }
    return ptr2;
}


/* vector helpers */
CVH_API_PRIV CVH_API_INL void* cvh_vector_realloc(void** const pvector,size_t new_size_in_bytes,size_t* pvector_capacity_in_bytes)  {
    // grows-only!
    const size_t vector_capacity_in_bytes = *pvector_capacity_in_bytes;
    if (new_size_in_bytes>vector_capacity_in_bytes) {
        (*pvector_capacity_in_bytes)=vector_capacity_in_bytes + (new_size_in_bytes-vector_capacity_in_bytes)+vector_capacity_in_bytes/2;
        return cvh_safe_realloc(pvector,*pvector_capacity_in_bytes);
    }
    return *pvector;
}
CVH_API_PRIV CVH_API_INL void cvh_vector_swap(void** const pvectorA,size_t* pvectorAsize,size_t* pvectorAcapacity,void** const pvectorB,size_t* pvectorBsize,size_t* pvectorBcapacity)  {
    void* const vectorT = *pvectorA;
    const size_t vectorTsize = *pvectorAsize;
    const size_t vectorTcapacity = *pvectorAcapacity;
    *pvectorA = *pvectorB;
    *pvectorAsize = *pvectorBsize;
    *pvectorAcapacity = *pvectorBcapacity;
    *pvectorB = vectorT;
    *pvectorBsize = vectorTsize;
    *pvectorBcapacity = vectorTcapacity;
}
CVH_API_PRIV CVH_API_INL size_t cvh_vector_linear_search(const void* v,size_t item_size_in_bytes,size_t num_items,const void* item_to_search,int (*ItemCmp)(const void* item0,const void* item1),int* match)  {
    int cmp=0;size_t i;
    const unsigned char* vec = (const unsigned char*)v;
    if (match) *match=0;
    if (num_items==0) return 0;  /* otherwise match will be 1 */
    for (i = 0; i < num_items; i++) {
        cmp = ItemCmp(item_to_search,&vec[i*item_size_in_bytes]);
        if (cmp>=0) {
            if (cmp==0 && match) *match=1;
            return i;
        }
    }
    CVH_ASSERT(i<=num_items);
    return i;
}
CVH_API_PRIV CVH_API_INL size_t cvh_vector_binary_search(const void* v,size_t item_size_in_bytes,size_t num_items,const void* item_to_search,int (*ItemCmp)(const void* item0,const void* item1),int* match)  {
    size_t first=0, last=num_items-1;
    size_t mid;int cmp;
    const unsigned char* vec = (const unsigned char*)v;
    if (match) *match=0;
    if (num_items==0) return 0;  /* otherwise match will be 1 */
    while (first <= last) {
        mid = (first + last) / 2;
        cmp = ItemCmp(item_to_search,&vec[mid*item_size_in_bytes]);
        if (cmp>0) {
            first = mid + 1;
        }
        else if (cmp<0) {
            if (mid==0) return 0;
            last = mid - 1;
        }
        else {if (match) *match=1;CVH_ASSERT(mid<num_items); return mid;}
    }
    CVH_ASSERT(mid<num_items);
    return cmp>0 ? (mid+1) : mid;
}
CVH_API_PRIV CVH_API_INL void cvh_vector_insert_at(void* v,size_t item_size_in_bytes,size_t num_items,const void* item_to_insert,size_t position)  {
    /* IMPORTANT: v must have enough space to insert one more item!!! */
    /* position is in [0,num_items] */
    unsigned char* vec = (unsigned char*) v;
    CVH_ASSERT(position<=num_items);
    if (position<num_items) memmove(&vec[(position+1)*item_size_in_bytes],&vec[position*item_size_in_bytes],(num_items-position)*item_size_in_bytes);
    memcpy(&vec[position*item_size_in_bytes],item_to_insert,item_size_in_bytes);
}
CVH_API_PRIV CVH_API_INL size_t cvh_vector_insert_sorted(void* v,size_t item_size_in_bytes,size_t num_items,const void* item_to_insert,int (*ItemCmp)(const void* item0,const void* item1),int* match,int insert_even_if_item_match)  {
    /* IMPORTANT: v must have enough space to insert one more item!!! */
    int my_match = 0;
    size_t position = cvh_vector_binary_search(v,item_size_in_bytes,num_items,item_to_insert,ItemCmp,&my_match);
    if (match) *match = my_match;
    if (my_match && !insert_even_if_item_match) return position;
    cvh_vector_insert_at(v,item_size_in_bytes,num_items,item_to_insert,position);
    return position;
}
CVH_API_PRIV CVH_API_INL void cvh_vector_remove_at(void* v,size_t item_size_in_bytes,size_t num_items,size_t position)  {
    /* position is in [0,num_items) */
    unsigned char* vec = (unsigned char*) v;
    CVH_ASSERT(position<num_items);
    memmove(&vec[position*item_size_in_bytes],&vec[(position+1)*item_size_in_bytes],(num_items-position-1)*item_size_in_bytes);
}


/* hashtable helpers */
//#define CVH_HASTABLE_UNSIGNED_SHORT   // much more memory vs some ms faster (better define it in client code if necessary)
#ifndef CVH_HASTABLE_UNSIGNED_SHORT
typedef unsigned char cvh_htuint;
#define CVH_NUM_HTUINT 256
#else // CVH_HASTABLE_UNSIGNED_SHORT
typedef unsigned short cvh_htuint;
#define CVH_NUM_HTUINT 65536
#endif // CVH_HASTABLE_UNSIGNED_SHORT
struct cvh_hashtable_t {
    size_t item_size_in_bytes;
    cvh_htuint (*item_hash)(const void* item);
    int (*item_cmp) (const void* item0,const void* item1);
    size_t initial_bucket_capacity_in_items;
    struct cvh_hashtable_vector_t   {
        void* p;
        size_t capacity_in_bytes;
        size_t num_items;
    } buckets[CVH_NUM_HTUINT]; /* CVH_NUM_HTUINT (256 by default) binary-sorted buckets */
};

// Give global visibility to structs 'cvh_hashtable_t' and 'cvh_hashtable_vector_t'
#ifdef __cplusplus
typedef struct cvh_hashtable_t cvh_hashtable_t;
typedef cvh_hashtable_t::cvh_hashtable_vector_t cvh_hashtable_vector_t;
#else
typedef struct cvh_hashtable_t cvh_hashtable_t;
typedef struct cvh_hashtable_vector_t cvh_hashtable_vector_t;
#endif

CVH_API_PRIV struct cvh_hashtable_t* cvh_hashtable_create(size_t item_size_in_bytes,cvh_htuint (*item_hash)(const void* item),int (*item_cmp) (const void* item0,const void* item1),size_t initial_bucket_capacity_in_items)   {
    cvh_hashtable_t* ht = (cvh_hashtable_t*) cvh_malloc(sizeof(struct cvh_hashtable_t));
    ht->item_size_in_bytes = item_size_in_bytes;
    ht->item_hash = item_hash;
    ht->item_cmp = item_cmp;
    ht->initial_bucket_capacity_in_items = initial_bucket_capacity_in_items>1 ? initial_bucket_capacity_in_items : 1;
    CVH_ASSERT(ht->item_size_in_bytes && ht->item_hash && ht->item_cmp);
    memset(ht->buckets,0,CVH_NUM_HTUINT*sizeof(cvh_hashtable_vector_t));
    return ht;
}
CVH_API_PRIV void cvh_hashtable_free(struct cvh_hashtable_t* ht)    {
    if (ht) {
        const unsigned short max_value = (CVH_NUM_HTUINT-1);
        unsigned short i=0;
        do    {
            cvh_hashtable_vector_t* v = &ht->buckets[i];
            if (v->p) {CVH_FREE(v->p);v->p=NULL;v->capacity_in_bytes=0;}
        }
        while (i++!=max_value);
        CVH_FREE(ht);
    }
}
CVH_API_PRIV void* cvh_hashtable_get_or_insert(struct cvh_hashtable_t* ht,const void* pvalue,int* match) {
    cvh_hashtable_vector_t* v = NULL;
    size_t position;unsigned char* vec;
    CVH_ASSERT(ht);
    v = &ht->buckets[ht->item_hash(pvalue)];
    if (!v->p)  v->p = cvh_malloc(ht->initial_bucket_capacity_in_items*ht->item_size_in_bytes);

    /* slightly faster */
    if (v->num_items==0)    {position=0;*match=0;}
    position =  v->num_items>2 ? cvh_vector_binary_search(v->p,ht->item_size_in_bytes,v->num_items,pvalue,ht->item_cmp,match) :
                cvh_vector_linear_search(v->p,ht->item_size_in_bytes,v->num_items,pvalue,ht->item_cmp,match);
    /* slightly slower */
    //position = cvh_vector_binary_search(v->p,ht->item_size_in_bytes,v->num_items,pvalue,ht->item_cmp,match);

    if (*match) {
        vec = (unsigned char*) v->p;
        return &vec[position*ht->item_size_in_bytes];
    }
    // we must insert an item at 'index'
    cvh_vector_realloc(&v->p,(v->num_items+1)*ht->item_size_in_bytes,&v->capacity_in_bytes);
    cvh_vector_insert_at(v->p,ht->item_size_in_bytes,v->num_items,pvalue,position);
    vec = (unsigned char*) v->p;
    memcpy(&vec[position*ht->item_size_in_bytes],pvalue,ht->item_size_in_bytes);
    ++v->num_items;
    return &vec[position*ht->item_size_in_bytes];

    /* // shorter but less efficient
    cvh_vector_realloc(&v->p,(v->num_items+1)*ht->item_size_in_bytes,&v->capacity_in_bytes);
    position = cvh_vector_insert_sorted(v->p,ht->item_size_in_bytes,v->num_items,pvalue,ht->item_cmp,match,0);
    if (!(*match)) ++v->num_items;
    vec = (unsigned char*) v->p;
    return &vec[position*ht->item_size_in_bytes];
    */
}
CVH_API_PRIV void* cvh_hashtable_get(struct cvh_hashtable_t* ht,const void* pvalue) {
    cvh_hashtable_vector_t* v = NULL;
    size_t position;unsigned char* vec;int match=0;
    CVH_ASSERT(ht);
    v = &ht->buckets[ht->item_hash(pvalue)];
    if (!v->p || v->num_items==0)  return NULL;

    position =  v->num_items>2 ? cvh_vector_binary_search(v->p,ht->item_size_in_bytes,v->num_items,pvalue,ht->item_cmp,&match) :
                cvh_vector_linear_search(v->p,ht->item_size_in_bytes,v->num_items,pvalue,ht->item_cmp,&match);
 
    if (match) {
        vec = (unsigned char*) v->p;
        return &vec[position*ht->item_size_in_bytes];
    }

    return NULL;
}
CVH_API_PRIV int cvh_hashtable_remove(struct cvh_hashtable_t* ht,const void* pvalue) {
    cvh_hashtable_vector_t* v = NULL;
    size_t position;int match = 0;
    CVH_ASSERT(ht);
    v = (cvh_hashtable_vector_t*) &ht->buckets[ht->item_hash(pvalue)];
    if (!v->p)  return 0;
    position = cvh_vector_binary_search(v->p,ht->item_size_in_bytes,v->num_items,pvalue,ht->item_cmp,&match);
    if (match) {
        cvh_vector_remove_at(v->p,ht->item_size_in_bytes,v->num_items,position);
        --v->num_items;
        return 1;
    }
    return 0;
}
#ifdef __cplusplus
}
#endif
//==================================================================================
#endif //C_VECTOR_AND_HASHTABLE_H

