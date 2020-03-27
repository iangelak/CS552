#define FALSE 0
#define TRUE 1


typedef unsigned char uint8;
typedef unsigned short int uint16;
typedef unsigned long int uint32;
typedef unsigned long long int uint64;

typedef signed char sint8, s8;
typedef signed short int sint16, s16;
typedef signed long int sint32, s32;
typedef signed long long int sint64, s64;


#ifndef _SIZE_T
typedef int size_t;
#define _SIZE_T 1
#endif

typedef signed char bool;

typedef unsigned long uint;
typedef signed long sint;

#ifndef _STDINT_
#define _STDINT_
typedef uint8 uint8_t;
typedef uint16 uint16_t;
typedef uint32 uint32_t;
typedef uint64 uint64_t;
#endif

#define MAX_THREADS 2


/* PCB for a process/thread */

struct proc_crtl_block{
	int tid;
	int *bp;
	int (*task)();
	int flag;
	struct proc_crtl_block *next;
	int *sp;
};


typedef struct proc_crtl_block pcb;
