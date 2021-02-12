/*
 * File:   Field.c
 * Author: Jarod Wong
 *
 */
#include "Field.h"
#include "FieldOled.h"
#include "Oled.h"
#include "OledDriver.h"
#include "Protocol.h"

void FieldInit(Field *f, FieldPosition p)
{
    int row;
    int col;
    for (row = 0; row < FIELD_ROWS; row++) {
        for (col = 0; col < FIELD_COLS; col++) {
            f->field[row][col] = p;
        }
    }
    f->smallBoatLives = FIELD_BOAT_LIVES_SMALL;
    f->mediumBoatLives = FIELD_BOAT_LIVES_MEDIUM;
    f->largeBoatLives = FIELD_BOAT_LIVES_LARGE;
    f->hugeBoatLives = FIELD_BOAT_LIVES_HUGE;
}

FieldPosition FieldAt(const Field *f, uint8_t row, uint8_t col)
{
    return f->field[row][col];
}

FieldPosition FieldSetLocation(Field *f, uint8_t row, uint8_t col, FieldPosition p)
{
    FieldPosition old = f->field[row][col];
    f->field[row][col] = p;
    return old;
}

uint8_t FieldAddBoat(Field *f, uint8_t row, uint8_t col, BoatDirection dir, BoatType type)
{
    int boundchecks;
    uint8_t temprow;
    uint8_t tempcol;

    //checks the direction
    if (dir == FIELD_BOAT_DIRECTION_NORTH) {
        // checks what boat and updates the field according to row and col and size
        if (type == FIELD_BOAT_SMALL) {
            //checks if the space is empty for a boat of this size
            boundchecks = row - FIELD_BOAT_LIVES_SMALL;
            if (boundchecks < -1) {
                return FALSE;
            }
            for (temprow = row; temprow > boundchecks; temprow--) {
                if (f->field[temprow][col]) {
                    return FALSE;
                }
            }
            for (temprow = row; temprow > boundchecks; temprow--) {
                f->field[temprow][col] = FIELD_BOAT_LIVES_SMALL;
            }
            return TRUE;
        } else if (type == FIELD_BOAT_MEDIUM) {
            //checks if the space is empty for a boat of this size
            boundchecks = row - FIELD_BOAT_LIVES_MEDIUM;
            if (boundchecks < -1) {
                return FALSE;
            }
            for (temprow = row; temprow > boundchecks; temprow--) {
                if (f->field[temprow][col]) {
                    return FALSE;
                }
            }
            for (temprow = row; temprow > boundchecks; temprow--) {
                f->field[temprow][col] = FIELD_BOAT_LIVES_MEDIUM;
            }
            return TRUE;
        } else if (type == FIELD_BOAT_LARGE) {

            //checks if the space is empty for a boat of this size
            boundchecks = row - FIELD_BOAT_LIVES_LARGE;
            if (boundchecks < -1) {
                return FALSE;
            }
            for (temprow = row; temprow > boundchecks; temprow--) {
                if (f->field[temprow][col]) {
                    return FALSE;
                }
            }
            for (temprow = row; temprow > boundchecks; temprow--) {
                f->field[temprow][col] = FIELD_BOAT_LIVES_LARGE;
            }
            return TRUE;
        } else if (type == FIELD_BOAT_HUGE) {

            //checks if the space is empty for a boat of this size
            boundchecks = row - FIELD_BOAT_LIVES_HUGE;
            if (boundchecks < -1) {
                return FALSE;
            }
            for (temprow = row; temprow > boundchecks; temprow--) {
                if (f->field[temprow][col]) {
                    return FALSE;
                }
            }
            for (temprow = row; temprow > boundchecks; temprow--) {
                f->field[temprow][col] = FIELD_BOAT_LIVES_HUGE;
            }
            return TRUE;
        }
        //checks direction
    } else if (dir == FIELD_BOAT_DIRECTION_EAST) {
        //checks if the spot is available
        if (type == FIELD_BOAT_SMALL) {
            //checks if the spce is empty for a boat of this size
            boundchecks = col + FIELD_BOAT_LIVES_SMALL;
            if (boundchecks > FIELD_COLS) {
                return FALSE;
            }
            for (tempcol = col; tempcol < boundchecks; tempcol++) {
                if (f->field[row][tempcol]) {
                    return FALSE;
                }
            }
            for (tempcol = col; tempcol < boundchecks; tempcol++) {
                f->field[row][tempcol] = FIELD_BOAT_LIVES_SMALL;
            }
            return TRUE;
        } else if (type == FIELD_BOAT_MEDIUM) {
            boundchecks = col + FIELD_BOAT_LIVES_MEDIUM;
            if (boundchecks > FIELD_COLS) {
                return FALSE;
            }
            for (tempcol = col; tempcol < boundchecks; tempcol++) {
                if (f->field[row][tempcol]) {
                    return FALSE;
                }
            }
            for (tempcol = col; tempcol < boundchecks; tempcol++) {
                f->field[row][tempcol] = FIELD_BOAT_LIVES_MEDIUM;
            }
            return TRUE;
        } else if (type == FIELD_BOAT_LARGE) {
            boundchecks = col + FIELD_BOAT_LIVES_LARGE;
            if (boundchecks > FIELD_COLS) {
                return FALSE;
            }
            for (tempcol = col; tempcol < boundchecks; tempcol++) {
                if (f->field[row][tempcol]) {
                    return FALSE;
                }
            }
            for (tempcol = col; tempcol < boundchecks; tempcol++) {
                f->field[row][tempcol] = FIELD_BOAT_LIVES_LARGE;
            }

            return TRUE;
        } else if (type == FIELD_BOAT_HUGE) {

            boundchecks = col + FIELD_BOAT_LIVES_HUGE;
            if (boundchecks > FIELD_COLS) {
                return FALSE;
            }
            for (tempcol = col; tempcol < boundchecks; tempcol++) {
                if (f->field[row][tempcol]) {
                    return FALSE;
                }
            }
            for (tempcol = col; tempcol < boundchecks; tempcol++) {
                f->field[row][tempcol] = FIELD_BOAT_LIVES_HUGE;
            }
            return TRUE;
        }
        //checks direction
    } else if (dir == FIELD_BOAT_DIRECTION_SOUTH) {
        if (type == FIELD_BOAT_SMALL) {

            //checks if the space is empty for a boat of this size
            boundchecks = row + FIELD_BOAT_LIVES_SMALL;
            if (boundchecks > FIELD_ROWS) {
                return FALSE;
            }
            for (temprow = row; temprow < boundchecks; temprow++) {
                if (f->field[temprow][col]) {
                    return FALSE;
                }
            }
            for (temprow = row; temprow < boundchecks; temprow++) {
                f->field[temprow][col] = FIELD_BOAT_LIVES_SMALL;
            }
            return TRUE;
        } else if (type == FIELD_BOAT_MEDIUM) {

            boundchecks = row + FIELD_BOAT_LIVES_MEDIUM;
            if (boundchecks > FIELD_ROWS) {
                return FALSE;
            }
            for (temprow = row; temprow < boundchecks; temprow++) {
                if (f->field[temprow][col]) {
                    return FALSE;
                }
            }
            for (temprow = row; temprow < boundchecks; temprow++) {
                f->field[row++][col] = FIELD_BOAT_LIVES_MEDIUM;
            }
            return TRUE;
        } else if (type == FIELD_BOAT_LARGE) {

            boundchecks = row + FIELD_BOAT_LIVES_LARGE;
            if (boundchecks > FIELD_ROWS) {
                return FALSE;
            }
            for (temprow = row; temprow < boundchecks; temprow++) {
                if (f->field[temprow][col]) {
                    return FALSE;
                }
            }
            for (temprow = row; temprow < boundchecks; temprow++) {
                f->field[row++][col] = FIELD_BOAT_LIVES_LARGE;
            }
            return TRUE;
        } else if (type == FIELD_BOAT_HUGE) {

            boundchecks = row + FIELD_BOAT_LIVES_HUGE;
            if (boundchecks > FIELD_ROWS) {
                return FALSE;
            }
            for (temprow = row; temprow < boundchecks; temprow++) {
                if (f->field[temprow][col]) {
                    return FALSE;
                }
            }
            for (temprow = row; temprow < boundchecks; temprow++) {
                f->field[row++][col] = FIELD_BOAT_LIVES_HUGE;
            }
            return TRUE;
        }
    } else if (dir == FIELD_BOAT_DIRECTION_WEST) {
        //checks if spot is open
        if (type == FIELD_BOAT_SMALL) {

            //checks if the space is empty for a boat of this size
            boundchecks = col - FIELD_BOAT_LIVES_SMALL;
            if (boundchecks < -1) {
                return FALSE;
            }
            for (tempcol = col; tempcol > boundchecks; tempcol--) {
                if (f->field[row][tempcol]) {
                    return FALSE;
                }
            }
            for (tempcol = col; tempcol > boundchecks; tempcol--) {
                f->field[row][tempcol] = FIELD_BOAT_LIVES_SMALL;
            }
            return TRUE;
        } else if (type == FIELD_BOAT_MEDIUM) {

            //checks if the space is empty for a boat of this size
            boundchecks = col - FIELD_BOAT_LIVES_MEDIUM;
            if (boundchecks < -1) {
                return FALSE;
            }
            for (tempcol = col; tempcol > boundchecks; tempcol--) {
                if (f->field[row][tempcol]) {
                    return FALSE;
                }
            }
            for (tempcol = col; tempcol > boundchecks; tempcol--) {
                f->field[row][tempcol] = FIELD_BOAT_LIVES_MEDIUM;
            }
            return TRUE;
        } else if (type == FIELD_BOAT_LARGE) {
            //checks if the space is empty for a boat of this size
            boundchecks = col - FIELD_BOAT_LIVES_LARGE;
            if (boundchecks < -1) {
                return FALSE;
            }
            for (tempcol = col; tempcol > boundchecks; tempcol--) {
                if (f->field[row][tempcol]) {
                    return FALSE;
                }
            }
            for (tempcol = col; tempcol > boundchecks; tempcol--) {
                f->field[row][tempcol] = FIELD_BOAT_LIVES_LARGE;
            }
            return TRUE;
        } else if (type == FIELD_BOAT_HUGE) {

            //checks if the space is empty for a boat of this size
            boundchecks = col - FIELD_BOAT_LIVES_HUGE;
            if (boundchecks < -1) {
                return FALSE;
            }
            for (tempcol = col; tempcol > boundchecks; tempcol--) {
                if (f->field[row][tempcol]) {
                    return FALSE;
                }
            }
            for (tempcol = col; tempcol > boundchecks; tempcol--) {
                f->field[row][tempcol] = FIELD_BOAT_LIVES_HUGE;
            }
            return TRUE;
        }
    }
}

FieldPosition FieldRegisterEnemyAttack(Field *f, GuessData * gData)
{
    FieldPosition test = f->field[gData->row][gData->col];
    if (test == FIELD_POSITION_HIT) {
        if (test != FIELD_POSITION_EMPTY) {
            if (test == FIELD_POSITION_SMALL_BOAT && f->smallBoatLives > 1)
                f->smallBoatLives--;
            gData->hit = HIT_HIT;
            f->field[gData->row][gData->col];
            return test;
        } else if (test == FIELD_POSITION_MEDIUM_BOAT && f->mediumBoatLives > 0) {
            f->mediumBoatLives--;
            gData->hit = FIELD_POSITION_HIT;
            return test;
        } else if (test == FIELD_POSITION_LARGE_BOAT && f->largeBoatLives > 0) {
            f->largeBoatLives--;
            gData->hit = FIELD_POSITION_HIT;
            return test;
        } else if (test == FIELD_POSITION_HUGE_BOAT && f->hugeBoatLives > 0) {
            f->hugeBoatLives--;
            gData->hit = FIELD_POSITION_HIT;
            return test;
        } else if (test == FIELD_POSITION_SMALL_BOAT && f->smallBoatLives == 0) {
            gData->hit = HIT_SUNK_SMALL_BOAT;
            return test;
        } else if (test == FIELD_POSITION_MEDIUM_BOAT && f->mediumBoatLives == 0) {
            gData->hit = HIT_SUNK_MEDIUM_BOAT;
            return test;
        } else if (test == FIELD_POSITION_LARGE_BOAT && f->largeBoatLives == 0) {
            gData->hit = HIT_SUNK_LARGE_BOAT;
            return test;
        } else if (test == FIELD_POSITION_HUGE_BOAT && f->hugeBoatLives == 0) {
            gData->hit = HIT_SUNK_HUGE_BOAT;
            return test;
        }
    } else {
        f->field[gData->row][gData->col] = FIELD_POSITION_MISS;
        gData->hit = FIELD_POSITION_MISS;
        return test;
    }
    return test;
}

FieldPosition FieldUpdateKnowledge(Field *f, const GuessData * gData)
{
    FieldPosition test = f->field[gData->row][gData->col];
    if (f->field[gData->row][gData->col] == gData->hit) {
        if (gData->hit == HIT_SUNK_HUGE_BOAT) {
            f->field[gData->row][gData->col] = FIELD_POSITION_HIT;
            f->hugeBoatLives = 0;
            return test;
        } else if (gData->hit == HIT_SUNK_LARGE_BOAT) {
            f->field[gData->row][gData->col] = FIELD_POSITION_HIT;
            f->largeBoatLives = 0;
            return test;
        } else if (gData->hit == HIT_SUNK_MEDIUM_BOAT) {
            f->field[gData->row][gData->col] = FIELD_POSITION_HIT;
            f->mediumBoatLives = 0;
            return test;
        } else if (gData->hit == HIT_SUNK_SMALL_BOAT) {
            f->field[gData->row][gData->col] = FIELD_POSITION_HIT;
            f->smallBoatLives = 0;
            return test;
        } else {
            f->field[gData->row][gData->col] = FIELD_POSITION_HIT;
            return test;
        }
    } else if (f->field[gData->row][gData->col] != gData->hit) {
        f->field[gData->row][gData->col] = FIELD_POSITION_EMPTY;
        return test;
    }
    return test;
}

uint8_t FieldGetBoatStates(const Field * f)
{
    uint8_t boatsAlive = 0x00;
    if (f->hugeBoatLives > 0) {
        boatsAlive |= FIELD_BOAT_STATUS_HUGE;
    }
    if (f->largeBoatLives > 0) {
        boatsAlive |= FIELD_BOAT_STATUS_LARGE;
    }
    if (f->mediumBoatLives > 0) {
        boatsAlive |= FIELD_BOAT_STATUS_MEDIUM;
    }
    if (f->smallBoatLives > 0) {
        boatsAlive |= FIELD_BOAT_STATUS_SMALL;
    }
    return boatsAlive;
}