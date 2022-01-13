
//
//  PluginProcessor.hpp
//  AudioPluginDemo - All
//
//  Copyright © 2022 JUCE. All rights reserved.
//

/*
  ==============================================================================

   This file is part of the JUCE examples.
   Copyright (c) 2020 - Raw Material Software Limited

   The code included in this file is provided under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license. Permission
   To use, copy, modify, and/or distribute this software for any purpose with or
   without fee is hereby granted provided that the above copyright notice and
   this permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES,
   WHETHER EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR
   PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

/*******************************************************************************
 The block below describes the properties of this PIP. A PIP is a short snippet
 of code that can be read by the Projucer and used to generate a JUCE project.

 BEGIN_JUCE_PIP_METADATA

 name:                  AudioPluginDemo
 version:               1.0.0
 vendor:                JUCE
 website:               http://juce.com
 description:           Synthesiser audio plugin.

 dependencies:          juce_audio_basics, juce_audio_devices, juce_audio_formats,
                        juce_audio_plugin_client, juce_audio_processors,
                        juce_audio_utils, juce_core, juce_data_structures,
                        juce_events, juce_graphics, juce_gui_basics, juce_gui_extra
 exporters:             xcode_mac, vs2017, vs2019, linux_make, xcode_iphone, androidstudio

 moduleFlags:           JUCE_STRICT_REFCOUNTEDPOINTER=1

 type:                  AudioProcessor
 mainClass:             JuceDemoPluginAudioProcessor

 useLocalCopy:          1

 pluginCharacteristics: pluginIsSynth, pluginWantsMidiIn, pluginProducesMidiOut,
                        pluginEditorRequiresKeys
 extraPluginFormats:    AUv3

 END_JUCE_PIP_METADATA

*******************************************************************************/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "SineWaveVoice.h"
#include "SineWaveSound.h"
//TODO: Change Juce Header include to ifndef
#include <JuceHeader.h>

//==============================================================================
JuceDemoPluginAudioProcessor::JuceDemoPluginAudioProcessor()
    : AudioProcessor (getBusesProperties()),
      state (*this, nullptr, "state",
             { std::make_unique<AudioParameterFloat> ("gain",  "Gain",           NormalisableRange<float> (0.0f, 1.0f), 0.9f),
              std::make_unique<AudioParameterFloat> ("delay", "Delay Feedback", NormalisableRange<float> (0.0f, 100.0f), 0.5f),
             std::make_unique<AudioParameterFloat> ("mod", "Modulation", NormalisableRange<float> (0.0f, 100.0f), 0.5f),
          std::make_unique<AudioParameterInt> ("indexNum", "Index Numerator", 1, 99, 1),
          std::make_unique<AudioParameterInt> ("indexDen", "Index Denominator", 1, 20, 2), //min, max, default
          std::make_unique<AudioParameterFloat> ("attack", "Attack", NormalisableRange<float> (0.0f, 100.0f), 0.5f), //min, max, default
          
          //TODO: define variables for sustain, release

      })

           //    std::make_unique<AudioParameterFloat> ("delay", "Delay Feedback", NormalisableRange<float> (0.0f, 1.0f), 0.5f) })
{
    // Add a sub-tree to store the state of our UI
    state.state.addChild ({ "uiState", { { "width",  800 }, { "height", 200 } }, {} }, -1, nullptr);

    initialiseSynth();
}

JuceDemoPluginAudioProcessor::~JuceDemoPluginAudioProcessor()
{
}
    
//==============================================================================
bool JuceDemoPluginAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    // Only mono/stereo and input/output must have same layout
    const auto& mainOutput = layouts.getMainOutputChannelSet();
    const auto& mainInput  = layouts.getMainInputChannelSet();

    // input and output layout must either be the same or the input must be disabled altogether
    if (! mainInput.isDisabled() && mainInput != mainOutput)
        return false;

    // do not allow disabling the main buses
    if (mainOutput.isDisabled())
        return false;

    // only allow stereo and mono
    if (mainOutput.size() > 2)
        return false;

    return true;
}

void JuceDemoPluginAudioProcessor::prepareToPlay (double newSampleRate, int /*samplesPerBlock*/)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    synth.setCurrentPlaybackSampleRate (newSampleRate);
    keyboardState.reset();

    if (isUsingDoublePrecision())
    {
        delayBufferDouble.setSize (2, 12000);
        delayBufferFloat .setSize (1, 1);
    }
    else
    {
        delayBufferFloat .setSize (2, 12000);
        delayBufferDouble.setSize (1, 1);
    }

    reset();
}

void JuceDemoPluginAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
    keyboardState.reset();
}

void JuceDemoPluginAudioProcessor::reset()
{
    // Use this method as the place to clear any delay lines, buffers, etc, as it
    // means there's been a break in the audio's continuity.
    delayBufferFloat .clear();
    delayBufferDouble.clear();
}

//==============================================================================
void JuceDemoPluginAudioProcessor::processBlock (AudioBuffer<float>& buffer, MidiBuffer& midiMessages)
{
    jassert (! isUsingDoublePrecision());
    process (buffer, midiMessages, delayBufferFloat);
}

void JuceDemoPluginAudioProcessor::processBlock (AudioBuffer<double>& buffer, MidiBuffer& midiMessages)
{
    jassert (isUsingDoublePrecision());
    process (buffer, midiMessages, delayBufferDouble);
}

//==============================================================================
bool JuceDemoPluginAudioProcessor::hasEditor() const
{
    return true;
}

AudioProcessorEditor* JuceDemoPluginAudioProcessor::createEditor()
{
    return new JuceDemoPluginAudioProcessorEditor (*this);
}

//==============================================================================
const String JuceDemoPluginAudioProcessor::getName() const                             { return "FMPlugin"; }
bool JuceDemoPluginAudioProcessor::acceptsMidi() const                                 { return true; }
bool JuceDemoPluginAudioProcessor::producesMidi() const                                { return true; }
double JuceDemoPluginAudioProcessor::getTailLengthSeconds() const                    { return 0.0; }

//==============================================================================
int JuceDemoPluginAudioProcessor::getNumPrograms()                                      { return 0; }
int JuceDemoPluginAudioProcessor::getCurrentProgram()                                   { return 0; }
void JuceDemoPluginAudioProcessor::setCurrentProgram (int)                              {}
const String JuceDemoPluginAudioProcessor::getProgramName (int)                        { return {}; }
void JuceDemoPluginAudioProcessor::changeProgramName (int, const String&)              {}

//==============================================================================
void JuceDemoPluginAudioProcessor::getStateInformation (MemoryBlock& destData)
{
    // Store an xml representation of our state.
    if (auto xmlState = state.copyState().createXml())
        copyXmlToBinary (*xmlState, destData);
}

void JuceDemoPluginAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // Restore our plug-in's state from the xml representation stored in the above
    // method.
    if (auto xmlState = getXmlFromBinary (data, sizeInBytes))
        state.replaceState (ValueTree::fromXml (*xmlState));
}

//==============================================================================
void JuceDemoPluginAudioProcessor::updateTrackProperties (const TrackProperties& properties)
{
    {
        const ScopedLock sl (trackPropertiesLock);
        trackProperties = properties;
    }

     MessageManager::callAsync ([this]
    {
        if (auto* editor = dynamic_cast<JuceDemoPluginAudioProcessorEditor*> (getActiveEditor()))
             editor->updateTrackProperties();
    });
}

juce::AudioProcessor::TrackProperties JuceDemoPluginAudioProcessor::getTrackProperties() const
{
    const ScopedLock sl (trackPropertiesLock);
    return trackProperties;
}

//==============================================================================
template <typename FloatType>
void JuceDemoPluginAudioProcessor::process (AudioBuffer<FloatType>& buffer, MidiBuffer& midiMessages, AudioBuffer<FloatType>& delayBuffer)
{
    auto gainParamValue  = state.getParameter ("gain") ->getValue();
    auto delayParamValue = state.getParameter ("delay")->getValue();
    auto modParamValue = state.getParameter ("mod")->getValue();
    auto numParamValue = state.getParameter ("indexNum")->getValue();
    auto denParamValue = state.getParameter ("indexDen")->getValue();


    auto numSamples = buffer.getNumSamples();

    // In case we have more outputs than inputs, we'll clear any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    for (auto i = getTotalNumInputChannels(); i < getTotalNumOutputChannels(); ++i)
        buffer.clear (i, 0, numSamples);

    //synth.defineModulation(1.0, delayParamValue);
    // Now pass any incoming midi messages to our keyboard state object, and let it
    // add messages to the buffer if the user is clicking on the on-screen keys
    keyboardState.processNextMidiBuffer (midiMessages, 0, numSamples, true);

    // and now get our synth to process these midi events and generate its output.
    for (int i = 0; i < synth.getNumVoices(); i++) {
        (synth.getVoice(i))->controllerMoved(0, 100 * modParamValue);
        (synth.getVoice(i))->controllerMoved(1, 100 * numParamValue);
        (synth.getVoice(i))->controllerMoved(2, 100 * denParamValue);
    }
    synth.renderNextBlock (buffer, midiMessages, 0, numSamples);

    // Apply our delay effect to the new output..
    applyDelay (buffer, delayBuffer, delayParamValue);

    // Apply our gain change to the outgoing data..
    applyGain (buffer, delayBuffer, gainParamValue);

    // Now ask the host for the current time so we can store it to be displayed later...
    updateCurrentTimeInfoFromHost();
}

template <typename FloatType>
void JuceDemoPluginAudioProcessor::applyGain (AudioBuffer<FloatType>& buffer, AudioBuffer<FloatType>& delayBuffer, float gainLevel)
{
    ignoreUnused (delayBuffer);

    for (auto channel = 0; channel < getTotalNumOutputChannels(); ++channel)
        buffer.applyGain (channel, 0, buffer.getNumSamples(), gainLevel);
}

template <typename FloatType>
void JuceDemoPluginAudioProcessor::applyDelay (AudioBuffer<FloatType>& buffer, AudioBuffer<FloatType>& delayBuffer, float delayLevel)
{
    auto numSamples = buffer.getNumSamples();

    auto delayPos = 0;

    for (auto channel = 0; channel < getTotalNumOutputChannels(); ++channel)
    {
        auto channelData = buffer.getWritePointer (channel);
        auto delayData = delayBuffer.getWritePointer (jmin (channel, delayBuffer.getNumChannels() - 1));
        delayPos = delayPosition;

        for (auto i = 0; i < numSamples; ++i)
        {
            auto in = channelData[i];
            channelData[i] += delayData[delayPos];
            delayData[delayPos] = (delayData[delayPos] + in) * delayLevel;

            if (++delayPos >= delayBuffer.getNumSamples())
                delayPos = 0;
        }
    }
    delayPosition = delayPos;
}

void JuceDemoPluginAudioProcessor::initialiseSynth()
{
    auto numVoices = 8;

    // Add some voices...
    for (auto i = 0; i < numVoices; ++i)
        synth.addVoice (new SineWaveVoice());

    // ..and give the synth a sound to play
    synth.addSound (new SineWaveSound());
}

void JuceDemoPluginAudioProcessor::updateCurrentTimeInfoFromHost()
{
    const auto newInfo = [&]
    {
        if (auto* ph = getPlayHead())
        {
            AudioPlayHead::CurrentPositionInfo result;

            if (ph->getCurrentPosition (result))
                return result;
        }

        // If the host fails to provide the current time, we'll just use default values
        AudioPlayHead::CurrentPositionInfo result;
        result.resetToDefault();
        return result;
    }();

    lastPosInfo.set (newInfo);
}

juce::AudioProcessor::BusesProperties JuceDemoPluginAudioProcessor::getBusesProperties()
{
    return BusesProperties().withInput  ("Input",  AudioChannelSet::stereo(), true)
                            .withOutput ("Output", AudioChannelSet::stereo(), true);
}
//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
return new JuceDemoPluginAudioProcessor();
}
