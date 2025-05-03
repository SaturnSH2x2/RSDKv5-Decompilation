#define LockAudioDevice() { }
#define UnlockAudioDevice() { }

namespace RSDK
{
  extern Thread audioThreadHandle;

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
