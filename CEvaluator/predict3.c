// This file contains a template for the implementation of Robo prediction
// algorithm

#include "predict.h"
#include <stdio.h>

const uint8_t HINTNOTFOUND = 0xff;
const uint32_t NOTFOUND = 0xffffffff;

uint8_t getformer(RoboMemory *RM, uint8_t val)
{
    return (val >> 4);
}

uint8_t gettag(RoboMemory *RM, uint8_t val)
{
    return val;
}

uint32_t keygen(RoboMemory *RM, uint64_t id)
{
    return (uint32_t)((id * 11111) % (RM->para->HASHSIZE));
}

uint8_t taggen(RoboMemory *RM, uint64_t id)
{
    return (uint8_t)(id & 0xff);
}

int judgeconf(RoboMemory *RM, int8_t conf)
{
    if (conf > RM->para->JUDGE)
        return 1;
    else
        return 0;
}

uint8_t gethisthint(uint8_t hist, int grouptag)
{
    uint8_t histhint;

    hist = hist & 0b111111;
    histhint = (grouptag >> 2) & 0xff;

    if((histhint & 0b10) == 0)
        return HINTNOTFOUND;

    if(hist == (histhint >> 2))
        return histhint;
    else
        return HINTNOTFOUND;
}

uint32_t accesshash(RoboMemory *RM, uint32_t key, uint8_t tag)
{
    uint8_t hashtag;
    uint32_t index = key;
    for (int i = 0; i < (RM->para->HASHLENGTH); i++) {
        hashtag = gettag(RM, (RM->planethash)[index][0]);

        if (hashtag == tag)
            return index;
        else if (hashtag == 0)
            return NOTFOUND;
        else {
            index += 1;
            if (index == (RM->para->HASHSIZE))
                return NOTFOUND;
        }
    }
    return NOTFOUND;
}

uint8_t matchhist(RoboMemory *RM, uint32_t index, uint8_t hist)
{
    uint8_t hashhist;
    uint8_t *val = (RM->planethash)[index];
    uint8_t offset = 1;
    for (int i = 0; i < RM->para->RLENGTH; i++) {
        hashhist = getformer(RM, val[offset]);
        if (hashhist == hist)
            return offset;
        else
            offset += 1;
    }
    return 0;
}

uint8_t addhist(RoboMemory *RM, uint32_t index)
{
    uint8_t hashhist;
    uint8_t *val = (RM->planethash)[index];
    uint8_t offset = 1;
    for (int i = 0; i < RM->para->RLENGTH; i++) {
        hashhist = getformer(RM, val[offset]);
        if (hashhist)
            offset += 1;
        else
            return offset;
    }
    return 0;
}

uint8_t gethconf(RoboMemory *RM, uint32_t index, uint8_t offset)
{
    return ((RM->planethash)[index][offset]) & 0xf;
}

void updatehconf(RoboMemory *RM, uint8_t hist, uint32_t index, uint8_t offset, int time)
{
    if (offset == 0) {
        offset = addhist(RM, index);
        if (offset) {
            if (time)
                RM->conf = RM->para->CONFINITD;
            else
                RM->conf = RM->para->CONFINIT;
        }
    }

    if (offset) {
        if (time) {
            RM->conf += RM->para->D1;
            if (RM->conf > 15)
                RM->conf = 15;
        }
        else {
            RM->conf -= RM->para->N1;
            if (RM->conf < 0)
                RM->conf = 0;
        }
        RM->planethash[index][offset] = RM->planethash[index][offset] & 0x00;
        RM->planethash[index][offset] = RM->planethash[index][offset] | (hist << 4);
        RM->planethash[index][offset] = RM->planethash[index][offset] | (RM->conf);
    }
}

void addrecord(RoboMemory *RM, uint32_t key, uint8_t tag, uint8_t hist, int time)
{
    uint8_t hashtag;
    uint32_t index = key;
    int found = 0;
    for (int i = 0; i < RM->para->HASHLENGTH; i++) {
        hashtag = gettag(RM, RM->planethash[index][0]);
        if (hashtag) {
            index += 1;
            if (index == RM->para->HASHSIZE)
                return;
        }
        else {
            found = 1;
            break;
        }
    }
    if (found) {
        RM->planethash[index][0] = tag;
        updatehconf(RM, hist, index, 0, time);
    }
}

int predictTimeOfDayOnNextPlanet(RoboMemory *roboMemory_ptr,
                                 uint64_t nextPlanetID, int spaceshipComputerPrediction,
                                 int nextPlanetGroupTag)
{
    uint8_t histhint = HINTNOTFOUND;
    histhint = gethisthint(roboMemory_ptr->hist, nextPlanetGroupTag);

    roboMemory_ptr->inhash = nextPlanetGroupTag & 0x1;
    roboMemory_ptr->hist = roboMemory_ptr->hist & 0xf;
    roboMemory_ptr->key = keygen(roboMemory_ptr, nextPlanetID);
    roboMemory_ptr->tag = taggen(roboMemory_ptr, nextPlanetID);
    roboMemory_ptr->index = accesshash(roboMemory_ptr, roboMemory_ptr->key, roboMemory_ptr->tag);

    if (histhint != HINTNOTFOUND) {
        return histhint & 0x1;
    }

    if (roboMemory_ptr->inhash) {
        if (roboMemory_ptr->index == NOTFOUND)
            return (nextPlanetGroupTag >> 1) & 0x1;
        else {
            roboMemory_ptr->offset = matchhist(roboMemory_ptr, roboMemory_ptr->index, roboMemory_ptr->hist);
            if (roboMemory_ptr->offset == 0)
                return (nextPlanetGroupTag >> 1) & 0x1;
            else
                roboMemory_ptr->conf = gethconf(roboMemory_ptr, roboMemory_ptr->index, roboMemory_ptr->offset);
        }
        return judgeconf(roboMemory_ptr, roboMemory_ptr->conf);
    }
    else
        return (nextPlanetGroupTag >> 1) & 0x1;
}

void observeAndRecordTimeofdayOnNextPlanet(RoboMemory *roboMemory_ptr,
                                           uint64_t nextPlanetID, int timeOfDayOnNextPlanet)
{
    if (roboMemory_ptr->inhash) {
        if (roboMemory_ptr->index == NOTFOUND) {
            addrecord(roboMemory_ptr, roboMemory_ptr->key, roboMemory_ptr->tag, roboMemory_ptr->hist, timeOfDayOnNextPlanet);
        }
        else
            updatehconf(roboMemory_ptr, roboMemory_ptr->hist, roboMemory_ptr->index, roboMemory_ptr->offset, timeOfDayOnNextPlanet);
    }
    roboMemory_ptr->hist = (uint8_t)(nextPlanetID & 0xff);
}

//------------------------------------------------------------------------------
// Please don't modify this file below
//
// Check if RoboMemory does not exceed 64KiB

// static_assert(
//     sizeof(struct RoboMemory) <= 65536,
//     "Robo's memory exceeds 65536 bytes (64KiB) in your implementation. "
//     "Prediction algorithms using so much "
//     "memory are ineligible. Please reduce the size of your RoboMemory struct.");
