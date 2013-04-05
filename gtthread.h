#include <stdio.h>
#include <stdlib.h>
#include <ucontext.h>
#include <unistd.h>
#include <signal.h>
#include <sys/time.h>
#include <malloc.h>

#define gtthread_t unsigned long int
#define STACKCAPACITY 40000 

struct gtthread {
	int running;
	int complete;
	int exited;
	int cancel;
	gtthread_t tID;
	ucontext_t tContext;
	void *retval;
	struct gtthread *next;
} *first, *main_thread, *current, *end;

typedef struct gtthread_mutex_t {
	long lock;
	gtthread_t owner;
} gtthread_mutex_t;

void gtthread_init(long period);
int  gtthread_create(gtthread_t *thread, void *(*start_routine)(void *), void *arg);
int  gtthread_join(gtthread_t thread, void **status);
void gtthread_exit(void *retval);
void gtthread_yield(void);
int  gtthread_equal(gtthread_t t1, gtthread_t t2);
int  gtthread_cancel(gtthread_t thread);
gtthread_t gtthread_self(void);
int  gtthread_mutex_init(gtthread_mutex_t *mutex);
int  gtthread_mutex_lock(gtthread_mutex_t *mutex);
int  gtthread_mutex_unlock(gtthread_mutex_t *mutex);
