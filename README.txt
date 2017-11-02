Homework 4: Worst. Homework. Ever.
This homework is due on Tuesday, October 31, at 11:59:59 PM (Eastern daylight time). You must use submit to turn in your homework like so: submit cs421_jtang hw4 hw4.c

Your program must be named hw4.c, and it will be compiled on Ubuntu 16.04 as follows:

  gcc ‐‐std=c99 ‐Wall ‐O2 ‐o hw4 hw4.c hw4_test.c ‐pthread
(Note the above is dash, dash, "std=c99", and the other flags likewise are preceded by dashes.) There must not be any compilation warnings in your submission; warnings will result in grading penalties. In addition, your code must be properly indented and have a file header comment, as described on the coding conventions page.
In this homework, you are writing a memory allocator from scratch. You will implement a worst-fit allocation algorithm. As proof that your code works, your program will run instructor-provided unit tests that will exercise all functions.

Part 1: Memory Region
Your program will simulate a machine's memory system. The machine has 12 page frames, where each frame holds 32 bytes, for 384 bytes total. Declare this "memory" as a global array. When your program starts, initialize its contents to 0. You will also need to design your own memory allocation table.

Implement the following function with this given signature. Note that the comments are formatted using Doxygen codes.

/**
 * Write to standard output information about the current memory
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
void my_malloc_stats(void);
Part 2: Simple Memory Allocation
Next, implement a worst-fit memory allocator. You must implement these functions with these given signatures:

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
void *my_malloc(size_t size);

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
void my_free(void *ptr);
For my_malloc(), you are to implement a worst-fit allocation strategy. Because each page frame is 32 bytes, my_malloc() must round up to the next 32-byte boundary when reserving space, and the returned address must be frame-aligned. Your function allocates using the space reserved in step 1 above.

For my_free(), your program deallocates space that was returned by my_malloc() or my_realloc() (see below). Thus, if my_malloc() previously allocated 5 frames and returned the address to the first frame, calling my_free() will deallocate all 5 frames (not just the first).

Part 3: Advanced Memory Allocation
Next, implement these functions with these given signatures:

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
void *my_realloc(void *ptr, size_t size);

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
size_t my_malloc_usable_size(void *ptr);
  
Part 4: Threading and Documentation
The final step is to ensure that your functions are thread-safe. If a thread is in the middle of any the above functions, another thread calling one of these functions must block.

In your code, add a comment block describing how you track which frames are allocated and how large each memory block is. Specifically, answer these questions:

How many bytes of overhead is consumbed by your memory allocation table? How did you determine that number?
How does my_free() know it needs to raise the SEGFAULT signal when the passed in pointer points into the middle of a memory block?
Part 5: Testing
Within your Ubuntu VM, use the wget command to fetch the unit test code file http://www.csee.umbc.edu/~jtang/cs421.f17/homework/hw4_test.c. This file declares a function with the following signature:

/**
 * Unit test of your memory allocator implementation. This will
 * allocate and free memory regions.
 */
extern void hw4_test(void);
In your main() function, after you have initialized your memory and allocation table, call hw4_test().
The grader will use a different unit test code file during grading. You may not make any assumptions about what hw4_test() will do, other than that its code will compile and that it will not cause compilation warnings. You should consider writing your own hw4_test.c, as that the supplied one does not thoroughly test all edge cases. You may safely assume that the grader will attempt all edge conditions.

Note how this code implements a basic testing framework, that prints what it is about to test, if the test passes or fails, and the sum of passed and failed tests. Read over hw4_test.c; for the projects you will need to write your own unit tests. If you continue your career in software engineering, be familiar with other unit testing frameworks.

Sample Output
Here is a sample output from running the program using the above hw4_test.c.

Test 1: Display initialized memory
Memory contents:
  ................................
  ................................
  ................................
  ................................
  ................................
  ................................
  ................................
  ................................
  ................................
  ................................
  ................................
  ................................
Memory allocations:
  ffffffffffff
Test 2: Simple allocations
Test 3: Simple freeing
Memory contents:
  AAAAAAAAAAAAAAAAAAAAAAAAAAAAAA..
  BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB
  BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB
  BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB
  BBBB............................
  CCCCCCCC........................
  ................................
  ................................
  ................................
  ................................
  ................................
  ................................
Memory allocations:
  fRRRRRffffff
Test 4: Out of memory condition
Test 5: Double-free
Caught signal 11: Segmentation fault: 11!
Memory contents:
  AAAAAAAAAAAAAAAAAAAAAAAAAAAAAA..
  BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB
  BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB
  BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB
  BBBB............................
  CCCCCCCC........................
  ................................
  ................................
  ................................
  ................................
  ................................
  ................................
Memory allocations:
  ffffffffffff
Test 6: Increasing memory
Memory contents:
  DDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDD
  DDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDD
  DDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDD
  DDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDD
  DBBB............................
  DDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDD
  DDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDD
  DDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDD
  DDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDD
  DBBB............................
  ................................
  ................................
Memory allocations:
  fffffRRRRRRf
Test 7: Decreasing memory
Memory contents:
  EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE
  EEEEEEEEDDDDDDDDDDDDDDDDDDDDDDDD
  DDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDD
  DDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDD
  DBBB............................
  DDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDD
  DDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDD
  DDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDD
  DDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDD
  DBBB............................
  ................................
  ................................
Memory allocations:
  RRfffRRRffff
12 tests passed, 0 tests failed.
Other Hints and Notes
Ask plenty of questions on the Blackboard discussion board.
At the top of your submission, list any help you received as well as web pages you consulted. Please do not use any URL shorteners, such as goo.gl or tinyurl.
Only submit hw4.c. The grader will ignore any submitted hw4_test.c.
You may not call malloc()/calloc()/realloc() (or any of its ilk) or free(). Using any built-in memory allocator will result in a zero for this assignment.
You may not change the signatures to any of the above functions.
As that you will be performing pointer arithmetic, be aware of the differences between int, size_t, and ptrdiff_t.
Having trouble rounding up to the next 32-byte interval? See the Linux kernel's very clever ALIGN macro.
