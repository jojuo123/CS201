#include "cachelab.h"
#include <getopt.h>
#include "stdio.h"
#include "string.h"
#include <stdbool.h>
#include <stdlib.h>
#define MISS 0
#define HIT 1
#define EVICTION 2
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

void charDeepCopy(char* source)
{
    uint n = strlen(source) + 1;
    traceFile = (char*)malloc(n);
    uint i = 0;
    while (source[i] != '\0') 
    {
        traceFile[i] = source[i];
        ++i;
    }
    traceFile[i] = '\0';
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
                charDeepCopy(optarg);
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

uint getTag(ull address)
{
    return (uint)(address >> (64 - tagBit));
}

uint getSet(ull address)
{
    return (uint)((address << tagBit) >> (tagBit+blockBit));
}

uint Access(uint tag, sSet* set)
{
    uint result = EVICTION;
    for (uint i = 0; i < Eway; ++i)
    {
        //update time
        ++(set->way[i].time);
        if (set->way[i].valid != 0 && set->way[i].tag == tag)
        {
            set->way[i].time = 0;
            result = HIT;
        }
    }
    //if (result == HIT) return result;
    
    
    if (result != HIT) 
    //if there is an invalid one
        for (uint i = 0; i < Eway; ++i)
            if (set->way[i].valid == 0) 
            {
                set->way[i].valid = 1;
                set->way[i].tag = tag;
                set->way[i].time = 0;
                result = MISS;
            }

    //if everything is valid
    if (result == EVICTION)
    {
        set->way[set->LRU].tag = tag;
        set->way[set->LRU].time = 0;
    }

    //update LRU
    uint max = 0; uint imax = 0;
    for (uint i = 0; i < Eway; ++i)
        if (set->way[i].time > max) { max = set->way[i].time; imax = i; }
    set->LRU = imax;

    return result;
}

uint LoadAndStore(ull address)
{
    uint set = getSet(address);
    uint tag = getTag(address);
    
    uint val = Access(tag, cache.s+set);
    switch (val)
    {
        case MISS:
            ++result[MISS];
            return MISS;
        case HIT:
            ++result[HIT];
            return HIT;
        case EVICTION:
            ++result[MISS];
            ++result[EVICTION];
            return EVICTION;
    }
}

uint Modify(ull address)
{
    uint set = getSet(address);
    uint tag = getTag(address);
    uint ret;
    uint val = Access(tag, cache.s+set);
    switch (val)
    {
        case MISS:
            ++result[MISS];
            break;
        case HIT:
            ++result[HIT];
            break;
        case EVICTION:
            ++result[MISS];
            ++result[EVICTION];
            break;
    }
    ret = val;
    
    val = Access(tag, cache.s+set);
    switch (val)
    {
        case MISS:
            ++result[MISS];
            break;
        case HIT:
            ++result[HIT];
            break;
        case EVICTION:
            ++result[MISS];
            ++result[EVICTION];
            break;
    }

    return val;
}



void HandleQuery(char instruction, ull address, uint size, uint verbose)
{
    //printf("%d %d %d\n",instruction,address,size);
    uint opt;
    switch (instruction)
    {
        case 'L':
        case 'S':
            opt = LoadAndStore(address);
            if (verbose == 0)
                break;
            else 
            {
                if (opt == HIT)
                    printf("%c %llx,%d hit\n",instruction, address, size);
                else if (opt == MISS)
                    printf("%c %llx,%d miss\n",instruction, address, size);
                else 
                    printf("%c %llx,%d miss eviction\n",instruction, address, size);
            }
            break;
        case 'M':
            opt = Modify(address);
            if (verbose == 0)
                break;
            else 
            {
                if (opt == HIT)
                    printf("%c %llx,%d hit hit\n",instruction, address, size);
                else if (opt == MISS)
                    printf("%c %llx,%d miss hit\n",instruction, address, size);
                else 
                    printf("%c %llx,%d miss eviction hit\n",instruction, address, size);
            }
            break;
    }
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
    if (fp == NULL)
    {
        printf("File not found \n");
        return;
    }
    while (fgets(buf, 255, (FILE*)fp) != NULL)
    {
        strtok(buf, "\n");
        char temp;
        uint instruction;
        ull address;
        uint size;
        if (buf[0] == '0') continue;
        sscanf(buf, " %c %llx,%d", &temp, &address, &size);
        //instruction = getInstruction(&temp);
        //if (verbose == 1)
        HandleQuery(temp, address, size, verbose);
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
