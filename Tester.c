// **** Include libraries here ****
// Standard libraries

//CMPE13 Support Library
#include "BOARD.h"

// Microchip libraries
#include <xc.h>
#include <plib.h>

// User libraries
#include "Buttons.h"
#include "Oled.h"
#include "Protocol.h"

// **** Set any macros or preprocessor directives here ****

// **** Declare any data types here ****

// **** Define any module-level, global, or external variables here ****
static uint32_t counter;
static uint8_t buttonEvents;

// **** Declare any function prototypes here ****



int main()
{
    BOARD_Init();

    // Configure Timer 2 using PBCLK as input. We configure it using a 1:16 prescalar, so each timer
    // tick is actually at F_PB / 16 Hz, so setting PR2 to F_PB / 16 / 100 yields a 10ms timer.
    OpenTimer2(T2_ON | T2_SOURCE_INT | T2_PS_1_16, BOARD_GetPBClock() / 16 / 100);

    // Set up the timer interrupt with a medium priority of 4.
    INTClearFlag(INT_T2);
    INTSetVectorPriority(INT_TIMER_2_VECTOR, INT_PRIORITY_LEVEL_4);
    INTSetVectorSubPriority(INT_TIMER_2_VECTOR, INT_SUB_PRIORITY_LEVEL_0);
    INTEnable(INT_T2, INT_ENABLED);

    // Disable buffering on stdout
    setbuf(stdout, NULL);

    ButtonsInit();

    OledInit();

    // Prompt the user to start the game and block until the first character press.
    OledDrawString("Press BTN4 to start.");
    OledUpdate();
    while ((buttonEvents & BUTTON_EVENT_4UP) == 0);


/******************************************************************************
 * Your code goes in between this comment and the following one with asterisks.
 *****************************************************************************/
    char string[PROTOCOL_MAX_MESSAGE_LEN];
    
    GuessData x;
    x.row = 134;
    x.col = 312;
    x.hit = HIT_SUNK_MEDIUM_BOAT;
    
    NegotiationData y;
    y.encryptedGuess = 1;
    y.encryptionKey = 2;
    y.guess = 3;
    y.hash = 4;
    
    NegotiationData a;
    GuessData b;
    
    printf("%i: %s", ProtocolEncodeCooMessage(string, &x), string);
    printf("%i: %s", ProtocolEncodeHitMessage(string, &x), string);
    printf("%i: %s", ProtocolEncodeChaMessage(string, &y), string);
    printf("%i: %s", ProtocolEncodeDetMessage(string, &y), string);
    
    int i;
    for (i = 0; string[i]; i++) {
        ProtocolDecode(string[i], &a, &b);
    }
    
    ProtocolGenerateNegotiationData(&a);
    ProtocolGenerateNegotiationData(&y);
    
    TurnOrder foo = ProtocolGetTurnOrder(&a, &y);
    
    printf("TurnOrder: %d", foo);
/******************************************************************************
 * Your code goes in between this comment and the preceeding one with asterisks
 *****************************************************************************/

    while (1);
}



/**
 * This is the interrupt for the Timer2 peripheral. It just keeps incrementing a counter used to
 * track the time until the first user input.
 */
void __ISR(_TIMER_2_VECTOR, IPL4AUTO) TimerInterrupt100Hz(void)
{
    // Clear the interrupt flag.
    IFS0CLR = 1 << 8;

    // Increment a counter to see the srand() function.
    counter++;

    // Also check for any button events
    buttonEvents = ButtonsCheckEvents();
}