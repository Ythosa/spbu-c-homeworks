#include "string.h"
#include <assert.h>
#include <stdlib.h>

#include "dict.h"

struct Element {
    struct Element* next;
    String key;
    void* value;
};

static element_t* elementCreate(element_t* next, String key, void* value)
{
    element_t* node = malloc(sizeof(element_t));

    node->next = next;
    node->key = key;
    node->value = value;

    return node;
}

static void elementFree(element_t* node, void (*freeElementValue)(void*))
{
    stringFree(node->key);
    freeElementValue(node->value);
    free(node);
}

String elementGetKey(element_t* element)
{
    return element->key;
}

void* elementGetValue(element_t* element)
{
    return element->value;
}

element_t* elementGetNext(element_t* element)
{
    return element->next;
}

#define INITIAL_SIZE (1024)
#define GROWTH_FACTOR (2)
#define MAX_LOAD_FACTOR (1)

typedef struct Dict {
    int bufferSize; // bufferSize of the pointer table
    int size; // number of elements stored
    element_t** table;
    void (*freeDictValue)(void*);
} dict_t;

Dict internalDictCreate(int size, void (*freeDictValue)(void*))
{
    Dict dict = malloc(sizeof(dict_t));

    dict->bufferSize = size;
    dict->size = 0;
    dict->table = calloc(dict->bufferSize, sizeof(element_t*));
    dict->freeDictValue = freeDictValue;

    return dict;
}

Dict dictCreate(void (*freeElementValue)(void*))
{
    return internalDictCreate(INITIAL_SIZE, freeElementValue);
}

void dictFree(Dict dict)
{
    for (int i = 0; i < dict->bufferSize; i++) {
        element_t* next = NULL;
        for (element_t* node = dict->table[i]; node; node = next) {
            next = node->next;
            elementFree(node, dict->freeDictValue);
        }
    }

    free(dict->table);
    free(dict);
}

int dictGetBufferSize(Dict dict)
{
    return dict->bufferSize;
}

int dictGetSize(Dict dict)
{
    return dict->size;
}

element_t* dictGetElementsByIndex(Dict dict, int index)
{
    return dict->table[index];
}

#define MULTIPLIER (97)

static unsigned long calculateHashOfString(String string)
{
    uint8_t* s = stringC(string);
    unsigned long hash = 0;

    for (unsigned const char* us = (unsigned const char*)s; *us; us++)
        hash = hash * MULTIPLIER + *us;

    return hash;
}

static unsigned long dictCalculateBinIndex(Dict dict, String key)
{
    return calculateHashOfString(key) % dict->bufferSize;
}

static void swapDicts(Dict first, Dict second)
{
    dict_t temp = *first;
    *first = *second;
    *second = temp;
}

static void dictGrow(Dict dict)
{
    Dict newDict = internalDictCreate(dict->bufferSize * GROWTH_FACTOR, dict->freeDictValue);

    for (int i = 0; i < dict->bufferSize; i++)
        for (element_t* element = dict->table[i]; element; element = element->next)
            dictPut(newDict, element->key, element->value);

    swapDicts(dict, newDict);
    dictFree(newDict);
}

element_t* dictFind(Dict dict, String key)
{
    for (element_t* element = dict->table[dictCalculateBinIndex(dict, key)]; element; element = element->next)
        if (!stringCmp(element->key, key))
            return element;

    return NULL;
}

void* dictGet(Dict dict, String key)
{
    element_t* element = dictFind(dict, key);
    if (element)
        return element->value;

    return NULL;
}

void dictPut(Dict dict, String key, void* value)
{
    assert(key);

    element_t* elementWithSuchKey = dictGet(dict, key);
    if (elementWithSuchKey) {
        elementWithSuchKey->value = value;

        return;
    }

    unsigned long binIndex = dictCalculateBinIndex(dict, key);
    dict->table[binIndex] = elementCreate(dict->table[binIndex], key, value);
    dict->size++;

    if (dict->size >= dict->bufferSize * MAX_LOAD_FACTOR)
        dictGrow(dict);
}

void dictDelete(Dict dict, String key)
{
    unsigned long binIndex = dictCalculateBinIndex(dict, key);
    for (element_t** nextPointer = &dict->table[binIndex]; *nextPointer; nextPointer = &((*nextPointer)->next))
        if (!stringCmp((*nextPointer)->key, key)) {
            element_t* nodeToDelete = NULL;
            nodeToDelete = *nextPointer;
            *nextPointer = nodeToDelete->next;
            elementFree(nodeToDelete, dict->freeDictValue);

            return;
        }
}

void dictPrint(Dict dict, String (*convertElementValueToString)(void*), FILE* dst)
{
    for (int i = 0; i < dictGetBufferSize(dict); i++)
        for (element_t* element = dictGetElementsByIndex(dict, i); element; element = elementGetNext(element)) {
            fprintf(dst, "key: ");
            stringPrint(elementGetKey(element), dst);
            fprintf(dst, " value: ");
            String s = convertElementValueToString(elementGetValue(element));
            stringPrint(s, dst);
            stringFree(s);
            fprintf(dst, "\n");
        }
}