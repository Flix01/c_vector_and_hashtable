/* https://github.com/Flix01/c_vector_and_hashtable */
/*==================================================================================*/
/* Plain C implementation of vector and hashtable */
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

#ifndef C_VECTOR_AND_HASHTABLE_H
#define C_VECTOR_AND_HASHTABLE_H

#ifdef __cplusplus
extern "C" {
#endif

#if (defined (NDEBUG) || defined (_NDEBUG))
#   undef CVH_NO_ASSERT
#   define CVH_NO_ASSERT
#   undef CVH_NO_STDIO
#   define CVH_NO_STDIO
#   undef CVH_NO_STDLIB
#   define CVH_NO_STDLIB
#endif

#ifndef CVH_MALLOC
#   undef CVH_NO_STDLIB  /* stdlib cannot be avoided in this case */
#   include <stdlib.h>	 /* malloc/realloc/free */
#   define CVH_MALLOC(X) malloc(X)
#endif

#ifndef CVH_FREE
#   define CVH_FREE(X) free(X)
#endif

#ifndef CVH_REALLOC
#   define CVH_REALLOC(x,y) realloc((x),(y))
#endif

#ifndef CVH_API_PRIV
#   define CVH_API_PRIV static
#endif
#ifndef CVH_API_IMPL
#   define CVH_API_IMPL /*no-op*/
#endif
#ifndef CVH_API_INL
#   define CVH_API_INL __inline /* if not recognized, try _inline or just inline (__inline should be widely supported by old C compilers) */
#endif

#include <stddef.h> /* size_t */


#ifndef CVH_ASSERT
#   ifdef CVH_NO_ASSERT
#       define CVH_ASSERT(X) /*no-op*/
#   else
#       include <assert.h>
#       define CVH_ASSERT(X) assert((X))
#   endif
#endif
#ifndef CVH_NO_STDIO
#   include <stdio.h> /*fprintf,printf,stderr*/
#endif
#ifndef CVH_NO_STDLIB
#   include <stdlib.h> /*exit*/
#endif

#include <string.h> /*memcpy,memmove,memset*/

/* base memory helpers */
CVH_API_IMPL void* cvh_malloc(size_t size) {
    void* p = CVH_MALLOC(size);
    if (!p)	{
        CVH_ASSERT(0);	/* No more memory error */
#       ifndef CVH_NO_STDIO
        fprintf(stderr,"CVH_ERROR: cvh_malloc(...) failed. Not enough memory.\n");
#       endif
#       ifndef CVH_NO_STDLIB
        exit(1);
#       endif
    }
    return p;
}
CVH_API_IMPL void cvh_free(void* p)                         {CVH_FREE(p);}
/*CVH_API_PRIV void* cvh_realloc(void *ptr, size_t new_size)  {return CVH_REALLOC(ptr,new_size);}*/
CVH_API_PRIV void* cvh_safe_realloc(void** const ptr, size_t new_size)  {
    void *ptr2 = CVH_REALLOC(*ptr,new_size);
    CVH_ASSERT(new_size!=0);    /* undefined behaviour */
    if (ptr2) *ptr=ptr2;
    else {
        CVH_FREE(*ptr);*ptr=NULL;
        CVH_ASSERT(0);	/* No more memory error */
#       ifndef CVH_NO_STDIO
        fprintf(stderr,"CVH_ERROR: cvh_safe_realloc(...) failed. Not enough memory.\n");
#       endif
#       ifndef CVH_NO_STDLIB
        exit(1);
#       endif
    }
    return ptr2;
}


/* vector helpers */
CVH_API_PRIV CVH_API_INL void* cvh_vector_reserve(void** const pvector,size_t new_size_in_items,size_t* pvector_capacity_in_items,const size_t sizeof_item_type_in_bytes)  {
    /* grows-only! */
    if (new_size_in_items>*pvector_capacity_in_items) {
        (*pvector_capacity_in_items)=(*pvector_capacity_in_items) + (new_size_in_items-(*pvector_capacity_in_items))+(*pvector_capacity_in_items)/2;
        return cvh_safe_realloc(pvector,(*pvector_capacity_in_items)*sizeof_item_type_in_bytes);
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
        if (cmp<=0) {
            if (cmp==0 && match) *match=1;
            return i;
        }
    }
    CVH_ASSERT(i==num_items);
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
CVH_API_PRIV CVH_API_INL int cvh_vector_remove_at(void* v,size_t item_size_in_bytes,size_t num_items,size_t position)  {
    /* position is in [0,num_items) */
    unsigned char* vec = (unsigned char*) v;
	const int removal_ok = (position<num_items) ? 1 : 0;
    CVH_ASSERT(removal_ok);	/* error: position>=num_items */
    memmove(&vec[position*item_size_in_bytes],&vec[(position+1)*item_size_in_bytes],(num_items-position-1)*item_size_in_bytes);
	return removal_ok;
}


/* hashtable helpers */
/* CVH_NUM_HTUINT:
-> defines the number of buckets in cvh_hashtable_t.
-> values returned by the user 'itemKey_hash' function should be in the range [0,CVH_NUM_HTUINT).
       usually a mod operation (like: return somevalue%CVH_NUM_HTUINT;) is enough, but
       it can be avoided when CVH_NUM_HTUINT is 256 (the default) or 65536 (its max value).
*/
#ifdef CVH_NUM_HTUINT
#   if CVH_NUM_HTUINT<=0
#       undef CVH_NUM_HTUINT
#       define CVH_NUM_HTUINT 256
#   elif CVH_NUM_HTUINT>65536
#       undef CVH_NUM_HTUINT
#       define CVH_NUM_HTUINT 65536
#   endif
#else
#   define CVH_NUM_HTUINT 256
#endif
#if CVH_NUM_HTUINT<=256
    typedef unsigned char cvh_htuint;
#elif CVH_NUM_HTUINT<=65536
    typedef unsigned short cvh_htuint;
#else
#   error Unsupported CVH_NUM_HTUINT
#endif

struct cvh_hashtable_t {
    size_t item_size_in_bytes;
    cvh_htuint (*itemKey_hash)(const void* item);
    int (*itemKey_cmp) (const void* item0,const void* item1);
    size_t initial_bucket_capacity_in_items;
    struct cvh_hashtable_vector_t   {
        void* p;
        size_t capacity_in_items;
        size_t num_items;
    } buckets[CVH_NUM_HTUINT]; /* CVH_NUM_HTUINT (256 by default) binary-sorted buckets */
};

/* Give global visibility to structs 'cvh_hashtable_t' and 'cvh_hashtable_vector_t' */
#ifdef __cplusplus
typedef struct cvh_hashtable_t cvh_hashtable_t;
typedef cvh_hashtable_t::cvh_hashtable_vector_t cvh_hashtable_vector_t;
#else
typedef struct cvh_hashtable_t cvh_hashtable_t;
typedef struct cvh_hashtable_vector_t cvh_hashtable_vector_t;
#endif

CVH_API_PRIV cvh_hashtable_t* cvh_hashtable_create(size_t item_size_in_bytes,cvh_htuint (*itemKey_hash)(const void* item),int (*itemKey_cmp) (const void* item0,const void* item1),size_t initial_bucket_capacity_in_items)   {
    cvh_hashtable_t* ht = (cvh_hashtable_t*) cvh_malloc(sizeof(cvh_hashtable_t));
    ht->item_size_in_bytes = item_size_in_bytes;
    ht->itemKey_hash = itemKey_hash;
    ht->itemKey_cmp = itemKey_cmp;
    ht->initial_bucket_capacity_in_items = initial_bucket_capacity_in_items>1 ? initial_bucket_capacity_in_items : 1;
    CVH_ASSERT(ht->item_size_in_bytes && ht->itemKey_hash && ht->itemKey_cmp);
    memset(ht->buckets,0,CVH_NUM_HTUINT*sizeof(cvh_hashtable_vector_t));
    return ht;
}
CVH_API_PRIV void cvh_hashtable_free(cvh_hashtable_t* ht)    {
    if (ht) {
        const unsigned short max_value = (CVH_NUM_HTUINT-1);
        unsigned short i=0;
        do    {
            cvh_hashtable_vector_t* v = &ht->buckets[i];
            if (v->p) {CVH_FREE(v->p);v->p=NULL;}
            v->capacity_in_items=v->num_items=0;
        }
        while (i++!=max_value);
        CVH_FREE(ht);
    }
}
CVH_API_PRIV void cvh_hashtable_clear(cvh_hashtable_t* ht) {
    if (ht) {
        const unsigned short max_value = (CVH_NUM_HTUINT-1);
        unsigned short i=0;
        do    {
            cvh_hashtable_vector_t* v = &ht->buckets[i];
            /* The following (single) line can probably be commented out to maximize performance */
            if (v->p) {CVH_FREE(v->p);v->p=NULL;v->capacity_in_items=0;}
            v->num_items = 0;
        }
        while (i++!=max_value);
    }
}
CVH_API_PRIV void* cvh_hashtable_get_or_insert(cvh_hashtable_t* ht,const void* pvalue,int* match) {
    cvh_hashtable_vector_t* v = NULL;
    size_t position;unsigned char* vec;cvh_htuint hash;
    CVH_ASSERT(ht);
    hash = ht->itemKey_hash(pvalue);
#   if (CVH_NUM_HTUINT!=256 && CVH_NUM_HTUINT!=65536)
    CVH_ASSERT(hash<CVH_NUM_HTUINT);    /* user 'itemKey_hash' should return values in [0,CVH_NUM_HTUINT). Please use: return somevalue%CVH_NUM_HTUINT */
#   endif
    v = &ht->buckets[hash];
    if (!v->p)  {
		v->p = cvh_malloc(ht->initial_bucket_capacity_in_items*ht->item_size_in_bytes);
		v->capacity_in_items = ht->initial_bucket_capacity_in_items;	
	}

    /* slightly faster */
    if (v->num_items==0)    {position=0;*match=0;}
    else {
        position =  v->num_items>2 ? cvh_vector_binary_search(v->p,ht->item_size_in_bytes,v->num_items,pvalue,ht->itemKey_cmp,match) :
                    cvh_vector_linear_search(v->p,ht->item_size_in_bytes,v->num_items,pvalue,ht->itemKey_cmp,match);
        /* slightly slower */
        /* position = cvh_vector_binary_search(v->p,ht->item_size_in_bytes,v->num_items,pvalue,ht->itemKey_cmp,match); */
    }

    if (*match) {
        vec = (unsigned char*) v->p;
        return &vec[position*ht->item_size_in_bytes];
    }

    /* we must insert an item at 'position' */
    cvh_vector_reserve(&v->p,v->num_items+1,&v->capacity_in_items,ht->item_size_in_bytes);
    /* faster */
    cvh_vector_insert_at(v->p,ht->item_size_in_bytes,v->num_items,pvalue,position);
    ++v->num_items;
    /* slower */
    /*position = cvh_vector_insert_sorted(v->p,ht->item_size_in_bytes,v->num_items,pvalue,ht->itemKey_cmp,match,0);
    if (!(*match)) ++v->num_items;*/
    vec = (unsigned char*) v->p;
    return &vec[position*ht->item_size_in_bytes];
}
CVH_API_PRIV void* cvh_hashtable_get(cvh_hashtable_t* ht,const void* pvalue) {
    cvh_hashtable_vector_t* v = NULL;
    size_t position;unsigned char* vec;cvh_htuint hash;int match=0;
    CVH_ASSERT(ht);
    hash = ht->itemKey_hash(pvalue);
#   if (CVH_NUM_HTUINT!=256 && CVH_NUM_HTUINT!=65536)
    CVH_ASSERT(hash<CVH_NUM_HTUINT);    /* user 'itemKey_hash' should return values in [0,CVH_NUM_HTUINT). Please use: return somevalue%CVH_NUM_HTUINT */
#   endif
    v = &ht->buckets[hash];
    if (!v->p || v->num_items==0)  return NULL;

    position =  v->num_items>2 ? cvh_vector_binary_search(v->p,ht->item_size_in_bytes,v->num_items,pvalue,ht->itemKey_cmp,&match) :
                cvh_vector_linear_search(v->p,ht->item_size_in_bytes,v->num_items,pvalue,ht->itemKey_cmp,&match);
    /* position =  cvh_vector_binary_search(v->p,ht->item_size_in_bytes,v->num_items,pvalue,ht->itemKey_cmp,&match); */

    if (match) {
        vec = (unsigned char*) v->p;
        return &vec[position*ht->item_size_in_bytes];
    }

    return NULL;
}
CVH_API_PRIV int cvh_hashtable_remove(cvh_hashtable_t* ht,const void* pvalue) {
    cvh_hashtable_vector_t* v = NULL;
    size_t position;int match = 0;
    CVH_ASSERT(ht);
    v = (cvh_hashtable_vector_t*) &ht->buckets[ht->itemKey_hash(pvalue)];
    if (!v->p)  return 0;
    position = cvh_vector_binary_search(v->p,ht->item_size_in_bytes,v->num_items,pvalue,ht->itemKey_cmp,&match);
    if (match) {
        cvh_vector_remove_at(v->p,ht->item_size_in_bytes,v->num_items,position);
        --v->num_items;
        return 1;
    }
    return 0;
}
CVH_API_PRIV size_t cvh_hashtable_get_num_items(cvh_hashtable_t* ht) {
    size_t i,sum=0;CVH_ASSERT(ht);
    for (i=0;i<CVH_NUM_HTUINT;i++) sum+=ht->buckets[i].num_items;
    return sum;
}
CVH_API_PRIV int cvh_hashtable_dbg_check(cvh_hashtable_t* ht) {
    size_t i,j,num_total_items=0,num_sorting_errors=0,min_num_bucket_items=(size_t)-1,max_num_bucket_items=0,min_cnt=0,max_cnt=0,avg_cnt=0,avg_round=0;
    double avg_num_bucket_items=0.0,std_deviation=0.0;
    const unsigned char* last_item = NULL;
    CVH_ASSERT(ht && ht->itemKey_cmp);
    for (i=0;i<CVH_NUM_HTUINT;i++) {
        const cvh_hashtable_vector_t* bck = &ht->buckets[i];
        num_total_items+=bck->num_items;
        if (min_num_bucket_items>bck->num_items) min_num_bucket_items=bck->num_items;
        if (max_num_bucket_items<bck->num_items) max_num_bucket_items=bck->num_items;
        if (bck->p && bck->num_items) {
            last_item = NULL;
            for (j=0;j<bck->num_items;j++)  {
                const unsigned char* item = (const unsigned char*)bck->p+j*ht->item_size_in_bytes;
                if (last_item) {
                    if ((*ht->itemKey_cmp)(last_item,item)>=0) {
                        /* When this happens, it can be a wrong user 'itemKey_cmp' function (that cannot sort keys in a consistent way) */
                        ++num_sorting_errors;
#                       ifndef CVH_NO_STDIO
                        printf("[cvh_hashtable_dbg_check] Error: in bucket[%lu]: itemKey_cmp(%lu,%lu)<=0 [num_items=%lu (in bucket)]\n",i,j-1,j,bck->num_items);
#                       endif
                    }
                }
                last_item=item;
            }
        }
    }
    avg_num_bucket_items = (double)num_total_items/(double) CVH_NUM_HTUINT;
    if (CVH_NUM_HTUINT<2) {std_deviation=0.;avg_cnt=min_cnt=max_cnt=1;avg_round=(min_num_bucket_items+max_num_bucket_items)/2;}
    else {
        const double dec = avg_num_bucket_items-(double)((size_t)avg_num_bucket_items); // in (0,1]
        avg_round = (size_t)avg_num_bucket_items;
        if (dec>=0.5) avg_round+=1;
        for (i=0;i<CVH_NUM_HTUINT;i++) {
            const cvh_hashtable_vector_t* bck = &ht->buckets[i];
            double tmp = bck->num_items-avg_num_bucket_items;
            std_deviation+=tmp*tmp;
            if (bck->num_items==min_num_bucket_items) ++min_cnt;
            if (bck->num_items==max_num_bucket_items) ++max_cnt;
            if (bck->num_items==avg_round) ++avg_cnt;
        }
        std_deviation/=(double)(CVH_NUM_HTUINT-1); /* this is the variance */
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
#   ifndef CVH_NO_STDIO
    printf("[cvh_hashtable_dbg_check] num_total_items=%lu in %d buckets [items per bucket: mean=%1.3f std_deviation=%1.3f min=%lu (in %lu/%d) avg=%lu (in %lu/%d) max=%lu (in %lu/%d)]\n",num_total_items,CVH_NUM_HTUINT,avg_num_bucket_items,std_deviation,min_num_bucket_items,min_cnt,CVH_NUM_HTUINT,avg_round,avg_cnt,CVH_NUM_HTUINT,max_num_bucket_items,max_cnt,CVH_NUM_HTUINT);
#   endif
    CVH_ASSERT(num_sorting_errors==0); /* When this happens, it can be a wrong user 'itemKey_cmp' function (that cannot sort keys in a consistent way) */
    return num_total_items;
}
#ifdef __cplusplus
} /* extern C */
#endif

/*==================================================================================*/
#endif /* C_VECTOR_AND_HASHTABLE_H */


