/*
 * File:   Protocol.c
 * Author: Singhung Wat
 *
 */

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "Protocol.h"
#include "BOARD.h"

typedef enum {
    WAITING,
    RECORDING,
    FIRST_CHECKSUM_HALF,
    SECOND_CHECKSUM_HALF,
    NEWLINE
} DecodeState;

unsigned char CalculateChecksum(char *string);
int CharToHex(char x);

int ProtocolEncodeCooMessage(char *message, const GuessData *data)
{
    char payload[PROTOCOL_MAX_PAYLOAD_LEN];
    unsigned char checksum = 0;

    sprintf(payload, PAYLOAD_TEMPLATE_COO, data->row, data->col);

    checksum = CalculateChecksum(payload);

    sprintf(message, MESSAGE_TEMPLATE, payload, checksum);

    return strlen(message);
}

int ProtocolEncodeHitMessage(char *message, const GuessData *data)
{
    char payload[PROTOCOL_MAX_PAYLOAD_LEN];
    unsigned char checksum = 0;

    sprintf(payload, PAYLOAD_TEMPLATE_HIT, data->row, data->col, data->hit);

    checksum = CalculateChecksum(payload);

    sprintf(message, MESSAGE_TEMPLATE, payload, checksum);

    return strlen(message);
}

int ProtocolEncodeChaMessage(char *message, const NegotiationData *data)
{
    char payload[PROTOCOL_MAX_PAYLOAD_LEN];
    unsigned char checksum = 0;

    sprintf(payload, PAYLOAD_TEMPLATE_CHA, data->encryptedGuess, data->hash);

    checksum = CalculateChecksum(payload);

    sprintf(message, MESSAGE_TEMPLATE, payload, checksum);

    return strlen(message);
}

int ProtocolEncodeDetMessage(char *message, const NegotiationData *data)
{
    char payload[PROTOCOL_MAX_PAYLOAD_LEN];
    unsigned char checksum = 0;

    sprintf(payload, PAYLOAD_TEMPLATE_DET, data->guess, data->encryptionKey);

    checksum = CalculateChecksum(payload);

    sprintf(message, MESSAGE_TEMPLATE, payload, checksum);

    return strlen(message);
}

ProtocolParserStatus ProtocolDecode(char in, NegotiationData *nData, GuessData *gData)
{
    static DecodeState currentState = WAITING;
    static int index = 0;
    static char payload[PROTOCOL_MAX_PAYLOAD_LEN];
    static unsigned char checksum = 0;

    switch (currentState) {
    case WAITING:
        if (in == '$') {
            currentState = RECORDING;
            index = 0;
            return PROTOCOL_PARSING_GOOD;
        } else {
            return PROTOCOL_WAITING;
        }
        break;
    case RECORDING:
        if (in == '*') {
            currentState = FIRST_CHECKSUM_HALF;
            return PROTOCOL_PARSING_GOOD;
        } else {
            payload[index++] = in;
            return PROTOCOL_PARSING_GOOD;
        }
        break;
    case FIRST_CHECKSUM_HALF:
        if (in >= '0' && in <= '9' || in >= 'a' && in <= 'f' || in >= 'A' && in <= 'F') {
            checksum = CharToHex(in);
            checksum = checksum << 4;
            currentState = SECOND_CHECKSUM_HALF;
            return PROTOCOL_PARSING_GOOD;
        } else {
            currentState = WAITING;
            return PROTOCOL_PARSING_FAILURE;
        }
        break;
    case SECOND_CHECKSUM_HALF:
        if (in >= '0' && in <= '9' || in >= 'a' && in <= 'f' || in >= 'A' && in <= 'F') {
            checksum |= CharToHex(in);
            if (checksum == CalculateChecksum(payload)) {
                currentState = NEWLINE;
                return PROTOCOL_PARSING_GOOD;
            }
        }
        currentState = WAITING;
        return PROTOCOL_PARSING_FAILURE;
        break;
    case NEWLINE:
        currentState = WAITING;
        if (in == '\n') {
            char *tok;
            tok = strtok(payload, ",");
            if (!strncmp(tok, "COO", 3)) {
                gData->row = atoi(strtok(NULL, ","));
                gData->col = atoi(strtok(NULL, ","));
                return PROTOCOL_PARSED_COO_MESSAGE;
            } else if (!strncmp(tok, "HIT", 3)) {
                gData->row = atoi(strtok(NULL, ","));
                gData->col = atoi(strtok(NULL, ","));
                gData->hit = atoi(strtok(NULL, ","));
                return PROTOCOL_PARSED_HIT_MESSAGE;
            } else if (!strncmp(tok, "CHA", 3)) {
                nData->encryptedGuess = atoi(strtok(NULL, ","));
                nData->hash = atoi(strtok(NULL, ","));
                return PROTOCOL_PARSED_CHA_MESSAGE;
            } else if (!strncmp(tok, "DET", 3)) {
                nData->guess = atoi(strtok(NULL, ","));
                nData->encryptionKey = atoi(strtok(NULL, ","));
                return PROTOCOL_PARSED_DET_MESSAGE;
            }
        }
        return PROTOCOL_PARSING_FAILURE;
        break;
    }
}

void ProtocolGenerateNegotiationData(NegotiationData *data)
{
    data->encryptionKey = rand() & 0xFFFF;
    data->guess = rand() & 0xFFFF;
    data->encryptedGuess = data->encryptionKey ^ data->guess;
    data->hash = 0;
    data->hash ^= (data->encryptionKey & 0xFF00) >> 8;
    data->hash ^= (data->guess & 0xFF00) >> 8;
    data->hash ^= data->encryptionKey & 0xFF;
    data->hash ^= data->guess & 0xFF;
}

uint8_t ProtocolValidateNegotiationData(const NegotiationData *data)
{
    int actualEncryptedGuess = data->encryptionKey ^ data->guess;
    int actualHash = 0;
    actualHash ^= (data->encryptionKey & 0xFF00) >> 8;
    actualHash ^= (data->guess & 0xFF00) >> 8;
    actualHash ^= data->encryptionKey & 0xFF;
    actualHash ^= data->guess & 0xFF;

    return (actualEncryptedGuess == data->encryptedGuess && actualHash == data->hash);
}

/**
 * This function returns a TurnOrder enum type representing which agent has won precedence for going
 * first. The value returned relates to the agent whose data is in the 'myData' variable. The turn
 * ordering algorithm relies on the XOR() of the 'encryptionKey' used by both agents. The least-
 * significant bit of XOR(myData.encryptionKey, oppData.encryptionKey) is checked so that if it's a
 * 1 the player with the largest 'guess' goes first otherwise if it's a 0, the agent with the
 * smallest 'guess' goes first. The return value of TURN_ORDER_START indicates that 'myData' won,
 * TURN_ORDER_DEFER indicates that 'oppData' won, otherwise a tie is indicated with TURN_ORDER_TIE.
 * There is no checking for NULL pointers within this function.
 * @param myData The negotiation data representing the current agent.
 * @param oppData The negotiation data representing the opposing agent.
 * @return A value from the TurnOrdering enum representing which agent should go first.
 */
TurnOrder ProtocolGetTurnOrder(const NegotiationData *myData, const NegotiationData *oppData)
{
    if ((myData->encryptionKey ^ oppData->encryptionKey) & 0x1) {
        if (myData->guess > oppData->guess)
            return TURN_ORDER_START;
        else if (myData->guess < oppData->guess)
            return TURN_ORDER_DEFER;
        else
            return TURN_ORDER_TIE;
    } else {
        if (myData->guess < oppData->guess)
            return TURN_ORDER_START;
        else if (myData->guess > oppData->guess)
            return TURN_ORDER_DEFER;
        else
            return TURN_ORDER_TIE;
    }
}

unsigned char CalculateChecksum(char *string)
{
    unsigned char checksum = 0;
    while (*string) {
        checksum ^= *string;
        string++;
    }

    return checksum;
}

// Converts input char to a hex integer, assuming the input is valid

int CharToHex(char x)
{
    if (x >= '0' && x <= '9') {
        return x - '0';
    } else if (x >= 'a' && x <= 'f') {
        return x - 'a' + 10;
    } else if (x >= 'A' && x <= 'F') {
        return x - 'A' + 10;
    } else {
        return 0;
    }
}