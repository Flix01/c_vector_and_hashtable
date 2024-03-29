/*
// compilation:
// gcc
gcc -O2 -no-pie -fno-pie -I"../include" c_hashtable_type_unsafe_main.c -o c_hashtable_type_unsafe_main
// clang
clang -O2 -no-pie -fno-pie -I"../include" c_hashtable_type_unsafe_main.c -o c_hashtable_type_unsafe_main
// mingw
x86_64-w64-mingw32-gcc -mconsole -O2 -I"../include" c_hashtable_type_unsafe_main.c -o c_hashtable_type_unsafe_main.exe
// cl.exe (from Visual C++ 7.1 2003)
cl /O2 /MT /Tc c_hashtable_type_unsafe_main.c /I"../include" /link /out:c_hashtable_type_unsafe_main.exe user32.lib kernel32.lib
*/

/*
// compile as C++ [not really necessary]
// gcc
gcc -O2 -x c++ -no-pie -fno-pie -I"../include" c_hashtable_type_unsafe_main.c -o c_hashtable_type_unsafe_main
// or just
g++ -O2 -no-pie -fno-pie -I"../include" c_hashtable_type_unsafe_main.c -o c_hashtable_type_unsafe_main
// clang and mingw are gcc based (try using clang++ and x86_64-w64-mingw32-g++)

// cl.exe (from Visual C++ 7.1 2003)
cl /O2 /MT /Tp c_hashtable_type_unsafe_main.c /I"../include" /EHsc /link /out:c_hashtable_type_unsafe_main_vc.exe user32.lib kernel32.lib
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
[chashtable_dbg_check]:
    num_total_items=2 (num_total_capacity=3) in 256 buckets [items per bucket: mean=0.008 std_deviation=0.088 min=0 (in 254/256) avg=0 (in 254/256) max=1 (in 2/256)].
    memory_used: 8 KB 308 Bytes. memory_minimal_possible: 8 KB 264 Bytes. mem_used_percentage: 100.52% (100% is the best possible result).
[chashtable_dbg_check]:
    num_total_items=2 (num_total_capacity=2) in 256 buckets [items per bucket: mean=0.008 std_deviation=0.088 min=0 (in 254/256) avg=0 (in 254/256) max=1 (in 2/256)].
    memory_used: 8 KB 264 Bytes. memory_minimal_possible: 8 KB 264 Bytes. mem_used_percentage: 100.00% (100% is the best possible result).

STRING-STRING TEST:
All items (generally unsorted):
0) ht["name"] = ["John"];
1) ht["gender"] = ["male"];
2) ht["brother"] = ["Eddie Duke of the Hills"];
3) ht["profession"] = ["plumber"];
4) ht["very famous nickname"] = ["Super Johnny"];
[chashtable_dbg_check]:
    num_total_items=5 (num_total_capacity=5) in 256 buckets [items per bucket: mean=0.020 std_deviation=0.139 min=0 (in 251/256) avg=0 (in 251/256) max=1 (in 5/256)].
    memory_used: 8 KB 256 Bytes. memory_minimal_possible: 8 KB 256 Bytes. mem_used_percentage: 100.00% (100% is the best possible result).
Removed item with key "gender".
All items (generally unsorted):
0) ht["name"] = ["John"];
1) ht["brother"] = ["Eddie Duke of the Hills"];
2) ht["profession"] = ["plumber"];
3) ht["very famous nickname"] = ["Super Johnny"];
Fetched item ht["name"]=["John"].
*/

#include "c_hashtable_type_unsafe.h"
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

static int mykey_cmp(const void* av,const void* bv) {
	const mykey* a = (const mykey*) av;
	const mykey* b = (const mykey*) bv;
    if (a->a>b->a) return 1;
    else if (a->a<b->a) return -1;
    if (a->b>b->b) return 1;
    else if (a->b<b->b) return -1;
    if (a->c>b->c) return 1;
    else if (a->c<b->c) return -1;
    return 0;
}
static __inline chtu_hash_uint mykey_hash(const void* kv) {
	const mykey* k = (const mykey*) kv;
    return (chtu_hash_uint)((k->a)
#   if CH_NUM_USED_BUCKETS!=CH_MAX_POSSIBLE_NUM_BUCKETS /* otherwise mod is unnecessary */
        %CH_NUM_USED_BUCKETS /* CH_MAX_POSSIBLE_NUM_BUCKETS is READ-ONLY and can be 256 or 65536, or 2147483648 (it depends on CH_NUM_USED_BUCKETS, and the type 'chtu_hash_uint' depends on it) */
#   endif
    );
}

static void SimpleTest(void)    {
    chashtable ht; /* we'll use 'chashtable_create(...)' for init; so we can skip here a C-style init */
    mykey tmpkey = {-10,200,-5};	/* tmp key used later */
    myvalue* fetched_value=NULL;
    size_t i,k,j;int match;

    printf("HASHTABLE TEST:\n");
    chashtable_create(&ht,sizeof(mykey),sizeof(myvalue),&mykey_hash,&mykey_cmp,1);

    /* OK, now if we don't define CH_DISABLE_FAKE_MEMBER_FUNCTIONS
       (that must be set globally, in the Project Options),
       we have access to all the OTHER 'chashtable_' functions
       (except 'chashtable_create' or 'chashtable_create_with',
       that MUST be called to make this feature work), by using
       'fake member functions'. For example these two calls are the equivalent:
       chashtable_get_or_insert(&ht,...);
       ht.get_or_insert(&ht,...);       // Note that ht appears twice in this line
       However in this demo we won't use the second syntax.
    */

    /* Fetch 'tmpkey', and, if not found, insert it and give it a 'value' */
    tmpkey.a = 100;tmpkey.b = 50;tmpkey.c = 25;    /* we don't need to set 'value' here */
    fetched_value = (myvalue*) chashtable_get_or_insert(&ht,&tmpkey,&match);CH_ASSERT(fetched_value);   /* casting to (myvalue*) is there just to allow compilation in c++ mode */
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
    fetched_value = chashtable_get_or_insert(&ht,&(mykey){1,2,3},&match);CH_ASSERT(fetched_value);
    if (!match)  {
        strcpy(fetched_value->name,"1-2-3");
        printf("Added ");
    }
    else printf("Fetched ");
    printf("item ht[\t%d,\t%d,\t%d]\t=\t[\"%s\"].\n",1,2,3,fetched_value->name);
#	endif*/

    /* Fetch 'tmpkey', and, if not found, insert it and give it a 'value' */
    tmpkey.a = 5;tmpkey.b = 43;tmpkey.c = 250;
    fetched_value = (myvalue*) chashtable_get_or_insert(&ht,&tmpkey,&match);CH_ASSERT(fetched_value);
    if (!match)  {
        /* item was not present, so we assign 'value' */
        strcpy(fetched_value->name,"5-43-250");
        printf("Added ");
    }
    else printf("Fetched ");
    printf("item ht[\t%d,\t%d,\t%d]\t=\t[\"%s\"].\n",tmpkey.a,tmpkey.b,tmpkey.c,fetched_value->name);

    /* Fetch 'tmpkey', and, if not found, insert it and give it a 'value' */
    tmpkey.a = 10;tmpkey.b = 250;tmpkey.c = 125;
    fetched_value = (myvalue*) chashtable_get_or_insert(&ht,&tmpkey,&match);CH_ASSERT(fetched_value);
    if (!match)  {
        /* item was not present, so we assign 'value' */
        strcpy(fetched_value->name,"10-250-125");
        printf("Added ");
    }
    else printf("Fetched ");
    printf("item ht[\t%d,\t%d,\t%d]\t=\t[\"%s\"].\n",tmpkey.a,tmpkey.b,tmpkey.c,fetched_value->name);

    /* Just fetch 'tmpkey', without inserting it if not found (this call can return NULL) */
    fetched_value = (myvalue*) chashtable_get(&ht,&tmpkey);
    if (!fetched_value)  {
        /* item is not present */
        printf("An item with key[\t%d,\t%d,\t%d] is NOT present.\n",tmpkey.a,tmpkey.b,tmpkey.c);
    }
    else printf("Fetched item ht[\t%d,\t%d,\t%d]\t=\t[\"%s\"].\n",tmpkey.a,tmpkey.b,tmpkey.c,fetched_value->name);


    /* Display all entries (in general they are not sorted) */
    k=0;printf("All items (generally unsorted):\n");
    for (i=0;i<CH_NUM_USED_BUCKETS;i++)	{
        const chvector* bucket = &ht.buckets[i];
        const mykey* keys   = (const mykey*) bucket->k; /* of course these pointers can be reallocated and must be set soon before usage */
        const myvalue* values = (const myvalue*) bucket->v;
        if (!keys)	continue;
        for (j=0;j<bucket->size;j++)	{
            const mykey* key = &keys[j];
            const myvalue* val = &values[j];
            printf("%lu) ht[\t%d,\t%d,\t%d]\t=\t[\"%s\"].\n",k++,key->a,key->b,key->c,val?val->name:"NULL");
            /* Please note that the correct formatting syntax for size_t is %zu, but it's C99 specific (and %llu too). So we use the %lu syntax */
        }
    }
    /* CH_NUM_USED_BUCKETS can be set globally, and it's equivalent to: */
    CH_ASSERT(CH_NUM_USED_BUCKETS==sizeof(chvectors)/sizeof(chvector));

    /* Remove 'tmpkey' */
    if (chashtable_remove(&ht,&tmpkey)) printf("Removed item [\t%d,\t%d,\t%d]\n",tmpkey.a,tmpkey.b,tmpkey.c);
    else printf("Can't remove item [\t%d,\t%d,\t%d], because it's not present.\n",tmpkey.a,tmpkey.b,tmpkey.c);

    /* Just fetch 'tmpkey', without inserting it if not found (this call can return NULL) */
    fetched_value = (myvalue*) chashtable_get(&ht,&tmpkey);
    if (!fetched_value)  {
        /* item is not present */
        printf("An item with key[\t%d,\t%d,\t%d] is NOT present.\n",tmpkey.a,tmpkey.b,tmpkey.c);
    }
    else printf("Fetched item ht[\t%d,\t%d,\t%d]=[\"%s\"].\n",tmpkey.a,tmpkey.b,tmpkey.c,fetched_value->name);

    /* Display all entries (in general they are not sorted) */
    k=0;printf("All items (generally unsorted):\n");
    for (i=0;i<CH_NUM_USED_BUCKETS;i++)	{
        const chvector* bucket = &ht.buckets[i];
        const mykey* keys   = (const mykey*) bucket->k;
        const myvalue* values = (const myvalue*) bucket->v;
        if (!keys)	continue;
        for (j=0;j<bucket->size;j++)	{
            const mykey* key = &keys[j];
            const myvalue* val = &values[j];
            printf("%lu) ht[\t%d,\t%d,\t%d]\t=\t[\"%s\"].\n",k++,key->a,key->b,key->c,val?val->name:"NULL");
        }
    }

    /* Debug function that can be useful to detect sorting errors and
       optimize 'mykey_hash' (to reduce the standard deviation)
       and 'CH_NUM_USED_BUCKETS' (but in the type-unsafe version it's common to all chashtables) */
    chashtable_dbg_check(&ht);
    chashtable_shrink_to_fit(&ht);    /* removes unused capacity (slow) */
    chashtable_dbg_check(&ht);

    /* destroy hashtable 'ht' */
    chashtable_free(&ht);
}
#endif /* NO_SIMPLE_TEST */

#ifndef NO_STRING_STRING_TEST
typedef char* string;

static int string_cmp(const void* av,const void* bv) {
	const string* a=(const string*)av;
	const string* b=(const string*)bv;
    if (*a==NULL) return (*b==NULL) ? 0 : 1;
    else if (*b==NULL) return 1;
    return strcmp(*a,*b);
}
static void string_ctr(void* av)    {string* a=(string*)av;*a=NULL;}
static void string_dtr(void* av)    {string* a=(string*)av;if (*a) {free(*a);*a=NULL;}}
static __inline void string_setter(string* a,const char* b)    {
    /* we have refactored 'string_cpy', because we'll use 'string_setter' later */
    if (!b)    {string_dtr(a);return;}
    else    {
        const size_t blen = strlen(b);
        if (!(*a) || strlen(*a)<blen) *a = (string) realloc(*a,blen+1);
        memcpy(*a,b,blen+1);
    }
}
static void string_cpy(void* av,const void* bv)    {string* a=(string*)av;const string* b=(const string*)bv;string_setter(a,*b);}
static __inline chtu_hash_uint string_hash(const void* kv) {
/*#   define TEST_MURMUR_3_HASH */  /* ...we should test this with a lot of strings... */
	const string* k=(const string*)kv;
    return (chtu_hash_uint) (((*k) ?
#   ifdef TEST_MURMUR_3_HASH
                 /* well, ch_hash_murmur3(...) returns a 32-bit unsigned int. Hope it works... */
                 ch_hash_murmur3((const unsigned char*) (*k),strlen(*k),7)  /* ...but its calculation is much slower */
#   else
                 strlen(*k)     /* naive hash but calculation is much faster */
#   endif
               : 0)
#   if CH_NUM_USED_BUCKETS!=CH_MAX_POSSIBLE_NUM_BUCKETS /* otherwise mod is unnecessary */
    %CH_NUM_USED_BUCKETS /*  CH_MAX_POSSIBLE_NUM_BUCKETS is READ-ONLY and can be 256 or 65536, or 2147483648 (it depends on CH_NUM_USED_BUCKETS, and the type 'chtu_hash_uint' depends on it) */
#   endif
    );
}
static void StringStringTest(void)  {
    chashtable ht;
    string* value = NULL;
    size_t i,k,j;
    const char* tmp[5]={"name","profession","brother","gender","very famous nickname"};

    printf("\nSTRING-STRING TEST:\n");
    chashtable_create_with(   &ht,	sizeof(string),sizeof(string),
									&string_hash,&string_cmp,
                                    &string_ctr,&string_dtr,&string_cpy,    /* key */
                                    &string_ctr,&string_dtr,&string_cpy,    /* value */
                                    1);

    /* same problem here: in type-unsafe versions, we can't have 'chashtable_get_or_insert_by_val(...)',
       so AFAIK we can't just pass the address of a static string as key (like: &"name"),
       but we need real addresses. That's why we've used 'tmp' here */

    /* this inserts: ht["name"]="John"; */
    string_setter((string*)chashtable_get_or_insert(&ht,&tmp[0],0),"John"); /* (string*) cast is there just to allow compilation in c++ mode */
    /* this inserts: ht["profession"]="plumber"; */
    string_setter((string*)chashtable_get_or_insert(&ht,&tmp[1],0),"plumber");
    /* this inserts: ht["brother"]="Eddie Duke of the Hills"; */
    string_setter((string*)chashtable_get_or_insert(&ht,&tmp[2],0),"Eddie Duke of the Hills");
    /* this inserts: ht["gender"]="male"; */
    string_setter((string*)chashtable_get_or_insert(&ht,&tmp[3],0),"male");
    /* this inserts: ht["very famous nickname"]="Super Johnny"; */
    string_setter((string*)chashtable_get_or_insert(&ht,&tmp[4],0),"Super Johnny");

    /* Display all entries (in general they are not sorted) */
    k=0;printf("All items (generally unsorted):\n");
    for (i=0;i<CH_NUM_USED_BUCKETS;i++)	{
        const chvector* bucket = &ht.buckets[i];
        const string* keys = (const string*) bucket->k; /* of course these pointers can be reallocated and must be set soon before usage */
        const string* values = (const string*) bucket->v;
        if (!keys)	continue;
        for (j=0;j<bucket->size;j++)	{
            const string* key = &keys[j];
            const string* val = &values[j];
            printf("%lu) ht[\"%s\"] = [\"%s\"];\n",k++,*key,*val);
        }
    }

    /* Debug function that can be useful to detect sorting errors and
       optimize 'string_hash' (to reduce the standard deviation)
       and 'CH_NUM_USED_BUCKETS' (but in the type-unsafe version it's common to all chashtables) */
    chashtable_dbg_check(&ht);

    /* remove one item */
    if (chashtable_remove(&ht,&tmp[3])) printf("Removed item with key \"%s\".\n",tmp[3]);
    else printf("Can't remove item with key \"%s\", because it's not present.\n",tmp[3]);

    /* Display all entries (in general they are not sorted) */
    k=0;printf("All items (generally unsorted):\n");
    for (i=0;i<CH_NUM_USED_BUCKETS;i++)	{
        const chvector* bucket = &ht.buckets[i];
        const string* keys = (const string*) bucket->k;
        const string* values = (const string*) bucket->v;
        if (!keys)	continue;
        for (j=0;j<bucket->size;j++)	{
            const string* key = &keys[j];
            const string* val = &values[j];
            printf("%lu) ht[\"%s\"] = [\"%s\"];\n",k++,*key,*val);
        }
    }

    /* Just fetch 'tmpkey', without inserting it if not found (this call can return NULL) */
    value = (string*)chashtable_get(&ht,&tmp[0]);
    if (!value)  {
        /* item is not present */
        printf("An item with key \"%s\" is NOT present.\n",tmp[0]);
    }
    else printf("Fetched item ht[\"%s\"]=[\"%s\"].\n",tmp[0],*value);


    chashtable_free(&ht);
}
#endif /* NO_STRING_STRING_TEST */


#ifndef NO_CPP_TEST
#ifdef __cplusplus


/* stuff needed to create a chashtable with <int,char[16]> */
typedef int ht_key;
typedef char ht_value[16];
int ht_key_cmp(const void* av,const void* bv) {
    const ht_key* a = (const ht_key*) av;
    const ht_key* b = (const ht_key*) bv;
    return (*a)<(*b)?-1:((*a)>(*b)?1:0);
}
chtu_hash_uint ht_key_hash(const void* kv) {
    const ht_key* k = (const ht_key*) kv;
    return (chtu_hash_uint) ((*k)
#   if CH_NUM_USED_BUCKETS!=CH_MAX_POSSIBLE_NUM_BUCKETS /* otherwise mod is unnecessary */
        %CH_NUM_USED_BUCKETS /*  CH_MAX_POSSIBLE_NUM_BUCKETS is READ-ONLY and can be 256 or 65536, or 2147483648 (it depends on CH_NUM_USED_BUCKETS, and the type 'chtu_hash_uint' depends on it) */
#   endif
    );
}

#   include <vector>   /* std::vector */

void CppTest(void)    {
    /* This makes only sense when porting existing C++ code to plain C. Some tips:
       -> try to replace a single 'std::unordered_map' with a 'chashtable', keep the code compilable (in C++) and repeat the process
       -> it's easier to include chashtables into STL containers than vice-versa
       -> so it's better to avoid placing STL containers, directly or indirectly, as keys or as values, into chashtables
       -> 'chashtable_create(...)' is still mandatory
       -> in buckets (of type 'chvector') we can use 'bck.key_at<keytype>(i)' and 'bck.value_at<valuetype>(i)' as a quick replacement of STL operator[] (but it does not work in plain C, so we'll need to convert it later)
       -> chashtable::~chashtable() calls 'chashtable_free(...)' for us (but in plain C it's not available. It can be useful to remember that it's harmless to call 'chashtable_free(...)' multiple times)
       -> some programmers prefer using the 'fake member function' syntax when porting code from std::unordered_map
    */

    size_t i,j,t,cnt=0;
    const int tmpk[10]={0,1,2,3,4,5,6,7,8,9};
    const char tmpv[16][10]={"zero","one","two","three","four","five","six","seven","eight","nine"};
    chashtable ht0,ht1;
    std::vector<chashtable> vstd;

    printf("\nCPP MODE TEST:\n");

    chashtable_create(&ht0,sizeof(ht_key),sizeof(ht_value),&ht_key_hash,&ht_key_cmp,0);   /* 'chashtable_create(...)' is still mandatory */
    chashtable_create(&ht1,sizeof(ht_key),sizeof(ht_value),&ht_key_hash,&ht_key_cmp,0);   /* 'chashtable_create(...)' is still mandatory */

    for (i=1;i<6;i++) strcpy((char*)chashtable_get_or_insert(&ht0,&tmpk[i],NULL),tmpv[i]); /* or ...ht0.get_or_insert(&ht0,&tmpk[i])... in the 'fake member function' syntax */
    for (i=9;i>6;i--) strcpy((char*)chashtable_get_or_insert(&ht1,&tmpk[i],NULL),tmpv[i]);

    /* enumerate elements */
    cnt=0;printf("ht0 = {%lu:\t[",chashtable_get_num_items(&ht0));
    for (j=0;j<CH_NUM_USED_BUCKETS;j++) {
        const chvector& b = ht0.buckets[j];if (!b.size) continue;
        for (i=0;i<b.size;i++)   {if (cnt>0) printf(",");printf("ht0[%d]=\"%s\"",b.key_at<ht_key>(i),b.value_at<ht_value>(i));++cnt;}  /* Warning: '.key_at<type>(i)' and '.value_at<type>(i)' are C++ specific */
    }
    printf("]};\n");
    cnt=0;printf("ht1 = {%lu:\t[",chashtable_get_num_items(&ht1));
    for (j=0;j<CH_NUM_USED_BUCKETS;j++) {
        const chvector& b = ht1.buckets[j];if (!b.size) continue;
        for (i=0;i<b.size;i++)   {if (cnt>0) printf(",");printf("ht1[%d]=\"%s\"",b.key_at<ht_key>(i),b.value_at<ht_value>(i));++cnt;}
    }
    printf("]};\n");

    printf("chashtable_swap(&ht1,&ht0):\n");
    chashtable_swap(&ht1,&ht0);

    /* enumerate elements */
    cnt=0;printf("ht0 = {%lu:\t[",chashtable_get_num_items(&ht0));
    for (j=0;j<CH_NUM_USED_BUCKETS;j++) {
        const chvector& b = ht0.buckets[j];if (!b.size) continue;
        for (i=0;i<b.size;i++)   {if (cnt>0) printf(",");printf("ht0[%d]=\"%s\"",b.key_at<ht_key>(i),b.value_at<ht_value>(i));++cnt;}
    }
    printf("]};\n");
    cnt=0;printf("ht1 = {%lu:\t[",chashtable_get_num_items(&ht1));
    for (j=0;j<CH_NUM_USED_BUCKETS;j++) {
        const chvector& b = ht1.buckets[j];if (!b.size) continue;
        for (i=0;i<b.size;i++)   {if (cnt>0) printf(",");printf("ht1[%d]=\"%s\"",b.key_at<ht_key>(i),b.value_at<ht_value>(i));++cnt;}
    }
    printf("]};\n");


    printf("std::vector<chashtable> test:\n");
    vstd.push_back(ht0);
    vstd.push_back(ht1);
    vstd.push_back(ht0);
    vstd.push_back(ht1);

    /* enumerate elements */
    for (t=0;t<vstd.size();t++) {
        const chashtable& ht = vstd[t];
        cnt=0;printf("vstd[%lu] = {%lu:\t[",t,chashtable_get_num_items(&ht));
        for (j=0;j<CH_NUM_USED_BUCKETS;j++) {
            const chvector& b = ht.buckets[j];if (!b.size) continue;
            for (i=0;i<b.size;i++)   {if (cnt>0) printf(",");printf("ht[%d]=\"%s\"",b.key_at<ht_key>(i),b.value_at<ht_value>(i));++cnt;}  /* Warning: '.key_at<type>(i)' and '.value_at<type>(i)' are C++ specific */
        }
        printf("]};\n");
    }


    printf("chashtable_cpy(&ht1,&ht0):\n");
    chashtable_cpy(&ht1,&ht0);

    /* enumerate elements */
    cnt=0;printf("ht0 = {%lu:\t[",chashtable_get_num_items(&ht0));
    for (j=0;j<CH_NUM_USED_BUCKETS;j++) {
        const chvector& b = ht0.buckets[j];if (!b.size) continue;
        for (i=0;i<b.size;i++)   {if (cnt>0) printf(",");printf("ht0[%d]=\"%s\"",b.key_at<ht_key>(i),b.value_at<ht_value>(i));++cnt;}
    }
    printf("]};\n");
    cnt=0;printf("ht1 = {%lu:\t[",chashtable_get_num_items(&ht1));
    for (j=0;j<CH_NUM_USED_BUCKETS;j++) {
        const chvector& b = ht1.buckets[j];if (!b.size) continue;
        for (i=0;i<b.size;i++)   {if (cnt>0) printf(",");printf("ht1[%d]=\"%s\"",b.key_at<ht_key>(i),b.value_at<ht_value>(i));++cnt;}
    }
    printf("]};\n");

    /* chashtable::~chashtable() calls 'chashtable_free(...)' for us (but in plain C it's not available) */

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

