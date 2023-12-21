#include <stdint.h>

typedef struct {
    uint64_t HASHSIZE;
    uint64_t HASHLENGTH;
    uint64_t CONFINIT;
    uint64_t CONFINITD;
    uint64_t CONFINITP;
    uint64_t CONFINITDP;

    uint64_t JUDGE;
    uint64_t JUDGE2;
    uint64_t D1;
    uint64_t N1;
    uint64_t D2;
    uint64_t N2;

    uint64_t RLENGTH;
} Parameter;

typedef struct {
    Parameter *para;

    uint8_t inhash;
    uint8_t ans;

    uint8_t hists[2];
    uint8_t hist;
    uint8_t tag;
    uint8_t offset;
    int8_t conf;
    int8_t freq;

    uint32_t key;
    uint32_t index;

    uint8_t** planethash;
} RoboMemory;

int predictTimeOfDayOnNextPlanet(RoboMemory *roboMemory_ptr, uint64_t nextPlanetID,
                                    int spaceshipComuterPrediction,
                                    int nextPlanetGroupTag);

void observeAndRecordTimeofdayOnNextPlanet(RoboMemory *roboMemory_ptr, uint64_t nextPlanetID,
                                            int timeOfDayOnNextPlanet);
