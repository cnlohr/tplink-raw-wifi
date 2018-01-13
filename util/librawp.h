#ifndef _LIBRAWP_H
#define _LIBRAWP_H

#include <stdint.h>

//Tool for arbitrary monitor + control.
//It is heavily platform-dependent.  Different dongles will need different version of the .c file.

typedef void (*librawp_cb_t)( void * id, void * rr, uint8_t * data, int dlen );

int librawp_setup( const char * interface, int channel );

int librawp_receive( int rawp, librawp_cb_t cb, void * id, int is_wifi );

#endif


