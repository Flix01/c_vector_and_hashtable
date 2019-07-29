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

CVH_API_PRIV cvh_hashtable_t* cvh_hashtable_create(size_t item_size_in_bytes,cvh_htuint (*item_hash)(const void* item),int (*item_cmp) (const void* item0,const void* item1),size_t initial_bucket_capacity_in_items)   {
    cvh_hashtable_t* ht = (cvh_hashtable_t*) cvh_malloc(sizeof(cvh_hashtable_t));
    ht->item_size_in_bytes = item_size_in_bytes;
    ht->item_hash = item_hash;
    ht->item_cmp = item_cmp;
    ht->initial_bucket_capacity_in_items = initial_bucket_capacity_in_items>1 ? initial_bucket_capacity_in_items : 1;
    CVH_ASSERT(ht->item_size_in_bytes && ht->item_hash && ht->item_cmp);
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
            v->capacity_in_bytes=v->num_items=0;
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
            // The following (single) line can probably be commented out to maximize performance
            if (v->p) {CVH_FREE(v->p);v->p=NULL;v->capacity_in_bytes=0;}
            v->num_items = 0;
        }
        while (i++!=max_value);
    }
}
CVH_API_PRIV void* cvh_hashtable_get_or_insert(cvh_hashtable_t* ht,const void* pvalue,int* match) {
    cvh_hashtable_vector_t* v = NULL;
    size_t position;unsigned char* vec;
    CVH_ASSERT(ht);
    v = &ht->buckets[ht->item_hash(pvalue)];
    if (!v->p)  v->p = cvh_malloc(ht->initial_bucket_capacity_in_items*ht->item_size_in_bytes);

    /* slightly faster */
    if (v->num_items==0)    {position=0;*match=0;}
    else {
        position =  v->num_items>2 ? cvh_vector_binary_search(v->p,ht->item_size_in_bytes,v->num_items,pvalue,ht->item_cmp,match) :
                    cvh_vector_linear_search(v->p,ht->item_size_in_bytes,v->num_items,pvalue,ht->item_cmp,match);
        /* slightly slower */
        //position = cvh_vector_binary_search(v->p,ht->item_size_in_bytes,v->num_items,pvalue,ht->item_cmp,match);
    }

    if (*match) {
        vec = (unsigned char*) v->p;
        return &vec[position*ht->item_size_in_bytes];
    }

    // we must insert an item at 'position'
    cvh_vector_realloc(&v->p,(v->num_items+1)*ht->item_size_in_bytes,&v->capacity_in_bytes);
    // faster
    cvh_vector_insert_at(v->p,ht->item_size_in_bytes,v->num_items,pvalue,position);
    ++v->num_items;
    /*// slower
    position = cvh_vector_insert_sorted(v->p,ht->item_size_in_bytes,v->num_items,pvalue,ht->item_cmp,match,0);
    if (!(*match)) ++v->num_items;*/
    vec = (unsigned char*) v->p;
    return &vec[position*ht->item_size_in_bytes];
}
CVH_API_PRIV void* cvh_hashtable_get(cvh_hashtable_t* ht,const void* pvalue) {
    cvh_hashtable_vector_t* v = NULL;
    size_t position;unsigned char* vec;int match=0;
    CVH_ASSERT(ht);
    v = &ht->buckets[ht->item_hash(pvalue)];
    if (!v->p || v->num_items==0)  return NULL;

    position =  v->num_items>2 ? cvh_vector_binary_search(v->p,ht->item_size_in_bytes,v->num_items,pvalue,ht->item_cmp,&match) :
                cvh_vector_linear_search(v->p,ht->item_size_in_bytes,v->num_items,pvalue,ht->item_cmp,&match);
//    position =  cvh_vector_binary_search(v->p,ht->item_size_in_bytes,v->num_items,pvalue,ht->item_cmp,&match);

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
CVH_API_PRIV size_t cvh_hashtable_get_num_items(cvh_hashtable_t* ht) {
    CVH_ASSERT(ht);size_t i,sum=0;
    for (i=0;i<CVH_NUM_HTUINT;i++) sum+=ht->buckets[i].num_items;
    return sum;
}
CVH_API_PRIV int cvh_hashtable_dbg_check(cvh_hashtable_t* ht) {
    size_t i,j,num_total_items=0;
    const unsigned char* last_item = NULL;
    CVH_ASSERT(ht && ht->item_cmp);
    for (i=0;i<CVH_NUM_HTUINT;i++) {
        const cvh_hashtable_vector_t* bck = &ht->buckets[i];
        num_total_items+=bck->num_items;
        if (bck->p && bck->num_items) {
            last_item = NULL;
            for (j=0;j<bck->num_items;j++)  {
                const unsigned char* item = (const unsigned char*)bck->p+j*ht->item_size_in_bytes;
                if (last_item) {
                    if ((*ht->item_cmp)(last_item,item)>=0) {
                        // When this happens, it can be a wrong user 'item_cmp' function (that cannot sort keys in a consistent way)
                        printf("[cvh_hashtable_dbg_check] Error: in bucket[%lu]: item_cmp(%lu,%lu)<=0 [num_items=%lu (in bucket)]\n",i,j-1,j,bck->num_items);
                    }
                }
                last_item=item;
            }
        }
        printf("[cvh_hashtable_dbg_check] num_total_items = %lu\n",num_total_items);
    }
    return num_total_items;
}
#ifdef __cplusplus
} // extern C
#endif

#ifdef __cplusplus

#ifndef NO_CPP_CVH_VECTOR_CLASS
// This is just a gift for C++ users:
// 'cvh_vector' is a 'std::vector' alternative implementation (well, without full iterator support).
// code based on 'ImVectorEx' from https://github.com/Flix01/imgui/blob/imgui_with_addons/addons/imguistring/imguistring.h
// it can be useful only for C++ users that want to avoid STL.

#ifndef CVH_FORCE_INLINE
#	ifdef _MSC_VER
#		define CVH_FORCE_INLINE __forceinline
#	elif (defined(__clang__) || defined(__GNUC__) || defined(__MINGW32__) || defined(__MINGW64__))
#		define CVH_FORCE_INLINE inline __attribute__((__always_inline__))
#	else
#		define CVH_FORCE_INLINE inline
#	endif
#endif//CVH_FORCE_INLINE

#ifndef CVH_HAS_PLACEMENT_NEW
#define CVH_HAS_PLACEMENT_NEW
struct CvhPlacementNewDummy {};
inline void* operator new(size_t, CvhPlacementNewDummy, void* ptr) { return ptr; }
inline void operator delete(void*, CvhPlacementNewDummy, void*) {}
#define CVH_PLACEMENT_NEW(_PTR)  new(CvhPlacementNewDummy() ,_PTR)
#endif //CVH_HAS_PLACEMENT_NEW


template<typename T>
class cvh_vector
{
public:
    size_t                      sz;
    size_t                      capacityInItems;
    T*                          vec;

    typedef T*         iterator;
    typedef const T*   const_iterator;

    cvh_vector(size_t size=0)   { sz = capacityInItems = 0; vec = NULL;if (size>0) resize(size);}
    ~cvh_vector()               { clear(); }

    CVH_FORCE_INLINE bool                 empty() const                   { return sz == 0; }
    CVH_FORCE_INLINE size_t               size() const                    { return sz; }
    CVH_FORCE_INLINE size_t               capacity() const                { return capacityInItems; }

    CVH_FORCE_INLINE T&          operator[](size_t i)               { CVH_ASSERT(i < sz); return vec[i]; }
    CVH_FORCE_INLINE const T&    operator[](size_t i) const         { CVH_ASSERT(i < sz); return vec[i]; }

    void                                  clear()                         {
        if (vec) {
            for (size_t i=0,isz=sz;i<isz;i++) vec[i].~T();
            cvh_free(vec);
            vec = NULL;
            sz = capacityInItems = 0;
        }
    }
    CVH_FORCE_INLINE iterator             begin()                         { return vec; }
    CVH_FORCE_INLINE const_iterator       begin() const                   { return vec; }
    CVH_FORCE_INLINE iterator             end()                           { return vec + sz; }
    CVH_FORCE_INLINE const_iterator       end() const                     { return vec + sz; }
    CVH_FORCE_INLINE T&          front()                         { CVH_ASSERT(sz > 0); return vec[0]; }
    CVH_FORCE_INLINE const T&    front() const                   { CVH_ASSERT(sz > 0); return vec[0]; }
    CVH_FORCE_INLINE T&          back()                          { CVH_ASSERT(sz > 0); return vec[sz-1]; }
    CVH_FORCE_INLINE const T&    back() const                    { CVH_ASSERT(sz > 0); return vec[sz-1]; }

    CVH_FORCE_INLINE size_t                  _grow_capacity(size_t new_size) const   {
        size_t new_capacity = capacityInItems ? (capacityInItems + capacityInItems/2) : 8; return new_capacity > new_size ? new_capacity : new_size;
    }

    void                        resize(size_t new_size)            {
        if (new_size > capacityInItems) {
            reserve(_grow_capacity(new_size));
        }
        if (new_size < sz)   {for (size_t i=new_size;i<sz;i++) vec[i].~T();}
        else {for (size_t i=sz;i<new_size;i++) {CVH_PLACEMENT_NEW(&vec[i]) T();}}
        sz = new_size;
    }
    void                        resize(size_t new_size,const T& v)            {
        if (new_size > capacityInItems) {
            reserve(_grow_capacity(new_size));
        }
        if (new_size < sz)   {for (size_t i=new_size;i<sz;i++) vec[i].~T();}
        else {for (size_t i=sz;i<new_size;i++) {CVH_PLACEMENT_NEW(&vec[i]) T();vec[i]=v;}}
        sz = new_size;
    }
    void                        reserve(size_t new_capacity)
    {
        if (new_capacity <= capacityInItems) return;

        // Note that we cannot use cvh_vector_realloc here, because we need to call all dsts/ctrs...
        // cvh_vector_realloc((void**)&vec,new_capacity*sizeof(T),&capacityInBytes);
        // That's the main reason why we have kept capacity in T units

        T* new_data = (T*)cvh_malloc((size_t)new_capacity * sizeof(T));
        for (size_t i=0;i<sz;i++) {
            CVH_PLACEMENT_NEW(&new_data[i]) T();       // Is this dstr/ctr pair really needed or can I just copy...?
            new_data[i] = vec[i];
            vec[i].~T();
        }
        //memcpy(new_data, Data, (size_t)Size * sizeof(value_type));
        cvh_free(vec);
        vec = new_data;
        capacityInItems = new_capacity;
    }

    CVH_FORCE_INLINE  void                 push_back(const T& v)  {
        if (sz == capacityInItems) {
            if ((&v >= vec) && (&v < (vec+sz)))  {
                const T v_val = v;	// Now v can point to old Data field
                reserve(_grow_capacity(sz+1));
                CVH_PLACEMENT_NEW(&vec[sz]) T();
                vec[sz++] = v_val;
            }
            else {
                reserve(_grow_capacity(sz+1));
                CVH_PLACEMENT_NEW(&vec[sz]) T();
                vec[sz++] = v;
            }
        }
        else {
            CVH_PLACEMENT_NEW(&vec[sz]) T();
            vec[sz++] = v;
        }
    }
    CVH_FORCE_INLINE  void                 pop_back()                      {
        CVH_ASSERT(sz > 0);
        if (sz>0) {
            sz--;
            vec[sz].~T();
        }
    }

    CVH_FORCE_INLINE const cvh_vector<T>& operator=(const cvh_vector<T>& o)  {
        resize(o.sz);
        for (size_t i=0;i<o.sz;i++) (*this)[i]=o[i];
        return *this;
    }

    // Not too sure about this
    inline void                 swap(cvh_vector<T>& rhs)          { size_t rhs_size = rhs.sz; rhs.sz = sz; sz = rhs_size; size_t rhs_cap = rhs.capacityInItems; rhs.capacityInItems = capacityInItems; capacityInItems = rhs_cap; T* rhs_data = rhs.vec; rhs.vec = vec; vec = rhs_data; }


private:

    // These 2 does not work: should invoke the dstr and cstr, and probably they should not use memmove
    inline iterator erase(const_iterator it)        {
        CVH_ASSERT(it >= vec && it < vec+sz);
        const ptrdiff_t off = it - vec;
        memmove(vec + off, vec + off + 1, ((size_t)sz - (size_t)off - 1) * sizeof(T));
        sz--;
        return vec + off;
    }
    inline iterator insert(const_iterator it, const T& v)  {
        CVH_ASSERT(it >= vec && it <= vec+sz);
        const ptrdiff_t off = it - vec;
        if (sz == capacityInItems) reserve(capacityInItems ? capacityInItems * 2 : 4);
        if (off < (size_t)sz) memmove(vec + off + 1, vec + off, ((size_t)sz - (size_t)off) * sizeof(T));
        vec[off] = v;
        sz++;
        return vec + off;
    }

};

#endif //NO_CPP_CVH_VECTOR_CLASS
#endif //__cplusplus

//==================================================================================
#endif //C_VECTOR_AND_HASHTABLE_H


