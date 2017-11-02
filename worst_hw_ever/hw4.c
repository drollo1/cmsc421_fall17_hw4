//***********************************************************
//File Name: hw4.c
//Author Name: Dominic Rollo
//Assignment: Homework 4
//
//  
//
//**************Outside Help*********************************
//  
//
//***********************************************************
//**************Questions************************************
//  1.
//***********************************************************
//#define _BSD_SOURCE

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <signal.h>
//#include <string.h>
//#include <unistd.h>
//#include <sys/types.h>
//#include <sys/wait.h>
#include <pthread.h>

extern void hw4_test(void);

#define PAGES 12
#define PAGE_SIZE 32

char static main_memory[PAGES][PAGE_SIZE];
int static mem_table[PAGES];

//**************************************************************
//  Initilizes the memory to zeros and lables memory as free
//**************************************************************
static void initilize(void){
	for(int i=0;i<PAGES;i++)
		for(int j=0;j<PAGE_SIZE;j++)
			main_memory[i][j]=0;

	for(int i=0;i<PAGES;i++){
		mem_table[i]=-1;
	}
}

/**
 * Write to standard input information about the current memory
 * allocations.
 *
 * Display to standard output the following:
 * - Memory contents, one frame per line, 12 lines total. Display the
 *   actual bytes stored in memory. If the byte is unprintable (ASCII
 *   value less than 32 or greater than 126), then display a dot
 *   instead.
 * - Current memory allocation table, one line of 12 columns. For each
 *   column, display a 'f' if the frame is free, 'R' if reserved.
 */
void my_malloc_stats(void){
	printf("Memory contents:\n");
	for(int i=0;i<PAGES;i++){
		printf("  ");
		for(int j=0;j<PAGE_SIZE;j++){
			if(main_memory[i][j]>=32&&main_memory[i][j]<=126)
				printf("%c", main_memory[i][j]);
			else
				printf(".");
		}
		printf("\n");
	}
	printf("Memory allocations:\n  ");
	for (int i=0;i<PAGES; i++){
		if(mem_table[i]==-1)
			printf("f");
		else
			printf("R");
	}
	printf("\n");

	//*******************************TEST Mark<<<<<<<<<
	printf("  ");
	for(int i=0;i<PAGES; i++)
		if(mem_table[i]==-1)
			printf("*");
		else
			printf("%d", mem_table[i]);
	printf("\n");
	//*************************************************
}

/**
 * Allocate and return a contiguous memory block that is within the
 * memory region. The allocated memory remains uninitialized.
 *
 * Search through all of available for the largest free memory region,
 * then return a pointer to the beginning of the region. If multiple
 * regions are the largest, return a pointer to the region closest to
 * address zero.
 *
 * The size of the returned block will be at least @a size bytes,
 * rounded up to the next 32-byte increment.
 *
 * @param size Number of bytes to allocate. If @c 0, your code may do
 * whatever it wants; my_malloc() of @c 0 is "implementation defined",
 * meaning it is up to you if you want to return @c NULL, segfault,
 * whatever.
 *
 * @return Pointer to allocated memory, or @c NULL if no space could
 * be found. If out of memory, set errno to @c ENOMEM.
 */
void *my_malloc(size_t size){
	if(size==0){
		return NULL;
	}
	unsigned int pages;
	if(size%32==0)
		pages = size/32;
	else
		pages = (size/32)+1;
	int large=-1;
	int large_size=0;
	int temp=-1;
	for(int i=0;i<PAGES;i++){
		if(temp==-1&&mem_table[i]==-1)
			temp=i;
		else if(temp!=-1&&mem_table[i]!=-1){
			if((i-temp)>large_size){
				large = temp;
				large_size = i-temp;
			}
			temp = -1;
		}
	}
	if(temp!=-1&&(12-temp)>large_size){
		large = temp;
		large_size = 12-temp;
	}

	if (pages>large_size){
		errno = ENOMEM;
		return NULL;
	}

	for(int i=large; i<large+pages; i++)
		mem_table[i]=large;

	return &main_memory[large];
}


//*************************************************************
//  returns a -1 if the pointer is not the start of a block 
//*************************************************************
static int is_start(void *ptr){
	for(int i=0;i<PAGES; i++){
		if(mem_table[i]==i&&ptr==&main_memory[i])
			return i;
	}
	return -1;
}

//*************************************************************
//   returns number of pages in block
//*************************************************************
static int block_size(int start){
	int size=0;
	for(int i=start;mem_table[i]==start;i++)
		size++;
	return size;
}

/**
 * Retrieve the size of an allocation block.
 *
 * If @a ptr is a pointer returned by my_malloc() or my_realloc(),
 * then return the size of the allocation block. Because my_malloc()
 * and my_realloc() round up to the next 32-byte increment, the
 * returned value may be larger than the originally requested amount.
 *
 * @return Usable size pointed to by @a ptr, or 0 if @a ptr is not a
 * pointer returned by my_malloc() or my_realloc() (such as @c NULL).
 */
size_t my_malloc_usable_size(void *ptr){
	int temp=is_start(ptr);
	if(ptr==NULL||temp==-1)
		return 0;
	temp = block_size(temp);
	return temp*32;
}

/**
 * Deallocate a memory region that was returned by my_malloc() or
 * my_realloc().
 *
 * If @a ptr is not a pointer returned by my_malloc() or my_realloc(),
 * then raise a SIGSEGV signal to the calling thread. Likewise,
 * calling my_free() on a previously freed region results in a
 * SIGSEGV.
 *
 * @param ptr Pointer to memory region to free. If @c NULL, do
 * nothing.
 */
void my_free(void *ptr){
	if(ptr!=NULL){
		int temp=is_start(ptr);
		if (temp>-1){
			int size = block_size(temp);
			for(int i=temp;i<temp+size;i++)
				mem_table[i]=-1;
		}
		else
			raise(SIGSEGV);
	}
}

/**
 * Change the size of the memory block pointed to by @a ptr.
 *
 * - If @a ptr is @c NULL, then treat this as if a call to
 *   my_malloc() for the requested size.
 * - Else if @a size is @c 0, then treat this as if a call to
 *   my_free().
 * - Else if @a ptr is not a pointer returned by my_malloc() or
 *   my_realloc(), then send a SIGSEGV signal to the calling process.
 *
 * Otherwise reallocate @a ptr as follows:
 *
 * - If @a size is smaller than the previously allocated size, then
 *   reduce the size of the memory block. Mark the excess memory as
 *   available. Memory sizes are rounded up to the next 32-byte
 *   increment.
 * - If @a size is the same size as the previously allocated size,
 *   then do nothing.
 * - If @a size is greater than the previously allocated size, then
 *   allocate a new contiguous block of at least @a size bytes,
 *   rounded up to the next 32-byte increment. Copy the contents from
 *   the old to the new block, then free the old block.
 *
 * @param ptr Pointer to memory region to reallocate.
 * @param size Number of bytes to reallocate.
 *
 * @return If allocating a new memory block or if resizing a block,
 * then pointer to allocated memory; @a ptr will become invalid. If
 * freeing a memory region or if allocation fails, return @c NULL. If
 * out of memory, set errno to @c ENOMEM.
 */
void *my_realloc(void *ptr, size_t size){
	if(ptr==NULL)
		return my_malloc(size);
	else if(ptr!=NULL&&size==0)
		free(ptr);
	else{
		int temp =is_start(ptr);
		int bl_size = block_size(temp);
		if(temp==-1)
			raise(SIGSEGV);
		else{
			int pages;
			if((size%32)==0)
				pages = size/32;
			else
				pages = size/32 +1;
			if(bl_size<pages){
				void *temp_ptr=my_malloc(size);
				int new = is_start(temp_ptr);
				for(int i=0; i<bl_size;i++)
					for(int j=0;j<PAGE_SIZE;j++)
						main_memory[new+i][j]=main_memory[temp+i][j];
				my_free(ptr);
				return temp_ptr;
			}else if(bl_size>pages){
				printf("Shrinking block size: %d pages: %d\n", bl_size, pages);
				for(int i=0;i<bl_size-pages;i++)
					mem_table[temp+pages+i]=-1;
			}

		}
	}
}

int main(int argc, char *argv[]){
	initilize();
	/*
	mem_table[0]=0;
	mem_table[3]=3;
	mem_table[4]=3;
	mem_table[8]=8;
	my_malloc_stats();
	my_malloc(120);
	*/
	hw4_test();

	return 0;
}