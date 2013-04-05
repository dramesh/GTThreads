Dhananjayan Ramesh
GT ID: 902892838
GTThreads: A preemptive user-level thread package

Platform used:
Ubuntu 12.04.2

Implementation of preemptive scheduler:
The scheduler is a preemptive round robin scheduler which has a queue of threads to be scheduled. The scheduler queue is implemented as a linked list. Any program using this thread library package must call the function gtthread_init() first. The preemption period in microseconds should be specified in gtthread_init(period). For each thread, this period of time is assigned as the time slice for which it is allowed to run and it is preempted if it uses up its quantum. Preemption is achieved by using the signal - SIGPROF as a timer. main() is considered as the 0th thread and its context is saved and it is first added to the queue. The first time gtthread_create() is called, the thread is added to the queue and the scheduler is invoked. The scheduler is invoked only after the creation of the first thread and not with the invocation of main() itself because if the user program does not have any other threads apart from the main thread, it would be unnecessary to run the scheduler for main() alone. Thereafter, every subsequent call to gtthread_create() creates a new context for the corresponding thread, allocates memory and adds it to the queue. The scheduler traverses through the queue in a round robin fashion at the specified time quantum intervals. If a thread gets preempted due to the timer signal, the associated signal handler is called, which saves the thread context and switches back to the scheduler which then switches to the next thread. Context switching is performed by using *context() functions.

How to compile the library and run a program:
1) Extract the tarball into a folder.
2) Run make to generate the library file gtthread.a:
	$ make
3) Compile the program which you want to run (dining.c) with the library and execute it:
	$ gcc dining.c gtthread.a
	$ ./a.out

Prevention of deadlocks in Dining Philosophers solution:
If the philosophers take one chopstick at a time, taking a chopstick from the left and then one from the right, there is danger of deadlock, with all the philosophers holding one chopstick and waiting for his/her right chopstick that is currently being locked by his/her right neighbor. To try to avoid this, a philosopher tries to grab a chopstick by executing a mutex_lock() on that chopstick and releases the chopsticks by executing mutex_unlock() on the appropriate chopsticks (left/right). The chopsticks are released by the philosophers in the order in which they were acquired. This solution guarantees that no two neighbors are eating simultaneously which reduces the possibility of deadlock occurence Also, the odd numbered philosophers acquire the left chopstick first and then the right, while the even numbered philosophers acquire the chopsticks in the reverse order which avoids deadlock.

Few thoughts:
Developing this library provided a deep insight into the internals of thread library implementation, which was a great experience. Implementation of semaphores is going to be the next task in my pipeline after which I am planning to implement a preemptive priority scheduler. Also, one other thing to be looked at is the following issue. Whenever the library was tested with test programs that contain a very large number of print statements inside the threads it spawned, the printing seemed to stop after a while and the program did not terminate.
