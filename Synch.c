/*Global variable bool*/
bool higher_priority_thread_unblocked = false;
-------------------------------------------------------------------------------------------------------------------
/* Down or "P" operation on a semaphore.  Waits for SEMA's value
t   to become positive and then atomically decrements it.

   This function may block the caller, so it must not be called within an
   interrupt handler.  This function may be called with
   interrupts disabled, but if it is blocked then the next scheduled
   thread will probably turn interrupts back on. */
void
sema_down (struct semaphore *sema)
{
  enum intr_level old_level;

  ASSERT (sema != NULL);
  ASSERT (!intr_context ());

  old_level = intr_disable ();
  while (sema->value == 0)
    {
      list_insert_ordered (&sema->waiters, &thread_current ()->elem, sort_priority, NULL);
     // list_push_back (&sema->waiters, &thread_current ()->elem);
      thread_block (THRD_BLOCKED);
    }
  sema->value--;
  intr_set_level (old_level);
}
/* Up or "V" operation on a semaphore.  Increments SEMA's value
   and wakes up one thread of those waiting for SEMA, if any.

   This function may be called from an interrupt handler. */
void
sema_up (struct semaphore *sema)
{
  enum intr_level old_level;

  higher_priority_thread_unblocked = false;
  ASSERT (sema != NULL);
  old_level = intr_disable ();
  if (!list_empty (&sema->waiters)) {
    struct thread *t = list_entry (list_pop_front (&sema->waiters), struct thread, elem);
    struct thread *cur = thread_current ();
    thread_unblock (t);
    if(cur->priority < t->priority)
         higher_priority_thread_unblocked = true;
  }

  sema->value++;
  intr_set_level (old_level);
  if ( higher_priority_thread_unblocked)
        thread_yield();
}
 
