/*
 * File:   ArtificialAgent.c
 * Author: Jarod Wong and Singhung Wat
 *
 */

//CMPE13 Support Library
#include "BOARD.h"

// Microchip libraries
#include <xc.h>
#include <plib.h>

// User libraries
#include "Agent.h"
#include "CircularBuffer.h"
#include "Oled.h"
#include "Buttons.h"
#include "Protocol.h"
#include "Uart1.h"
#include "Field.h"
#include "OledDriver.h"
#include "FieldOled.h"
#include "Leds.h"

#define MAX_BOAT_LENGTH 4
#define OLED_CLEAR(x) OledClear(OLED_COLOR_BLACK); \
        OledDrawString(x); \
        OledUpdate()

#define THINKING() do {\
        unsigned long i; \
        for (i = 0; i < (BOARD_GetPBClock() / 8); i++); \
        } while (0)

static Field myField, enemyField;
static AgentState currentState = AGENT_STATE_GENERATE_NEG_DATA;
static NegotiationData myNegData, oppNegData;
static GuessData myGuessData, oppGuessData;
static AgentEvent event = AGENT_EVENT_NONE;
static ProtocolParserStatus parserStatus = PROTOCOL_WAITING;
static FieldOledTurn turn = FIELD_OLED_TURN_NONE;
static int pastGuessRow, pastGuessCol, hit = 0;

void AgentInit(void)
{
    FieldInit(&enemyField, FIELD_POSITION_UNKNOWN);
    FieldInit(&myField, FIELD_POSITION_EMPTY);

    while (TRUE) {
        if (FieldAddBoat(&myField, (rand() % FIELD_ROWS), (rand() % FIELD_COLS), (rand() % MAX_BOAT_LENGTH), FIELD_BOAT_SMALL))
            break;
    }
    while (TRUE) {
        if (FieldAddBoat(&myField, (rand() % FIELD_ROWS), (rand() % FIELD_COLS), (rand() % MAX_BOAT_LENGTH), FIELD_BOAT_MEDIUM))
            break;
    }
    while (TRUE) {
        if (FieldAddBoat(&myField, (rand() % FIELD_ROWS), (rand() % FIELD_COLS), (rand() % MAX_BOAT_LENGTH), FIELD_BOAT_LARGE))
            break;
    }
    while (TRUE) {
        if (FieldAddBoat(&myField, (rand() % FIELD_ROWS), (rand() % FIELD_COLS), (rand() % MAX_BOAT_LENGTH), FIELD_BOAT_HUGE))
            break;
    }

    FieldOledDrawScreen(&myField, &enemyField, turn); //update for initial display
}

/**
 * The Run() function for an Agent takes in a single character. It then waits until enough
 * data is read that it can decode it as a full sentence via the Protocol interface. This data
 * is processed with any output returned via 'outBuffer', which is guaranteed to be 255
 * characters in length to allow for any valid NMEA0183 messages. The return value should be
 * the number of characters stored into 'outBuffer': so a 0 is both a perfectly valid output and
 * means a successful run.
 * @param in The next character in the incoming message stream.
 * @param outBuffer A string that should be transmit to the other agent. NULL if there is no
 *                  data.
 * @return The length of the string pointed to by outBuffer (excludes \0 character).
 */
int AgentRun(char in, char *outBuffer)
{
    if (in) {
        parserStatus = ProtocolDecode(in, &oppNegData, &oppGuessData);
        switch (parserStatus) {
        case PROTOCOL_WAITING:
            event = AGENT_EVENT_NONE;
            break;
        case PROTOCOL_PARSING_GOOD:
            event = AGENT_EVENT_NONE;
            break;
        case PROTOCOL_PARSED_CHA_MESSAGE:
            event = AGENT_EVENT_RECEIVED_CHA_MESSAGE;
            break;
        case PROTOCOL_PARSED_DET_MESSAGE:
            event = AGENT_EVENT_RECEIVED_DET_MESSAGE;
            break;
        case PROTOCOL_PARSED_HIT_MESSAGE:
            event = AGENT_EVENT_RECEIVED_HIT_MESSAGE;
            break;
        case PROTOCOL_PARSED_COO_MESSAGE:
            event = AGENT_EVENT_RECEIVED_COO_MESSAGE;
            break;
        case PROTOCOL_PARSING_FAILURE:
            event = AGENT_EVENT_MESSAGE_PARSING_FAILED;
            break;
        }
    }

    switch (currentState) {
    case AGENT_STATE_GENERATE_NEG_DATA:
        ProtocolGenerateNegotiationData(&myNegData);
        currentState = AGENT_STATE_SEND_CHALLENGE_DATA;
        return ProtocolEncodeChaMessage(outBuffer, &myNegData);
        break;
    case AGENT_STATE_SEND_CHALLENGE_DATA:
        if (event == AGENT_EVENT_RECEIVED_CHA_MESSAGE) {
            currentState = AGENT_STATE_DETERMINE_TURN_ORDER;
            return ProtocolEncodeDetMessage(outBuffer, &myNegData);
        } else if (event == AGENT_EVENT_MESSAGE_PARSING_FAILED) {
            currentState = AGENT_STATE_INVALID;
            OLED_CLEAR(AGENT_ERROR_STRING_PARSING);
        }
        break;
    case AGENT_STATE_DETERMINE_TURN_ORDER:
        if (event == AGENT_EVENT_RECEIVED_DET_MESSAGE) {
            if (ProtocolValidateNegotiationData(&oppNegData)) {
                TurnOrder flag = ProtocolGetTurnOrder(&myNegData, &oppNegData);
                switch (flag) {
                case TURN_ORDER_START:
                    currentState = AGENT_STATE_SEND_GUESS;
                    turn = FIELD_OLED_TURN_MINE;
                    FieldOledDrawScreen(&myField, &enemyField, turn);
                    break;
                case TURN_ORDER_DEFER:
                    currentState = AGENT_STATE_WAIT_FOR_GUESS;
                    turn = FIELD_OLED_TURN_THEIRS;
                    FieldOledDrawScreen(&myField, &enemyField, turn);
                    break;
                case TURN_ORDER_TIE:
                    currentState = AGENT_STATE_INVALID;
                    OLED_CLEAR(AGENT_ERROR_STRING_PARSING);
                    break;
                }
            } else {
                currentState = AGENT_STATE_INVALID;
                OLED_CLEAR(AGENT_ERROR_STRING_PARSING);
            }
        } else if (event == AGENT_EVENT_MESSAGE_PARSING_FAILED) {
            currentState = AGENT_STATE_INVALID;
            OLED_CLEAR(AGENT_ERROR_STRING_PARSING);
        }
        break;
    case AGENT_STATE_SEND_GUESS:
        THINKING();

        if (!AgentGetStatus()) {
            currentState = AGENT_STATE_LOST;
            break;
        } else if (!AgentGetEnemyStatus()) {
            currentState = AGENT_STATE_WON;
            break;
        }

        while (1) {
            if (FieldAt(&enemyField, myGuessData.row, myGuessData.col) == FIELD_POSITION_HIT) {
                pastGuessRow = myGuessData.row;
                pastGuessCol = myGuessData.col;
                hit = TRUE;
            }

            if (hit) {
                int north = TRUE, south = TRUE, east = TRUE, west = TRUE;
                while (north || south || east || west) {
                    BoatDirection direction = rand() % MAX_BOAT_LENGTH;
                    if (direction == FIELD_BOAT_DIRECTION_NORTH) {
                        if (FieldAt(&enemyField, pastGuessRow, pastGuessCol + 1) == FIELD_POSITION_UNKNOWN) {
                            myGuessData.row = pastGuessRow;
                            myGuessData.col = pastGuessCol + 1;
                            break;
                        }
                        north = FALSE;
                    } else if (direction == FIELD_BOAT_DIRECTION_SOUTH) {
                        if (FieldAt(&enemyField, pastGuessRow, pastGuessCol - 1) == FIELD_POSITION_UNKNOWN) {
                            myGuessData.row = pastGuessRow;
                            myGuessData.col = pastGuessCol - 1;
                            break;
                        }
                        south = FALSE;
                    } else if (direction == FIELD_BOAT_DIRECTION_EAST) {
                        if (FieldAt(&enemyField, pastGuessRow + 1, pastGuessCol) == FIELD_POSITION_UNKNOWN) {
                            myGuessData.row = pastGuessRow + 1;
                            myGuessData.col = pastGuessCol;
                            break;
                        }
                        east = FALSE;
                    } else if (direction == FIELD_BOAT_DIRECTION_WEST) {
                        if (FieldAt(&enemyField, pastGuessRow, pastGuessCol - 1) == FIELD_POSITION_UNKNOWN) {
                            myGuessData.row = pastGuessRow;
                            myGuessData.col = pastGuessCol - 1;
                            break;
                        }
                        west = FALSE;
                    }
                }
            } else {
                myGuessData.row = rand() % FIELD_ROWS;
                myGuessData.col = rand() % FIELD_COLS;
            }
            
            if (FieldAt(&enemyField, myGuessData.row, myGuessData.col) == FIELD_POSITION_UNKNOWN) {
                break;
            }
        }
        
        hit = FALSE;
        currentState = AGENT_STATE_WAIT_FOR_HIT;
        return ProtocolEncodeCooMessage(outBuffer, &myGuessData);
        break;
    case AGENT_STATE_WAIT_FOR_HIT:
        if (!AgentGetEnemyStatus()) {
            currentState = AGENT_STATE_WON;
            turn = FIELD_OLED_TURN_NONE;
            FieldUpdateKnowledge(&enemyField, &oppGuessData);
            FieldOledDrawScreen(&myField, &enemyField, turn);
        } else if (event = AGENT_EVENT_RECEIVED_HIT_MESSAGE) {
            currentState = AGENT_STATE_WAIT_FOR_GUESS;
            turn = FIELD_OLED_TURN_THEIRS;
            FieldUpdateKnowledge(&enemyField, &oppGuessData);
            FieldOledDrawScreen(&myField, &enemyField, turn);
        } else if (event == AGENT_EVENT_MESSAGE_PARSING_FAILED) {
            currentState = AGENT_STATE_INVALID;
            OLED_CLEAR(AGENT_ERROR_STRING_PARSING);
        }
        break;
    case AGENT_STATE_WAIT_FOR_GUESS:
        FieldOledDrawScreen(&myField, &enemyField, turn);

        if (!AgentGetEnemyStatus()) {
            currentState = AGENT_STATE_LOST;
            turn = FIELD_OLED_TURN_NONE;
            FieldOledDrawScreen(&myField, &enemyField, turn);
            return ProtocolEncodeHitMessage(outBuffer, &oppGuessData);
        } else if (event == AGENT_EVENT_RECEIVED_COO_MESSAGE) {
            currentState = AGENT_STATE_SEND_GUESS;
            turn = FIELD_OLED_TURN_MINE;
            FieldRegisterEnemyAttack(&myField, &oppGuessData);
            FieldOledDrawScreen(&myField, &enemyField, turn);
            return ProtocolEncodeHitMessage(outBuffer, &oppGuessData);
        } else if (event == AGENT_EVENT_MESSAGE_PARSING_FAILED) {
            currentState = AGENT_STATE_INVALID;
            OLED_CLEAR(AGENT_ERROR_STRING_PARSING);
        }
        break;
    }

    return 0;
}

uint8_t AgentGetStatus(void)
{
    return FieldGetBoatStates(&myField);
}

uint8_t AgentGetEnemyStatus(void)
{
    return FieldGetBoatStates(&enemyField);
}


/* *****************************************************************************
 End of File
 */
