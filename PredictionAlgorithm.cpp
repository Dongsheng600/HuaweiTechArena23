// This file contains a template for the implementation of Robo prediction
// algorithm

#include "PredictionAlgorithm.hpp"
#define MAX(A, B) (A>B)?(A):(B)
#define MIN(A, B) (A>B)?(B):(A)

inline int8_t compressid(int32_t planetid) {
  planetid = planetid ^ (planetid >> 16);
  return ((planetid ^ (planetid >> 8)) & 0xff);
}

inline int8_t verifygen(int8_t prev, int8_t curr) {
  curr = curr ^ prev;
  return (curr^(curr >> 4))&0xf;
}

struct RoboPredictor::RoboMemory {
  // Place your RoboMemory content here
  // Note that the size of this data structure can't exceed 64KiB!
  bool roboguess = false; // Guest of robo
  bool found = false; // If found the relationship in the hash table
  int8_t prev = 0; // Last planet id with t/f
  int8_t verify = 0; // Verify code
  int16_t hashptr = 0; // Point to index of hashtable visited
  int16_t qptr = 0; // Point to oldest element in the queue
  int8_t queue[8] = {0};    // Queue used to store past planets
  int16_t hashpos[8] = {0}; // Pos of current relationships
  int8_t histhash[8][4096] = {0}; // Hashtable store history relationship
};

inline int16_t hashgen(unsigned long val) {
  int16_t hash = val * 0x9e370001UL;
  return hash >> (16);
}

bool RoboPredictor::predictTimeOfDayOnNextPlanet(
    std::uint64_t nextPlanetID, bool spaceshipComputerPrediction) {
  // Robo can consult data structures in its memory while predicting.
  // Example: access Robo's memory with roboMemory_ptr-><your RoboMemory
  // content>

  // Robo can perform computations using any data in its memory during
  // prediction. It is important not to exceed the computation cost threshold
  // while making predictions and updating RoboMemory. The computation cost of
  // prediction and updating RoboMemory is calculated by the playground
  // automatically and printed together with accuracy at the end of the
  // evaluation (see main.cpp for more details).

  // Simple prediction policy: follow the spaceship computer's suggestions

  int8_t compid, hist, val, confidence;
  roboMemory_ptr->roboguess = false;
  roboMemory_ptr->found = false;
  compid = compressid(nextPlanetID);
  roboMemory_ptr->verify = ((roboMemory_ptr->prev)^compid)<<4;
  for (int8_t i = roboMemory_ptr->qptr - 1, j = 0; i != roboMemory_ptr->qptr;  i = (i - 1) & 0b111) { // Generate hash index
    hist ^= roboMemory_ptr->queue[i];
    roboMemory_ptr->hashpos[j] = (hist<<4)|roboMemory_ptr->verify;
  }
  for (int8_t i = 7; i != -1 && !(roboMemory_ptr->found); i--) {
    val = roboMemory_ptr->histhash[i][roboMemory_ptr->hashpos[i]];
    if(val) {
      if((val&0xf0)==roboMemory_ptr->verify) {
        roboMemory_ptr->found = true;
        roboMemory_ptr->roboguess = (val > 13);
        roboMemory_ptr->hashptr = i;
      }
      else {
        roboMemory_ptr->hashpos[i]++;
        val = roboMemory_ptr->histhash[i][(roboMemory_ptr->hashpos[i]) & 0xfff];
        if ((val & 0xf0) == roboMemory_ptr->verify) {
          roboMemory_ptr->found = true;
          roboMemory_ptr->roboguess = (val & 0b1000 > 13);
          roboMemory_ptr->hashptr = i;
        }
        else {
          roboMemory_ptr->hashpos[i]++;
          val = roboMemory_ptr->histhash[i][(roboMemory_ptr->hashpos[i]) & 0xfff];
          if ((val & 0xf0) == roboMemory_ptr->verify) {
            roboMemory_ptr->found = true;
            roboMemory_ptr->roboguess = (val & 0b1000 > 13);
            roboMemory_ptr->hashptr = i;
          }
        }
      }
    }
  }
  roboMemory_ptr->prev = compid;

  return roboMemory_ptr->roboguess;
}

void RoboPredictor::observeAndRecordTimeofdayOnNextPlanet(
    std::uint64_t nextPlanetID, bool timeOfDayOnNextPlanet) {
  // Robo can consult/update data structures in its memory
  // Example: access Robo's memory with roboMemory_ptr-><your RoboMemory
  // content>

  // It is important not to exceed the computation cost threshold while making
  // predictions and updating RoboMemory. The computation cost of prediction and
  // updating RoboMemory is calculated by the playground automatically and
  // printed together with accuracy at the end of the evaluation (see main.cpp
  // for more details).

  // Simple prediction policy: do nothing
  int8_t val, confidence, init;
  val = roboMemory_ptr->histhash[roboMemory_ptr->hashptr][roboMemory_ptr->hashpos[roboMemory_ptr->hashptr]];
  confidence = val & 0xf;
  if(roboMemory_ptr->found) {
    if (roboMemory_ptr->roboguess == timeOfDayOnNextPlanet) {
      if(timeOfDayOnNextPlanet) confidence = MIN(8, confidence + 1);
      else confidence = MAX(0, confidence - 1);
    }
    else if(roboMemory_ptr->roboguess != timeOfDayOnNextPlanet) {
      // Move to more history
      if(confidence == 7 && timeOfDayOnNextPlanet) {
        roboMemory_ptr->histhash[roboMemory_ptr->hashptr][roboMemory_ptr->hashpos[roboMemory_ptr->hashptr]] = 0;
        roboMemory_ptr->hashptr++;
        roboMemory_ptr->hashptr = MAX(7, roboMemory_ptr->hashptr);
        if (roboMemory_ptr->histhash[roboMemory_ptr->hashptr][roboMemory_ptr->hashpos[roboMemory_ptr->hashptr]] == 0) {
          roboMemory_ptr->histhash[roboMemory_ptr->hashptr][roboMemory_ptr->hashpos[roboMemory_ptr->hashptr]] = (roboMemory_ptr->verify<<4)|(0b1101);
        }
        else {
          roboMemory_ptr->hashpos[roboMemory_ptr->hashptr]++;
          roboMemory_ptr->hashptr = MAX(roboMemory_ptr->hashptr, 4095);
          if (roboMemory_ptr->histhash[roboMemory_ptr->hashptr][roboMemory_ptr->hashpos[roboMemory_ptr->hashptr]] == 0) {
            roboMemory_ptr->histhash[roboMemory_ptr->hashptr][roboMemory_ptr->hashpos[roboMemory_ptr->hashptr]] = (roboMemory_ptr->verify << 4) | (0b1101);
          }
          else {
            roboMemory_ptr->hashpos[roboMemory_ptr->hashptr]++;
            roboMemory_ptr->hashptr = MAX(roboMemory_ptr->hashptr, 4095);
            if (roboMemory_ptr->histhash[roboMemory_ptr->hashptr][roboMemory_ptr->hashpos[roboMemory_ptr->hashptr]] == 0){
              roboMemory_ptr->histhash[roboMemory_ptr->hashptr][roboMemory_ptr->hashpos[roboMemory_ptr->hashptr]] = (roboMemory_ptr->verify << 4) | (0b1101);
            }
          }
        }
      }
      else if(confidence == 8 && !timeOfDayOnNextPlanet) {
        roboMemory_ptr->histhash[roboMemory_ptr->hashptr][roboMemory_ptr->hashpos[roboMemory_ptr->hashptr]] = 0;
        roboMemory_ptr->hashptr++;
        roboMemory_ptr->hashptr = MAX(7, roboMemory_ptr->hashptr);
        if (roboMemory_ptr->histhash[roboMemory_ptr->hashptr][roboMemory_ptr->hashpos[roboMemory_ptr->hashptr]] == 0)
        {
          roboMemory_ptr->histhash[roboMemory_ptr->hashptr][roboMemory_ptr->hashpos[roboMemory_ptr->hashptr]] = (roboMemory_ptr->verify << 4) | (0b0110);
        }
        else
        {
          roboMemory_ptr->hashpos[roboMemory_ptr->hashptr]++;
          roboMemory_ptr->hashpos[roboMemory_ptr->hashptr] = MAX(roboMemory_ptr->hashpos[roboMemory_ptr->hashptr], 4095);
          if (roboMemory_ptr->histhash[roboMemory_ptr->hashptr][roboMemory_ptr->hashpos[roboMemory_ptr->hashptr]] == 0)
          {
            roboMemory_ptr->histhash[roboMemory_ptr->hashptr][roboMemory_ptr->hashpos[roboMemory_ptr->hashptr]] = (roboMemory_ptr->verify << 4) | (0b0110);
          }
          else
          {
            roboMemory_ptr->hashpos[roboMemory_ptr->hashptr]++;
            roboMemory_ptr->hashpos[roboMemory_ptr->hashptr] = MAX(roboMemory_ptr->hashpos[roboMemory_ptr->hashptr], 4095);
            if (roboMemory_ptr->histhash[roboMemory_ptr->hashptr][roboMemory_ptr->hashpos[roboMemory_ptr->hashptr]] == 0)
            {
              roboMemory_ptr->histhash[roboMemory_ptr->hashptr][roboMemory_ptr->hashpos[roboMemory_ptr->hashptr]] = (roboMemory_ptr->verify << 4) | (0b0110);
            }
          }
        }
      }
      else {
        if(timeOfDayOnNextPlanet) confidence = MIN(8, confidence + 1);
        else confidence = MAX(0, confidence - 1);
      }
    }
  }
  else {
    // Create a new record
    roboMemory_ptr->hashptr = 0;
    if(timeOfDayOnNextPlanet) {
      init = 0b1001;
    }
    else {
      init = 0b0110;
    }
    if (roboMemory_ptr->histhash[roboMemory_ptr->hashptr][roboMemory_ptr->hashpos[roboMemory_ptr->hashptr]] == 0)
    {
      roboMemory_ptr->histhash[roboMemory_ptr->hashptr][roboMemory_ptr->hashpos[roboMemory_ptr->hashptr]] = (roboMemory_ptr->verify << 4) | init;
    }
    else
    {
      roboMemory_ptr->hashpos[roboMemory_ptr->hashptr]++;
      roboMemory_ptr->hashptr = MAX(roboMemory_ptr->hashptr, 4095);
      if (roboMemory_ptr->histhash[roboMemory_ptr->hashptr][roboMemory_ptr->hashpos[roboMemory_ptr->hashptr]] == 0)
      {
        roboMemory_ptr->histhash[roboMemory_ptr->hashptr][roboMemory_ptr->hashpos[roboMemory_ptr->hashptr]] = (roboMemory_ptr->verify << 4) | init;
      }
      else
      {
        roboMemory_ptr->hashpos[roboMemory_ptr->hashptr]++;
        roboMemory_ptr->hashptr = MAX(roboMemory_ptr->hashptr, 4095);
        if (roboMemory_ptr->histhash[roboMemory_ptr->hashptr][roboMemory_ptr->hashpos[roboMemory_ptr->hashptr]] == 0)
        {
          roboMemory_ptr->histhash[roboMemory_ptr->hashptr][roboMemory_ptr->hashpos[roboMemory_ptr->hashptr]] = (roboMemory_ptr->verify << 4) | init;
        }
      }
    }
  }
}


//------------------------------------------------------------------------------
// Please don't modify this file below
//
// Check if RoboMemory does not exceed 64KiB
static_assert(
    sizeof(RoboPredictor::RoboMemory) <= 65536,
    "Robo's memory exceeds 65536 bytes (64KiB) in your implementation. "
    "Prediction algorithms using so much "
    "memory are ineligible. Please reduce the size of your RoboMemory struct.");

// Declare constructor/destructor for RoboPredictor
RoboPredictor::RoboPredictor() {
  roboMemory_ptr = new RoboMemory;
}
RoboPredictor::~RoboPredictor() {
  delete roboMemory_ptr;
}