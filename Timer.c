/*List of sleep threads sorted by wake up time */
static struct list sleep_threads;

/*Compare wake up time of two threads*/
static bool sort_wake_up_time(const struct list_elem *el1,const struct list_elem *el2>
  struct thread *t1= list_entry(el1,struct thread, elem);
  struct thread *t2= list_entry(el2,struct thread, elem);
  return (t1->wake_up_time < t2->wake_up_time);
}
--------------------------------------------------------------------------------------------------------------------
/* Sleeps for approximately TICKS timer ticks.  Interrupts must
   be turned on. */
void
timer_sleep (int64_t ticks)
{
  int64_t start = timer_ticks ();
  struct thread *cur = thread_current();
  cur->wake_up_time = start + ticks;
  enum intr_level old_level = intr_disable();
  list_insert_ordered(&sleep_threads, &cur->elem, sort_wake_up_time, NULL);
  thread_block(THRD_SLEEP);
  intr_set_level(old_level);
}
--------------------------------------------------------------------------------------------------------------------
/* Timer interrupt handler. */
static void
timer_interrupt (struct intr_frame *args UNUSED)
{
  struct list_elem *el = list_begin(&sleep_threads);
  ticks++;
  while(el != list_end(&sleep_threads)) {
        struct thread *thrd = list_entry(el,struct thread, elem);
        struct list_elem *temp_elem;
        if (thrd->wake_up_time > ticks) break;
        temp_elem = el;
        el = list_next(el);
        list_remove(temp_elem);
        thread_unblock(thrd);
  }
  thread_tick ();
}
