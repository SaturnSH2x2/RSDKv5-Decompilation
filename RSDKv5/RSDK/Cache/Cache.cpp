#include "RSDK/Core/RetroEngine.hpp"

#include "CacheFiles.cpp"

using namespace RSDK;

uint8_t RSDK::cachesEnabled = 0;

uint8_t* RSDK::staticVarCache = NULL;
uint8_t* RSDK::spriteBinCache = NULL;
uint8_t* RSDK::spriteCache = NULL;

StaticVarRef RSDK::staticVarRefs[STATIC_VAR_FILE_COUNT];

void RSDK::InitCache(uint8_t cacheFlags) {
  char pathBuf[0x40];
  uint8_t* ptr;

  if (cacheFlags & ENABLE_STATIC_CACHE) {
    // allocate cache
    staticVarCache = (uint8_t*) malloc(STATIC_VAR_CACHE_SIZE * sizeof(uint8_t));
    if (staticVarCache) {
      ptr = staticVarCache;

      // if not, read from Data file/folder and save whole thing to user directory
      FileInfo info;
      InitFileInfo(&info);
      for (int i = 0; i < STATIC_VAR_FILE_COUNT; i++) {
        sprintf_s(pathBuf, sizeof(pathBuf), "Data/Objects/Static/%s", staticVarFilenames[i]);
        if (LoadFile(&info, pathBuf, FMODE_RB)) {
          // four characters *should* be enough for a hash comparison
          staticVarRefs[i].hash[0] = staticVarFilenames[i][0];
          staticVarRefs[i].hash[1] = staticVarFilenames[i][1];
          staticVarRefs[i].hash[2] = staticVarFilenames[i][2];
          staticVarRefs[i].hash[3] = staticVarFilenames[i][3];

          staticVarRefs[i].ref = ptr;
          staticVarRefs[i].size = info.fileSize;

          while (info.readPos < info.fileSize) {
            *ptr = ReadInt8(&info);
            ptr++;
          }

          CloseFile(&info);
        }
      }

      // set flag
      cachesEnabled |= ENABLE_STATIC_CACHE; 
    } else {
      PrintLog(PRINT_NORMAL, "Failed to allocate cache for static variables");
    }
  }

  if (cacheFlags & ENABLE_SPRITEBIN_CACHE) {
    spriteBinCache = (uint8_t*) malloc(SPRITEBIN_CACHE_SIZE * sizeof(uint8_t));
    if (spriteBinCache) {
      // only allocate sprite cache if spriteBinCache active
      if (cacheFlags & ENABLE_SPRITE_CACHE) {
        spriteCache = (uint8_t*) malloc(SPRITE_CACHE_SIZE * sizeof(uint8_t));
        if (spriteCache)
          cachesEnabled |= ENABLE_SPRITE_CACHE;
        else 
          PrintLog(PRINT_NORMAL, "Failed to allocate cache for sprites");
      }

      // TODO: read in sprite bins
    } else {
      PrintLog(PRINT_NORMAL, "Failed to allocate cache for sprite bins");
    }

    cachesEnabled |= ENABLE_SPRITEBIN_CACHE;
  }
}

void RSDK::FreeCache() {
  if (staticVarCache)
    free(staticVarCache);
}
