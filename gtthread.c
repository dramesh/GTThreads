#include "gtthread.h"

int init = 0;
static int thread_num = 1;
static int thread_count = 0;

struct itimerval time_slice;
struct sigaction preempt;
static ucontext_t sched_context;

static void signal_handler (int sig_id) {
        if (sig_id == SIGPROF) {
		swapcontext(&current->tContext, &sched_context);
        }
        else return;
}

static void run_thread(void *thread_function(), void *arg) {
	current->running = 1;
	current->retval = thread_function(arg);
        if (!current->exited) {
                current->complete = 1;
                gtthread_cancel(current->tID);
        }
        swapcontext(&current->tContext, &sched_context);
        return;
}

static void schedule () {
        while (thread_count != 0) {
                if (current->complete != 1 && current->exited != 1) {
                        swapcontext(&sched_context, &current->tContext);
                }

                if (thread_count == 1 && main_thread->exited == 1) {
                        exit(0);
                }

		if (current == end) {
			current = first;
		} else {
	        	current = current->next;
		}
         }
}


void gtthread_init (long period) {
	if (getcontext(&sched_context) == 0) {
		sched_context.uc_stack.ss_sp = malloc(STACKCAPACITY);
		sched_context.uc_stack.ss_size = STACKCAPACITY;
		sched_context.uc_stack.ss_flags = 0;
		sched_context.uc_link = NULL;
	} else {
		printf("Error - Initialization of scheduler context failed\n");
		exit(-1);
	}

	time_slice.it_value.tv_sec = 0; 
	time_slice.it_value.tv_usec = (long) period;
        time_slice.it_interval = time_slice.it_value;
	preempt.sa_handler = signal_handler;
        preempt.sa_flags = SA_RESTART | SA_SIGINFO;
        sigemptyset(&preempt.sa_mask);
        if (sigaction(SIGPROF, &preempt, NULL) == -1) {
                printf("Error - Initialization of signal handler failed\n");
                exit(-1);
        }

	main_thread = (struct gtthread*) malloc (sizeof(struct gtthread));
	main_thread->tID = 0; 
	main_thread->running = 0;
        main_thread->complete = 0;
        main_thread->cancel = 0;
        main_thread->next = NULL;
	
	getcontext(&main_thread->tContext);

	if (setitimer(ITIMER_PROF, &time_slice, NULL) != 0) {
        	printf("Error - Initialization of Timer failed\n");
		exit(-1);
	}

	if (init != 1) {
		first = main_thread;
		current = main_thread;
		end = main_thread;
	        thread_count++;
	}
	
	init = 1;
	
        return;
}

int gtthread_create (gtthread_t *thread, void *(*start_routine)(void *), void *arg) {
	if (init != 1) {
		printf("Error - GTThread not initialized\n");
		exit(-1);
	}
	
	*thread = thread_num;
	struct gtthread *new_thread = (struct gtthread*) malloc (sizeof(struct gtthread));
	new_thread->tID = *thread;
	new_thread->running = 0;
	new_thread->complete = 0;
	new_thread->cancel = 0;
	new_thread->next = NULL;
	
	getcontext(&new_thread->tContext);

	new_thread->tContext.uc_link = &sched_context;
	new_thread->tContext.uc_stack.ss_sp = malloc(STACKCAPACITY);
	new_thread->tContext.uc_stack.ss_size = STACKCAPACITY;
	new_thread->tContext.uc_stack.ss_flags = 0;

	if (new_thread->tContext.uc_stack.ss_sp == 0)  {
                return -1;
        }

	makecontext(&new_thread->tContext, (void(*)(void)) run_thread, 2, start_routine, arg);
	
	end->next = new_thread;
	end = new_thread;
        thread_count++;

	if (thread_num++ == 1) {
		current = new_thread;
		schedule();
	} else {
		swapcontext(&current->tContext, &sched_context);
	}

	return 0;
}

int gtthread_cancel(gtthread_t thread) {
	if (first == NULL) {
		return -1;
	}

	struct gtthread *temp = first;
	for (temp = first; temp != NULL; temp = temp->next) {
		if (temp->tID == thread && temp->cancel != 1) {
			free(temp->tContext.uc_stack.ss_sp);
			temp->running = 0;
			temp->cancel = 1;
        		thread_count--;
			break;
		}
	}
        return 0;
}

int gtthread_join(gtthread_t thread, void **status) {
	struct gtthread *temp = first;
	for (temp = first; temp != NULL; temp = temp->next)
                if (temp->tID == thread) break;
	while (temp->exited != 1 && temp->complete != 1 && temp->cancel != 1) {}
        if ((temp->exited || temp->complete || temp->cancel) && status != NULL) {
                *status = (void *)temp->retval;
                return 0;
        }

        return -1;	
}

void gtthread_exit(void *retval) {
	current->retval = retval;
        current->exited = 1;
        if (current != main_thread) gtthread_cancel(current->tID);
        swapcontext(&current->tContext, &sched_context);
        return;
}

void gtthread_yield(void) {
	int flag = 0;
        getcontext(&current->tContext);
        if (flag == 0) {
                flag = 1;
                setcontext(&sched_context);
        }
}

int gtthread_equal(gtthread_t t1, gtthread_t t2) {
	if (t1 == t2) return 1;
        else return 0;
}

gtthread_t gtthread_self(void) {
	return current->tID;
}

int gtthread_mutex_init(gtthread_mutex_t *mutex) {
	if (mutex->lock == 1) return -1;
        mutex->lock = 0;
        mutex->owner = -1;
        return 0;
}

int gtthread_mutex_lock(gtthread_mutex_t *mutex) {
	if ((mutex->owner) == current->tID) {
                return -1;
        }

        while (mutex->lock != 0 && mutex->owner != current->tID) gtthread_yield();

        mutex->lock = 1;
        mutex->owner = current->tID;
        return 0;
}

int gtthread_mutex_unlock(gtthread_mutex_t *mutex) {
	if (mutex->lock == 1 && mutex->owner == current->tID) { 
                mutex->lock = 0;
                mutex->owner = -1;
                return 0;
        }
        return -1;
}
