#ifndef LOCIC_STRINGMAP_H
#define LOCIC_STRINGMAP_H

#ifdef __cplusplus
extern "C" {
#endif

typedef void * Locic_StringMap;

Locic_StringMap Locic_StringMap_Alloc();

void Locic_StringMap_Free(Locic_StringMap stringMap);

void Locic_StringMap_Clear(Locic_StringMap stringMap);

void * Locic_StringMap_Find(Locic_StringMap stringMap, const char * str);

// Return value is existing value in map, which if not NULL indicates the insert failed.
void * Locic_StringMap_Insert(Locic_StringMap stringMap, const char * str, void * data);

void * Locic_StringMap_Erase(Locic_StringMap stringMap, const char * str);

size_t Locic_StringMap_Size(Locic_StringMap stringMap);

#ifdef __cplusplus
}
#endif

#endif