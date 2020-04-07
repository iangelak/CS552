#include "threads.h"
#include "helper.h"
#include "multiboot.h"
#include "vga.h"
#include "schedule.h"
#include "types.h"
#include "pic.h"
/*
	Store the usable memory regions in an array for
	future use. Used the same methodology as memos-2
*/



unsigned long base_addr[10];
unsigned long end_addr[10];
int mem_pointer[10];
int cnt = 0;   // Memory Region Counter

extern int CountDown;

pcb fifos_threads[MAX_THREADS];

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

void test(void){
	PIC_sendEOI();
	print_s("yo");
	return;
}



void kmain (multiboot_info_t* mbt, unsigned long magic) {
	
	int i=0;

	/*if (mbt->flags & 0b1000000){*/
		/*mmap_entry_t* ent = (mmap_entry_t*) mbt->mmap_addr;*/
		/*avail_regs = get_mem_regs(mbt,ent);*/
	/*}*/
	
	/* Initialize the PCB pool*/
//	pcb_init();
	

	/* Initialize 8259 PIC */
	 init_pic();

	/* Initialize 8254 PIT */
	 init_pit();

	/* Creating autostarting threads */

	terminal_initialize();

	
//	asm ("int $0x20");
	print_s("Creating Threads!!!\n");
	
	init_threads();
	
	schedule();
	


	return ;


}
