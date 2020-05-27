#include "cachelab.h"
#include <getopt.h>
#include "stdio.h"
#include "string.h"
#include <stdbool.h>
#include <stdlib.h>
#define MISS 0
#define HIT 1
#define IGNORE -1
#define LOAD 0
#define MODIFY 1
#define STORE 2

typedef unsigned int uint;
typedef unsigned long long ull;

uint result[3] = {0, 0, 0};
uint hitCount, missCount, evictionCount;
uint Eway, setBit, blockBit, tagBit, verbose, noOfSet;
FILE* fp;
char* traceFile;


struct Way {
    ull time;
    uint valid;
    uint tag;
    uint block;
};

typedef struct Way sWay;

struct Set {
    uint LRU;
    //uint set;
    sWay* way;
};

typedef struct Set sSet;

struct Cache {
    sSet* s;
} cache;

void Halt()
{
    free(traceFile);
    traceFile = NULL;
    for (uint i = 0 ; i < noOfSet; ++i)
    {
        free(cache.s[i].way);
        cache.s[i].way = NULL; 
    }
    free(cache.s);
    cache.s = NULL;
    //close(fp);
    exit(0);
}

void charDeepCopy(char* source, char* destination)
{
    uint n = strlen(source) + 1;
    destination = (char*)malloc(n);
    uint i = 0;
    while (source[i] != '\0') destination[i] = source[i];
}

void getFlag(uint argc, char **argv)
{
    uint opt = 0;
    while ((opt = getopt(argc, argv, "E:s:t:b:v")) != -1)
    {
        switch (opt)
        {
            case 'E':
                Eway = atoi(optarg);
                break;
            case 's':
                setBit = atoi(optarg);
                break;
            case 't':
                charDeepCopy(optarg, traceFile);
                break;
            case 'b':
                blockBit = atoi(optarg);
                break;
            case 'v':
                verbose = 1;
                break;
            default:
                break;
        }
    }
    //debug
    //fpruintf(stderr, "E: %d, s: %d, t: %s, b: %d", Eway, setBit, traceFile, blockBit);
    tagBit = 64 - setBit - blockBit;
    noOfSet = 1 << setBit;
}

void Init()
{
    //uint noOfSet = 1 << setBit;
    cache.s = (sSet*)malloc(sizeof(sSet) * noOfSet);
    for (uint i = 0 ; i < noOfSet; ++i)
    {
        cache.s[i].LRU = 0;
        cache.s[i].way = (sWay*)malloc(sizeof(sWay) * Eway);
        for (uint j = 0; j < Eway; ++j)
            cache.s[i].way[j].time = cache.s[i].way[j].block = cache.s[i].way[j].tag = cache.s[i].way[j].valid = 0;
    }
}

void HandleQuery(int instruction, int address, int size)
{
    printf("%d %d %d\n",instruction,address,size);
}

int getInstruction(char* temp)
{
    if (temp == NULL) return -1;
    switch (temp[0])
    {
        case 'I':
            return IGNORE;
        case 'L':
            return LOAD;
        case 'M':
            return MODIFY;
        case 'S':
            return STORE;
        default:
            break;
    }
    return -1;
}

void inputHandle()
{
    
    char buf[255];
    fp = fopen(traceFile, "r");
    while (fgets(buf, 255, (FILE*)fp) != NULL)
    {
        strtok(buf, "\n");
        char temp;
        int instruction;
        ull address;
        int size;
        if (buf[0] == '0') continue;
        sscanf(buf, " %c %llx,%d", temp, address, size);
        instruction = getInstruction(&temp);
        // temp = strtok(buf," ,");
        // if (temp == NULL) continue;
        // instruction = getInstruction(temp);
        
        // temp = strtok(NULL, " ,");
        // if (temp == NULL) continue;
        // address = strtol(temp, NULL, 0);

        // temp = strtok(NULL, " ,");
        // if (temp == NULL) continue;
        // size = strtol(temp, NULL, 10);
        
        HandleQuery(instruction, address, size);
    }
    fclose(fp);
}

int main(int argc, char **argv)
{
    getFlag(argc, argv);
    Init();
    inputHandle();
    printSummary(0, 0, 0);
    Halt();
    return 0;
}
