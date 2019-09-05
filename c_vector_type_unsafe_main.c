/*
// (plain C) compilation:
// gcc
gcc -O2 -no-pie -fno-pie c_vector_type_unsafe_main.c -o c_vector_type_unsafe_main
// clang
clang -O2 -no-pie -fno-pie c_vector_type_unsafe_main.c -o c_vector_type_unsafe_main
// mingw
x86_64-w64-mingw32-gcc -mconsole -O2 c_vector_type_unsafe_main.c -o c_vector_type_unsafe_main.exe
// cl.exe (from Visual C++ 7.1 2003)
cl /O2 /MT /Tc c_vector_type_unsafe_main.c /link /out:c_vector_type_unsafe_main.exe user32.lib kernel32.lib
*/

/*
// compile as C++ [not really necessary (we could just scope code with 'extern "C"', like in <c_vector.h>)]
// gcc
gcc -O2 -x c++ -no-pie -fno-pie c_vector_type_unsafe_main.c -o c_vector_type_unsafe_main
// or just
g++ -O2 -no-pie -fno-pie c_vector_type_unsafe_main.c -o c_vector_type_unsafe_main
*/

/* Program Output:

*/
#include "c_vector_type_unsafe.h"
#include <stdio.h>  /* printf */

/*#define NO_SIMPLE_TEST*/
/*#define NO_STRINGVECTOR_TEST*/
/*#define NO_COMPLEXTEST*/


#ifndef NO_SIMPLE_TEST
/* The struct we'll use in a vector */
/* 'typedef is mandatory: we need global visibility */
typedef struct {
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
    /* we skip C-style initialization because we'll use 'cvector_create(...)' later */

	const mystruct* p = NULL;	/* we'll use this to list items in a type-safe way */

    mystruct tmp = {-10,200,-5};	/* tmp item used later */
    size_t i,position;int match;

    printf("VECTOR TEST:\n");

    /* Note that we could just have initialized 'v' this way: 'cvector v={...};' but
       by using 'cvector_create(...)' we enable the 'fake member function call syntax' */
    cvector_create(&v,sizeof(mystruct),NULL);
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

    /* 'cvector_create(...)' can be called only to replace the initializer list, or after 'cvector_free(...)' */
    /* we need it just because we must change 'v.item_cmp', which is const to prevent users from changing it on the fly and break sorting */
    cvector_create(&v,sizeof(mystruct),&mystruct_cmp);    /* we can set 'mystruct_cmp' in its initialization list, or in 'cvector_create(...)' too */

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
        cvector_insert_sorted(&v,pitem,NULL,1); /* of course we can just use '&v.v[itemIdx]' instead of 'pitem' here */
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

    cvector_free(&v);	/* this is clear + free memory (note that it's ready to be reused (but then you must free it again at the end)) */
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
        memcpy(*a,*b,blen+1);
    }
}



static void StringVectorTest(void) {
    cvector s;    /* a.k.a. std::vector<string> */

    size_t i,position;int match=0;
    const string* p = NULL;
    const char* tmp[6]={"good morning","hello world","ciao","hi","golden day","bye bye"};

    printf("\nSORTED STRINGVECTOR TEST:\n");

    /* in our case we can simply replace '&string_ctr' with 'NULL', because new allocated memory is always cleared (to increase code robustness) */
    cvector_create_with(&s,sizeof(string),&string_cmp,&string_ctr,&string_dtr,&string_cpy);

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

    cvector_free(&s);
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
    cvector v;  /* this is like a std::vector<mystruct> */
} big_t;
/* to make vectors of 'big_t' work,
   we need to specify ctr/dtr/cpy helpers: */
static void big_t_ctr(void* av)    {
    big_t* a=(big_t*)av;
    a->a=0;
    cvector_create(&a->v,sizeof(mystruct),NULL);  /* this can be thought as the 'cv' ctr */
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


static void ComplexTest(void) {
    cvector v; /* a.k.a. std::vector<big_t> */

    const big_t* pb = NULL;

    big_t tmp;
    mystruct ts = {1,2,3};  /* a tmp item for 'tmp.v' */
    const mystruct* pm=NULL;
    size_t i,j;

    printf("\nCOMPLEX VECTOR TEST:\n");

    cvector_create_with(&v,sizeof(big_t),NULL,&big_t_ctr,&big_t_dtr,&big_t_cpy);

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

    return 1;
}

