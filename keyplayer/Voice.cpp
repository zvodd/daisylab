#include "keyplayer.h"
#include "Channel.h"
#include "FmAlgorithm.h"
#include "Voice.h"
#include "Program.h"

Voice allVoices[NUM_VOICES];

void initVoices() {
  for (Voice &v: allVoices)
    v.Init();
}

Voice *findVoice(unsigned channel, unsigned key) {
  for (Voice &v: allVoices)
    if (v.getKey()==key)
      return &v;

  return nullptr;
}

Voice *allocateVoice(unsigned channel, unsigned key) {
  Voice *v=findVoice(channel, key);
  unsigned timestamp=getAudioPathTimestamp();
  
  if (!v) {
    v=&allVoices[0];
    
    for (unsigned i=1; i<NUM_VOICES; ++i) {
      v = v->voiceStealing(&allVoices[i], timestamp);
    }
  }
  
  return v;
}

void Voice::Init() {
  for (FmOperator &op: mOp)
    op.Init();
  mAlgorithm=nullptr;
}

void Voice::noteOn(const Channel *ch, unsigned key, unsigned velocity) {
  mChannel=ch;
  mProgram=ch->getProgram();
  mAlgorithm=FmAlgorithm::getAlgorithm(mProgram->algorithm);
  mKey=key;
  mGate=true;
  mTimestamp=getAudioPathTimestamp();

  std::int32_t deltaPhi=theKeyPlayer.midiToPhaseIncrement(key);
  float com=0.25f/mAlgorithm->getNumOutputs();
  for (unsigned i=0; i<NUM_OPERATORS; ++i) {
    float outputScaling=mAlgorithm->isOutput(i)? com : 1.0f;
    mOp[i].noteOn(&mProgram->op[i], key, velocity, deltaPhi, outputScaling);
  }
}

void Voice::noteOff() {
  mGate=false;
  mTimestamp=getAudioPathTimestamp();
  for (unsigned i=0; i<NUM_OPERATORS; ++i)
    mOp[i].noteOff(&mProgram->op[i]);
}

void Voice::addToBuffer(float *buffer) {
  float pitchMod=1.0;

  
  // Handle modulation of amplitude
  float volume=mChannel->getVolume();
  float lfo=1.0f;
  for (unsigned i=0; i<NUM_OPERATORS; ++i) {
    float nextAm=mAlgorithm->isOutput(i)? volume : 1.0f;
    mOp[i].handleAm(nextAm, lfo);
  }
  
  if (mAlgorithm) {
    mAlgorithm->fillBuffer(buffer, buffer, mOp, pitchMod, mProgram->feedback);
  }
}
