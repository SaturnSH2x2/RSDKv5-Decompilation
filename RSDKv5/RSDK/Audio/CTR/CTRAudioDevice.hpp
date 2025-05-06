#define LOCK_TIMEOUT (32760)

namespace RSDK
{
  extern Thread audioThreadHandle;
  extern LightLock asyncFileLock;
  extern LightLock audioThreadLock;

  inline void LockAudioDevice() {
    for (int i = 0; i < LOCK_TIMEOUT; i++) {
      if (LightLock_TryLock(&audioThreadLock) == 0)
        break;
    }
  }

  inline void UnlockAudioDevice() {
    LightLock_Unlock(&audioThreadLock);
  }

  class AudioDevice : public AudioDeviceBase
  {
    public:
      static bool32 Init();
      static void Release();

      static void FrameInit();

      static void HandleStreamLoad(ChannelInfo* channel, bool32 async);

    private:
      static void InitAudioChannels();
      static void InitMixBuffer() { };
  };

} // namespace RSDK
