#include "RSDK/Core/RetroEngine.hpp"

#include "CacheFiles.cpp"

using namespace RSDK;

uint8_t RSDK::cachesEnabled = 0;

uint8_t* RSDK::staticVarCache = NULL;
uint8_t* RSDK::spriteBinCache = NULL;
uint8_t* RSDK::spriteCache = NULL;

StaticVarRef RSDK::staticVarRefs[STATIC_VAR_FILE_COUNT];
SpriteRef    RSDK::spriteBinRefs[SPRITEBIN_FILE_COUNT];

void RSDK::InitCache(uint8_t cacheFlags) {
  char pathBuf[0x40];
  uint8_t* ptr;
  FILE* fCache;
  int32 ptrOffset;

  if (cacheFlags & ENABLE_STATIC_CACHE) {
    // allocate cache
    staticVarCache = (uint8_t*) malloc(STATIC_VAR_CACHE_SIZE * sizeof(uint8_t));
    if (staticVarCache) {
      ptr = staticVarCache;

      // read data from decrypted cached file 
      sprintf_s(pathBuf, sizeof(pathBuf), "%sstaticcache.bin", SKU::userFileDir);
      fCache = fOpen(pathBuf, "rb");
      if (fCache) {
        for (int i = 0; i < STATIC_VAR_FILE_COUNT; i++) {
          fRead(staticVarRefs[i].hash, sizeof(uint8_t), 4, fCache);
          fRead(&ptrOffset, sizeof(int32), 1, fCache);
          fRead(&staticVarRefs[i].size, sizeof(int32), 1, fCache);

          staticVarRefs[i].ref = staticVarCache + ptrOffset;
        }

        fRead(staticVarCache, sizeof(uint8_t), STATIC_VAR_CACHE_SIZE, fCache);
        fClose(fCache);
      } else {
        // if not, read from Data file/folder and save whole thing to user directory
        fCache = fOpen(pathBuf, "wb+");

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

            ptrOffset = (int32) (ptr - staticVarCache);

            if (fCache) {
              fWrite(staticVarRefs[i].hash, sizeof(uint8_t), 4, fCache);
              fWrite(&ptrOffset, sizeof(int32), 1, fCache);
              fWrite(&staticVarRefs[i].size, sizeof(int32), 1, fCache);
            }

            while (info.readPos < info.fileSize) {
              *ptr = ReadInt8(&info);
              ptr++;
            }

            CloseFile(&info);
          }
        }

        if (fCache) {
          fWrite((void*) staticVarCache, sizeof(uint8_t), STATIC_VAR_CACHE_SIZE, fCache);
          fClose(fCache);
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

      sprintf_s(pathBuf, sizeof(pathBuf), "%ssbincache.bin", SKU::userFileDir);
      fCache = fOpen(pathBuf, "rb");

      if (fCache) {
        for (int i = 0; i < SPRITEBIN_FILE_COUNT; i++) {
          GEN_HASH_MD5(spriteBinFilenames[i], spriteBinRefs[i].hash);
          fRead(&ptrOffset, sizeof(int32), 1, fCache);

          spriteBinRefs[i].binRef = spriteBinCache + ptrOffset;
        }

        fRead(spriteBinCache, sizeof(uint8_t), SPRITEBIN_CACHE_SIZE, fCache);
        fClose(fCache);
      } else {
        fCache = fOpen(pathBuf, "wb+");

        char* strPtr;
        int sprSheetRef = 0;

        FileInfo binInfo, sprInfo;
        InitFileInfo(&binInfo);
        InitFileInfo(&sprInfo);

        ptr = spriteBinCache;

        for (int i = 0; i < SPRITEBIN_FILE_COUNT; i++) {
          sprintf_s(pathBuf, sizeof(pathBuf), "Data/Sprites/%s", spriteBinFilenames[i]);
          if (LoadFile(&binInfo, pathBuf, FMODE_RB)) {
            spriteBinRefs[i].binRef = ptr;
            GEN_HASH_MD5(spriteBinFilenames[i], spriteBinRefs[i].hash);

            ptrOffset = (int32) (ptr - spriteBinCache);

            if (fCache)
              fWrite(&ptrOffset, sizeof(int32), 1, fCache);

            while (binInfo.readPos < binInfo.fileSize) {
              *ptr = ReadInt8(&binInfo);
              ptr++;
            }

            CloseFile(&binInfo);
          }
        }

        if (fCache) {
          fWrite((void*) spriteBinCache, sizeof(uint8_t), SPRITEBIN_CACHE_SIZE, fCache);
          fClose(fCache);
        }
      }

    } else {
      PrintLog(PRINT_NORMAL, "Failed to allocate cache for sprite bins");
    }

    cachesEnabled |= ENABLE_SPRITEBIN_CACHE;
  }

  if (spriteBinCache) {
    // TODO: cache all GIF files
  }
}

void RSDK::FreeCache() {
  if (staticVarCache)
    free(staticVarCache);

  if (spriteBinCache)
    free(spriteBinCache);

  if (spriteCache)
    free(spriteCache);
}
