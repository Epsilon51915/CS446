#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>

typedef struct _mblock_t {
struct _mblock_t * prev;
struct _mblock_t * next;
size_t size;
int status;
void * payload;
} mblock_t;

typedef struct _mlist_t {
mblock_t * head;
} mlist_t;

#define MBLOCK_HEADER_SZ offsetof(mblock_t, payload)
#define MAX(a, b) ((a) > (b) ? (a) : (b))

mlist_t list;

mblock_t * findFirstOfSize(size_t size)
{
    mblock_t *cur = list.head;
    while(cur != NULL)
    {
        if(cur->size < size && cur->status == 0)
        {
            return cur;
        }
        cur = cur->next;
    }
    return NULL;
}

mblock_t* findLastBlock()
{
    mblock_t* cur = list.head;
    if(cur == NULL)
    {
        return NULL;
    }
    while(cur->next != NULL)
    {
        cur = cur->next;
    }
    return cur;
}

mblock_t* makeNewBlock(size_t size)
{
    void* newaddr = sbrk(MAX(size + MBLOCK_HEADER_SZ, 1024));
    if(*(int*)newaddr != -1)
    {
        mblock_t* last = findLastBlock();
        mblock_t* newblock = (mblock_t*)newaddr; // reinterperet new address as mblockt
        newblock->prev = last;
        newblock->next = NULL;
        newblock->size = size;
        newblock->status = 0;
        if(last == NULL)
        {
            list.head = newblock;
        }
        else
        {
            last->next = newblock;
        }
        return newblock;
    }
    return NULL;
}

void* mymalloc(size_t size)
{
    mblock_t *block = findFirstOfSize(size);
    if(list.head == NULL)
    {
        //printf("Make first element\n");
        block = makeNewBlock(size);
        if(block == NULL)
        {
            return NULL;
        }
    }
    else if (block==NULL)
    {
        block = makeNewBlock(size);
        if(block == NULL)
        {
            return NULL;
        }
    }   

    if(size == block->size)
    {
        block->status = 1;
    }
    else
    {
        void* endaddr = &block + MBLOCK_HEADER_SZ + block->size;
        mblock_t* remaining_block = (mblock_t*)endaddr;
        remaining_block->next = block->next;
        remaining_block->prev = block;
        remaining_block->size = block->size - size;
        remaining_block->status = 0;
        block->next = remaining_block;
        block->next->prev = remaining_block;
        block->size = size;
        block->status = 1;
    }

    return &block->payload;
}

void myfree(void* ptr)
{
    //printf("%p\n", (void*)ptr);
    void* main_block_loc = (char*)ptr - MBLOCK_HEADER_SZ;
    //printf("%p\n", (void*)main_block_loc);
    mblock_t* main_block = (mblock_t*)main_block_loc;
    // Coalesce with next block
    //printf("%p\n", (void*)main_block);
    if(main_block->next != NULL)
    {
        //printf("next not null\n");
        if(main_block->next->status == 0)
        {
            mblock_t* second = main_block->next;
            second->next->prev = main_block;
            main_block->next = second->next;
            main_block->size += second->size; 
        }
    }
    
    // Coalesce with previous block
    if(main_block->prev != NULL)
    {
        //printf("prev not null\n");
        if(main_block->prev->status == 0)
        {
            mblock_t* first = main_block->prev;
            main_block->prev = first;
            first->next = main_block->next;
            first->size += main_block->size;
            main_block = first;
        }
    }
    //printf("stat 0\n");
    //printf("%d\n", main_block->status);
    main_block->status = 0;
    //printf("%d\n", main_block->status);
}

void printMemList(const mblock_t* head) {
  const mblock_t* p = head;
  
  size_t i = 0;
  while(p != NULL) {
    printf("[%ld] p: %p\n", i, (void*)p);
    printf("[%ld] p->size: %ld\n", i, p->size);
    printf("[%ld] p->status: %s\n", i, p->status > 0 ? "allocated" : "free");
    printf("[%ld] p->status: %d\n", i, p->status);
    printf("[%ld] p->prev: %p\n", i, (void*)p->prev);
    printf("[%ld] p->next: %p\n", i, (void*)p->next);
    printf("___________________________\n");
    ++i;
    p = p->next;
  }
  printf("===========================\n");
}

int main(int argc, char* argv[])
{
    //printf("%ld\n", MBLOCK_HEADER_SZ);
    //printf("begin\n");
    void * p1 = mymalloc(10);
    //printf("%p\n", (void*)p1);

    //printf("8=========8=========8=========8=========8\n");
    printMemList(list.head);
    void * p2 = mymalloc(100);
    //printf("8=========8=========8=========8=========8\n");
    printMemList(list.head);
    void * p3 = mymalloc(200);
    //printf("8=========8=========8=========8=========8\n");
    printMemList(list.head);
    void * p4 = mymalloc(500);
    //printf("8=========8=========8=========8=========8\n");
    printMemList(list.head);

    myfree(p3);
    //printf("8=========8=========8=========8=========8\n");
    printMemList(list.head);
    p3 = NULL;
    myfree(p2); 
    //printf("8=========8=========8=========8=========8\n");
    printMemList(list.head);
    p2 = NULL;

    void * p5 = mymalloc(150);
    //printf("8=========8=========8=========8=========8\n");
    printMemList(list.head);
    void * p6 = mymalloc(500);
    //printf("8=========8=========8=========8=========8\n");
    printMemList(list.head);

    myfree(p4); 
    //printf("8=========8=========8=========8=========8\n");
    printMemList(list.head);
    p4 = NULL;
    myfree(p5);
    //printf("8=========8=========8=========8=========8\n");
    printMemList(list.head);
    p5 = NULL;
    myfree(p6);
    //printf("8=========8=========8=========8=========8\n");
    printMemList(list.head);
    p6 = NULL;
    myfree(p1);
    //printf("8=========8=========8=========8=========8\n");
    printMemList(list.head);
    p1 = NULL;

    //printf("Prog End\n");
}