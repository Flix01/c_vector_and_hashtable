/* 
-> plain C compilation:
gcc -O2 -no-pie -fno-pie -I"../../include/deprecated" main.c -o main

-> C++ compilation:
gcc -x c++ -O2 -no-pie -fno-pie -I"../../include/deprecated" main.c -o main
[or just: g++ -O2 -no-pie -fno-pie -I"../../include/deprecated" main.c -o main]

-> run 
./main

-> output: 

VECTOR TEST:
mystruct_vector[0]={	100,	-20,	50};
mystruct_vector[1]={	-10,	200,	-5};
mystruct_vector[2]={	3,	-300,	0};
Removed item 1.
mystruct_vector[0]={	100,	-20,	50};
mystruct_vector[1]={	3,	-300,	0};

SORTED VECTOR TEST:
mystruct_vector[0]={	50,	10,	80};
mystruct_vector[1]={	100,	-50,	80};
mystruct_vector[2]={	100,	10,	80};
mystruct_vector[3]={	200,	-50,	80};
mystruct_vector[4]={	500,	10,	80};
item={	100,	10,	80}	found at mystruct_vector[2].
item={	100,	10,	100}	not found.

HASHTABLE TEST:
Added item myht[	100,	50,	25]	["100-50-25"].
Added item myht[	5,	43,	-250]	["5-43-250"].
Added item myht[	10,	250,	125]	["10-250-125"].
Fetched item myht[	10,	250,	125]	["10-250-125"].
All items (generally unsorted):
0) myht[	5,	43,	-250]	["5-43-250"].
1) myht[	10,	250,	125]	["10-250-125"].
2) myht[	100,	50,	25]	["100-50-25"].
Removed item [	10,	250,	125]
An item with key[	10,	250,	125] is NOT present.


-> Extra stuff: mingw and (an old) cl compiler command-lines
x86_64-w64-mingw32-gcc -mconsole -O2 -I"../../include/deprecated" main.c -o main_mingw.exe
cl /O2 /MT /Tc /I"../../include/deprecated" main.c /link /out:main_vc.exe user32.lib kernel32.lib

*/

/*#define CVH_NUM_HTUINT 32*/  /* optional; must be in [1,65536]. Default is 256. */
#include "c_vector_and_hashtable.h"
#include <stdio.h> /* printf */

/* stuff needed for the vector test */
struct mystruct {int a,b,c;};
const size_t mystructsizeinbytes = sizeof(struct mystruct);
/* this will be used for the sorted vector test */
static int mystruct_cmp(const void* pa,const void* pb) {
	const struct mystruct* a = (const struct mystruct*) pa;
	const struct mystruct* b = (const struct mystruct*) pb;
	if (a->a>b->a) return 1;
	else if (a->a<b->a) return -1;
	if (a->b>b->b) return 1;
	else if (a->b<b->b) return -1;
	if (a->c>b->c) return 1;
	else if (a->c<b->c) return -1;
	return 0;
}

/* stuff needed for the hashtable test (basically we use 'a,b,c' as key and 'value' as value) */
struct myhtitem_key_value {int a,b,c;char value[16];};
const size_t myhtitem_key_value_sizeinbytes = sizeof(struct myhtitem_key_value);
/* 'myhtitem_key_value' key sorter */
static int myhtitem_key_cmp(const void* pa,const void* pb) {
	/* MANDATORY: only and ALL the 'key' part of the struct MUST be used here (and NOT the 'value' part!!!). */
	const struct myhtitem_key_value* a = (const struct myhtitem_key_value*) pa;
	const struct myhtitem_key_value* b = (const struct myhtitem_key_value*) pb;
	if (a->a>b->a) return 1;
	else if (a->a<b->a) return -1;
	if (a->b>b->b) return 1;
	else if (a->b<b->b) return -1;
	if (a->c>b->c) return 1;
	else if (a->c<b->c) return -1;
	return 0;
}
/* 'myhtitem_key_value' key hash  */
static CVH_API_INL cvh_htuint myhtitem_key_hash(const void* item)  {
	/* MANDATORY: only the 'key' part of the struct (or a sub-part of it) MUST be used here (and NOT the 'value' part!!!).	*/
	return (cvh_htuint)(((const struct myhtitem_key_value *)item)->a
#   if (CVH_NUM_HTUINT!=256 && CVH_NUM_HTUINT!=65536) /* return value should be in [0,CVH_NUM_HTUINT), but this is already guaranteed in these two cases */
				%CVH_NUM_HTUINT
#	endif
				);
}



int main(int argc,char* argv[])
{
size_t i,j,k;

/*-----------------------------------------------------------------------------------------*/
/* vector test */
/*-----------------------------------------------------------------------------------------*/

/* Mandatory to define: 'mystruct_vector','mystruct_vector_size' and 'mystruct_vector_capacity_in_items' */
struct mystruct *mystruct_vector=NULL;
size_t mystruct_vector_size = 0, mystruct_vector_capacity_in_items = 0;

/*------------------------------------------------------------------*/
/* vector test */
/*------------------------------------------------------------------*/
{
struct mystruct * mystruct_p =NULL;	/* just a ptr we'll use later...*/

printf("VECTOR TEST:\n");

/* add capacity for three (more) items */
cvh_vector_reserve((void**)&mystruct_vector,mystruct_vector_size+3,&mystruct_vector_capacity_in_items,mystructsizeinbytes);

/* fill the first new item (and increment 'mystruct_vector_size'): */              
mystruct_p = &mystruct_vector[mystruct_vector_size++];
mystruct_p->a=100;mystruct_p->b=-20;mystruct_p->c=50;

/* fill the second new item (and increment 'mystruct_vector_size'): */
mystruct_p = &mystruct_vector[mystruct_vector_size++];
mystruct_p->a=-10;mystruct_p->b=200;mystruct_p->c=-5;

/* fill the third new item (and increment 'mystruct_vector_size'): */
mystruct_p = &mystruct_vector[mystruct_vector_size++];
mystruct_p->a=3;mystruct_p->b=-300;mystruct_p->c=0;

/* display all items */
for (i=0;i<mystruct_vector_size;i++) printf("mystruct_vector[%lu]={\t%d,\t%d,\t%d};\n",i,mystruct_vector[i].a,mystruct_vector[i].b,mystruct_vector[i].c);

/* remove element 1 */
if (cvh_vector_remove_at((void*)mystruct_vector,mystructsizeinbytes,mystruct_vector_size,1))	{
	--mystruct_vector_size;	/* MANDATORY: 'mystruct_vector_size' is handled by the user */
	printf("Removed item 1.\n");
}

/* note that if you want to clear 'mystruct_vector', you can just set 'mystruct_vector_size' to zero */

/* display all items */
for (i=0;i<mystruct_vector_size;i++) printf("mystruct_vector[%lu]={\t%d,\t%d,\t%d};\n",i,mystruct_vector[i].a,mystruct_vector[i].b,mystruct_vector[i].c);

/* delete vector */
cvh_free(mystruct_vector);mystruct_vector=NULL;mystruct_vector_size=mystruct_vector_capacity_in_items=0;
}

/*---------------------------------------------------------------------*/
/* sorted vector test */
/*---------------------------------------------------------------------*/
{
struct mystruct mystruct_item = {200,-50,80};
struct mystruct* mystruct_p = NULL;	/* just a ptr we'll use later... */
size_t position = 0;int match=0;

printf("\nSORTED VECTOR TEST:\n");

/* add capacity for 5 (more) items */
cvh_vector_reserve((void**)&mystruct_vector,mystruct_vector_size+5,&mystruct_vector_capacity_in_items,mystructsizeinbytes);

/* IMPORTANT: before calling 'cvh_vector_insert_sorted', 'mystruct_vector' my have space for at least one MORE item to insert. */
/* we add 5 items to the sorted vector (please note that we increment 'mystruct_vector_size' each call): */
mystruct_item.a=200;cvh_vector_insert_sorted(mystruct_vector,mystructsizeinbytes,mystruct_vector_size++,&mystruct_item,&mystruct_cmp,NULL,1);
mystruct_item.a=100;cvh_vector_insert_sorted(mystruct_vector,mystructsizeinbytes,mystruct_vector_size++,&mystruct_item,&mystruct_cmp,NULL,1);
mystruct_item.b=10;cvh_vector_insert_sorted(mystruct_vector,mystructsizeinbytes,mystruct_vector_size++,&mystruct_item,&mystruct_cmp,NULL,1);
mystruct_item.a=50;cvh_vector_insert_sorted(mystruct_vector,mystructsizeinbytes,mystruct_vector_size++,&mystruct_item,&mystruct_cmp,NULL,1);
mystruct_item.a=500;cvh_vector_insert_sorted(mystruct_vector,mystructsizeinbytes,mystruct_vector_size++,&mystruct_item,&mystruct_cmp,NULL,1);
/* last 2 arguments are (int* match=NULL,int insert_even_if_item_match=1): we set the latter to 1 (true), so that it's safe to always (post) increment 'mystruct_vector_size' on each call. */
/* otherwise we should pass a pointer to an int, '&match', and increment 'mystruct_vector_size' only if it's 1 */
/* Also it can be useful to know that 'cvh_vector_insert_sorted' returns a size_t insertion 'position'. */

/* display all elements */
for (i=0;i<mystruct_vector_size;i++) printf("mystruct_vector[%lu]={\t%d,\t%d,\t%d};\n",i,mystruct_vector[i].a,mystruct_vector[i].b,mystruct_vector[i].c);

/* now we can serch the sorted vector for certain items this way */
mystruct_item.a=100;mystruct_item.b=10;mystruct_item.c=80;
position = cvh_vector_binary_search(mystruct_vector,mystructsizeinbytes,mystruct_vector_size,&mystruct_item,&mystruct_cmp,&match);
printf("item={\t%d,\t%d,\t%d}\t",mystruct_item.a,mystruct_item.b,mystruct_item.c);
if (match) printf("found at mystruct_vector[%lu].\n",position);
else printf("not found.\n");

mystruct_item.a=100;mystruct_item.b=10;mystruct_item.c=100;
position = cvh_vector_binary_search(mystruct_vector,mystructsizeinbytes,mystruct_vector_size,&mystruct_item,&mystruct_cmp,&match);
printf("item={\t%d,\t%d,\t%d}\t",mystruct_item.a,mystruct_item.b,mystruct_item.c);
if (match) printf("found at mystruct_vector[%lu].\n",position);
else printf("not found.\n");

/* note that to remove an item, once we have 'position' 
[returned by 'cvh_vector_insert_sorted(...)' or by 'cvh_vector_binary_search(...)'] 
we can use 'cvh_vector_remove_at(...)' like above */

/* delete vector */
cvh_free(mystruct_vector);mystruct_vector=NULL;mystruct_vector_size=mystruct_vector_capacity_in_items=0;

} 

/*---------------------------------------------------------------------*/
/* hashtable test */
/*---------------------------------------------------------------------*/
{
	cvh_hashtable_t* myht=NULL;
	struct myhtitem_key_value item_to_search,*fetched_item;
	int match=0;

	printf("\nHASHTABLE TEST:\n");

	/* create hashtable  'myht' (must be freed with 'cvh_hashtable_free(...)') */
	myht = cvh_hashtable_create(myhtitem_key_value_sizeinbytes,&myhtitem_key_hash,&myhtitem_key_cmp,0);	/* last arg is 'initial_bucket_capacity_in_items' */
    
	/* Fetch 'item_to_search', and, if not found, insert it and give it a 'value' */
    item_to_search.a = 100;item_to_search.b = 50;item_to_search.c = 25;    /* we don't need to set 'value' here */
	fetched_item = (struct myhtitem_key_value*) cvh_hashtable_get_or_insert(myht,&item_to_search,&match);CVH_ASSERT(fetched_item); /* return-value casting is required for C++ compilation only. */
    if (!match)  {
		/* item was not present, so we assign 'value' */
		strcpy(fetched_item->value,"100-50-25");	
		printf("Added ");
	}
	else printf("Fetched ");
	printf("item myht[\t%d,\t%d,\t%d]\t[\"%s\"].\n",fetched_item->a,fetched_item->b,fetched_item->c,fetched_item->value);

    /* Fetch 'item_to_search', and, if not found, insert it and give it a 'value' */
    item_to_search.a = 5;item_to_search.b = 43;item_to_search.c = -250;    /* we don't need to set 'value' here */
	fetched_item = (struct myhtitem_key_value*) cvh_hashtable_get_or_insert(myht,&item_to_search,&match);CVH_ASSERT(fetched_item); /* return-value casting is required for C++ compilation only */
	if (!match)  {
		/* item was not present, so we assign 'value' */
		strcpy(fetched_item->value,"5-43-250");	
		printf("Added ");
	}
	else printf("Fetched ");
	printf("item myht[\t%d,\t%d,\t%d]\t[\"%s\"].\n",fetched_item->a,fetched_item->b,fetched_item->c,fetched_item->value);
		
    /* Fetch 'item_to_search', and, if not found, insert it and give it a 'value' */
    item_to_search.a = 10;item_to_search.b = 250;item_to_search.c = 125;    /* we don't need to set 'value' here */
	fetched_item = (struct myhtitem_key_value*) cvh_hashtable_get_or_insert(myht,&item_to_search,&match);CVH_ASSERT(fetched_item); /* return-value casting is required for C++ compilation only */
	if (!match)  {
		/* item was not present, so we assign 'value' */
		strcpy(fetched_item->value,"10-250-125");	
		printf("Added ");
	}
	else printf("Fetched ");
	printf("item myht[\t%d,\t%d,\t%d]\t[\"%s\"].\n",fetched_item->a,fetched_item->b,fetched_item->c,fetched_item->value);
	
    /* Just fetch 'item_to_search', without inserting it if not found (this call can return NULL) */
    fetched_item = (struct myhtitem_key_value*) cvh_hashtable_get(myht,&item_to_search); /* return-value casting is required for C++ compilation only */
    if (!fetched_item)  {
		/* item is not present */
		printf("An item with key[\t%d,\t%d,\t%d] is NOT present.\n",item_to_search.a,item_to_search.b,item_to_search.c);
	}
	else printf("Fetched item myht[\t%d,\t%d,\t%d]\t[\"%s\"].\n",fetched_item->a,fetched_item->b,fetched_item->c,fetched_item->value);

	/* IMPORTANT: of course we CAN'T change the 'key' part of any of the fetched items (items returned by 'cvh_hashtable_get(...)' or 'cvh_hashtable_get_or_insert(...)') 
       and pretend that the hashtable does not break! (The 'key' part defines the position of the item inside the hashtable).
       But we can change any other member (including 'value' if we want).
	*/

	/* Display all entries (in general they are not sorted) */
	k=0;printf("All items (generally unsorted):\n");	
	for (i=0;i<CVH_NUM_HTUINT;i++)	{
		const cvh_hashtable_vector_t* bucket = &myht->buckets[i];
		if (!bucket->p)	continue;
		for (j=0;j<bucket->num_items;j++)	{
			const struct myhtitem_key_value* pitem = (const struct myhtitem_key_value*) ((const unsigned char*)bucket->p+myht->item_size_in_bytes*j);
			printf("%lu) myht[\t%d,\t%d,\t%d]\t[\"%s\"].\n",k++,pitem->a,pitem->b,pitem->c,pitem->value);		
		}
	}

	/* Remove 'item_to_search' */
	if (cvh_hashtable_remove(myht,&item_to_search)) printf("Removed item [\t%d,\t%d,\t%d]\n",item_to_search.a,item_to_search.b,item_to_search.c);
	else printf("Can't remove item [\t%d,\t%d,\t%d], because it's not present.\n",item_to_search.a,item_to_search.b,item_to_search.c);

    /* Just fetch 'item_to_search', without inserting it if not found (this call can return NULL) */
    fetched_item = (struct myhtitem_key_value*) cvh_hashtable_get(myht,&item_to_search); /* return-value casting is required for C++ compilation only */
    if (!fetched_item)  {
		/* item is not present */
		printf("An item with key[\t%d,\t%d,\t%d] is NOT present.\n",item_to_search.a,item_to_search.b,item_to_search.c);
	}
	else printf("Fetched item myht[\t%d,\t%d,\t%d]\t[\"%s\"].\n",fetched_item->a,fetched_item->b,fetched_item->c,fetched_item->value);

	/* Debug function that can be useful to detect sorting errors and optimize 'myhtitem_key_hash' (to reduce the standard deviation)
	and 'CVH_NUM_HTUINT' (but the latter is unique for all the 'cvh_hashtable_t' instances) */
	/*cvh_hashtable_dbg_check(myht);*/

	/* destroy hashtable 'myht' */
	cvh_hashtable_free(myht);myht=NULL;
}

return 0;
}

