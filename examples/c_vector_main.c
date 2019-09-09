/*
// (plain C) compilation:
// gcc
gcc -O2 -no-pie -fno-pie -I"../include" c_vector_main.c -o c_vector_main
// clang
clang -O2 -no-pie -fno-pie -I"../include" c_vector_main.c -o c_vector_main
// mingw
x86_64-w64-mingw32-gcc -mconsole -O2 -I"../include" c_vector_main.c -o c_vector_main.exe
// cl.exe (from Visual C++ 7.1 2003)
cl /O2 /MT /Tc c_vector_main.c /I"../include" /link /out:c_vector_main.exe user32.lib kernel32.lib
*/

/*
// compile as C++ [not really necessary]
// gcc
gcc -O2 -x c++ -no-pie -fno-pie -I"../include" c_vector_main.c -o c_vector_main
// or just
g++ -O2 -no-pie -fno-pie -I"../include" c_vector_main.c -o c_vector_main
// clang and mingw are gcc based (try using clang++ and x86_64-w64-mingw32-g++)

// cl.exe (from Visual C++ 7.1 2003)
cl /O2 /MT /Tp c_vector_main.c /I"../include" /EHsc /link /out:c_vector_main_vc.exe user32.lib kernel32.lib
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
    memory_used: 332 Bytes. memory_minimal_possible: 308 Bytes. mem_used_percentage: 107.79% (100% is the best possible result).
[cv_mystruct_dbg_check]:
    size: 5. capacity: 5. sizeof(mystruct): 12 Bytes.
    sorting: OK.
    memory_used: 308 Bytes. memory_minimal_possible: 308 Bytes. mem_used_percentage: 100.00% (100% is the best possible result).
re-inserting v[0]={	50,	10,	333};
v[0]={	50,	10,	333};
v[1]={	50,	10,	333};
v[2]={	100,	10,	333};
v[3]={	100,	50,	333};
v[4]={	200,	50,	333};
v[5]={	500,	10,	333};
item={	100,	50,	333}	found at v[3].
item={	100,	10,	100}	not found.

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

COMPLEX VECTOR TEST:
v[0]={	2.500,	{[3:]	(1,2,3),(2,2,3),(2,3,3)}}
v[1]={	20.000,	{[2:]	(10,3,3),(20,3,3)}}
v[2]={	-100.000,	{[4:]	(20,3,-100),(20,333,-100),(333,333,-100),(333,333,333)}}
v[0]={	20.000,	{[2:]	(10,3,3),(20,3,3)}}
*/
#include <stdio.h>  /* printf */

/*#define NO_SIMPLE_TEST*/
/*#define NO_STRINGVECTOR_TEST*/
/*#define NO_COMPLEXTEST*/
/*#define NO_CPP_TEST*/

#ifndef NO_SIMPLE_TEST
/* The struct we'll use in a vector */
/* 'typedef is mandatory: we need global visibility */
typedef struct {
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
#   define CV_TYPE mystruct
#   include "c_vector.h"
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
    /* we skip C-style initialization, because we use 'cv_mystruct_create(...)' */

    mystruct tmp = {-10,200,-5};	/* tmp item used later */
    size_t i,position;int match;

    printf("VECTOR TEST:\n");

    /* Note that we could just have initialized 'v' this way: 'cv_mystruct v={0};' but
       by using 'cv_mystruct_create(...)' we enable the 'fake member function call syntax' */
    cv_mystruct_create(&v,NULL);
    /* With the 'fake member function call syntax', we can make fake member calls like:
       v.reserve(&v,4); // note that 'v' appears twice
       However in this demo we don't use this syntax (that can be removed by defining
       CV_DISABLE_FAKE_MEMBER_FUNCTIONS globally (= in the Project Options)
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

    /* 'cv_mystruct_create(...)' or 'cv_mystruct_create_with(...)' can be called only to replace the initializer list, or after 'cv_mystruct_free(...)' */
    /* we need it just because we must change 'v.item_cmp', which is const to prevent users from changing it on the fly and break sorting */
    cv_mystruct_create(&v,&mystruct_cmp);    /* we can set 'mystruct_cmp' in its initialization list, or in 'cv_mystruct_create(...)' too */

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
        CV_ASSERT(v.v && pitem>=v.v && pitem<(v.v+v.size)); /* if a realloc happens, 'pitem' will be invalidated before we can copy it back in the vector, unless the code is robust enough to detect this border case */
        cv_mystruct_insert_sorted(&v,pitem,NULL,1); /* of course we can just use '&v.v[itemIdx]' instead of 'pitem' here */
    }

    /* display all items */
    for (i=0;i<v.size;i++) printf("v[%lu]={\t%d,\t%d,\t%d};\n",i,v.v[i].a,v.v[i].b,v.v[i].c);

    /* now we can serch the sorted vector for certain items this way */
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

    cv_mystruct_free(&v);	/* this is clear + free memory (note that it's ready to be reused (but then you must free it again at the end)) */
}

#endif /* NO_SIMPLE_TEST */

#ifndef NO_STRINGVECTOR_TEST
typedef char* string;
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
        memcpy(*a,*b,blen+1);
    }
}

/* Here we do the same to generate cv_string (== std::vector<string>): */
#ifndef C_VECTOR_string_H
#define C_VECTOR_string_H
#   define CV_TYPE string
#   include "c_vector.h"
#endif


static void StringVectorTest(void) {
    cv_string s;    /* a.k.a. std::vector<string> */

    size_t i,position;int match=0;

    printf("\nSORTED STRINGVECTOR TEST:\n");

    /* cv_string_create_with(...) is like cv_string_create(...) with additional params */
    /* in our case we can simply replace '&string_ctr' with 'NULL', because new allocated memory is always cleared (to increase code robustness) */
    cv_string_create_with(&s,&string_cmp,&string_ctr,&string_dtr,&string_cpy);

    /* we add 5 items in a sorted way */
    /* note that, by using the '_by_val' overloads, we can pass text strings directly.
       Also note that all the casts to '(const string)' in the code below are there
       just to silence a compilation warning that appears ONLY when the code is
       compiled as C++ (=> you can safely remove them all in plain C).
    */
    cv_string_insert_sorted_by_val(&s,(const string)"good morning",NULL,1);
    cv_string_insert_sorted_by_val(&s,(const string)"hello world",NULL,1);
    cv_string_insert_sorted_by_val(&s,(const string)"ciao",NULL,1);
    cv_string_insert_sorted_by_val(&s,(const string)"hi",NULL,1);
    cv_string_insert_sorted_by_val(&s,(const string)"golden day",NULL,1);
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
    position = cv_string_binary_search_by_val(&s,(const string)"bye bye",&match);
    printf("item=\"bye bye\"\t");
    if (match) printf("found at s[%lu].\n",position);
    else printf("not found.\n");

    position = cv_string_binary_search_by_val(&s,(const string)"hello world",&match);
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

    cv_string_free(&s);
}

#endif /* NO_STRINGVECTOR_TEST */

#ifndef NO_COMPLEXTEST
#ifdef NO_SIMPLE_TEST
#   error Please undefine NO_SIMPLE_TEST
#endif
/* Now something much more difficult.
   Tip for newbies: skip this part. */
typedef struct {
    float a;
    cv_mystruct v;  /* this is like a std::vector<mystruct> */
} big_t;
/* to make vectors of 'big_t' (= 'cv_big_t') work,
   we need to specify ctr/dtr/cpy helpers: */
static void big_t_ctr(big_t* a)    {
    a->a=0;
    cv_mystruct_create(&a->v,NULL);  /* this can be thought as the 'cv_mystruct' ctr */
}
static void big_t_dtr(big_t* a)    {
    cv_mystruct_free(&a->v);    /* this can be thought as the 'cv_mystruct' dtr */
}
static void big_t_cpy(big_t* a,const big_t* b)    {
    a->a=b->a;
    cv_mystruct_cpy(&a->v,&b->v);   /* 'cv_mystruct_cpy(...)' has been created by CV_DECLARE(mystruct) */
}
/* same macro as above, but now for 'big_t' */
#ifndef C_VECTOR_big_t_H
#   define CV_TYPE big_t
#   include "c_vector.h"
#endif /* C_VECTOR_big_t_H */

static void ComplexTest(void) {
    cv_big_t v; /* a.k.a. std::vector<big_t> */

    big_t tmp;
    mystruct ts = {1,2,3};  /* a tmp item for 'tmp.v' */
    size_t i,j;

    printf("\nCOMPLEX VECTOR TEST:\n");

    cv_big_t_create_with(&v,NULL,&big_t_ctr,&big_t_dtr,&big_t_cpy);

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
        printf("}}\n");
    }

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
        printf("}}\n");
    }

    cv_big_t_free(&v);
}

#endif /* NO_COMPLEXTEST */



#ifndef NO_CPP_TEST
#ifdef __cplusplus

#ifndef C_VECTOR_int_H
#   define CV_TYPE int
#   include "c_vector.h"
#endif /* C_VECTOR_int_H */

#   include <vector> /* std::vector */

void CppTest(void)    {
    /* This makes only sense when porting existing C++ code to plain C. Some tips:
       -> try to replace a single 'std::vector' with a 'cv_xxx', keep the code compilable (in C++) and repeat the process
       -> std::vector<cv_xxx> works
       -> a cv_xxx that includes (directly or indirectly) a std::vector (or any other STL container) does NOT work
       -> 'cv_xxx_create(...)' in C++ can be omitted (if we don't need 'item_ctr', 'item_dtr', 'item_cpy' or 'item_cmp')
       -> in C++ we can use 'v[i]' (i.e. operator[]) (but it does not work in plain C, so we'll need to convert it later)
       -> cv_xxx::~cv_xxx() calls 'cv_xxx_free(...)' for us (but in plain C it's not available. It can be useful to remember that it's harmless to call 'cv_xxx_free(...)' multiple times)
       -> some programmers prefer using the 'fake member function' syntax when porting code from std::vector
    */

    size_t i,j;
    const int tmp[]={0,1,2,3,4,5,6,7,8,9};
    cv_int v0,v1;
    std::vector<cv_int> vstd;

    printf("\nCPP MODE TEST:\n");

    /* Note that in plain C, v0 and v1 are in a bad state (no initialization and no 'cv_int_create(...)' call); but in C++ it works */

    for (i=0;i<5;i++) cv_int_push_back(&v0,&tmp[i]); /* or v0.push_back(&v0,&tmp[i]) in the 'fake member function' syntax */
    cv_int_insert_range_at(&v1,&tmp[3],4,0);            /* or v1.insert_range_at(&v1,&tmp[3],4,0) in the 'fake member function' syntax */

    for (i=0;i<v0.size;i++) printf("v0[%lu] = %d;\n",i,v0[i]);  /* Warning: 'operator[]' is C++ specific */
    for (i=0;i<v1.size;i++) printf("v1[%lu] = %d;\n",i,v1[i]);

    printf("cv_int_swap(&v1,&v0):\n");
    cv_int_swap(&v1,&v0);

    for (i=0;i<v0.size;i++) printf("v0[%lu] = %d;\n",i,v0[i]);
    for (i=0;i<v1.size;i++) printf("v1[%lu] = %d;\n",i,v1[i]);

    printf("std::vector<cv_int> test:\n");
    vstd.push_back(v0);
    vstd.push_back(v1);
    vstd.push_back(v0);
    vstd.push_back(v1);
    vstd.push_back(v0);
    vstd.push_back(v1);

    for (j=0;j<vstd.size();j++) {
        const cv_int& v = vstd[j];
        printf("vstd[%lu]:\n",j);
        for (i=0;i<v.size;i++) printf("\tv[%lu] = %d;\n",i,v[i]);
    }

    printf("cv_int_cpy(&v1,&v0):\n");
    cv_int_cpy(&v1,&v0);

    for (i=0;i<v0.size;i++) printf("v0[%lu] = %d;\n",i,v0[i]);
    for (i=0;i<v1.size;i++) printf("v1[%lu] = %d;\n",i,v1[i]);

    /* cv_int_free(...) is called for us at cv_int::~cv_int() */
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

#ifndef NO_CPP_TEST
#ifdef __cplusplus
    CppTest();
#endif /* __cpusplus */
#endif /* NO_CPP_TEST */

    return 1;
}

