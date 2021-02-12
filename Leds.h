/* 
 * File:   Leds.h
 * Author: Singhung (Wilson) Wat
 */

#ifndef LEDS_H
#define	LEDS_H

#include "BOARD.h"
#include <xc.h>
#include <plib.h>

#define LEDS_INIT() do { \
    TRISE = 0; \
    LATE = 0; \
} while(0)

#define LEDS_GET() LATE

#define LEDS_SET(x) LATE = (x)

#endif	/* LEDS_H */