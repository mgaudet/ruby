#include <stdio.h>
#include <stdlib.h>
#include "listener.h"
#include "ruby.h"

/**
 * Event names.
 */
const char * listener_event_names[] = {
   "GENERIC",
   "BOP_REDEFINITION",
   "OP_CODE_EXECUTION",
   "CONSTANT_REDEFINITION",
   "DEFINE_CLASS",
   "DEFINE_METHOD",
   NULL
};


/**
 * A listener entry is a triple: {event_type, static_data, notify_function}
 *
 * When a particular listener fires, its notify_function is called with the
 * listener_entry itself passed as an argument. In addition, if desired, more
 * data about the event can be passed to the notify function.
 */
struct listener_entry {
   enum listener_event event;
   void *data;
   void (*notify_function)(enum listener_event event, void* listener_data, void* event_data);
};

/**
 * For now we limit to MAX_LISTENERS_PER_TYPE to keep memory usage under control,
 * and to ensure timely execution of notification.
 */
#define MAX_LISTENERS_PER_TYPE 16

/**
 * State structure for keeping track of the set of listener entries
 */
struct listener_set {
   int registered_listeners[LISTENER_LAST];

   /* For prototyping purposes, I'm going do dump the listeners into a statically
    * allocated fixed size array.
    */
   struct listener_entry listeners[LISTENER_LAST][MAX_LISTENERS_PER_TYPE];

   /**
    * For statistics gathering purposes.
    */
   long fire_count[LISTENER_LAST];
};

/**
 * Global listener manager
 */
static struct listener_set listener_manager;

int rb_vm_register_listener(enum listener_event event,
                            void *listener_data,
                            void (*listener_function)(enum listener_event, void*, void*)) {

   int index = listener_manager.registered_listeners[event];
   struct listener_entry * listener_slot = &(listener_manager.listeners[event][index]);

   /* Don't overflow the listener table. Just decline to register a new listener */
   if (listener_manager.registered_listeners[event] == MAX_LISTENERS_PER_TYPE) {
      return 0;
   }

   /* Don't register a listener function that's NULL. This perhaps ought to be
    * a VM_ASSERT instead.
    */
   if (listener_function == 0) {
      return 0;
   }

   listener_slot->event           = event;
   listener_slot->data            = listener_data;
   listener_slot->notify_function = listener_function;

   /* Next listener goes in next slot */
   listener_manager.registered_listeners[event]++;

   return 1;
}

/**
 * Notify each listener for the event type.
 */
void rb_vm_notify_listeners(enum listener_event event, void* event_data) {
   int i, registered_listeners;
   struct listener_entry * listener_slot;
   registered_listeners = listener_manager.registered_listeners[event];
   for (i = 0; i < registered_listeners; i++) {
      listener_slot = &(listener_manager.listeners[event][i]);
      listener_slot->notify_function(event, listener_slot->data, event_data);
   }

   /* Update statistics */
   listener_manager.fire_count[event] += registered_listeners;
   return;
}

/**
 * A basic listener useful for debugging.
 * @param event_type    The event type
 * @param listener_data A string which is looked up via getenv to determine activation.
 * @param event_data    Another string to print when the lister fires.
 */
void echo_listener(enum listener_event event, void* listener_data, void* event_data) {
   char* envvar = (char*)listener_data;
   char* str    = (char*)event_data;
   if (getenv(envvar)) {
      fprintf(stderr, "%s\n", str);
   }
}

/* Trace Buffer support.
 *
 * This listener code will be invoked for every notification to allow
 * retrospective debugging
 */


/**
 * Single trace entry
 *
 * For now, only the event an an entry ID to indicate sequencing.
 */
struct listener_trace_entry {
   int entry_id;
   enum listener_event event;
};

/**
 * Size of event buffer.
 */
#define LISTENER_TRACE_BUFFER_SIZE 512

/**
 * trace of listener events. Can be dumped for problem determination reasons
 */
struct listener_trace_buffer {
   int index;
   struct listener_trace_entry entries[LISTENER_TRACE_BUFFER_SIZE];
};

/**
 * Global trace buffer
 */
static struct listener_trace_buffer trace_buffer;

/**
 * Fills in a circular trace buffer. Useful for post-hoc debugging.
 */
void trace_listener(enum listener_event event, void* listener_data, void* event_data) {
   int index = trace_buffer.index++;
   struct listener_trace_entry * entry = &(trace_buffer.entries[index % LISTENER_TRACE_BUFFER_SIZE]);
   entry->entry_id = index;
   entry->event = event;
   return;
}

/**
 * Register a trace listener for all events.
 */
void register_trace_listeners(void) {
   int i = 0;
   for (; i < LISTENER_LAST; i++) {
      rb_vm_register_listener((enum listener_event)i, NULL, trace_listener);
   }
}

/*
 * Return a dictionary of listener statistics.
 */
VALUE listener_statistics(void) {
   int i;
   VALUE h = rb_hash_new();
   for (i = 0; i < LISTENER_LAST; i++) {
      rb_hash_aset(h, rb_str_new_cstr(listener_event_names[i]), INT2FIX(listener_manager.fire_count[i]));
   }
   return h;
}
