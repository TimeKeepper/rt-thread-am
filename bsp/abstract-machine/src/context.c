#include <am.h>
#include <klib.h>
#include <rtthread.h>
#include <stddef.h>

struct rt_cont_switch{
  rt_ubase_t from;
  rt_ubase_t to;
};

static Context* switch_handler(Context *c){
  rt_thread_t thread = rt_thread_self();
  struct rt_cont_switch *cont_switch = (struct rt_cont_switch *)(thread->user_data);
  rt_ubase_t from = cont_switch->from;
  rt_ubase_t to = cont_switch->to;

  if(from) *(Context**)from = c;
  return *(Context**)to;
}

static Context* ev_handler(Event e, Context *c) {
  switch (e.event) {
    case EVENT_YIELD: c = switch_handler(c); break;
    default: printf("Unhandled event ID = %d\n", e.event); assert(0);
  }
  return c;
}

void __am_cte_init() {
  cte_init(ev_handler);
}

void rt_hw_context_switch(rt_ubase_t from, rt_ubase_t to) {
  rt_thread_t thread = rt_thread_self();
  rt_ubase_t user_data_cache = thread->user_data;

  struct rt_cont_switch* cont_switch = malloc(sizeof(struct rt_cont_switch));
  cont_switch->from = from;
  cont_switch->to = to;

  thread->user_data = (rt_ubase_t)cont_switch;

  yield();

  thread->user_data = user_data_cache;
}

void rt_hw_context_switch_to(rt_ubase_t to) {
  rt_hw_context_switch(0, to);
}


void rt_hw_context_switch_interrupt(void *context, rt_ubase_t from, rt_ubase_t to, struct rt_thread *to_thread) {
  assert(0);
}

rt_uint8_t *rt_hw_stack_init(void *tentry, void *parameter, rt_uint8_t *stack_addr, void *texit) {
  stack_addr -= (uintptr_t)stack_addr % sizeof(uintptr_t);
  Context * con = kcontext((Area){stack_addr, stack_addr}, tentry, parameter);
  con->texit = texit;
  return (rt_uint8_t *)con;
}
