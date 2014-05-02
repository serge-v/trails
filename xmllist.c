#include <stdio.h>
#include <stdlib.h>
#include <libxml/list.h>

struct data
{
    int id;
};

static int print_id(const void* data, const void* user)
{
    xmlListPtr p = (xmlListPtr)user;
    struct data* d = (struct data*)data;
    
    printf("%d %p\n", d->id, d);
}

int compare_data(const void * data0, const void * data1)
{
    struct data* d0 = (struct data*)data0;
    struct data* d1 = (struct data*)data1;
    if (d0->id > d1->id)
        return 1;
    if (d0->id < d1->id)
        return -1;
    return 0;
}

int main()
{
    int i;
    
    xmlListPtr list = xmlListCreate(NULL, compare_data);
    struct data* ptrs[10];

    for (i = 9; i >= 0; i--)
    {
        struct data* p = malloc(sizeof(struct data));
        ptrs[i] = p;
    }

    for (i = 0; i < 10; i++)
    {
        ptrs[i]->id = i;
        xmlListAppend(list, ptrs[i]);
    }
    
    xmlListWalk(list, print_id, list);
    printf("\n");
}
