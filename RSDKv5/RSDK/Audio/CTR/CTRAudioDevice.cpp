using namespace RSDK;

#include <thread>

#if !(SAMPLE_USE_S16 || SAMPLE_USE_U8)
#error "ERROR: Incompatible sample format for 3DS builds."
#endif

#if SAMPLE_USE_S16
#define TARGET_OUT_FORMAT (NDSP_FORMAT_STEREO_PCM16)
#endif

// heavily referenced:  https://github.com/devkitPro/3ds-examples/blob/master/audio/opus-decoding/source/main.c

#define SAMPLE_RATE         (AUDIO_FREQUENCY)
#define SAMPLES_PER_BUF     (SAMPLE_RATE * 60 / 1000)
#define CHANNELS_PER_SAMPLE (2)
#define THREAD_STACK_SZ     (32 * 1024)
#define WAVEBUF_COUNT       (3)

static const int WAVEBUF_SIZE = SAMPLES_PER_BUF * CHANNELS_PER_SAMPLE *
                                sizeof(int16);

using namespace RSDK;

Thread RSDK::audioThreadHandle;
Handle audioThreadRequest;
volatile bool threadRunning;
ChannelInfo* streamChannel;

ndspWaveBuf wbuf[WAVEBUF_COUNT];
int16* audioBuffer = NULL;

void AudioCallback(void *const nul_) {
  (void) nul_;

  if (!threadRunning)
    return;

  svcSignalEvent(audioThreadRequest);
}

void AudioThread(void* arg);

inline s32 GetThreadPriority() {
  s32 prio = 0x30;
  svcGetThreadPriority(&prio, CUR_THREAD_HANDLE);
  prio -= 1;
  prio = prio < 0x18 ? 0x18 : prio;
  prio = prio > 0x3f ? 0x3f : prio;

  return prio;
}

// 3DS samples are SIGNED
inline s16 floatToS16(float f) {
  float fOut = (f * 32768);
  return (s16) fOut;
}

bool32 AudioDevice::Init()
{
  // taken from 3DS audio example
  ndspInit();
  ndspChnReset(0);
  ndspSetOutputMode(NDSP_OUTPUT_STEREO);
  ndspChnSetInterp(0, NDSP_INTERP_POLYPHASE);
  ndspChnSetRate(0, SAMPLE_RATE);
  ndspChnSetFormat(0, TARGET_OUT_FORMAT);

  AudioDeviceBase::InitAudioChannels();

  
  // allocate space for audio buffer
  const size_t bufferSize = WAVEBUF_SIZE * WAVEBUF_COUNT;
  audioBuffer = (int16*) linearAlloc(bufferSize);
  if (!audioBuffer) {
    PrintLog(PRINT_NORMAL, "ERROR: failed to allocate audio buffer\n");
    return false;
  }

  // set up ndsp wavebufs
  memset(&wbuf, 0, sizeof(wbuf));
  int16* buf = audioBuffer;
  for (int i = 0; i < WAVEBUF_COUNT; i++) {
    wbuf[i].data_vaddr = buf;
    wbuf[i].status     = NDSP_WBUF_DONE;

    buf += WAVEBUF_SIZE / sizeof(buf[0]);
  }

  APT_SetAppCpuTimeLimit(30);

  // create audio thread
  s32 prio = GetThreadPriority();
  threadRunning = true;
  audioThreadHandle = threadCreate(AudioThread, 0,
                                          THREAD_STACK_SZ, prio,
                                          1, false);
  if (!audioThreadHandle) {
    PrintLog(PRINT_NORMAL, "ERROR: failed to init audio thread\n");
  }

  // create event for audio thread 
  svcCreateEvent(&audioThreadRequest, RESET_ONESHOT);
  
  // set audio callback (simply triggers audioThreadRequest event)
  ndspSetCallback(AudioCallback, NULL);

  // set async stream channel ref to null 
  streamChannel = NULL;
  
  return true;
}

void AudioDevice::Release()
{
  // close thread
  threadRunning = false;
  svcSignalEvent(audioThreadRequest);

  threadJoin(audioThreadHandle, UINT64_MAX);
  threadFree(audioThreadHandle);

  // close event handle 
  svcCloseHandle(audioThreadRequest);

  // ... then de-init audio stuff 
  ndspChnReset(0);
  linearFree(audioBuffer);
  ndspExit();

  AudioDeviceBase::Release();
}

void AudioDevice::FrameInit() {
  //svcSignalEvent(audioThreadRequest);
}

void AudioDevice::HandleStreamLoad(ChannelInfo* channel, bool32 async)
{
  if (async) {
    //std::thread thread(LoadStream, channel);
    //thread.detach();
    streamChannel = channel;
  } else {
    LoadStream(channel, false);
  }
}

void AudioThread(void* arg) {
  PrintLog(PRINT_NORMAL, "CTRAudioDevice: audio thread created\n");

  while (threadRunning) {
    if (streamChannel) {
      LoadStream(streamChannel, true);
      streamChannel = NULL;
    }

    for (size_t i = 0; i < WAVEBUF_COUNT; i++) {
      if (wbuf[i].status != NDSP_WBUF_DONE)
        continue;

      AudioDevice::ProcessAudioMixing(wbuf[i].data_pcm16, 
                                      SAMPLES_PER_BUF * CHANNELS_PER_SAMPLE);
      wbuf[i].nsamples = SAMPLES_PER_BUF;
      ndspChnWaveBufAdd(0, &wbuf[i]);
      DSP_FlushDataCache(wbuf[i].data_pcm16, WAVEBUF_SIZE);
    }

    svcWaitSynchronization(audioThreadRequest, UINT64_MAX);
    svcClearEvent(audioThreadRequest);
  }
}
