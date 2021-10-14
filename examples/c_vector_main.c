/*
// (plain C) compilation:
// gcc
gcc -O2 -no-pie -fno-pie -I"../include" c_vector_main.c -o c_vector_main
// clang
clang -O2 -no-pie -fno-pie -I"../include" c_vector_main.c -o c_vector_main
// mingw
x86_64-w64-mingw32-gcc -mconsole -O2 -I"../include" c_vector_main.c -o c_vector_main.exe
// cl.exe (from Visual C++ 7.1 2003) [last tested with 'c_vector_h' version 1.15 rev2]
cl /O2 /MT /Tc c_vector_main.c /I"../include" /link /out:c_vector_main.exe user32.lib kernel32.lib
// zig (embedded c compiler) (for linux)    Please see: https://andrewkelley.me/post/zig-cc-powerful-drop-in-replacement-gcc-clang.html
zig cc -O2 -target x86_64-linux-gnu -I"../include" c_vector.c -o c_vector_main
// zig (embedded c compiler) (for windows)
zig cc -O2 -target x86_64-windows-gnu -I"../include" c_vector.c -o c_vector_main.exe
*/

/*
// compile as C++ [not really necessary]
// gcc
gcc -O2 -x c++ -no-pie -fno-pie -I"../include" c_vector_main.c -o c_vector_main
// or just
g++ -O2 -no-pie -fno-pie -I"../include" c_vector_main.c -o c_vector_main
// clang and mingw are gcc based (try using clang++ and x86_64-w64-mingw32-g++)

// cl.exe (from Visual C++ 7.1 2003) [last tested with 'c_vector_h' version 1.15 rev2]
cl /O2 /MT /Tp c_vector_main.c /I"../include" /EHsc /link /out:c_vector_main_vc.exe user32.lib kernel32.lib

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
[cv_mystruct_dbg_check]:
    size: 5. capacity: 7. sizeof(mystruct): 12 Bytes.
    sorting: OK.
    memory_used: 364 Bytes. memory_minimal_possible: 340 Bytes. mem_used_percentage: 107.06% (100% is the best possible result).
[cv_mystruct_dbg_check]:
    size: 5. capacity: 5. sizeof(mystruct): 12 Bytes.
    sorting: OK.
    memory_used: 340 Bytes. memory_minimal_possible: 340 Bytes. mem_used_percentage: 100.00% (100% is the best possible result).
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
v[0]={	2.500,	{[3:]	(1,2,3),(2,2,3),(2,3,3)}	}
v[1]={	20.000,	{[2:]	(10,3,3),(20,3,3)}	}
v[2]={	-100.000,	{[4:]	(20,3,-100),(20,333,-100),(333,333,-100),(333,333,333)}	}
After serialization and deserialization:
v[0]={	2.500,	{[3:]	(1,2,3),(2,2,3),(2,3,3)}	}
v[1]={	20.000,	{[2:]	(10,3,3),(20,3,3)}	}
v[2]={	-100.000,	{[4:]	(20,3,-100),(20,333,-100),(333,333,-100),(333,333,333)}	}
Removing element at 0 and then resizing to just one element:
v[0]={	20.000,	{[2:]	(10,3,3),(20,3,3)}	}

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

'std::vector<cv_int>' test:
vstd[0] = {4:	[3,4,5,6]	};
vstd[1] = {5:	[0,1,2,3,4]	};
vstd[2] = {4:	[3,4,5,6]	};
vstd[3] = {5:	[0,1,2,3,4]	};
vstd[4] = {4:	[3,4,5,6]	};
vstd[5] = {5:	[0,1,2,3,4]	};
cv_int_cpy(&v1,&v0):
v0[0] = 3;
v0[1] = 4;
v0[2] = 5;
v0[3] = 6;
v1[0] = 3;
v1[1] = 4;
v1[2] = 5;
v1[3] = 6;
'cv_stdvector_int' test:
v[0] = {3:	[3,2,1]	};
v[1] = {2:	[4,5]	};
v[2] = {1:	[6]	};
*/

/*#define CV_ENABLE_CLEARING_ITEM_MEMORY */    /* just for testing */
/*#define CV_DISABLE_FAKE_MEMBER_FUNCTIONS*/ /* just for testing */

/* The following line is completely optional, and must be used only
   if you need to silence some compiler or static analyzer warning.
   For further info, see: https://en.cppreference.com/w/c/string/byte/memcpy
*/
/*#define __STDC_WANT_LIB_EXT1__ 1*/ /* use memcpy_s, memmove_s and memset_s instead of memcpy, memmove and memset, if bound-checking functions are supported */
#include "c_vector.h"
#include <stdio.h>  /* printf */

/*#define NO_SIMPLE_TEST*/
/*#define NO_STRINGVECTOR_TEST*/
/*#define NO_COMPLEXTEST*/
/*#define NO_CVH_STRING_T_TEST*/
/*#define NO_CPP_TEST*/

#ifndef NO_SIMPLE_TEST
/* The struct we'll use in a vector */
/* 'typedef is mandatory: we need global visibility */
typedef struct mystruct {
int a,b,c;
} mystruct;
/* this will be used for the sorted vector test only */
static int mystruct_cmp(const mystruct* a,const mystruct* b) {
	if (a->a>b->a) return 1;
	else if (a->a<b->a) return -1;
	if (a->b>b->b) return 1;
	else if (a->b<b->b) return -1;
	if (a->c>b->c) return 1;
	else if (a->c<b->c) return -1;
	return 0;
}

#ifndef C_VECTOR_mystruct_H     /* I think it's safer to add these guards */
#define C_VECTOR_mystruct_H
CV_DECLARE_AND_DEFINE(mystruct) /* here we declare and define cv_mystruct (a.k.a. std::vector<mystruct>) */
#endif /* C_VECTOR_mystruct_H */
/* What the lines above do, is to create the type-safe vector structure:
typedef struct {
	mystruct * v;
	const size_t size;
	const size_t capacity;
    [...] // a few function pointers
} cv_mystruct;
together with a lot of type-safe functions starting with 'cv_mystruct_'
*/

static void SimpleTest(void)   {
    cv_mystruct v;                  /* a.k.a. std::vector<mystruct> */
    /* we skip initialization here, because we'll use 'cv_mystruct_init(...)' later */

    mystruct tmp = {-10,200,-5};	/* tmp item used later */
    size_t i,position;int match;
    cvh_serializer_t serializer = cvh_serializer_create();    /* we'll use 'serializer' at the end. NEVER leave cv_xxx of cvh_xxx structs unitialized. Use create(...) or init(...) to initialize them */

    printf("VECTOR TEST:\n");

    /* Note that we could just have initialized 'v' this way: 'cv_mystruct v = cv_mystruct_create(NULL);' too.
       Please never use cv_xxx and cvh_xxx structs unitialized */
    cv_mystruct_init(&v,NULL);
    /* After initialization, we can use the 'fake member function call syntax'. We can make fake member calls like:
       v.reserve(&v,4); // note that 'v' appears twice
       However in this demo we don't use this syntax (that can be removed by defining
       CV_DISABLE_FAKE_MEMBER_FUNCTIONS globally (= in the Project Options))
    */

    /* optional */
    cv_mystruct_reserve(&v,4);

    /* we insert 4 items */
    tmp.a=tmp.b=tmp.c=333;	cv_mystruct_push_back(&v,&tmp);
    tmp.a=100;			cv_mystruct_push_back(&v,&tmp);
    tmp.a=-10;			cv_mystruct_push_back(&v,&tmp);
    tmp.b=50;			cv_mystruct_push_back(&v,&tmp);

    /* Note that if C99 is available we can use 'compound literal'
    to specify structs on the fly in function arguments (i.e. we don't need 'tmp'): */
/*#	if __STDC_VERSION__>= 199901L
    cv_mystruct_push_back(&v,&(mystruct){10,20,30});
#	endif*/

    /* display all items */
    for (i=0;i<v.size;i++) printf("v[%lu]={\t%d,\t%d,\t%d};\n",i,v.v[i].a,v.v[i].b,v.v[i].c);

    /* remove element 1 */
    if (cv_mystruct_remove_at(&v,1))	printf("Removed item 1.\n");

    /* display all items */
    for (i=0;i<v.size;i++) printf("v[%lu]={\t%d,\t%d,\t%d};\n",i,v.v[i].a,v.v[i].b,v.v[i].c);

    /* cv_mystruct_clear(&v);	*/ /* this will clear the vector WITHOUT deallocating its capacity (the memory is still allocated and MUST be freed) */

    cv_mystruct_free(&v);	/* this clears + frees memory (note that it's ready to be reused (but then you must free it again at the end)) */
    cv_mystruct_free(&v);	/* calling it more than once is useless, but safe */

    printf("\nSORTED VECTOR TEST:\n");

    /* 'cv_mystruct_init(...)' or 'cv_mystruct_init_with(...)' can be called only after a variable declaration, or after 'cv_mystruct_free(...)' */
    /* we need it just because we must change 'v.item_cmp', which is const to prevent users from changing it on the fly and break sorting */
    cv_mystruct_init(&v,&mystruct_cmp);    /* initialization with an 'item_compare fuction' allows us to call: 'cv_mystruct_insert_sorted(...)', to keep the vector sorted (if we need it). */

    /* we add 5 items in a sorted way */
    tmp.a=200;cv_mystruct_insert_sorted(&v,&tmp,NULL,1);
    tmp.a=100;cv_mystruct_insert_sorted(&v,&tmp,NULL,1);
    tmp.b=10;cv_mystruct_insert_sorted(&v,&tmp,NULL,1);
    tmp.a=50;cv_mystruct_insert_sorted(&v,&tmp,NULL,1);
    tmp.a=500;cv_mystruct_insert_sorted(&v,&tmp,NULL,1);
    /* last 2 arguments are (int* match=NULL,int insert_even_if_item_match=1): we set the latter to 1 (true) */
    /* Also it can be useful to know that 'cv_xxx_insert_sorted' returns a size_t insertion 'position'. */

    /* display all items */
    for (i=0;i<v.size;i++) printf("v[%lu]={\t%d,\t%d,\t%d};\n",i,v.v[i].a,v.v[i].b,v.v[i].c);

    /* (optional) debug and optimize memory usage */
    cv_mystruct_dbg_check(&v);  /* displays dbg info */
    cv_mystruct_shrink_to_fit(&v);  /* frees all unused memory (slow) */
    CV_ASSERT(v.size==v.capacity);
    cv_mystruct_dbg_check(&v);  /* displays dbg info again */

    /* (test) we re-add an already inserted item (right after 'cv_mystruct_shrink_to_fit(...)' so that a reallocation will happen invalidating the item pointer) */
    if (v.size>0) {
        const mystruct* pitem = &v.v[0];
        printf("re-inserting v[0]={\t%d,\t%d,\t%d};\n",v.v[0].a,v.v[0].b,v.v[0].c);
        CV_ASSERT(v.v && pitem>=v.v && pitem<(v.v+v.size)); /* if a realloc happens, 'pitem' will be invalidated before we can copy it back into the vector, unless the code is robust enough to detect and handle this border case */
        cv_mystruct_insert_sorted(&v,pitem,NULL,1); /* of course we can just use '&v.v[0]' instead of 'pitem' here */
    }

    /* display all items */
    for (i=0;i<v.size;i++) printf("v[%lu]={\t%d,\t%d,\t%d};\n",i,v.v[i].a,v.v[i].b,v.v[i].c);

    /* now we can search the sorted vector for certain items this way */
    tmp.a=100;tmp.b=50;tmp.c=333;
    position = cv_mystruct_binary_search(&v,&tmp,&match);
    printf("item={\t%d,\t%d,\t%d}\t",tmp.a,tmp.b,tmp.c);
    if (match) printf("found at v[%lu].\n",position);
    else printf("not found.\n");

    tmp.a=100;tmp.b=10;tmp.c=100;
    position = cv_mystruct_binary_search(&v,&tmp,&match);
    printf("item={\t%d,\t%d,\t%d}\t",tmp.a,tmp.b,tmp.c);
    if (match) printf("found at v[%lu].\n",position);
    else printf("not found.\n");

    /* note that to remove an item, once we have 'position'
    [returned by 'xxx_insert_sorted(...)' or by 'xxx_binary_search(...)']
    we can use 'xxx_remove_at(...)' like above */

    printf("After serialization and deserialization:\n");
    cv_mystruct_serialize(&v,&serializer);  /* serializes 'v' into 'serializer' */

/*#   define SERIALIZE_TO_FILE*/    /* just to show that's possible to save/load 'serializer' to/from file */
#   ifdef SERIALIZE_TO_FILE    
    cvh_serializer_save(&serializer,"cv_mystruct.bin");
    cvh_serializer_free(&serializer);    /* (optional) now it's empty */
    cvh_serializer_load(&serializer,"cv_mystruct.bin");     /* now it's full */
#   endif

    cv_mystruct_free(&v);   /* (optional) now 'v' it's empty */
    cv_mystruct_deserialize(&v,&serializer);    /* deserializes 'serializer' into 'v' */
    cvh_serializer_free(&serializer); /* mandatory */

    /* display all items */
    for (i=0;i<v.size;i++) printf("v[%lu]={\t%d,\t%d,\t%d};\n",i,v.v[i].a,v.v[i].b,v.v[i].c);

    cv_mystruct_free(&v);	/* this is clear + free memory (note that it's ready to be reused (but then you must free it again at the end)) */
                            /* also note that if you want to reuse it in an unsorted way, you must explicitely call: cv_mystruct_init(&v,NULL); after cv_mystruct_free(&v); */
                            /* IMPORTANT: please never call: v=cv_mystruct_create(...); or  cv_mystruct_init(&v,...); except: a) Soon after 'v' declaration. b) After cv_mystruct_free(&v); */

}

#endif /* NO_SIMPLE_TEST */

#ifndef NO_STRINGVECTOR_TEST
typedef char* string;   /* IMPORTANT: sometimes we MUST use a typedef (when using a pointer or a const qualifier for example) */
static int string_cmp(const string* a,const string* b) {
    if (*a==NULL) return (*b==NULL) ? 0 : 1;
    else if (*b==NULL) return 1;
    return strcmp(*a,*b);
}
static void string_ctr(string* a)    {*a=NULL;}
static void string_dtr(string* a)    {if (*a) {free(*a);*a=NULL;}}
static void string_cpy(string* a,const string* b)    {
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
/*#define USE_LOW_LEVEL_SERIALIZATION */ /* very good exercise to understand how serialization works */
static void string_serialize(const string* a,cvh_serializer_t* serializer)    {
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
static int string_deserialize(string* a,const cvh_serializer_t* deserializer)    {
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


/* Here we do the same to generate cv_string (== std::vector<string>): */
#ifndef C_VECTOR_string_H
#define C_VECTOR_string_H
CV_DECLARE_AND_DEFINE(string)   /* Note that CV_DECLARE_AND_DEFINE(char*) would NOT work! */
#endif


static void StringVectorTest(void) {
    cv_string s;    /* a.k.a. std::vector<string> */

    size_t i,position;int match=0;
    cvh_serializer_t serializer = cvh_serializer_create();  /* mandatory initialization */

    printf("\nSORTED STRINGVECTOR TEST:\n");

    /* cv_string_init_with(...) is like cv_string_init(...) with additional params */
    cv_string_init_with(&s,&string_cmp,&string_ctr,&string_dtr,&string_cpy,
                          &string_serialize,&string_deserialize /* only necessary if we use cv_string_serialize(...)/cv_string_deserialize(...); otherwise we can set them to NULL */
                          );

    /* we add 5 items in a sorted way */
    /* note that, by using the '_by_val' overloads, we can pass text strings directly (char*, instead of char**).
       Also note that all the casts to '(string)' in the code below are there
       just to silence a compilation warning that appears ONLY when the code is
       compiled as C++ (=> you can safely remove them all in plain C).
    */
    cv_string_insert_sorted_by_val(&s,(string)"good morning",NULL,1);
    cv_string_insert_sorted_by_val(&s,(string)"hello world",NULL,1);
    cv_string_insert_sorted_by_val(&s,(string)"ciao",NULL,1);
    cv_string_insert_sorted_by_val(&s,(string)"hi",NULL,1);
    cv_string_insert_sorted_by_val(&s,(string)"golden day",NULL,1);
    /* last 2 arguments are (int* match=NULL,int insert_even_if_item_match=1): we set the latter to 1 (true) */
    /* Also it can be useful to know that 'cv_xxx_insert_sorted' returns a size_t insertion 'position'. */

    /* somebody might think that this works only for static strings. This is not true. */
    /*{
        char* tmp = strdup("good afternoon");
        cv_string_insert_sorted_by_val(&s,tmp,NULL,1);
        free(tmp);
    }*/

    /* display all items */
    for (i=0;i<s.size;i++) printf("s[%lu]=\"%s\";\n",i,s.v[i]?s.v[i]:"NULL");

    /* now we can serch the sorted vector for certain items this way */
    position = cv_string_binary_search_by_val(&s,(string)"bye bye",&match);
    printf("item=\"bye bye\"\t");
    if (match) printf("found at s[%lu].\n",position);
    else printf("not found.\n");

    position = cv_string_binary_search_by_val(&s,(string)"hello world",&match);
    printf("item=\"hello world\"\t");
    if (match) printf("found at s[%lu].\n",position);
    else printf("not found.\n");

    /* remove last searched item (if found) */
    if (match)  {
        printf("removing item s[%lu]=\"%s\".\n",position,s.v[position]?s.v[position]:"NULL");
        cv_string_remove_at(&s,position);
    }

    /* display all items */
    for (i=0;i<s.size;i++) printf("s[%lu]=\"%s\";\n",i,s.v[i]?s.v[i]:"NULL");

    printf("After serialization and deserialization:\n");
    cv_string_serialize(&s,&serializer);

/*#   define SERIALIZE_TO_FILE*/    /* just to show that's possible */
#   ifdef SERIALIZE_TO_FILE
    cvh_serializer_save(&serializer,"cv_string.bin");
    cvh_serializer_free(&serializer);    /* (optional) now it's empty */
    cvh_serializer_load(&serializer,"cv_string.bin");     /* now it's full */
#   endif

    cv_string_free(&s);   /* (optional) now it's empty */
    cv_string_deserialize(&s,&serializer);    /* now it's full */
    cvh_serializer_free(&serializer); /* mandatory */

    /* display all items */
    for (i=0;i<s.size;i++) printf("s[%lu]=\"%s\";\n",i,s.v[i]?s.v[i]:"NULL");

    cv_string_free(&s); /* mandatory */
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
    cv_mystruct v;  /* this is like a std::vector<mystruct> */
} big_t;
/* to make vectors of 'big_t' (= 'cv_big_t') work,
   we need to specify ctr/dtr/cpy helpers: */
static void big_t_ctr(big_t* a)    {
    a->a=0;
    cv_mystruct_init(&a->v,NULL);  /* this can be thought as the 'cv_mystruct' ctr */
}
static void big_t_dtr(big_t* a)    {
    cv_mystruct_free(&a->v);    /* this can be thought as the 'cv_mystruct' dtr */
}
static void big_t_cpy(big_t* a,const big_t* b)    {
    a->a=b->a;
    cv_mystruct_cpy(&a->v,&b->v);   /* 'cv_mystruct_cpy(...)' has been created together with cv_mystruct in the first test */
}
static void big_t_serialize(const big_t* a,cvh_serializer_t* serializer)    {
#   ifdef USE_LOW_LEVEL_SERIALIZATION
    const size_t size_of_float = sizeof(a->a);
    cvh_serializer_reserve(serializer,serializer->size + size_of_float); /* space reserved for serialization */
    *((float*) (&serializer->v[serializer->size])) = a->a;serializer->size+=size_of_float; /* a->a written */
#   else
    cvh_serializer_write_float(serializer,a->a);
#   endif
    cv_mystruct_serialize(&a->v,serializer);
}
static int big_t_deserialize(big_t* a,const cvh_serializer_t* deserializer)    {
#   ifdef USE_LOW_LEVEL_SERIALIZATION
    const size_t size_of_float = sizeof(a->a);
    int check = deserializer->offset+size_of_float<=deserializer->size;CV_ASSERT(check);if (!check) return 0;
    a->a = *((float*) &deserializer->v[deserializer->offset]);*((size_t*) &deserializer->offset)+=size_of_float;
#   else
    if (!cvh_serializer_read_float(deserializer,&a->a)) return 0;
#   endif
    if (!cv_mystruct_deserialize(&a->v,deserializer)) return 0;
    return 1;
}
/* same macro as above, but now for 'big_t' */
#ifndef C_VECTOR_big_t_H
CV_DECLARE_AND_DEFINE(big_t)
#endif /* C_VECTOR_big_t_H */

static void ComplexTest(void) {
    cv_big_t v; /* a.k.a. std::vector<big_t> */

    big_t tmp;      mystruct ts = {1,2,3};  /* a tmp item for 'tmp.v' */
    size_t i,j;
    cvh_serializer_t serializer = cvh_serializer_create();  /* mandatory initialization (we could have used: cvh_serializer_init(&serializer); instead) */

    printf("\nCOMPLEX VECTOR TEST:\n");

    /* Note that we could have used: 'cv_big_t v = cv_big_t_create_with(NULL,&big_t_ctr,&big_t_dtr,&big_t_cpy,&big_t_serialize,&big_t_deserialize);', instead of 'cv_big_t_init_with(&v,...)' */
    /* First 'NULL' here is 'item_cmp', because we don't need sorted vector features ('xxx_insert_sorted' and 'xxx_binary_search') */
    cv_big_t_init_with(&v,NULL,&big_t_ctr,&big_t_dtr,&big_t_cpy,
                         &big_t_serialize,&big_t_deserialize /* only necessary if we use cv_big_t_serialize(...)/cv_big_t_deserialize(...); otherwise we can set them to NULL */
                         );

    big_t_ctr(&tmp);
    /* element 0 */
    tmp.a=2.5f;
    ts.a=1;cv_mystruct_push_back(&tmp.v,&ts);
    ts.a=2;cv_mystruct_push_back(&tmp.v,&ts);
    ts.b=3;cv_mystruct_push_back(&tmp.v,&ts);
    cv_big_t_push_back(&v,&tmp);
    /* element 1 */
    tmp.a=20.f;cv_mystruct_clear(&tmp.v);
    ts.a=10;cv_mystruct_push_back(&tmp.v,&ts);
    ts.a=20;cv_mystruct_push_back(&tmp.v,&ts);
    cv_big_t_push_back(&v,&tmp);
    /* element 2 */
    tmp.a=-100.f;cv_mystruct_clear(&tmp.v);
    ts.c=-100;cv_mystruct_push_back(&tmp.v,&ts);
    ts.b=333;cv_mystruct_push_back(&tmp.v,&ts);
    ts.a=333;cv_mystruct_push_back(&tmp.v,&ts);
    ts.c=333;cv_mystruct_push_back(&tmp.v,&ts);
    cv_big_t_push_back(&v,&tmp);
    big_t_dtr(&tmp);

    /* display elements */
    for (i=0;i<v.size;i++) {
        const big_t* b = &v.v[i];
        const float ba = b->a;
        const cv_mystruct* bv = &b->v;
        printf("v[%lu]={\t%1.3f,\t{[%lu:]\t",i,ba,bv->size);
        for (j=0;j<bv->size;j++)    {
            if (j>0) printf(",");
            printf("(%d,%d,%d)",bv->v[j].a,bv->v[j].b,bv->v[j].c);
        }
        printf("}\t}\n");
    }

    printf("After serialization and deserialization:\n");
    cv_big_t_serialize(&v,&serializer);
/*#   define SERIALIZE_TO_FILE*/    /* just to show that's possible */
#   ifdef SERIALIZE_TO_FILE
    cvh_serializer_save(&serializer,"cv_big_t.bin");
    cvh_serializer_free(&serializer);    /* (optional) now it's empty */
    cvh_serializer_load(&serializer,"cv_big_t.bin");     /* now it's full */
#   endif
    cv_big_t_free(&v);   /* (optional) now it's empty */
    cv_big_t_deserialize(&v,&serializer);    /* now it's full */
    cvh_serializer_free(&serializer); /* mandatory */
    /* display elements */
    for (i=0;i<v.size;i++) {
        const big_t* b = &v.v[i];
        const float ba = b->a;
        const cv_mystruct* bv = &b->v;
        printf("v[%lu]={\t%1.3f,\t{[%lu:]\t",i,ba,bv->size);
        for (j=0;j<bv->size;j++)    {
            if (j>0) printf(",");
            printf("(%d,%d,%d)",bv->v[j].a,bv->v[j].b,bv->v[j].c);
        }
        printf("}\t}\n");
    }

    printf("Removing element at 0 and then resizing to just one element:\n");
    /* remove element 0 */
    cv_big_t_remove_at(&v,0);
    /* resize to just one element */
    cv_big_t_resize(&v,1);

    /* display elements again */
    for (i=0;i<v.size;i++) {
        const big_t* b = &v.v[i];
        const float ba = b->a;
        const cv_mystruct* bv = &b->v;
        printf("v[%lu]={\t%1.3f,\t{[%lu:]\t",i,ba,bv->size);
        for (j=0;j<bv->size;j++)    {
            if (j>0) printf(",");
            printf("(%d,%d,%d)",bv->v[j].a,bv->v[j].b,bv->v[j].c);
        }
        printf("}\t}\n");
    }

    cv_big_t_free(&v);
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
CV_DECLARE_AND_DEFINE(city_t)
typedef struct country_t {size_t name;cv_city_t cities;cvh_string_t city_names;} country_t; /* this can't be copied with memcpy. Additional functions needed) */
void country_ctr(country_t* p)  {    
    p->name=0;
    cv_city_t_init(&p->cities,NULL);
    cvh_string_init(&p->city_names);
}
void country_dtr(country_t* p)  {
    cv_city_t_free(&p->cities);
    cvh_string_free(&p->city_names);
}
void country_cpy(country_t* a,const country_t* b)  {
    a->name = b->name;
    cv_city_t_cpy(&a->cities,&b->cities);
    cvh_string_cpy(&a->city_names,&b->city_names);
}
void country_serialize(const country_t* p,cvh_serializer_t* s)  {
    cvh_serializer_write_size_t(s,p->name);
    cv_city_t_serialize(&p->cities,s);
    cvh_string_serialize(&p->city_names,s);
}
int country_deserialize(country_t* p,const cvh_serializer_t* d)  {
    if (!cvh_serializer_read_size_t(d,&p->name)) return 0;
    if (!cv_city_t_deserialize(&p->cities,d)) return 0;
    if (!cvh_string_deserialize(&p->city_names,d)) return 0;
    return 1;
}
CV_DECLARE_AND_DEFINE(country_t)


void CvhStringTTest(void)   {
    cv_country_t v = cv_country_t_create_with(NULL,&country_ctr,&country_dtr,&country_cpy,&country_serialize,&country_deserialize); /* initialization is mandatory! */
    cvh_string_t country_names = cvh_string_create();  /* initialization is mandatory! */
    cvh_serializer_t serializer = cvh_serializer_create();
    size_t i,j;

    printf("\nCVH_STRING_T TEST:\n");

    /* add countries */
    {
        country_t country; /* unitialized. Never use anything unitialized */
        /*--- Germany----*/
        country_ctr(&country);  /* initialized */
        country.name = cvh_string_push_back(&country_names,"Germany",NULL);
        /* add cities to 'c' (the population field is just a random positiove here) */
        {city_t c = {cvh_string_push_back(&country.city_names,"Berlin",NULL),100000};cv_city_t_push_back(&country.cities,&c);}
        {city_t c = {cvh_string_push_back(&country.city_names,"Munchen",NULL),50000};cv_city_t_push_back(&country.cities,&c);}
        {city_t c = {cvh_string_push_back(&country.city_names,"Hamburg",NULL),20000};cv_city_t_push_back(&country.cities,&c);}
        {city_t c = {cvh_string_push_back(&country.city_names,"Bonn",NULL),10000};cv_city_t_push_back(&country.cities,&c);}
        {city_t c = {cvh_string_push_back(&country.city_names,"Frankfurt",NULL),40000};cv_city_t_push_back(&country.cities,&c);}
        cv_country_t_push_back(&v,&country);
        country_dtr(&country);  /* free */
        /*---- France -----*/
        country_ctr(&country);  /* initialized */
        country.name = cvh_string_push_back(&country_names,"France",NULL);
        /* add cities to 'c' (the population field is just a random positiove here) */
        {city_t c = {cvh_string_push_back(&country.city_names,"Paris",NULL),200000};cv_city_t_push_back(&country.cities,&c);}
        {city_t c = {cvh_string_push_back(&country.city_names,"Marseille",NULL),100000};cv_city_t_push_back(&country.cities,&c);}
        {city_t c = {cvh_string_push_back(&country.city_names,"Lyon",NULL),25000};cv_city_t_push_back(&country.cities,&c);}
        cv_country_t_push_back(&v,&country);
        country_dtr(&country);  /* free */
        /*---- Italy -----*/
        country_ctr(&country);  /* initialized */
        country.name = cvh_string_push_back(&country_names,"Italy",NULL);
        /* add cities to 'c' (the population field is just a random positiove here) */
        {city_t c = {cvh_string_push_back(&country.city_names,"Rome",NULL),50000};cv_city_t_push_back(&country.cities,&c);}
        {city_t c = {cvh_string_push_back(&country.city_names,"Milan",NULL),20000};cv_city_t_push_back(&country.cities,&c);}
        {city_t c = {cvh_string_push_back(&country.city_names,"Florence",NULL),15000};cv_city_t_push_back(&country.cities,&c);}
        {city_t c = {cvh_string_push_back(&country.city_names,"Venice",NULL),25000};cv_city_t_push_back(&country.cities,&c);}
        cv_country_t_push_back(&v,&country);
        country_dtr(&country);  /* free */
        /*-------------------------*/
    }

    /* display */
    printf("v content:\n");
    for (i=0;i<v.size;i++)  {
        const country_t* country = &v.v[i];
        printf("%lu) \"%s\":\n",i,&country_names.v[country->name]);
        for (j=0;j<country->cities.size;j++)  {
            const city_t* c = &country->cities.v[j];
            printf("\t%lu) \"%s\"\t\tpopulation: %u\n",j,&country->city_names.v[c->name],c->population);
        }
    }

    printf("After serialization and deserialization:\n");
    /* serialize */
    cv_country_t_serialize(&v,&serializer);
    cvh_string_serialize(&country_names,&serializer);
/*#   define SERIALIZE_TO_FILE*/    /* just to show that's possible */
#   ifdef SERIALIZE_TO_FILE
    cvh_serializer_save(&serializer,"cv_country_t.bin");
    cvh_serializer_free(&serializer);    /* (optional) now it's empty */
    cvh_serializer_load(&serializer,"cv_country_t.bin");     /* now it's full */
#   endif
    cv_country_t_free(&v);              /* (optional) now it's empty */
    cvh_string_free(&country_names);    /* (optional) now it's empty */
    /* deserialize */
    cv_country_t_deserialize(&v,&serializer);               /* now it's full */
    cvh_string_deserialize(&country_names,&serializer);    /* now it's full */
    /* display */
    printf("v content:\n");
    for (i=0;i<v.size;i++)  {
        const country_t* country = &v.v[i];
        printf("%lu) \"%s\":\n",i,&country_names.v[country->name]);
        for (j=0;j<country->cities.size;j++)  {
            const city_t* c = &country->cities.v[j];
            printf("\t%lu) \"%s\"\t\tpopulation: %u\n",j,&country->city_names.v[c->name],c->population);
        }
    }

    cvh_serializer_free(&serializer);
    cvh_string_free(&country_names);
    cv_country_t_free(&v);
}
#endif /* (defined(NO_CVH_STRING_T_TEST) && !defined(CV_NO_CVH_STRING_T)) */


#ifndef NO_CPP_TEST
#ifdef __cplusplus

#ifndef C_VECTOR_int_H
CV_DECLARE_AND_DEFINE(int) /* for 'cv_int' (the cvector.h alternative to std::vector<int>) */
#endif /* C_VECTOR_int_H */

#   include <vector> /* std::vector */

typedef std::vector<int> stdvector_int;
#ifndef C_VECTOR_stdvector_int_H
CV_DECLARE_AND_DEFINE(stdvector_int)    /* for 'cv_stdvector_int' */
#endif /* C_VECTOR_stdvector_int_H */


void CppTest(void)    {
    /* This makes only sense when porting existing C++ code to plain C. Some tips:
       -> try to replace a single 'std::vector' with a 'cv_xxx', keep the code compilable (in C++) and repeat the process
       -> a std::vector<cv_xxx> is easier to setup than a cv_stdvector_type, so start with inner/nested std::vectors
       -> a cv_xxx that includes (directly or indirectly) a std::vector (or any other STL container) is a bit harder to make it work, so it's better to avoid it if possible
       -> 'cv_xxx_init(...)' or 'cv_xxx_create(...)' in C++ can be omitted (if we don't need 'item_ctr', 'item_dtr', 'item_cpy', 'item_cmp', etc.)
       -> in C++ we can use 'v[i]' (i.e. operator[]) (but it does not work in plain C, so we'll need to convert it later)
       -> cv_xxx::~cv_xxx() calls 'cv_xxx_free(...)' for us (but in plain C it's not available. It can be useful to remember that it's harmless to call 'cv_xxx_free(...)' multiple times)
       -> some programmers prefer using the 'fake member function' syntax when porting code from std::vector
    */

    size_t i,j;
    const int tmp[]={0,1,2,3,4,5,6,7,8,9};
    cv_int v0,v1;
    std::vector<cv_int> vstd;

    printf("\nCPP MODE TEST:\n");

    /* Note that in plain C, v0 and v1 are in a bad state (no initialization and no 'cv_int_init(...)' or 'cv_int_create(...)' call); but in C++ it works */

    for (i=0;i<5;i++) cv_int_push_back(&v0,&tmp[i]); /* or v0.push_back(&v0,&tmp[i]) in the 'fake member function' syntax */
    cv_int_insert_range_at(&v1,&tmp[3],4,0);            /* or v1.insert_range_at(&v1,&tmp[3],4,0) in the 'fake member function' syntax */

    for (i=0;i<v0.size;i++) printf("v0[%lu] = %d;\n",i,v0[i]);  /* Warning: 'operator[]' is C++ specific */
    for (i=0;i<v1.size;i++) printf("v1[%lu] = %d;\n",i,v1[i]);

    printf("cv_int_swap(&v1,&v0):\n");
    cv_int_swap(&v1,&v0);

    for (i=0;i<v0.size;i++) printf("v0[%lu] = %d;\n",i,v0[i]);
    for (i=0;i<v1.size;i++) printf("v1[%lu] = %d;\n",i,v1[i]);

    printf("'std::vector<cv_int>' test:\n");
    vstd.push_back(v0);
    vstd.push_back(v1);
    vstd.push_back(v0);
    vstd.push_back(v1);
    vstd.push_back(v0);
    vstd.push_back(v1);

    for (j=0;j<vstd.size();j++) {
        const cv_int& v = vstd[j];
        printf("vstd[%lu] = {%lu:\t[",j,v.size);
        for (i=0;i<v.size;i++) {
            if (i>0) printf(",");
            printf("%i",v[i]);
        }
        printf("]\t};\n");
    }

    printf("cv_int_cpy(&v1,&v0):\n");
    cv_int_cpy(&v1,&v0);

    for (i=0;i<v0.size;i++) printf("v0[%lu] = %d;\n",i,v0[i]);
    for (i=0;i<v1.size;i++) printf("v1[%lu] = %d;\n",i,v1[i]);

    /* in C++ mode ONLY, cv_int_free(...) is called for us at cv_int::~cv_int() */

    {
        std::vector<int> stdi;
        cv_stdvector_int v;

        printf("'cv_stdvector_int' test:\n");

        /* 'cpp_ctr','cpp_dtr' and 'cpp_cpy' are provided by 'c_vector.h' (in C++ mode only). */
        cv_stdvector_int_init_with(&v,NULL,
                            &cpp_ctr,
                            &cpp_dtr,
                            &cpp_cpy,
                            NULL,NULL);

        stdi.clear();stdi.push_back(3);stdi.push_back(2);stdi.push_back(1);
        cv_stdvector_int_push_back(&v,&stdi);
        stdi.clear();stdi.push_back(4);stdi.push_back(5);
        cv_stdvector_int_push_back(&v,&stdi);
        stdi.clear();stdi.push_back(6);
        cv_stdvector_int_push_back(&v,&stdi);

        for (j=0;j<v.size;j++)  {
            const std::vector<int>& si = v[j];
            printf("v[%lu] = {%lu:\t[",j,si.size());
            for (i=0;i<si.size();i++) {
                if (i>0) printf(",");
                printf("%i",si[i]);
            }
            printf("]\t};\n");
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

