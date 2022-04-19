
//  SineWaveVoice.hpp
//  AudioPluginDemo - All
//

//  Copyright © 2022 JUCE. All rights reserved.
//
#include "SineWaveVoice.h"
#include "SineWaveSound.h"
//==============================================================================
/** A simple demo synth voice that just plays a sine wave.. */
SineWaveVoice::SineWaveVoice() {}

  bool SineWaveVoice::canPlaySound (SynthesiserSound* sound)
  {
      return dynamic_cast<SineWaveSound*> (sound) != nullptr;
  }

  void SineWaveVoice::startNote (int midiNoteNumber, float velocity,
                  SynthesiserSound* /*sound*/, int)
  {
  
      
     
      //release  = release_param;
      
      /*adsrParams.attack = attack_param;//0.01f;
      adsrParams.sustain = sustain_param;//1.0f;
      adsrParams.release = release_param;//0.01f;
      */
      adsrParams.decay = 10.0f;

      currentAngle = 0.0;
      level = velocity * adsrParams.sustain;
      release = 0.0;
      attack = .1;
      
      mrelease = 0.0;
      mattack = .1;

      if (modRatioDen == 0) {
          modRatio = 1.0;
      } else {
          modRatio = (float(modRatioNum) / float(modRatioDen));
      }
      auto cyclesPerSecond = MidiMessage::getMidiNoteInHertz (midiNoteNumber);
      auto cyclesPerSample = (cyclesPerSecond)/ getSampleRate();

      angleDelta = cyclesPerSample * MathConstants<double>::twoPi;
      int modFreq = int(cyclesPerSecond * modRatio); //new
      // float freqDev = 10; //new
      
      auto modCyclesPerSample = modFreq / getSampleRate(); //new
      modAngleDelta = modCyclesPerSample * MathConstants<double>::twoPi;//new
      modIndex = freqDev / float(modFreq); //new
      adsr.noteOn();
      
  }
float SineWaveVoice::chebyshevCalulation(int chebyshev, float x)
{
    if (chebyshev == 0)
    {
        return 1;
    }
    if (chebyshev == 1)
    {
        return x;
    }
    else
    {
        return 2 * x * chebyshevCalulation(chebyshev-1, x) - chebyshevCalulation(chebyshev-2, x);
        
    }
}
  void SineWaveVoice::stopNote (float /*velocity*/, bool allowTailOff)
  {
      if (allowTailOff)
      {
          // start a tail-off by setting this flag. The render callback will pick up on
          // this and do a fade out, calling clearCurrentNote() when it's finished.

          if (release == 0.0) {// we only need to begin a tail-off if it's not already doing so - the
                              // stopNote method could be called more than once.
              release = adsrParams.release;
          }
      }
      else
      {
          // we're being told to stop playing immediately, so reset everything..

          clearCurrentNote();
          angleDelta = 0.0;
      }
      adsr.noteOff();
  }

  void SineWaveVoice::pitchWheelMoved (int /*newValue*/)
  {
      // not implemented for the purposes of this demo!
  }
  
  
  void SineWaveVoice::controllerMoved (int paramNum, int value)
  {
      
      if (paramNum == 0) { //freqDev
          freqDev = value;
      } else if (paramNum == 1) {
          modRatioNum = value;
      } else if (paramNum == 2) {
          modRatioDen = value;
      } else if (paramNum == 3) {
          adsrParams.attack = float(value / 100.0f);
      } else if (paramNum == 4) {
          adsrParams.sustain = 0.185 * float(value / 100.0f) ;
      } else if (paramNum == 5) {
          adsrParams.release    = float(value / 100.0f);
      } else if (paramNum == 6) {
          modAdsrParams.attack = float(value / 100.0f);
     } else if (paramNum == 7) {
         modAdsrParams.sustain = float(value / 100.0f) ;
     } else if (paramNum == 8) {
         modAdsrParams.release    = float(value / 100.0f);
     } else if (paramNum == 14) {
         chebyshevAmp1 = float(value / 100.0f);
     } else if (paramNum == 15) {
         chebyshevAmp2 = float(value / 100.0f);
     } else if (paramNum == 16) {
         chebyshevAmp3 = float(value / 100.0f);
     } else if (paramNum == 17) {
         chebyshevAmp4 = float(value / 100.0f);
     } else if (paramNum == 18) {
         chebyshevAmp5 = float(value / 100.0f);
     }  else if (paramNum == 19) {
         chebyshevAmp6 = float(value / 100.0f);
     }  else if (paramNum == 20) {
         chebyshevAmp7 = float(value / 100.0f);
     }  else if (paramNum == 21) {
        chebyshevAmp8 = float(value / 100.0f);
     }  else if (paramNum == 22) {
            chebyshevAmp9 = float(value / 100.0f);
     }
      
      /*else if (paramNum == 9) {
         if (value == 0) {
             att_is_exp = 1;
         } else {
             att_is_exp = 0;
         }
         if (value == 0) {
             dec_is_exp = 1;
         } else {
             dec_is_exp = 0;
         }
     } else if (paramNum == 10) {
         if (value == 1) {
             att_is_exp = 1;
         } else {
             att_is_exp = 0;
         }
         if (value == 1) {
             dec_is_exp = 1;
         } else {
             dec_is_exp = 0;
         }
        
      dec_is_exp = value;
     } else if (paramNum == 11) {
             
         if (value == 2) {
             dec_is_exp = 0;
         } else {
             att_is_exp = 1;
         }
         if (value == 2) {
             modatt_is_exp = 1;
         } else {
             modatt_is_exp = 0;
         }
     } else if (paramNum == 12) {
         if (value == 3) {
             att_is_exp = 1;
         } else {
             att_is_exp = 0;
         }
         if (value == 3) {
             moddec_is_exp = 1;
         } else {
             moddec_is_exp = 0;
         }
     }*/
  }

  void SineWaveVoice::renderNextBlock (AudioBuffer<float>& outputBuffer, int startSample, int numSamples)
  {
      if (angleDelta != 0.0)
      {
         
        if (release > 0.0)
          {
              while (--numSamples >= 0)
              {
                  // auto currentSample = (float) (sin (currentAngle) * level * release);
                  auto currentSample = (float) (sin (currentAngle + mrelease * mattack * modAdsrParams.sustain * modIndex * sin( modCurrentAngle ) ) );
                  auto chebyshevsignal = (float) (chebyshevAmp1 * currentSample) + (chebyshevAmp2 * chebyshevCalulation(2, currentSample)) + (chebyshevAmp3 * chebyshevCalulation(3, currentSample)) + (chebyshevAmp4 * chebyshevCalulation(4, currentSample)) + (chebyshevAmp5 * chebyshevCalulation(5, currentSample)) + (chebyshevAmp6 * chebyshevCalulation(6, currentSample)) + (chebyshevAmp7 * chebyshevCalulation(7, currentSample)) + (chebyshevAmp8 * chebyshevCalulation(8, currentSample)) + (chebyshevAmp9 * chebyshevCalulation(9, currentSample));
                  currentSample = (float) ( level * release * attack *  chebyshevsignal);

                  for (auto i = outputBuffer.getNumChannels(); --i >= 0;)
                      outputBuffer.addSample (i, startSample, currentSample);

                  currentAngle += angleDelta;
                  modCurrentAngle += modAngleDelta;
                  ++startSample;

                  if (adsrParams.release != 0) {
                      release *= 1.0 - pow(.1, adsrParams.release * 5.0) ;
                  } else {
                      release *= 1.0;
                  }
                  
                  if (modAdsrParams.release != 0) {
                      mrelease *= 1.0 - pow(.1, modAdsrParams.release * 5.0) ;
                  } else {
                      mrelease = 1.0;
                  }
                  if (release <= 0.005)
                  {
                      // tells the synth that this voice has stopped
                      clearCurrentNote();

                      angleDelta = 0.0;
                      break;
                  }
              }
          }
          else
          {
              while (--numSamples >= 0)
              {
                  if (attack >= .999)
                  {
                      attack = 1.0;
                  } else {
                      if (adsrParams.attack != 0.0){
                          if (att_is_exp) {
                              attack *= 1.0 + pow(.1, adsrParams.attack * 5.0);
                          } else {
                              attack +=  pow(.1, adsrParams.attack * 5.0);
                          }
                      } else {
                          attack = 1.0;
                      }
                  }
                  
                  if (mattack >= .999)
                  {
                      mattack = 1.0;
                  } else {
                      if (modAdsrParams.attack != 0.0){
                          mattack *= 1.0 + pow(.1, modAdsrParams.attack * 5.0);
                      } else {
                          mattack = 1.0;
                      }
                  }
                  
                  
                  
                  //auto currentSample = (float) (sin (currentAngle )) * level);
                  auto currentSample = (float) (sin (currentAngle  +  mattack * modAdsrParams.sustain * modIndex * sin( modCurrentAngle ) ) );
                  auto chebyshevsignal = (float) (chebyshevAmp1 * currentSample) + (chebyshevAmp2 * chebyshevCalulation(2, currentSample)) + (chebyshevAmp3 * chebyshevCalulation(3, currentSample)) + (chebyshevAmp4 * chebyshevCalulation(4, currentSample)) + (chebyshevAmp5 * chebyshevCalulation(5, currentSample)) + (chebyshevAmp6 * chebyshevCalulation(6, currentSample)) + (chebyshevAmp7 * chebyshevCalulation(7, currentSample)) + (chebyshevAmp8 * chebyshevCalulation(8, currentSample)) + (chebyshevAmp9 * chebyshevCalulation(9, currentSample));
                  currentSample = (float) ( level * release * attack *  chebyshevsignal);
                  
                  for (auto i = outputBuffer.getNumChannels(); --i >= 0;)
                      outputBuffer.addSample (i, startSample, currentSample);

                  currentAngle += angleDelta;
                  modCurrentAngle += modAngleDelta;

                  ++startSample;

                  
              }
          }
          
          //adsr.setParameters(adsrParams);
          //adsr.applyEnvelopeToBuffer(outputBuffer, startSample, numSamples);
      }
  }
