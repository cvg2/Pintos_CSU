/* Creates a new kernel thread named NAME with the given initial
   PRIORITY, which executes FUNCTION passing AUX as the argument,
   and adds it to the ready queue.  Returns the thread identifier
   for the new thread, or TID_ERROR if creation fails.

   If thread_start() has been called, then the new thread may be
   scheduled before thread_create() returns.  It could even exit
   before thread_create() returns.  Contrariwise, the original
   thread may run for any amount of time before the new thread is
   scheduled.  Use a semaphore or some other form of
   synchronization if you need to ensure ordering.

   The code provided sets the new thread's `priority' member to
   PRIORITY, but no actual priority scheduling is implemented.
   Priority scheduling is the goal of Problem 1-3. */
tid_t
thread_create (const char *name, int priority,
               thread_func *function, void *aux)
{
  struct thread *t;
  struct kernel_thread_frame *kf;
  struct switch_entry_frame *ef;
  struct switch_threads_frame *sf;
  tid_t tid;
  enum intr_level old_level;

  ASSERT (function != NULL);

  /* Allocate thread. */
  t = palloc_get_page (PAL_ZERO);
  if (t == NULL)
    return TID_ERROR;

  /* Initialize thread. */
  init_thread (t, name, priority);
  tid = t->tid = allocate_tid ();

  /* Prepare thread for first run by initializing its stack.
     Do this atomically so intermediate values for the 'stack'
     member cannot be observed. */
  old_level = intr_disable ();

  /* Stack frame for kernel_thread(). */
  kf = alloc_frame (t, sizeof *kf);
  kf->eip = NULL;
  kf->function = function;
  kf->aux = aux;

  /* Stack frame for switch_entry(). */
  ef = alloc_frame (t, sizeof *ef);
  ef->eip = (void (*) (void)) kernel_thread;

  /* Stack frame for switch_threads(). */
  sf = alloc_frame (t, sizeof *sf);
  sf->eip = switch_entry;
  sf->ebp = 0;

  intr_set_level (old_level);

  /* Add to run queue. */
  struct thread *cur = thread_current();
  thread_unblock (t);
  if (t->priority > cur->priority)
        thread_yield();

  return tid;
}
--------------------------------------------------------------------------------------------------------------------
/* Suspend the current thread.  It will not be scheduled
   again until awoken by thread_unblock().
   This function must be called with interrupts turned off.  It
   is usually a better idea to use one of the synchronization
   primitives in synch.h. */
void
thread_block (enum thread_status status)
{
  ASSERT (!intr_context ());
  ASSERT (intr_get_level () == INTR_OFF);
  //thread_current()->status = THRD_BLOCKED;
  thread_current()->status = status;
  schedule ();
}
--------------------------------------------------------------------------------------------------------------------

/* Transitions a blocked thread T to the ready-to-run state.
   This is an error if T is not blocked.  (Use thread_yield() to
   make the running thread ready.)

   This function does not preempt the running thread.  This can
   be important: if the caller had disabled interrupts itself,
   it may expect that it can atomically unblock a thread and
   update other data. */
void
thread_unblock (struct thread *t)
{
  enum intr_level old_level;

  ASSERT (is_thread (t));

  old_level = intr_disable ();
  ASSERT (t->status == THRD_BLOCKED || t->status == THRD_SLEEP);
  //list_push_back (&ready_list, &t->elem);
  list_insert_ordered (&ready_list, &t->elem, sort_priority, NULL);
  t->status = THRD_READY;
  intr_set_level (old_level);
}
--------------------------------------------------------------------------------------------------------------------
/* Yields the CPU.  The current thread is not blocked and
   may be scheduled again immediately at the scheduler's whim. */
void
thread_yield (void)
{
  struct thread *cur = thread_current ();
  enum intr_level old_level;

  ASSERT (!intr_context ());

  old_level = intr_disable ();
  if (cur != idle_thread)
    list_insert_ordered (&ready_list, &cur->elem, sort_priority, NULL);
   // list_push_back (&ready_list, &cur->elem);
  cur->status = THRD_READY;
  schedule ();
  intr_set_level (old_level);
}
--------------------------------------------------------------------------------------------------------------------
/* Sets the current thread's priority to NEW_PRIORITY. */
void
thread_set_priority (int new_priority)
{
  struct thread *cur = thread_current();
  cur->priority = new_priority;
  if (!list_empty (&ready_list)) {
        struct list_elem *e;
        e = list_begin (&ready_list);
        struct thread *t = list_entry (e, struct thread, elem);
        if (cur->priority < t->priority)  thread_yield();
  }
}
--------------------------------------------------------------------------------------------------------------------
/* Returns the current thread's priority. */
int
thread_get_priority (struct thread *t)
{
  return t->priority;
}
