#include "predict.h"
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define THREADNUM 16
#define PARANUM 13
#define ADDRESS "../Evaluator/finalroute.txt"

uint32_t pararange[PARANUM][2] = {
    {1024, 1024},//HASHSIZE;
    {16, 16},//HASHLENGTH;
    {6, 6},//CONFINIT;
    {6, 6},//CONFINITD;
    {2, 2},//CONFINITP n2;
    {4, 4},//CONFINITDP n2;

    {7, 7},//JUDGE;
    {4, 4},//JUDGE2 n2;
    {1, 1},//D1;
    {1, 1},//N1;
    {1, 1},//D2 n2;
    {1, 1},//N2 n2;
    {8, 16} //RLENGTH
};

typedef struct {
    int ans;
    int grouptag;
    uint64_t id;
} planet;

typedef struct {
    int id;
    pthread_t tid;
    uint64_t correct;
    uint64_t hashsize;
    uint64_t hashused;
    Parameter* p;
    RoboMemory* rm;
} threaddata;

planet data[7000000] = {};
threaddata tdatas[THREADNUM];

void initpara(Parameter* para, uint64_t ival[PARANUM]) {
    para->HASHSIZE = ival[0];
    para->HASHLENGTH = ival[1];
    para->CONFINIT = ival[2];
    para->CONFINITD = ival[3];
    para->CONFINITP = ival[4];
    para->CONFINITDP = ival[5];

    para->JUDGE = ival[6];
    para->JUDGE2 = ival[7];
    para->D1 = ival[8];
    para->N1 = ival[9];
    para->D2 = ival[10];
    para->N2 = ival[11];
    para->RLENGTH = ival[12];
}

void printpara(uint64_t best, uint64_t hashused, Parameter* para) {
    printf("\n%.8f%% %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu\n",
    100 * (best / 7000000.0),
    hashused,
    para->HASHSIZE,
    para->HASHLENGTH,
    para->CONFINIT,
    para->CONFINITD,
    para->CONFINITP,
    para->CONFINITDP,
    para->JUDGE,
    para->JUDGE2,
    para->D1,
    para->N1,
    para->D2,
    para->N2,
    para->RLENGTH);
}

void initmem(RoboMemory* mem) {
    mem->inhash = 0;
    mem->hist = 0;
    mem->tag = 0;
    mem->offset = 0;
    mem->conf = 0;
    mem->freq = mem->para->CONFINIT;

    mem->key = 0;
    mem->index = 0;
    mem->planethash = calloc(1, (mem->para->HASHSIZE) * sizeof(uint8_t*));

    for (int i = 0; i < (mem->para->HASHSIZE); i++) {
        (mem->planethash)[i] = calloc(1, mem->para->RLENGTH+1);
    }
}

void freemem(RoboMemory* mem) {
    for (int i = 0; i < (mem->para->HASHSIZE); i++) {
        free((mem->planethash)[i]);
    }
    free(mem->planethash);
}

void *eval(void* arg) {
    uint8_t val = 0;
    int result = 0;
    uint64_t correct = 0;
    uint64_t used = 0;

    threaddata *tdata = (threaddata *)arg;
    RoboMemory *mem = tdata->rm;
    tdata->hashsize = tdata->rm->para->HASHSIZE;

    for(int i = 0; i < 7000000; i++) {
        result = predictTimeOfDayOnNextPlanet(mem, data[i].id, 0, data[i].grouptag);
        observeAndRecordTimeofdayOnNextPlanet(mem, data[i].id, data[i].ans);
        if(result == data[i].ans) correct++;
    }

    for(int i = 0; i < tdata->hashsize; i++) {
        val = tdata->rm->planethash[i][0];
        if(val)
            used++;
    }

    tdata->hashused = used;
    tdata->correct = correct;
    return NULL;
}

int main(int argc, char *argv[]) {
    FILE *fp = NULL;
    char buff[255];

    uint64_t best = 0;
    uint64_t besthash = 0;
    uint64_t totalnum = 1;
    uint64_t finishednum = 0;
    Parameter* bestpara = malloc(1);
    uint64_t currpara[PARANUM];

    printf("Start loading data...\n");
    fp = fopen(ADDRESS, "r"); // 70r
    fgets(buff, 255, fp);
    for(size_t i = 0; i < 7000000; i++) {
        fscanf(fp, "%llu", &(data[i].id));
        fscanf(fp, "%s", buff);
        fscanf(fp, "%d", &(data[i].grouptag)); //! For group i
        fgetc(fp);
        data[i].ans = (buff[0] == 'D')?1:0;
    }
    fclose(fp);
    printf("Data loaded\n");
    printf("%llu %d %u\n", data[0].id, data[0].ans, data[0].grouptag);

    for(int i = 0; i < PARANUM; i++)
        totalnum *= pararange[i][1] - pararange[i][0] + 1;
    printf("%llu\n", totalnum);

    for(int i = 0; i < PARANUM; i++)
        currpara[i] = pararange[i][0];

    for(int i = 0; i < THREADNUM; i++) {
        tdatas[i].correct = 1;
        tdatas[i].id = i;
    }

    for(int i = 0; i < THREADNUM; i++) {
        tdatas[i].rm = calloc(1, sizeof(RoboMemory));
        tdatas[i].p = calloc(1, sizeof(Parameter));
        initpara(tdatas[i].p, currpara);
        tdatas[i].rm->para = tdatas[i].p;
        initmem(tdatas[i].rm);
        pthread_create(&(tdatas[i].tid), NULL, eval, &(tdatas[i]));
    }

    while(1) {
        int flag = 0;
        for(int i = 0; i < THREADNUM; i++) {
            if(tdatas[i].correct != 0) {
                pthread_join(tdatas[i].tid, NULL);
                if(tdatas[i].correct > best) {
                    free(bestpara);
                    best = tdatas[i].correct;
                    bestpara = tdatas[i].p;
                    besthash = tdatas[i].hashused;
                    printpara(best, besthash, bestpara);
                    freemem(tdatas[i].rm);
                    free(tdatas[i].rm);
                }
                else {
                    freemem(tdatas[i].rm);
                    free(tdatas[i].rm);
                    free(tdatas[i].p);
                }

                tdatas[i].correct = 0;
                tdatas[i].p = calloc(1, sizeof(Parameter));
                initpara(tdatas[i].p, currpara);
                tdatas[i].rm = calloc(1, sizeof(RoboMemory));
                tdatas[i].rm->para = tdatas[i].p;
                initmem(tdatas[i].rm);

                pthread_create(&(tdatas[i].tid), NULL, eval, &(tdatas[i]));

                currpara[0]++;
                for (int i = 0; i < PARANUM - 1; i++) {
                    if (currpara[i] > pararange[i][1]) {
                        currpara[i + 1]++;
                        currpara[i] = pararange[i][0];
                    }
                }
                if(currpara[PARANUM - 1] > pararange[PARANUM - 1][1]) {
                    flag = 1;
                    break;
                }
                finishednum++;
            }
        }
        if(flag) {
            break;
        }
    }
    sleep(5);
    for(int i = 0; i < THREADNUM; i++) {
        if(tdatas[i].correct != 0) {
            pthread_join(tdatas[i].tid, NULL);
            if(tdatas[i].correct > best) {
                free(bestpara);
                best = tdatas[i].correct;
                bestpara = tdatas[i].p;
                besthash = tdatas[i].hashused;
                printpara(best, besthash, bestpara);
            }
            else {
                free(tdatas[i].p);
            }

            tdatas[i].correct = 0;
            freemem(tdatas[i].rm);
            free(tdatas[i].rm);

            finishednum++;
        }
    }
    printpara(best, 0, bestpara);
    free(bestpara);

    exit(0);
}