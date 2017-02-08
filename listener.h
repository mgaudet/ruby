#ifndef RUBY_LISTENER_H
#define RUBY_LISTENER_H 1

#include "ruby/ruby.h" /* for VALUE */

/*
 * An event listener system for CRuby. This allows components to register their
 * interest in particular events occurring.
 */

/**
 * Defined listener types
 */
enum listener_event {
   LISTENER_GENERIC,               /* for debug / unspecified purposes */
   LISTENER_BOP_REDEFINITION,      /* for basic op redefinition */
   LISTENER_OP_CODE_EXECUTION,
   LISTENER_CONSTANT_REDEFINITION,
   LISTENER_DEFINE_CLASS,
   LISTENER_DEFINE_METHOD,
   LISTENER_LAST,
};


/**
 * Registers a particular Listener
 *
 * Note: Listeners should not depend on a particular order of listener evaluation.
 *
 * @param event                 The event this listener gets called on
 * @param listener_data         A pointer to data the listener function will need to execute.
 *                              Depedent on the listener function, may be NULL.
 *
 * @param listener_function     Invoked when an event of 'type' happens.
 *
 *
 * Returns 1 on success, and 0 otherwise.
 */
int rb_vm_register_listener(enum listener_event event,
                            void *listener_data,
                            void (*listener_function)(enum listener_event event,
                                                      void* listener_data,
                                                      void* event_data));


/**
 * Notify listeners.
 *
 * @param event         The event listener types to notify
 * @param event_data    The data to pass to the listener.
 */
void rb_vm_notify_listeners(enum listener_event event, void* event_data);

/* 
 * For registering trace listeners on all events. 
 *
 * Should be called only once.
 */
void register_trace_listeners(void); 

/*---- Listener Functions --------*/

/**
 * A basic listener useful for debugging.
 * @param event_type    The event type
 * @param listener_data A string which is looked up via getenv to determine activation.
 * @param event_data    Another string to print when the lister fires.
 */
void echo_listener(enum listener_event event, void* listener_data, void* event_data);

/*
 * To pass more data across the void* barrier we can define some structures
 * here.
 */

/**
 * For communicating information about BOP redefinition we need to communicate both
 * the BOP and the flag being set
 */
struct bop_redefinition_data {
   int bop;
   int flag;
};

/**
 * For communicating information about constant redefinition.
 */
struct constant_redefinition_data {
   VALUE klass;
   ID    id;
   VALUE old_value;
   VALUE new_value;
};

/**
 * For communicating information about class definition.
 */
struct class_definition_data {
   VALUE klass; /* The defined klass */
   ID    id;    /* ID of defined class */
   VALUE super; /* Super class */
};

#endif /* RUBY_LISTENER_H */
