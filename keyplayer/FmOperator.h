#pragma once
#ifndef FmOperator_H
#define FmOperator_H

#include <cstdint>
#include "EnvelopeState.h"

class FmOperatorParam;

class FmOperator {
 public:
  void noteOn(const FmOperatorParam *p,
	      unsigned key,
	      unsigned velocity,
	      std::int32_t deltaPhi,
	      float levelCom);

  void noteOff(const FmOperatorParam *param);

  void handleAm(float gain, float lfo);
  
  void fillBuffer(float *out,
		  const float *in,
		  const float *mod,
		  float pitchMod,
		  float lfo,
		  int feedback);
  
 private:
  const FmOperatorParam *mParam;
  unsigned mPhi{0}, mDeltaPhiKey;
  float mDelay1, mDelay2;
  float mCurrAm;
  EnvelopeState mEnvelope;
};

#endif
