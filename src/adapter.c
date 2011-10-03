/**
 * The adapter-pattern-based wrapper for interfacing C to D.
 * This is a collection of extern global variables
 * needed to translate D types to C in order to abstract them. This should have
 * been in a header file, but 'extern' is initialisation and therefore invalid
 * there.
 */  

#include "adapter.h"

extern int Turn;
