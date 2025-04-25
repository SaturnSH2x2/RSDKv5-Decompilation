#ifndef CACHE_HPP
#define CACHE_HPP

#define STATIC_VAR_CACHE_SIZE (1024 * 24)   // 24 KB

#define STATIC_VAR_FILE_COUNT (70)

#define ENABLE_STATIC_CACHE     0b00000001
#define ENABLE_TILECONFIG_CACHE 0b00000010
#define ENABLE_SPRITEBIN_CACHE  0b00000100
#define ENABLE_SFX_CACHE        0b00001000
#define ENABLE_SPRITE_CACHE     0b00010000

#define ENABLE_O3DS (ENABLE_STATIC_CACHE | ENABLE_TILECONFIG_CACHE | ENABLE_SPRITEBIN_CACHE)
#define ENABLE_N3DS (ENABLE_O3DS | ENABLE_SFX_CACHE | ENABLE_SPRITE_CACHE)

struct StaticVarRef {
  char hash[4]; 
  int32 size;
  uint8_t* ref;
};

namespace RSDK {
  extern uint8_t cachesEnabled;
  extern uint8_t* staticVarCache;

  extern StaticVarRef staticVarRefs[STATIC_VAR_FILE_COUNT];

  inline void GetCachedStaticVar(char* hash, StaticVarRef** ptr) {
    for (int i = 0; i < STATIC_VAR_FILE_COUNT; i++) {
      if (hash[0] == staticVarRefs[i].hash[0] &&
          hash[1] == staticVarRefs[i].hash[1] &&
          hash[2] == staticVarRefs[i].hash[2] &&
          hash[3] == staticVarRefs[i].hash[3]) {
        *ptr = &staticVarRefs[i];
        break;
      }
    }

    ptr = NULL;
  }

  void InitCache(uint8_t cacheFlags);
  void FreeCache();

} // namespace RSDK

#endif
