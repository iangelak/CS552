#include "types.h"
#include "threads.h"
#include "vga.h"
#include "schedule.h"

extern pcb fifos_threads[MAX_THREADS];
extern pcb * runqueue;

static uint32_t dstack[1024]; 			//dummy stack for the 1st context switch


pcb * get_current_thread()
{
	return current->next;
}
void schedule () {
	int prev_tid;
	

	/*TODO*/
	/*
	 * One case for the initial context switch : current_tid == -1
	 * One case when traversing the runqeue
	 * One case when the last thread exits and there are no more threads to run
	 */

	for (;;)
	{
		if ( current->next == 0 ) // we haven't chosen one yet or nothing in the queue anymore
		{
			current->next = runqueue->next; // the one that is going to run now

			// Create a dummy first context to pass to swtch
			struct context *dummy = (struct context *) (&dstack[1023] - sizeof(struct context *));
			// Check if we have an available in the queue
			if (current->next != 0)
			{
				swtch(&dummy, fifos_threads[(current->next)->tid].ctx);
			}
			// We' never get here
		}
		else {
			int prev_id = current->next->tid;
			if ( current->next->next == 0 )
			{
				// we reached the end of list
				//  go back to the beginning again
				current->next = runqueue->next;
			}
			else
			{
				current->next = current->next->next;
			}
			runqueue_remove(fifos_threads[prev_id]);
			print_s("Context switching to the next thread\n");
			swtch(&fifos_threads[prev_tid].ctx, &fifos_threads[current->next->tid].ctx);
		}

	}

}
