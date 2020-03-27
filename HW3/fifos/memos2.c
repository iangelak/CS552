#include "multiboot.h"
#include "crtn_sample.h"

/*
	Store the usable memory regions in an array for
	future use. Used the same methodology as memos-2
*/
unsigned long base_addr[10];
unsigned long end_addr[10];
int mem_pointer[10];
int cnt = 0;   // Memory Region Counter

/*
 * 	We assume that the total memory is below 4GB
 */ 
unsigned long get_mem_regs(multiboot_info_t* mbt, mmap_entry_t* ent){
	
	while( ent < mbt->mmap_addr + mbt->mmap_length){
		// Case we hit the 4GB boundary
		if (ent->base_low + ent->len_low == 0)
			break;

		if (ent->type == 1)// Usable memory
		{
			base_addr[cnt] = ent->base_low;
			end_addr[cnt] = ent->base_low + ent->len_low;
			mem_pointer[cnt] = ent->base_low;
			cnt++;
		}

		ent = (mmap_entry_t*) ((unsigned int) ent + ent->size + sizeof(ent->size));
	}

	
	cnt--;
	return cnt; 
}



/*
	Custom malloc implementation
*/

void *my_malloc(int size){
	int j;
	unsigned long temp;

	for (j = cnt; j >= 0; j--){
		if ( (mem_pointer[j] + size) <= end_addr[j]){
			temp = mem_pointer[j];
			mem_pointer[j] += size;
			return (void*)temp;
		}
	}

	return 0;
}


/*
	The function that is going to create the threads!!!!!
*/


int thread_create(void) {

  int i;
  rq *ptr, *pptr;

  /* Build a "runqueue" */
  f[0] = thread1;
 // f[1] = thread2;

  // Setup runqueue head
  head.task = f[0];
  head.tid = 0;
  head.next = 0;
  head.prev = 0;

  // Add any additional threads after the first
  for (i = 1; i < MAX_THREADS; i++) {
    ptr = (rq *) my_malloc((int)sizeof(rq));

    if (i == 1) {
      head.next = ptr;
      pptr = &head;
    }
    else { 
      pptr = pptr->next;
      pptr->next = ptr;
    }

    ptr->prev = pptr;

    ptr->task = f[i];
    ptr->tid = i;
    ptr->next = &head; // Wraparound
    head.prev = ptr;
  }
  
  // Let's get going...
  schedule ();

  return 0;
}





void kmain (multiboot_info_t* mbt, unsigned long magic) {
	print("\n\n\n\n\n\n\n\n");
	int avail_regs;          // # of available memory regions
	

	print("Getting memory regions!!!!\n");

	if (mbt->flags & 0b1000000){
		mmap_entry_t* ent = (mmap_entry_t*) mbt->mmap_addr;
		avail_regs = get_mem_regs(mbt,ent);
	}
	
	/* Initialize the PCB pool*/
//	pcb_init();
	

	/* Initialize 8259 PIC */
//	pic_init();

	/* Initialize 8254 PIT */
// 	pit_init();

	/* Creating autostarting threads */
//	threads_init();

	print("Creating Threads!!!\n");
	thread_create();


	return ;


}