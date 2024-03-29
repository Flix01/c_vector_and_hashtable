/*
// compilation:
// gcc
gcc -O2 -no-pie -fno-pie -I"../include" c_hashtable_main.c -o c_hashtable_main
// clang
clang -O2 -no-pie -fno-pie -I"../include" c_hashtable_main.c -o c_hashtable_main
// mingw
x86_64-w64-mingw32-gcc -mconsole -O2 -I"../include" c_hashtable_main.c -o c_hashtable_main.exe
// cl.exe (from Visual C++ 7.1 2003)
cl /O2 /MT /Tc c_hashtable_main.c /I"../include" /link /out:c_hashtable_main.exe user32.lib kernel32.lib
*/

/*
// compile as C++ [not really necessary]
// gcc
gcc -O2 -x c++ -no-pie -fno-pie -I"../include" c_hashtable_main.c -o c_hashtable_main
// or just
g++ -O2 -no-pie -fno-pie -I"../include" c_hashtable_main.c -o c_hashtable_main
// clang and mingw are gcc based (try using clang++ and x86_64-w64-mingw32-g++)

// cl.exe (from Visual C++ 7.1 2003)
cl /O2 /MT /Tp c_hashtable_main.c /I"../include" /EHsc /link /out:c_hashtable_main_vc.exe user32.lib kernel32.lib
*/

/* Program Output:
HASHTABLE TEST:
Added item ht[	100,	50,	25]	=	["100-50-25"].
Added item ht[	5,	43,	250]	=	["5-43-250"].
Added item ht[	10,	250,	125]	=	["10-250-125"].
Fetched item ht[	10,	250,	125]	=	["10-250-125"].
All items (generally unsorted):
0) ht[	5,	43,	250]	=	["5-43-250"].
1) ht[	10,	250,	125]	=	["10-250-125"].
2) ht[	100,	50,	25]	=	["100-50-25"].
Removed item [	10,	250,	125]
An item with key[	10,	250,	125] is NOT present.
All items (generally unsorted):
0) ht[	5,	43,	250]	=	["5-43-250"].
1) ht[	100,	50,	25]	=	["100-50-25"].
[ch_mykey_myvalue_dbg_check]:
    num_total_items=2 (num_total_capacity=3) in 256 buckets [items per bucket: mean=0.008 std_deviation=0.088 min=0 (in 254/256) avg=0 (in 254/256) max=1 (in 2/256)].
    memory_used: 6 KB 324 Bytes. memory_minimal_possible: 6 KB 280 Bytes. mem_used_percentage: 100.68% (100% is the best possible result).
[ch_mykey_myvalue_dbg_check]:
    num_total_items=2 (num_total_capacity=2) in 256 buckets [items per bucket: mean=0.008 std_deviation=0.088 min=0 (in 254/256) avg=0 (in 254/256) max=1 (in 2/256)].
    memory_used: 6 KB 280 Bytes. memory_minimal_possible: 6 KB 280 Bytes. mem_used_percentage: 100.00% (100% is the best possible result).

STRING-STRING TEST:
All items (generally unsorted):
0) ht["name"] = ["John"];
1) ht["gender"] = ["male"];
2) ht["brother"] = ["Eddie Duke of the Hills"];
3) ht["profession"] = ["plumber"];
4) ht["very famous nickname"] = ["Super Johnny"];
[ch_string_string_dbg_check]:
    num_total_items=5 (num_total_capacity=5) in 256 buckets [items per bucket: mean=0.020 std_deviation=0.139 min=0 (in 251/256) avg=0 (in 251/256) max=1 (in 5/256)].
    memory_used: 6 KB 272 Bytes. memory_minimal_possible: 6 KB 272 Bytes. mem_used_percentage: 100.00% (100% is the best possible result).
Removed item with key "gender".
All items (generally unsorted):
0) ht["name"] = ["John"];
1) ht["brother"] = ["Eddie Duke of the Hills"];
2) ht["profession"] = ["plumber"];
3) ht["very famous nickname"] = ["Super Johnny"];
Fetched item ht["name"]=["John"].
*/

#include <stdio.h>  /* printf */

/*#define NO_SIMPLE_TEST*/
/*#define NO_STRING_STRING_TEST*/
/*#define NO_CPP_TEST*/

#ifndef NO_SIMPLE_TEST
/* 'typedef is mandatory: we need global visibility */
typedef struct {
	int a,b,c;
} mykey;
typedef struct	{
	char name[32];
} myvalue;
/* MANDATORY: for EACH mykey-myvalue pair we'll use in a hashtable, write something like
 (please replace all occurrences of 'mykey_myvalue' with your struct types): */
#ifndef C_HASHTABLE_mykey_myvalue_H
#define C_HASHTABLE_mykey_myvalue_H
#   define CH_KEY_TYPE mykey
#   define CH_VALUE_TYPE myvalue
#   define CH_NUM_BUCKETS_mykey_myvalue 256 /* in (0,65536] */
#   define CH_NUM_BUCKETS CH_NUM_BUCKETS_mykey_myvalue /* CH_NUM_BUCKETS defaults to 256 but it GETS UNDEFINED (==0) after each <c_hashtable.h> inclusion. That's why we copy it in a type-safe definition */
#   include "c_hashtable.h"	/* this header has no header guards inside! */
#endif /* C_HASHTABLE_mykey_myvalue_H */
/* What the lines above do, is to create
   the type-safe hashtable structure 'ch_mykey_myvalue',
   together with a lot of type-safe functions starting
   with 'ch_mykey_myvalue_'

    typedef struct {
        mykey k;
        myvalue v;
    } mykey_myvalue;

    typedef struct {
        [...]   // some other stuff we skip here

        // 'CH_NUM_BUCKETS_mykey_myvalue' sorted buckets
        struct chv_mykey_myvalue {
            mykey_myvalue * v;
            const size_t size;
            const size_t capacity;
        } buckets[CH_NUM_BUCKETS_mykey_myvalue];
    } ch_mykey_myvalue;

    typedef struct chv_mykey_myvalue chv_mykey_myvalue;  // gives global visibility to the 'hashtable_vector_type'

    Note there are also some PRIVATE function starting with 'chv_'. Do NOT use them:
    use only the type 'chv_mykey_myvalue' when you need to display items.
*/
static int mykey_cmp(const mykey* a,const mykey* b) {
    if (a->a>b->a) return 1;
    else if (a->a<b->a) return -1;
    if (a->b>b->b) return 1;
    else if (a->b<b->b) return -1;
    if (a->c>b->c) return 1;
    else if (a->c<b->c) return -1;
    return 0;
}
static __inline ch_hash_uint mykey_hash(const mykey* k) {
    return (ch_hash_uint)((k->a)
#   if CH_NUM_BUCKETS_mykey_myvalue!=CH_MAX_NUM_BUCKETS /* otherwise mod is unnecessary */
    %CH_NUM_BUCKETS_mykey_myvalue /* CH_MAX_NUM_BUCKETS can be 256 or 65536, but must be set globally (i.e. in Project Options) */
#   endif
    );
}

static void SimpleTest(void)    {
    ch_mykey_myvalue ht;/* we'll use 'ch_mykey_myvalue_create(...)' for init, so we skip C-style initialization */
    mykey tmpkey = {-10,200,-5};	/* tmp key used later */
    myvalue* fetched_value=NULL;
    size_t i,k,j;int match;

    printf("HASHTABLE TEST:\n");
    ch_mykey_myvalue_create(&ht,&mykey_hash,&mykey_cmp,1);

    /* OK, now if we don't define CH_DISABLE_FAKE_MEMBER_FUNCTIONS
       (that must be set globally, in the Project Options),
       we have access to all the OTHER 'ch_mykey_myvalue_' functions
       (except 'ch_mykey_myvalue_create' or 'ch_mykey_myvalue_create_with',
       that MUST be called to make this feature work), by using
       'fake member functions'. For example these two calls are the equivalent:
       ch_mykey_myvalue_get_or_insert(&ht,...);
       ht.get_or_insert(&ht,...);       // Note that ht appears twice in this line
       However in this demo we won't use the second syntax.
    */

    /* Fetch 'tmpkey', and, if not found, insert it and give it a 'value' */
    tmpkey.a = 100;tmpkey.b = 50;tmpkey.c = 25;    /* we don't need to set 'value' here */
    fetched_value = ch_mykey_myvalue_get_or_insert(&ht,&tmpkey,&match);CH_ASSERT(fetched_value);
    if (!match)  {
        /* item was not present, so we assign 'value' */
        strcpy(fetched_value->name,"100-50-25");
        printf("Added ");
    }
    else printf("Fetched ");
    printf("item ht[\t%d,\t%d,\t%d]\t=\t[\"%s\"].\n",tmpkey.a,tmpkey.b,tmpkey.c,fetched_value->name);

    /* Note that if C99 is available we can use 'compound literal'
    to specify structs on the fly in function arguments (i.e. we don't need 'tmpKey'): */
/*#	if __STDC_VERSION__>= 199901L
    fetched_value = ch_mykey_myvalue_get_or_insert(&ht,&(mykey){1,2,3},&match);CH_ASSERT(fetched_value);
    if (!match)  {
        strcpy(fetched_value->name,"1-2-3");
        printf("Added ");
    }
    else printf("Fetched ");
    printf("item ht[\t%d,\t%d,\t%d]\t=\t[\"%s\"].\n",1,2,3,fetched_value->name);
#	endif*/

    /* Fetch 'tmpkey', and, if not found, insert it and give it a 'value' */
    tmpkey.a = 5;tmpkey.b = 43;tmpkey.c = 250;
    fetched_value = ch_mykey_myvalue_get_or_insert(&ht,&tmpkey,&match);CH_ASSERT(fetched_value);
    if (!match)  {
        /* item was not present, so we assign 'value' */
        strcpy(fetched_value->name,"5-43-250");
        printf("Added ");
    }
    else printf("Fetched ");
    printf("item ht[\t%d,\t%d,\t%d]\t=\t[\"%s\"].\n",tmpkey.a,tmpkey.b,tmpkey.c,fetched_value->name);

    /* Fetch 'tmpkey', and, if not found, insert it and give it a 'value' */
    tmpkey.a = 10;tmpkey.b = 250;tmpkey.c = 125;
    fetched_value = ch_mykey_myvalue_get_or_insert(&ht,&tmpkey,&match);CH_ASSERT(fetched_value);
    if (!match)  {
        /* item was not present, so we assign 'value' */
        strcpy(fetched_value->name,"10-250-125");
        printf("Added ");
    }
    else printf("Fetched ");
    printf("item ht[\t%d,\t%d,\t%d]\t=\t[\"%s\"].\n",tmpkey.a,tmpkey.b,tmpkey.c,fetched_value->name);

    /* Just fetch 'tmpkey', without inserting it if not found (this call can return NULL) */
    fetched_value = ch_mykey_myvalue_get(&ht,&tmpkey);
    if (!fetched_value)  {
        /* item is not present */
        printf("An item with key[\t%d,\t%d,\t%d] is NOT present.\n",tmpkey.a,tmpkey.b,tmpkey.c);
    }
    else printf("Fetched item ht[\t%d,\t%d,\t%d]\t=\t[\"%s\"].\n",tmpkey.a,tmpkey.b,tmpkey.c,fetched_value->name);


    /* Display all entries (in general they are not sorted) */
    k=0;printf("All items (generally unsorted):\n");
    for (i=0;i<CH_NUM_BUCKETS_mykey_myvalue;i++)	{
        const chv_mykey_myvalue* bucket = &ht.buckets[i];
        if (!bucket->v)	continue;
        for (j=0;j<bucket->size;j++)	{
            const mykey_myvalue* pitem = &bucket->v[j];
            printf("%lu) ht[\t%d,\t%d,\t%d]\t=\t[\"%s\"].\n",k++,pitem->k.a,pitem->k.b,pitem->k.c,pitem->v.name);
            /* Please note that the correct formatting syntax for size_t is %zu, but it's C99 specific (and %llu too). So we use the %lu syntax */
        }
    }
    /* Please note that the user defined, type-safe definition 'CH_NUM_BUCKETS_mykey_myvalue'
       is not strictly necessary, because we can use a syntax like:
       '(sizeof(chvs_mykey_myvalue)/sizeof(chv_mykey_myvalue))' instread.
       However I think that the type-safe definition is more user friendly
    */
    CH_ASSERT(CH_NUM_BUCKETS_mykey_myvalue==sizeof(chvs_mykey_myvalue)/sizeof(chv_mykey_myvalue));

    /* Remove 'tmpkey' */
    if (ch_mykey_myvalue_remove(&ht,&tmpkey)) printf("Removed item [\t%d,\t%d,\t%d]\n",tmpkey.a,tmpkey.b,tmpkey.c);
    else printf("Can't remove item [\t%d,\t%d,\t%d], because it's not present.\n",tmpkey.a,tmpkey.b,tmpkey.c);

    /* Just fetch 'tmpkey', without inserting it if not found (this call can return NULL) */
    fetched_value = ch_mykey_myvalue_get(&ht,&tmpkey);
    if (!fetched_value)  {
        /* item is not present */
        printf("An item with key[\t%d,\t%d,\t%d] is NOT present.\n",tmpkey.a,tmpkey.b,tmpkey.c);
    }
    else printf("Fetched item ht[\t%d,\t%d,\t%d]=[\"%s\"].\n",tmpkey.a,tmpkey.b,tmpkey.c,fetched_value->name);

    /* Display all entries (in general they are not sorted) */
    k=0;printf("All items (generally unsorted):\n");
    for (i=0;i<CH_NUM_BUCKETS_mykey_myvalue;i++)	{
        const chv_mykey_myvalue* bucket = &ht.buckets[i];
        if (!bucket->v)	continue;
        for (j=0;j<bucket->size;j++)	{
            const mykey_myvalue* pitem = &bucket->v[j];
            printf("%lu) ht[\t%d,\t%d,\t%d]\t=\t[\"%s\"].\n",k++,pitem->k.a,pitem->k.b,pitem->k.c,pitem->v.name);
        }
    }

    /* Debug function that can be useful to detect sorting errors and
       optimize 'mykey_hash' (to reduce the standard deviation) and 'CH_NUM_BUCKETS_mykey_myvalue' */
    ch_mykey_myvalue_dbg_check(&ht);
    ch_mykey_myvalue_shrink_to_fit(&ht);    /* removes unused capacity (slow) */
    ch_mykey_myvalue_dbg_check(&ht);

    /* destroy hashtable 'ht' */
    ch_mykey_myvalue_free(&ht);
}
#endif /* NO_SIMPLE_TEST */

#ifndef NO_STRING_STRING_TEST
typedef char* string;
#ifndef C_HASHTABLE_string_string_H
#define C_HASHTABLE_string_string_H
#   define CH_KEY_TYPE string
#   define CH_VALUE_TYPE string
#   define CH_NUM_BUCKETS_string_string 256 /* in (0,65536] */
#   define CH_NUM_BUCKETS CH_NUM_BUCKETS_string_string /* CH_NUM_BUCKETS defaults to 256 but it GETS UNDEFINED (==0) after each <c_hashtable.h> inclusion. That's why we copy it in a type-safe definition */
#   include "c_hashtable.h"	/* this header has no header guards inside! */
#endif /* C_HASHTABLE_string_string_H */
static int string_cmp(const string* a,const string* b) {
    if (*a==NULL) return (*b==NULL) ? 0 : 1;
    else if (*b==NULL) return 1;
    return strcmp(*a,*b);
}
static void string_ctr(string* a)    {*a=NULL;}
static void string_dtr(string* a)    {if (*a) {free(*a);*a=NULL;}}
static __inline void string_setter(string* a,const char* b)    {
    /* we have refactored 'string_cpy', because we'll use 'string_setter' later */
    if (!b)    {string_dtr(a);return;}
    else    {
        const size_t blen = strlen(b);
        if (!(*a) || strlen(*a)<blen) *a = (string) realloc(*a,blen+1);
        memcpy(*a,b,blen+1);
    }
}
static void string_cpy(string* a,const string* b)    {string_setter(a,*b);}
static __inline ch_hash_uint string_hash(const string* k) {
/*#   define TEST_MURMUR_3_HASH */  /* ...we should test this with a lot of strings... */
    return (ch_hash_uint)(((*k) ?
#   ifdef TEST_MURMUR_3_HASH
                 /* well, ch_hash_murmur3(...) returns a 32-bit unsigned int. Hope it works... */
                 ch_hash_murmur3((const unsigned char*) (*k),strlen(*k),7)  /* ...but its calculation is much slower */
#   else
                 strlen(*k)     /* naive hash but calculation is much faster */
#   endif
               : 0)
#   if CH_NUM_BUCKETS_string_string!=CH_MAX_NUM_BUCKETS /* otherwise mod is unnecessary */
    %CH_NUM_BUCKETS_string_string /* CH_MAX_NUM_BUCKETS can be 256 or 65536, but must be set globally (i.e. in Project Options) */
#   endif
    );
}
static void StringStringTest(void)  {
    ch_string_string ht;
    string* value = NULL;
    size_t i,k,j;

    printf("\nSTRING-STRING TEST:\n");
    ch_string_string_create_with(   &ht,&string_hash,&string_cmp,
                                    &string_ctr,&string_dtr,&string_cpy,    /* key */
                                    &string_ctr,&string_dtr,&string_cpy,    /* value */
                                    1);

    /* this inserts: ht["name"]="John"; */
    string_setter(ch_string_string_get_or_insert_by_val(&ht,(string)"name",0),"John");  /* the (string) cast is there just to remove a warning when compiled in c++ mode */
    /* this inserts: ht["profession"]="plumber"; */
    string_setter(ch_string_string_get_or_insert_by_val(&ht,(string)"profession",0),"plumber");
    /* this inserts: ht["brother"]="Eddie Duke of the Hills"; */
    string_setter(ch_string_string_get_or_insert_by_val(&ht,(string)"brother",0),"Eddie Duke of the Hills");
    /* this inserts: ht["gender"]="male"; */
    string_setter(ch_string_string_get_or_insert_by_val(&ht,(string)"gender",0),"male");
    /* this inserts: ht["very famous nickname"]="Super Johnny"; */
    string_setter(ch_string_string_get_or_insert_by_val(&ht,(string)"very famous nickname",0),"Super Johnny");

    /* Display all entries (in general they are not sorted) */
    k=0;printf("All items (generally unsorted):\n");
    for (i=0;i<CH_NUM_BUCKETS_string_string;i++)	{
        const chv_string_string* bucket = &ht.buckets[i];
        if (!bucket->v)	continue;
        for (j=0;j<bucket->size;j++)	{
            const string_string* pitem = &bucket->v[j];
            printf("%lu) ht[\"%s\"] = [\"%s\"];\n",k++,pitem->k,pitem->v);
        }
    }

    /* Debug function that can be useful to detect sorting errors and
       optimize 'string_hash' (to reduce the standard deviation) and 'CH_NUM_BUCKETS_string_string' */
    ch_string_string_dbg_check(&ht);

    /* remove one item */
    if (ch_string_string_remove_by_val(&ht,(string)"gender")) printf("Removed item with key \"gender\".\n");
    else printf("Can't remove item with key \"gender\", because it's not present.\n");

    /* Display all entries (in general they are not sorted) */
    k=0;printf("All items (generally unsorted):\n");
    for (i=0;i<CH_NUM_BUCKETS_string_string;i++)	{
        const chv_string_string* bucket = &ht.buckets[i];
        if (!bucket->v)	continue;
        for (j=0;j<bucket->size;j++)	{
            const string_string* pitem = &bucket->v[j];
            printf("%lu) ht[\"%s\"] = [\"%s\"];\n",k++,pitem->k,pitem->v);
        }
    }

    /* Just fetch 'tmpkey', without inserting it if not found (this call can return NULL) */
    value = ch_string_string_get_by_val(&ht,(string)"name");
    if (!value)  {
        /* item is not present */
        printf("An item with key \"name\" is NOT present.\n");
    }
    else printf("Fetched item ht[\"name\"]=[\"%s\"].\n",*value);


    ch_string_string_free(&ht);
}
#endif /* NO_STRING_STRING_TEST */

#ifndef NO_CPP_TEST
#ifdef __cplusplus

typedef char char16[16];
#ifndef C_HASHTABLE_int_char16_H
#define C_HASHTABLE_int_char16_H
#   define CH_KEY_TYPE int
#   define CH_VALUE_TYPE char16
#   define CH_NUM_BUCKETS_int_char16 256 /* in (0,65536] */
#   define CH_NUM_BUCKETS CH_NUM_BUCKETS_int_char16 /* CH_NUM_BUCKETS defaults to 256 but it GETS UNDEFINED (==0) after each <c_hashtable.h> inclusion. That's why we copy it in a type-safe definition */
#   include "c_hashtable.h"	/* this header has no header guards inside! */
#endif /* C_HASHTABLE_int_char16_H */
int int_cmp(const int* a,const int* b) {return (*a)<(*b)?-1:((*a)>(*b)?1:0);}
ch_hash_uint int_hash(const int* k) {
    return (ch_hash_uint) ((*k)
#   if CH_NUM_BUCKETS_int_char16!=CH_MAX_NUM_BUCKETS /* otherwise mod is unnecessary */
        %CH_NUM_BUCKETS_int_char16
#   endif
    );
}

#   include <vector>   /* std::vector */

void CppTest(void)    {
    /* This makes only sense when porting existing C++ code to plain C. Some tips:
       -> try to replace a single 'std::unordered_map' with a 'ch_xxx_xxx', keep the code compilable (in C++) and repeat the process
       -> it's easier to include a 'ch_xxx_xxx' into a STL container than vice-versa
       -> so it's better to avoid placing STL containers, directly or indirectly, as keys or as values, into 'ch_xxx_xxx'
       -> 'ch_xxx_xxx_create(...)' is still mandatory in C++, (to specify key_hash and key_cmp)
       -> in buckets (of type 'chv_xxx_xxx') we can use 'operator[](i).k' and 'operator[](i).v' as a quick replacement of STL operator[] (but it does not work in plain C, so we'll need to convert it later)
       -> ch_xxx_xxx::~ch_xxx_xxx() calls 'ch_xxx_xxx_free(...)' for us (but in plain C it's not available. It can be useful to remember that it's harmless to call 'ch_xxx_xxx_free(...)' multiple times)
       -> some programmers prefer using the 'fake member function' syntax when porting code from std::unordered_map
    */

    size_t i,j,t,cnt=0;
    const int tmpk[10]={0,1,2,3,4,5,6,7,8,9};
    const char tmpv[16][10]={"zero","one","two","three","four","five","six","seven","eight","nine"};
    ch_int_char16 ht0,ht1;
    std::vector<ch_int_char16> vstd;

    printf("\nCPP MODE TEST:\n");

    ch_int_char16_create(&ht0,&int_hash,&int_cmp,0);   /* 'ch_int_char16_create(...)' is still mandatory */
    ch_int_char16_create(&ht1,&int_hash,&int_cmp,0);   /* 'ch_int_char16_create(...)' is still mandatory */

    for (i=1;i<6;i++) strcpy((char*)ch_int_char16_get_or_insert(&ht0,&tmpk[i],NULL),tmpv[i]); /* or ...ht0.get_or_insert(&ht0,&tmpk[i])... in the 'fake member function' syntax */
    for (i=9;i>6;i--) strcpy((char*)ch_int_char16_get_or_insert(&ht1,&tmpk[i],NULL),tmpv[i]);

    /* enumerate elements */
    cnt=0;printf("ht0 = {%lu:\t[",ch_int_char16_get_num_items(&ht0));
    for (j=0;j<CH_NUM_BUCKETS_int_char16;j++) {
        const chv_int_char16& b = ht0.buckets[j];if (!b.size) continue;
        for (i=0;i<b.size;i++)   {if (cnt>0) printf(",");printf("ht0[%d]=\"%s\"",b[i].k,b[i].v);++cnt;}  /* Warning: 'b[i].k' and 'b[i].v' are C++ specific */
    }
    printf("]};\n");
    cnt=0;printf("ht1 = {%lu:\t[",ch_int_char16_get_num_items(&ht1));
    for (j=0;j<CH_NUM_BUCKETS_int_char16;j++) {
        const chv_int_char16& b = ht1.buckets[j];if (!b.size) continue;
        for (i=0;i<b.size;i++)   {if (cnt>0) printf(",");printf("ht1[%d]=\"%s\"",b[i].k,b[i].v);++cnt;}
    }
    printf("]};\n");

    printf("ch_int_char16_swap(&ht1,&ht0):\n");
    ch_int_char16_swap(&ht1,&ht0);

    /* enumerate elements */
    cnt=0;printf("ht0 = {%lu:\t[",ch_int_char16_get_num_items(&ht0));
    for (j=0;j<CH_NUM_BUCKETS_int_char16;j++) {
        const chv_int_char16& b = ht0.buckets[j];if (!b.size) continue;
        for (i=0;i<b.size;i++)   {if (cnt>0) printf(",");printf("ht0[%d]=\"%s\"",b[i].k,b[i].v);++cnt;}
    }
    printf("]};\n");
    cnt=0;printf("ht1 = {%lu:\t[",ch_int_char16_get_num_items(&ht1));
    for (j=0;j<CH_NUM_BUCKETS_int_char16;j++) {
        const chv_int_char16& b = ht1.buckets[j];if (!b.size) continue;
        for (i=0;i<b.size;i++)   {if (cnt>0) printf(",");printf("ht1[%d]=\"%s\"",b[i].k,b[i].v);++cnt;}
    }
    printf("]};\n");


    printf("std::vector<ch_int_char16> test:\n");
    vstd.push_back(ht0);
    vstd.push_back(ht1);
    vstd.push_back(ht0);
    vstd.push_back(ht1);

    /* enumerate elements */
    for (t=0;t<vstd.size();t++) {
        const ch_int_char16& ht = vstd[t];
        cnt=0;printf("vstd[%lu] = {%lu:\t[",t,ch_int_char16_get_num_items(&ht));
        for (j=0;j<CH_NUM_BUCKETS_int_char16;j++) {
            const chv_int_char16& b = ht.buckets[j];if (!b.size) continue;
            for (i=0;i<b.size;i++)   {if (cnt>0) printf(",");printf("ht[%d]=\"%s\"",b[i].k,b[i].v);++cnt;}
        }
        printf("]};\n");
    }


    printf("ch_int_char16_cpy(&ht1,&ht0):\n");
    ch_int_char16_cpy(&ht1,&ht0);

    /* enumerate elements */
    cnt=0;printf("ht0 = {%lu:\t[",ch_int_char16_get_num_items(&ht0));
    for (j=0;j<CH_NUM_BUCKETS_int_char16;j++) {
        const chv_int_char16& b = ht0.buckets[j];if (!b.size) continue;
        for (i=0;i<b.size;i++)   {if (cnt>0) printf(",");printf("ht0[%d]=\"%s\"",b[i].k,b[i].v);++cnt;}
    }
    printf("]};\n");
    cnt=0;printf("ht1 = {%lu:\t[",ch_int_char16_get_num_items(&ht1));
    for (j=0;j<CH_NUM_BUCKETS_int_char16;j++) {
        const chv_int_char16& b = ht1.buckets[j];if (!b.size) continue;
        for (i=0;i<b.size;i++)   {if (cnt>0) printf(",");printf("ht1[%d]=\"%s\"",b[i].k,b[i].v);++cnt;}
    }
    printf("]};\n");

    /* ch_int_char16::~ch_int_char16() calls 'ch_int_char16_free(...)' for us (but in plain C it's not available) */

}
#endif /* __cpusplus */
#endif /* NO_CPP_TEST */



int main(int argc,char* argv[])
{
#   ifndef NO_SIMPLE_TEST
    SimpleTest();
#   endif
#   ifndef NO_STRING_STRING_TEST
    StringStringTest();
#   endif

#   ifndef NO_CPP_TEST
#   ifdef __cplusplus
    CppTest();
#   endif /* __cpusplus */
#   endif /* NO_CPP_TEST */

    return 1;
}

