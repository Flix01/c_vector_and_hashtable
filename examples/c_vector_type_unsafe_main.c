/*
// (plain C) compilation:
// gcc
gcc -O2 -no-pie -fno-pie -I"../include" c_vector_type_unsafe_main.c -o c_vector_type_unsafe_main
// clang
clang -O2 -no-pie -fno-pie -I"../include" c_vector_type_unsafe_main.c -o c_vector_type_unsafe_main
// mingw
x86_64-w64-mingw32-gcc -mconsole -O2 -I"../include" c_vector_type_unsafe_main.c -o c_vector_type_unsafe_main.exe
// cl.exe (from Visual C++ 7.1 2003)
cl /O2 /MT /Tc c_vector_type_unsafe_main.c /I"../include" /link /out:c_vector_type_unsafe_main.exe user32.lib kernel32.lib
// zig (embedded c compiler) (for linux)    Please see: https://andrewkelley.me/post/zig-cc-powerful-drop-in-replacement-gcc-clang.html
zig cc -O2 -target x86_64-linux-gnu -I"../include" c_vector_type_unsafe_main.c -o c_vector_type_unsafe_main
// zig (embedded c compiler) (for windows)
zig cc -O2 -target x86_64-windows-gnu -I"../include" c_vector_type_unsafe_main.c -o c_vector_type_unsafe_main.exe
*/

/*
// compile as C++ [not really necessary]
// gcc
gcc -O2 -x c++ -no-pie -fno-pie -I"../include" c_vector_type_unsafe_main.c -o c_vector_type_unsafe_main
// or just
g++ -O2 -no-pie -fno-pie -I"../include" c_vector_type_unsafe_main.c -o c_vector_type_unsafe_main
// clang and mingw are gcc based (try using clang++ and x86_64-w64-mingw32-g++)

// cl.exe (from Visual C++ 7.1 2003)
cl /O2 /MT /Tp c_vector_type_unsafe_main.c /I"../include" /EHsc /link /out:c_vector_type_unsafe_main_vc.exe user32.lib kernel32.lib

// zig should have the "zig c++" syntax for compiling c++ files. Please see: https://zig.news/kristoff/compile-a-c-c-project-with-zig-368j
*/

/* Program Output:

VECTOR TEST:
v[0]={	333,	333,	333};
v[1]={	100,	333,	333};
v[2]={	-10,	333,	333};
v[3]={	-10,	50,	333};
Removed item 1.
v[0]={	333,	333,	333};
v[1]={	-10,	333,	333};
v[2]={	-10,	50,	333};

SORTED VECTOR TEST:
v[0]={	50,	10,	333};
v[1]={	100,	10,	333};
v[2]={	100,	50,	333};
v[3]={	200,	50,	333};
v[4]={	500,	10,	333};
[cvector_dbg_check]:
    size: 5. capacity: 7. sizeof(item): 12
    sorting: OK.
    memory_used: 324 Bytes. memory_minimal_possible: 300 Bytes. mem_used_percentage: 108.00% (100% is the best possible result).
[cvector_dbg_check]:
    size: 5. capacity: 5. sizeof(item): 12
    sorting: OK.
    memory_used: 300 Bytes. memory_minimal_possible: 300 Bytes. mem_used_percentage: 100.00% (100% is the best possible result).
re-inserting v[0]={	50,	10,	333};
v[0]={	50,	10,	333};
v[1]={	50,	10,	333};
v[2]={	100,	10,	333};
v[3]={	100,	50,	333};
v[4]={	200,	50,	333};
v[5]={	500,	10,	333};
item={	100,	50,	333}	found at v[3].
item={	100,	10,	100}	not found.
After serialization and deserialization:
v[0]={	50,	10,	333};
v[1]={	50,	10,	333};
v[2]={	100,	10,	333};
v[3]={	100,	50,	333};
v[4]={	200,	50,	333};
v[5]={	500,	10,	333};

SORTED STRINGVECTOR TEST:
s[0]="ciao";
s[1]="golden day";
s[2]="good morning";
s[3]="hello world";
s[4]="hi";
item="bye bye"	not found.
item="hello world"	found at s[3].
removing item s[3]="hello world".
s[0]="ciao";
s[1]="golden day";
s[2]="good morning";
s[3]="hi";
After serialization and deserialization:
s[0]="ciao";
s[1]="golden day";
s[2]="good morning";
s[3]="hi";

COMPLEX VECTOR TEST:
v[0]={	2.500,	{[3:]	(1,2,3),(2,2,3),(2,3,3)}}
v[1]={	20.000,	{[2:]	(10,3,3),(20,3,3)}}
v[2]={	-100.000,	{[4:]	(20,3,-100),(20,333,-100),(333,333,-100),(333,333,333)}}
After serialization and deserialization:
v[0]={	2.500,	{[3:]	(1,2,3),(2,2,3),(2,3,3)}}
v[1]={	20.000,	{[2:]	(10,3,3),(20,3,3)}}
v[2]={	-100.000,	{[4:]	(20,3,-100),(20,333,-100),(333,333,-100),(333,333,333)}}
Removing element at 0 and then resizing to just one element:
v[0]={	20.000,	{[2:]	(10,3,3),(20,3,3)}}

CVH_STRING_T TEST:
v content:
0) "Germany":
    0) "Berlin"		population: 100000
    1) "Munchen"		population: 50000
    2) "Hamburg"		population: 20000
    3) "Bonn"		population: 10000
    4) "Frankfurt"		population: 40000
1) "France":
    0) "Paris"		population: 200000
    1) "Marseille"		population: 100000
    2) "Lyon"		population: 25000
2) "Italy":
    0) "Rome"		population: 50000
    1) "Milan"		population: 20000
    2) "Florence"		population: 15000
    3) "Venice"		population: 25000
After serialization and deserialization:
v content:
0) "Germany":
    0) "Berlin"		population: 100000
    1) "Munchen"		population: 50000
    2) "Hamburg"		population: 20000
    3) "Bonn"		population: 10000
    4) "Frankfurt"		population: 40000
1) "France":
    0) "Paris"		population: 200000
    1) "Marseille"		population: 100000
    2) "Lyon"		population: 25000
2) "Italy":
    0) "Rome"		population: 50000
    1) "Milan"		population: 20000
    2) "Florence"		population: 15000
    3) "Venice"		population: 25000

======= THIS OUTPUT IS PRESENT ONLY WHEN this file is compiled as c++: ======

CPP MODE TEST:
v0[0] = 0;
v0[1] = 1;
v0[2] = 2;
v0[3] = 3;
v0[4] = 4;
v1[0] = 3;
v1[1] = 4;
v1[2] = 5;
v1[3] = 6;
cvector_swap(&v1,&v0):
v0[0] = 3;
v0[1] = 4;
v0[2] = 5;
v0[3] = 6;
v1[0] = 0;
v1[1] = 1;
v1[2] = 2;
v1[3] = 3;
v1[4] = 4;
std::vector<cvector> test:
vstd[0] = {4:	[3,4,5,6]};
vstd[1] = {5:	[0,1,2,3,4]};
vstd[2] = {4:	[3,4,5,6]};
vstd[3] = {5:	[0,1,2,3,4]};
vstd[4] = {4:	[3,4,5,6]};
vstd[5] = {5:	[0,1,2,3,4]};
cvector_cpy(&v1,&v0):
v0[0] = 3;
v0[1] = 4;
v0[2] = 5;
v0[3] = 6;
v1[0] = 3;
v1[1] = 4;
v1[2] = 5;
v1[3] = 6;
cvector< std::vector<int> > test:
v[0] = {3:	[3,2,1]};
v[1] = {2:	[4,5]};
v[2] = {1:	[6]};

*/

/*#define CV_ENABLE_CLEARING_ITEM_MEMORY */    /* just for testing */
/*#define CV_DISABLE_FAKE_MEMBER_FUNCTIONS*/ /* just for testing */

/* The following line is completely optional, and must be used only
   if you need to silence some compiler or static analyzer warning.
   For further info, see: https://en.cppreference.com/w/c/string/byte/memcpy
*/
/*#define __STDC_WANT_LIB_EXT1__ 1*/ /* use memcpy_s, memmove_s and memset_s instead of memcpy, memmove and memset, if bound-checking functions are supported */
#include "c_vector_type_unsafe.h"
#include <stdio.h>  /* printf */

/*#define NO_SIMPLE_TEST*/
/*#define NO_STRINGVECTOR_TEST*/
/*#define NO_COMPLEXTEST*/
/*#define NO_CPP_TEST*/
/*#define NO_CVH_STRING_T_TEST*/

#ifndef NO_SIMPLE_TEST
/* The struct we'll use in a vector */
/* 'typedef is mandatory: we need global visibility */
typedef struct mystruct {
int a,b,c;
} mystruct;
/* this will be used for the sorted vector test only */
static int mystruct_cmp(const void* av,const void* bv) {
    const mystruct* a=(const mystruct*)av;
    const mystruct* b=(const mystruct*)bv;
	if (a->a>b->a) return 1;
	else if (a->a<b->a) return -1;
	if (a->b>b->b) return 1;
	else if (a->b<b->b) return -1;
	if (a->c>b->c) return 1;
	else if (a->c<b->c) return -1;
	return 0;
}


static void SimpleTest(void)   {
    cvector v;                  /* a.k.a. std::vector<mystruct> */
    /* we skip initialization here because we'll use 'cvector_init(...)' later */

	const mystruct* p = NULL;	/* we'll use this to list items in a type-safe way */

    mystruct tmp = {-10,200,-5};	/* tmp item used later */
    size_t i,position;int match;
    cvh_serializer_t serializer = cvh_serializer_create();    /* we'll use 'serializer' at the end. NEVER leave 'cvector' (or 'cvh_serializer_t') unitialized. Use xxx_create(...) or xxx_init(...) to initialize them */

    printf("VECTOR TEST:\n");

    /* Note that we could just have initialized 'v' this way: 'cvector v=cvector_create(sizeof(mystruct),NULL);' too.
       Please never use unitialized cvectors and cvh_xxx structs unitialized  */
    cvector_init(&v,sizeof(mystruct),NULL);
    /* With the 'fake member function call syntax', we can make fake member calls like:
       v.reserve(&v,4); // note that 'v' appears twice
       However in this demo we don't use this syntax (that can be removed by defining
       CV_DISABLE_FAKE_MEMBER_FUNCTIONS globally (= in the Project Options)
    */

    /* optional */
    cvector_reserve(&v,4);

    /* we insert 4 items */
    tmp.a=tmp.b=tmp.c=333;	cvector_push_back(&v,&tmp);
    tmp.a=100;			cvector_push_back(&v,&tmp);
    tmp.a=-10;			cvector_push_back(&v,&tmp);
    tmp.b=50;			cvector_push_back(&v,&tmp);

    /* Note that if C99 is available we can use 'compound literal'
    to specify structs on the fly in function arguments (i.e. we don't need 'tmp'): */
/*#	if __STDC_VERSION__>= 199901L
    cvector_push_back(&v,&(mystruct){10,20,30});
#	endif*/

    /* display all items */
    p = (const mystruct*) v.v; /* of course this pointer can be reallocated and must be set soon before usage */
    for (i=0;i<v.size;i++) printf("v[%lu]={\t%d,\t%d,\t%d};\n",i,p[i].a,p[i].b,p[i].c);

    /* remove element 1 */
    if (cvector_remove_at(&v,1))	printf("Removed item 1.\n");

    /* display all items */
    p = (const mystruct*) v.v;
    for (i=0;i<v.size;i++) printf("v[%lu]={\t%d,\t%d,\t%d};\n",i,p[i].a,p[i].b,p[i].c);

    /* cvector_clear(&v);	*/ /* this will clear the vector WITHOUT deallocating its capacity (the memory is still allocated and MUST be freed) */

    cvector_free(&v);	/* this clears + frees memory (note that it's ready to be reused (but then you must free it again at the end)) */
    cvector_free(&v);	/* calling it more than once is useless, but safe */


    printf("\nSORTED VECTOR TEST:\n");

    /* 'cvector_init(...)' can be called only to replace the initializer list, or after 'cvector_free(...)' */
    /* we need it just because we must change 'v.item_cmp', which is const to prevent users from changing it on the fly and break sorting */
    cvector_init(&v,sizeof(mystruct),&mystruct_cmp);    /* we can set 'mystruct_cmp' in its initialization list, or in 'cvector_init(...)' too */

    /* we add 5 items in a sorted way */
    tmp.a=200;cvector_insert_sorted(&v,&tmp,NULL,1);
    tmp.a=100;cvector_insert_sorted(&v,&tmp,NULL,1);
    tmp.b=10;cvector_insert_sorted(&v,&tmp,NULL,1);
    tmp.a=50;cvector_insert_sorted(&v,&tmp,NULL,1);
    tmp.a=500;cvector_insert_sorted(&v,&tmp,NULL,1);
    /* last 2 arguments are (int* match=NULL,int insert_even_if_item_match=1): we set the latter to 1 (true) */
    /* Also it can be useful to know that 'cvector_xxx_insert_sorted' returns a size_t insertion 'position'. */

    /* display all items */
    p = (const mystruct*) v.v;
    for (i=0;i<v.size;i++) printf("v[%lu]={\t%d,\t%d,\t%d};\n",i,p[i].a,p[i].b,p[i].c);

    /* (optional) debug and optimize memory usage */
    cvector_dbg_check(&v);  /* displays dbg info */
    cvector_shrink_to_fit(&v);  /* frees all unused memory (slow) */
    CV_ASSERT(v.size==v.capacity);
    cvector_dbg_check(&v);  /* displays dbg info again */

    /* (test) we re-add an already inserted item (right after 'cvector_shrink_to_fit(...)' so that a reallocation will happen invalidating the item pointer) */
    if (v.size>0) {
	    const mystruct* pitem;
        p = (const mystruct*) v.v;
        pitem = &p[0];
        printf("re-inserting v[0]={\t%d,\t%d,\t%d};\n",p[0].a,p[0].b,p[0].c);
        CV_ASSERT(p && pitem>=p && pitem<(p+v.size)); /* if a realloc happens, 'pitem' will be invalidated before we can copy it back in the vector, unless the code is robust enough to detect this border case */
        cvector_insert_sorted(&v,pitem,NULL,1);
    }

    /* display all items */
    p = (const mystruct*) v.v;
    for (i=0;i<v.size;i++) printf("v[%lu]={\t%d,\t%d,\t%d};\n",i,p[i].a,p[i].b,p[i].c);

    /* now we can serch the sorted vector for certain items this way */
    tmp.a=100;tmp.b=50;tmp.c=333;
    position = cvector_binary_search(&v,&tmp,&match);
    printf("item={\t%d,\t%d,\t%d}\t",tmp.a,tmp.b,tmp.c);
    if (match) printf("found at v[%lu].\n",position);
    else printf("not found.\n");

    tmp.a=100;tmp.b=10;tmp.c=100;
    position = cvector_binary_search(&v,&tmp,&match);
    printf("item={\t%d,\t%d,\t%d}\t",tmp.a,tmp.b,tmp.c);
    if (match) printf("found at v[%lu].\n",position);
    else printf("not found.\n");

    /* note that to remove an item, once we have 'position'
    [returned by 'xxx_insert_sorted(...)' or by 'xxx_binary_search(...)']
    we can use 'xxx_remove_at(...)' like above */

    printf("After serialization and deserialization:\n");
    cvector_serialize(&v,&serializer);  /* serializes 'v' into 'serializer' */

/*#   define SERIALIZE_TO_FILE*/    /* just to show that's possible to save/load 'serializer' to/from file */
#   ifdef SERIALIZE_TO_FILE
    cvh_serializer_save(&serializer,"cvtu_mystruct.bin");
    cvh_serializer_free(&serializer);    /* (optional) now it's empty */
    cvh_serializer_load(&serializer,"cvtu_mystruct.bin");     /* now it's full */
#   endif

    cvector_free(&v);   /* (optional) now 'v' it's empty */
    cvector_deserialize(&v,&serializer);    /* deserializes 'serializer' into 'v' */
    cvh_serializer_free(&serializer); /* mandatory */

    /* display all items */
    p = (const mystruct*) v.v;
    for (i=0;i<v.size;i++) printf("v[%lu]={\t%d,\t%d,\t%d};\n",i,p[i].a,p[i].b,p[i].c);

    cvector_free(&v);	/* this is clear + free memory (note that it's ready to be reused (but then you must free it again at the end)) */
                        /* also note that if you want to reuse it in an unsorted way, you must explicitely call: cvector_init(&v,sizeof(mystruct),NULL); after cv_mystruct_free(&v); */
                        /* IMPORTANT: please never call: v=cvectorcreate(...); or  cvector_init(&v,...); except: a) Soon after 'v' declaration. b) After cvector_free(&v); */

}

#endif /* NO_SIMPLE_TEST */

#ifndef NO_STRINGVECTOR_TEST
typedef char* string;
static int string_cmp(const void* av,const void* bv) {
    const string* a = (const string*) av;
    const string* b = (const string*) bv;
    if (*a==NULL) return (*b==NULL) ? 0 : 1;
    else if (*b==NULL) return 1;
    return strcmp(*a,*b);
}
static void string_ctr(void* av)    {string* a=(string*) av;*a=NULL;}
static void string_dtr(void* av)    {string* a=(string*) av;if (*a) {free(*a);*a=NULL;}}
static void string_cpy(void* av,const void* bv)    {
    string* a=(string*) av;
    const string* b=(const string*) bv;
    if (!*b)    {string_dtr(a);return;}
    else    {
        const size_t blen = strlen(*b);
        if (!(*a) || strlen(*a)<blen) *a = (string) realloc(*a,blen+1);
        CV_MEMCPY(*a,*b,blen+1); /* CV_MEMCPY(...) is the same as memcpy (but can be converted to memcpy_s in same cases) */
    }
}
/* if we need to serialize/deserialize our vector we need these 2 functions */
/* because our 'item struct' (char*) contains (directly or indirectly) pointers */
/* and cannot be deep-copied with plain memcpy calls */
#define USE_LOW_LEVEL_SERIALIZATION /* very good exercise to understand how serialization works */
static void string_serialize(const void* av,cvh_serializer_t* serializer)    {
    const string* a = (const string*) av;
#   ifdef USE_LOW_LEVEL_SERIALIZATION
    /* we serialize strlen(*a)+1 [size_t], and then the string text [strlen(*a)+1 bytes, because we serialize '\0' too] */
    /* (note that we could just serialize the zero-terminated string without its length (or its length and the text without the trailing '\0')) */
    const size_t size_t_size_in_bytes = sizeof(size_t);
    const size_t a_len_plus_one = (*a) ? (strlen(*a)+1) : 0;    /* we set it to 0 when (*a)==NULL */
    CV_ASSERT(a && serializer);
    cvh_serializer_reserve(serializer,serializer->size + size_t_size_in_bytes+a_len_plus_one); /* space reserved for serialization */
    *((size_t*) (&serializer->v[serializer->size])) = a_len_plus_one;serializer->size+=size_t_size_in_bytes; /* a_len_plus_one written, now the text: */
    if (*a) CV_MEMCPY(&serializer->v[serializer->size],(const unsigned char*)(*a),a_len_plus_one);  /* CV_MEMCPY(...) is the same as memcpy (but can be converted to memcpy_s in same cases) */
    serializer->size+=a_len_plus_one;
#   else
    /* Tip: there are plenty of functions cvh_serializer_write_xxx(...) that can be used to simplify and shorten this code, */
    /* but they are untested and hide too much the serialization mechanism to be useful in this demo. */
    cvh_serializer_write_string(serializer,*a,NULL);   /* this is supposed to be a good alternative to all the low-level code above */
#   endif
}
static int string_deserialize(void* av,const cvh_serializer_t* deserializer)    {
    string* a = (string*) av;
#   ifdef USE_LOW_LEVEL_SERIALIZATION
    /* this is more difficult. Deserialization starts at the 'mutable' deserializer->offset (that must be incremented) */
    /* also we must return 0 on failure and 1 on success */
    /* also we now that the initial state of 'a' is valid, but its value is unknown. This means that: */
    /* -> if (CV_ENABLE_CLEARING_ITEM_MEMORY is defined) first its memory was blanked (not by default) */
    /* -> then 'item_ctr' (i.e. 'string_ctr' in our case) was called */
    /* -> then 'a' could be changed or not (possibly by 'item_cpy'), but for sure 'item_dtr' was not called */
    const size_t size_t_size_in_bytes = sizeof(size_t);
    size_t a_len_plus_one,b_len;int check;const char* b;
    CV_ASSERT(a && deserializer);
    check = (deserializer->offset+size_t_size_in_bytes<=deserializer->size);
    CV_ASSERT(check);  /* otherwise deserialization will fail */
    if (!check) return 0; /* but when we compile with NDEBUG, or CV_NO_ASSERT, the caller must be notified of the failure */
    a_len_plus_one = *((size_t*) &deserializer->v[deserializer->offset]);*((size_t*) &deserializer->offset)+=size_t_size_in_bytes; /* 'a_len_plus_one' read, 'deserializer->offset' incremented */
    check = deserializer->offset+a_len_plus_one<=deserializer->size;
    CV_ASSERT(check);  /* otherwise deserialization will fail*/
    if (!check) return 0; /* but when we compile with NDEBUG, or CV_NO_ASSERT, the caller must be notified of the failure */
    /* now we must copy the text into '*a' */
    if (a_len_plus_one>0) {
        b = (const char*) &deserializer->v[deserializer->offset];
        *((size_t*) &deserializer->offset)+=a_len_plus_one; /* never forget to increase the 'mutable' 'deserializer->offset' */
        /* The following code is adapted from the last part of 'string_cpy': */
        b_len = strlen(b);CV_ASSERT(a_len_plus_one==b_len+1);  /* we can have this additional check, because we have serialized the string length too */
        /*if (!check) return 0; */ /* optional, or we just rely on 'a_len_plus_one' */
        if (!(*a) || strlen(*a)<b_len) *a = (string) realloc(*a,a_len_plus_one);
        CV_MEMCPY(*a,b,a_len_plus_one);   /* CV_MEMCPY(...) is the same as memcpy (but can be converted to memcpy_s in same cases) */
    }
    else {
        /* serialized string was NULL */
        string_dtr(a);
        return 1;
    }
    return 1;
#   else
    /* Tip: there are plenty of functions cvh_serializer_read_xxx(...) that can be used to simplify and shorten this code, */
    /* but they are untested and hide too much the deserialization mechanism to be useful in this demo */
    return cvh_serializer_read_string(deserializer,a,&realloc,&free);   /* this is supposed to be a good alternative to all the code above */
#   endif
}


static void StringVectorTest(void) {
    cvector s;    /* a.k.a. std::vector<string> */

    size_t i,position;int match=0;
    const string* p = NULL;
    const char* tmp[6]={"good morning","hello world","ciao","hi","golden day","bye bye"};
    cvh_serializer_t serializer = cvh_serializer_create();  /* mandatory initialization */

    printf("\nSORTED STRINGVECTOR TEST:\n");

    /* cvector_init_with(...) is like cvector_init(...) with additional params */
    cvector_init_with(&s,sizeof(string),&string_cmp,&string_ctr,&string_dtr,&string_cpy,
                      &string_serialize,&string_deserialize  /* only necessary if we use cvector_serialize(...)/cvector_deserialize(...); otherwise we can set them to NULL */
                      );

    /* we add 5 items in a sorted way */
    /* note that, in  'c_vector_type_unsafe' the '_by_val' overloads are not present (they'd be void arguments),
       so we can'pass static text strings directly as function arguments AFAIK.
    */
    for (i=0;i<5;i++) cvector_insert_sorted(&s,&tmp[i],NULL,1);
    /* last 2 arguments are (int* match=NULL,int insert_even_if_item_match=1): we set the latter to 1 (true) */
    /* Also it can be useful to know that 'cvector_xxx_insert_sorted' returns a size_t insertion 'position'. */

    /* somebody might think that this works only for static strings. This is not true. */
    /*{
        char* tmp = strdup("good afternoon");
        cvector_insert_sorted(&s,&tmp,NULL,1);
        free(tmp);
    }*/

    /* display all items */
    p = (const string*) s.v;
    for (i=0;i<s.size;i++) printf("s[%lu]=\"%s\";\n",i,p[i]?p[i]:"NULL");

    /* now we can serch the sorted vector for certain items this way */
    position = cvector_binary_search(&s,&tmp[5],&match);
    printf("item=\"%s\"\t",tmp[5]);
    if (match) printf("found at s[%lu].\n",position);
    else printf("not found.\n");

    position = cvector_binary_search(&s,&tmp[1],&match);
    printf("item=\"%s\"\t",tmp[1]);
    if (match) printf("found at s[%lu].\n",position);
    else printf("not found.\n");

    /* remove last searched item (if found) */
    if (match)  {
        p = (const string*) s.v;
        printf("removing item s[%lu]=\"%s\".\n",position,p[position]?p[position]:"NULL");
        cvector_remove_at(&s,position);
    }

    /* display all items */
    p = (const string*) s.v;
    for (i=0;i<s.size;i++) printf("s[%lu]=\"%s\";\n",i,p[i]?p[i]:"NULL");

    printf("After serialization and deserialization:\n");
    cvector_serialize(&s,&serializer);

/*#   define SERIALIZE_TO_FILE*/    /* just to show that's possible */
#   ifdef SERIALIZE_TO_FILE
    cvh_serializer_save(&serializer,"cvtu_string.bin");
    cvh_serializer_free(&serializer);    /* (optional) now it's empty */
    cvh_serializer_load(&serializer,"cvtu_string.bin");     /* now it's full */
#   endif

    cvector_free(&s);   /* (optional) now it's empty */
    cvector_deserialize(&s,&serializer);    /* now it's full */
    cvh_serializer_free(&serializer); /* mandatory */

    /* display all items */
    p = (const string*) s.v;
    for (i=0;i<s.size;i++) printf("s[%lu]=\"%s\";\n",i,p[i]?p[i]:"NULL");

    cvector_free(&s);
}

#endif /* NO_STRINGVECTOR_TEST */

#ifndef NO_COMPLEXTEST
#ifdef NO_SIMPLE_TEST
#   error Please undefine NO_SIMPLE_TEST
#endif
/* Now something much more difficult.
   Tip for newbies: skip this part. */
typedef struct big_t {
    float a;
    cvector v;  /* this is like a std::vector<mystruct> */
} big_t;
/* to make vectors of 'big_t' work,
   we need to specify ctr/dtr/cpy helpers: */
static void big_t_ctr(void* av)    {
    big_t* a=(big_t*)av;
    a->a=0;
    cvector_init(&a->v,sizeof(mystruct),NULL);  /* this can be thought as the 'cv' ctr */
}
static void big_t_dtr(void* av)    {
    big_t* a=(big_t*)av;
    cvector_free(&a->v);    /* this can be thought as the 'cv' dtr */
}
static void big_t_cpy(void* av,const void* bv)    {
    big_t* a=(big_t*)av;
    const big_t* b=(const big_t*)bv;
    a->a=b->a;
    cvector_cpy(&a->v,&b->v);
}
static void big_t_serialize(const void* av,cvh_serializer_t* serializer)    {
    const big_t* a = (const big_t*) av;
#   ifdef USE_LOW_LEVEL_SERIALIZATION
    const size_t size_of_float = sizeof(a->a);
    cvh_serializer_reserve(serializer,serializer->size + size_of_float); /* space reserved for serialization */
    *((float*) (&serializer->v[serializer->size])) = a->a;serializer->size+=size_of_float; /* a->a written */
#   else
    cvh_serializer_write_float(serializer,a->a);
#   endif
    cvector_serialize(&a->v,serializer);
}
static int big_t_deserialize(void* av,const cvh_serializer_t* deserializer)    {
    big_t* a = (big_t*) av;
#   ifdef USE_LOW_LEVEL_SERIALIZATION
    const size_t size_of_float = sizeof(a->a);
    int check = deserializer->offset+size_of_float<=deserializer->size;CV_ASSERT(check);if (!check) return 0;
    a->a = *((float*) &deserializer->v[deserializer->offset]);*((size_t*) &deserializer->offset)+=size_of_float;
#   else
    if (!cvh_serializer_read_float(deserializer,&a->a)) return 0;
#   endif
    if (!cvector_deserialize(&a->v,deserializer)) return 0;
    return 1;
}


static void ComplexTest(void) {
    cvector v; /* a.k.a. std::vector<big_t> */

    const big_t* pb = NULL;

    big_t tmp;
    mystruct ts = {1,2,3};  /* a tmp item for 'tmp.v' */
    const mystruct* pm=NULL;
    size_t i,j;
    cvh_serializer_t serializer = cvh_serializer_create();  /* mandatory initialization (we could have used: cvh_serializer_init(&serializer); instead) */

    printf("\nCOMPLEX VECTOR TEST:\n");

    /* Note that we could have used: 'cvector v = cvector_create_with(sizeof(big_t),NULL,&big_t_ctr,&big_t_dtr,&big_t_cpy,&big_t_serialize,&big_t_deserialize);', instead of 'cvector_init_with(&v,...)' */
    /* First 'NULL' here is 'item_cmp', because we don't need sorted vector features ('cvector_insert_sorted' and 'cvector_binary_search') */
    cvector_init_with(&v,sizeof(big_t),NULL,&big_t_ctr,&big_t_dtr,&big_t_cpy,
                      &big_t_serialize,&big_t_deserialize /* only necessary if we use cvector_serialize(...)/cvector_deserialize(...); otherwise we can set them to NULL */
                      );

    big_t_ctr(&tmp);
    /* element 0 */
    tmp.a=2.5f;
    ts.a=1;cvector_push_back(&tmp.v,&ts);
    ts.a=2;cvector_push_back(&tmp.v,&ts);
    ts.b=3;cvector_push_back(&tmp.v,&ts);
    cvector_push_back(&v,&tmp);
    /* element 1 */
    tmp.a=20.f;cvector_clear(&tmp.v);
    ts.a=10;cvector_push_back(&tmp.v,&ts);
    ts.a=20;cvector_push_back(&tmp.v,&ts);
    cvector_push_back(&v,&tmp);
    /* element 2 */
    tmp.a=-100.f;cvector_clear(&tmp.v);
    ts.c=-100;cvector_push_back(&tmp.v,&ts);
    ts.b=333;cvector_push_back(&tmp.v,&ts);
    ts.a=333;cvector_push_back(&tmp.v,&ts);
    ts.c=333;cvector_push_back(&tmp.v,&ts);
    cvector_push_back(&v,&tmp);
    big_t_dtr(&tmp);

    /* display elements */
    pb = (const big_t*) v.v;
    for (i=0;i<v.size;i++) {
        const big_t* b = &pb[i];
        const float ba = b->a;
        const cvector* bv = &b->v;
        pm = (const mystruct*) bv->v;
        printf("v[%lu]={\t%1.3f,\t{[%lu:]\t",i,ba,bv->size);
        for (j=0;j<bv->size;j++)    {
            if (j>0) printf(",");
            printf("(%d,%d,%d)",pm[j].a,pm[j].b,pm[j].c);
        }
        printf("}}\n");
    }

    printf("After serialization and deserialization:\n");
    cvector_serialize(&v,&serializer);
/*#   define SERIALIZE_TO_FILE*/    /* just to show that's possible */
#   ifdef SERIALIZE_TO_FILE
    cvh_serializer_save(&serializer,"cvtu_big_t.bin");
    cvh_serializer_free(&serializer);    /* (optional) now it's empty */
    cvh_serializer_load(&serializer,"cvtu_big_t.bin");     /* now it's full */
#   endif
    cvector_free(&v);   /* (optional) now it's empty */
    cvector_deserialize(&v,&serializer);    /* now it's full */
    cvh_serializer_free(&serializer); /* mandatory */
    /* display elements */
    pb = (const big_t*) v.v;
    for (i=0;i<v.size;i++) {
        const big_t* b = &pb[i];
        const float ba = b->a;
        const cvector* bv = &b->v;
        pm = (const mystruct*) bv->v;
        printf("v[%lu]={\t%1.3f,\t{[%lu:]\t",i,ba,bv->size);
        for (j=0;j<bv->size;j++)    {
            if (j>0) printf(",");
            printf("(%d,%d,%d)",pm[j].a,pm[j].b,pm[j].c);
        }
        printf("}}\n");
    }

    printf("Removing element at 0 and then resizing to just one element:\n");
    /* remove element 0 */
    cvector_remove_at(&v,0);
    /* resize to just one element */
    cvector_resize(&v,1);

    /* display elements again */
    pb = (const big_t*) v.v;
    for (i=0;i<v.size;i++) {
        const big_t* b = &pb[i];
        const float ba = b->a;
        const cvector* bv = &b->v;
        pm = (const mystruct*) bv->v;
        printf("v[%lu]={\t%1.3f,\t{[%lu:]\t",i,ba,bv->size);
        for (j=0;j<bv->size;j++)    {
            if (j>0) printf(",");
            printf("(%d,%d,%d)",pm[j].a,pm[j].b,pm[j].c);
        }
        printf("}}\n");
    }

    cvector_free(&v);
}

#endif /* NO_COMPLEXTEST */


#if (!defined(NO_CVH_STRING_T_TEST) && !defined(CV_NO_CVH_STRING_T))
/* This test introduces the completely-optional 'cvh_string_t' struct. */
/* It's a constant, grow-only string pool. Basically you store the 'size_t' returned by 'cvh_string_push_back(...)' */
/* instead of a directly allocated char*. Why? Because 'size_t' is not a pointer. */
/* For example, the struct 'city_t' could contain a 'char* name', */
/* but replacing it with a 'size_t' now 'city_t' can be copied using plain memcpy calls, */
/* and this saves us from the tedious task of defining many item_xxx functions for it */
/* Note that there are a lot of limitations: 'cvh_string_t' can't remove items, for example. */
/* The definition CV_NO_CVH_STRING_T can be used to remove 'cvh_string_t'. */
typedef struct city_t {size_t name;unsigned population;} city_t;    /* this can be copied with memcpy! No additional functions needed */
typedef struct country_t {size_t name;cvector cities;cvh_string_t city_names;} country_t; /* this can't be copied with memcpy. Additional functions needed) */
void country_ctr(void* pv)  {
    country_t* p = (country_t*) pv;
    p->name=0;
    cvector_init(&p->cities,sizeof(city_t),NULL);
    cvh_string_init(&p->city_names);    /* 'cvh_string_t' is itself a (type-safe) vector: we need to init it as well  */
}
void country_dtr(void* pv)  {
    country_t* p = (country_t*) pv;
    cvector_free(&p->cities);
    cvh_string_free(&p->city_names);
}
void country_cpy(void* av,const void* bv)  {
    country_t* a = (country_t*) av;
    const country_t* b = (const country_t*) bv;
    a->name = b->name;
    cvector_cpy(&a->cities,&b->cities);
    cvh_string_cpy(&a->city_names,&b->city_names);
}
void country_serialize(const void* pv,cvh_serializer_t* s)  {
    const country_t* p = (const country_t*) pv;
    cvh_serializer_write_size_t(s,p->name);
    cvector_serialize(&p->cities,s);
    cvh_string_serialize(&p->city_names,s);
}
int country_deserialize(void* pv,const cvh_serializer_t* d)  {
    country_t* p = (country_t*) pv;
    if (!cvh_serializer_read_size_t(d,&p->name)) return 0;
    if (!cvector_deserialize(&p->cities,d)) return 0;
    if (!cvh_string_deserialize(&p->city_names,d)) return 0;
    return 1;
}


void CvhStringTTest(void)   {
    cvector v = cvector_create_with(sizeof(country_t),NULL,&country_ctr,&country_dtr,&country_cpy,&country_serialize,&country_deserialize); /* initialization is mandatory! */
    cvh_string_t country_names = cvh_string_create();  /* initialization is mandatory! */
    cvh_serializer_t serializer = cvh_serializer_create();
    size_t i,j;
    const country_t* pcn=NULL;const city_t* pct=NULL;

    printf("\nCVH_STRING_T TEST:\n");

    /* add countries */
    {
        country_t country; /* unitialized. Never use anything unitialized */
        /*--- Germany----*/
        country_ctr(&country);  /* initialized */
        country.name = cvh_string_push_back(&country_names,"Germany",NULL);
        /* add cities to 'c' (the population field is just a random positiove here) */
        {city_t c = {cvh_string_push_back(&country.city_names,"Berlin",NULL),100000};cvector_push_back(&country.cities,&c);}
        {city_t c = {cvh_string_push_back(&country.city_names,"Munchen",NULL),50000};cvector_push_back(&country.cities,&c);}
        {city_t c = {cvh_string_push_back(&country.city_names,"Hamburg",NULL),20000};cvector_push_back(&country.cities,&c);}
        {city_t c = {cvh_string_push_back(&country.city_names,"Bonn",NULL),10000};cvector_push_back(&country.cities,&c);}
        {city_t c = {cvh_string_push_back(&country.city_names,"Frankfurt",NULL),40000};cvector_push_back(&country.cities,&c);}
        cvector_push_back(&v,&country);
        country_dtr(&country);  /* free */
        /*---- France -----*/
        country_ctr(&country);  /* initialized */
        country.name = cvh_string_push_back(&country_names,"France",NULL);
        /* add cities to 'c' (the population field is just a random positiove here) */
        {city_t c = {cvh_string_push_back(&country.city_names,"Paris",NULL),200000};cvector_push_back(&country.cities,&c);}
        {city_t c = {cvh_string_push_back(&country.city_names,"Marseille",NULL),100000};cvector_push_back(&country.cities,&c);}
        {city_t c = {cvh_string_push_back(&country.city_names,"Lyon",NULL),25000};cvector_push_back(&country.cities,&c);}
        cvector_push_back(&v,&country);
        country_dtr(&country);  /* free */
        /*---- Italy -----*/
        country_ctr(&country);  /* initialized */
        country.name = cvh_string_push_back(&country_names,"Italy",NULL);
        /* add cities to 'c' (the population field is just a random positiove here) */
        {city_t c = {cvh_string_push_back(&country.city_names,"Rome",NULL),50000};cvector_push_back(&country.cities,&c);}
        {city_t c = {cvh_string_push_back(&country.city_names,"Milan",NULL),20000};cvector_push_back(&country.cities,&c);}
        {city_t c = {cvh_string_push_back(&country.city_names,"Florence",NULL),15000};cvector_push_back(&country.cities,&c);}
        {city_t c = {cvh_string_push_back(&country.city_names,"Venice",NULL),25000};cvector_push_back(&country.cities,&c);}
        cvector_push_back(&v,&country);
        country_dtr(&country);  /* free */
        /*-------------------------*/
    }

    /* display */
    printf("v content:\n");
    pcn = (const country_t*) v.v;
    for (i=0;i<v.size;i++)  {
        const country_t* country = &pcn[i];
        printf("%lu) \"%s\":\n",i,&country_names.v[country->name]);
        pct = (const city_t*) country->cities.v;
        for (j=0;j<country->cities.size;j++)  {
            const city_t* c = &pct[j];
            printf("\t%lu) \"%s\"\t\tpopulation: %u\n",j,&country->city_names.v[c->name],c->population);
        }
    }

    printf("After serialization and deserialization:\n");
    /* serialize */
    cvector_serialize(&v,&serializer);
    cvh_string_serialize(&country_names,&serializer);
/*#   define SERIALIZE_TO_FILE*/    /* just to show that's possible */
#   ifdef SERIALIZE_TO_FILE
    cvh_serializer_save(&serializer,"cvtu_country_t.bin");
    cvh_serializer_free(&serializer);    /* (optional) now it's empty */
    cvh_serializer_load(&serializer,"cvtu_country_t.bin");     /* now it's full */
#   endif
    cvector_free(&v);              /* (optional) now it's empty */
    cvh_string_free(&country_names);    /* (optional) now it's empty */
    /* deserialize */
    cvector_deserialize(&v,&serializer);               /* now it's full */
    cvh_string_deserialize(&country_names,&serializer);    /* now it's full */
    /* display */
    printf("v content:\n");
    pcn = (const country_t*) v.v;
    for (i=0;i<v.size;i++)  {
        const country_t* country = &pcn[i];
        printf("%lu) \"%s\":\n",i,&country_names.v[country->name]);
        pct = (const city_t*) country->cities.v;
        for (j=0;j<country->cities.size;j++)  {
            const city_t* c = &pct[j];
            printf("\t%lu) \"%s\"\t\tpopulation: %u\n",j,&country->city_names.v[c->name],c->population);
        }
    }

    cvh_serializer_free(&serializer);
    cvh_string_free(&country_names);
    cvector_free(&v);
}
#endif /* (defined(NO_CVH_STRING_T_TEST) && !defined(CV_NO_CVH_STRING_T)) */


#ifndef NO_CPP_TEST
#ifdef __cplusplus
#   include <vector> /* std::vector */


void CppTest(void)    {
    /* This makes only sense when porting existing C++ code to plain C. Some tips:
       -> try to replace a single 'std::vector' with a 'cvector', keep the code compilable (in C++) and repeat the process
       -> a std::vector<cvector> is easier to setup than a cvector of std::vector<type>
       -> a cvector that includes (directly or indirectly) a std::vector (or any other STL container) is a bit harder to make it work, so it's better to avoid it if possible
       -> 'cvector_init(...)' is still mandatory
       -> we can use 'v.at<type>(i)' as a quick replacement of STL operator[] (i.e. 'v[i]') (but it does not work in plain C, so we'll need to convert it later)
       -> cvector::~cvector() calls 'cvector_free(...)' for us (but in plain C it's not available. It can be useful to remember that it's harmless to call 'cvector_free(...)' multiple times)
       -> some programmers prefer using the 'fake member function' syntax when porting code from std::vector
    */

    size_t i,j;
    const int tmp[]={0,1,2,3,4,5,6,7,8,9};
    cvector v0,v1;
    std::vector<cvector> vstd;

    printf("\nCPP MODE TEST:\n");

    cvector_init(&v0,sizeof(int),NULL);   /* 'cvector_init(...)' is still mandatory */
    cvector_init(&v1,sizeof(int),NULL);   /* 'cvector_init(...)' is still mandatory */

    for (i=0;i<5;i++) cvector_push_back(&v0,&tmp[i]); /* or v0.push_back(&v0,&tmp[i]) in the 'fake member function' syntax */
    cvector_insert_range_at(&v1,&tmp[3],4,0);            /* or v1.insert_range_at(&v1,&tmp[3],4,0) in the 'fake member function' syntax */

    for (i=0;i<v0.size;i++) printf("v0[%lu] = %d;\n",i,v0.at<int>(i));  /* Warning: '.at<type>(i)' is C++ specific */
    for (i=0;i<v1.size;i++) printf("v1[%lu] = %d;\n",i,v1.at<int>(i));

    printf("cvector_swap(&v1,&v0):\n");
    cvector_swap(&v1,&v0);

    for (i=0;i<v0.size;i++) printf("v0[%lu] = %d;\n",i,v0.at<int>(i));
    for (i=0;i<v1.size;i++) printf("v1[%lu] = %d;\n",i,v1.at<int>(i));

    printf("std::vector<cvector> test:\n");
    vstd.push_back(v0);
    vstd.push_back(v1);
    vstd.push_back(v0);
    vstd.push_back(v1);
    vstd.push_back(v0);
    vstd.push_back(v1);

    for (j=0;j<vstd.size();j++) {
        const cvector& v = vstd[j];
        printf("vstd[%lu] = {%lu:\t[",j,v.size);
        for (i=0;i<v.size;i++) {
            if (i>0) printf(",");
            printf("%i",v.at<int>(i));
        }
        printf("]};\n");
    }

    printf("cvector_cpy(&v1,&v0):\n");
    cvector_cpy(&v1,&v0);

    for (i=0;i<v0.size;i++) printf("v0[%lu] = %d;\n",i,v0.at<int>(i));
    for (i=0;i<v1.size;i++) printf("v1[%lu] = %d;\n",i,v1.at<int>(i));


    /* in C++ mode ONLY, cvector_free(...) is called for us at cvector::~cvector() */


    {
        std::vector<int> stdi;
        cvector v;

        printf("cvector< std::vector<int> > test:\n");

        /* 'cpp_ctr_tu','cpp_dtr_tu' and 'cpp_cpy_tu' are template functions that 'cvector_type_unsafe.h'
           kindly provides for us (in C++ mode only). They can be used for every c++ class AFAIK
           (the 'tu' suffix stands for 'type_unsafe' to avoid name clashes when used together with 'cvector.h')
        */
        cvector_init_with(&v,sizeof(std::vector<int>),NULL,
                            &cpp_ctr_tu< std::vector<int> >,
                            &cpp_dtr_tu< std::vector<int> >,
                            &cpp_cpy_tu< std::vector<int> >,
                            NULL,NULL);

        stdi.clear();stdi.push_back(3);stdi.push_back(2);stdi.push_back(1);
        cvector_push_back(&v,&stdi);
        stdi.clear();stdi.push_back(4);stdi.push_back(5);
        cvector_push_back(&v,&stdi);
        stdi.clear();stdi.push_back(6);
        cvector_push_back(&v,&stdi);

        for (j=0;j<v.size;j++)  {
            const std::vector<int>& si = v.at< std::vector<int> >(j);
            printf("v[%lu] = {%lu:\t[",j,si.size());
            for (i=0;i<si.size();i++) {
                if (i>0) printf(",");
                printf("%i",si[i]);
            }
            printf("]};\n");
        }
    }
}
#endif /* __cpusplus */
#endif /* NO_CPP_TEST */



int main(int argc,char* argv[])
{
#ifndef NO_SIMPLE_TEST
    SimpleTest();
#endif /* NO_SIMPLE_TEST */
#ifndef NO_STRINGVECTOR_TEST
    StringVectorTest();
#endif /* NO_STRINGVECTOR_TEST */
#ifndef NO_COMPLEXTEST
    ComplexTest();
#endif /* NO_COMPLEXTEST */
#if (!defined(NO_CVH_STRING_T_TEST) && !defined(CV_NO_CVH_STRING_T))
    CvhStringTTest();
#endif

#ifndef NO_CPP_TEST
#ifdef __cplusplus
    CppTest();
#endif /* __cpusplus */
#endif /* NO_CPP_TEST */

    return 1;
}

