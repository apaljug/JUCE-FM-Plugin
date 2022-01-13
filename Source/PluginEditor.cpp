//
//  PluginProcessorEditor.hpp
//  AudioPluginDemo - All
//
//  Created by Keary Mobley on 1/13/22.
//  Copyright © 2022 JUCE. All rights reserved.
//

#include "PluginEditor.h"



JuceDemoPluginAudioProcessorEditor::JuceDemoPluginAudioProcessorEditor (JuceDemoPluginAudioProcessor& owner)
    : AudioProcessorEditor (owner),
      midiKeyboard         (owner.keyboardState, MidiKeyboardComponent::horizontalKeyboard),
      gainAttachment       (owner.state, "gain",  gainSlider),
      delayAttachment      (owner.state, "delay", delaySlider),
      modAttachment        (owner.state, "mod", modSlider),
      attackAttachment (owner.state, "attack", attackSlider),
      iNumAttachment   (owner.state, "indexNum", iNumBox),
      iDenAttachment   (owner.state, "indexDen", iDenBox)
{
    // add some sliders..
    addAndMakeVisible (gainSlider);
    gainSlider.setSliderStyle (Slider::Rotary);

    addAndMakeVisible (delaySlider);
    delaySlider.setSliderStyle (Slider::Rotary);
    
    addAndMakeVisible (modSlider);
    modSlider.setSliderStyle (Slider::Rotary);
    
    
    addAndMakeVisible (attackSlider);
    attackSlider.setSliderStyle (Slider::Rotary);
    
    // add some checkboxes:
    
    addAndMakeVisible(iNumBox);
    
    for (int i = 1; i < 100; i++)
    { iNumBox.addItem(std::to_string(i), i);
    }
    
    addAndMakeVisible(iDenBox);
    for (int i = 1; i <= 20; i++)
    {
        iDenBox.addItem(std::to_string(i), i);
    }
        
        


    attackLabel.attachToComponent (&attackSlider, false);
    attackLabel.setFont (Font (11.0f));
    
    // add some labels for the sliders..
    gainLabel.attachToComponent (&gainSlider, false);
    gainLabel.setFont (Font (11.0f));

    delayLabel.attachToComponent (&delaySlider, false);
    delayLabel.setFont (Font (11.0f));
    
    modLabel.attachToComponent (&modSlider, false);
    modLabel.setFont (Font (11.0f));
    
    numLabel.attachToComponent (&iNumBox, false);
    numLabel.setFont (Font (11.0f));
    
    
    
    denLabel.attachToComponent (&iDenBox, false);
    denLabel.setFont (Font (11.0f));

    // add the midi keyboard component..
    addAndMakeVisible (midiKeyboard);

    // add a label that will display the current timecode and status..
    addAndMakeVisible (timecodeDisplayLabel);
    timecodeDisplayLabel.setFont (Font (Font::getDefaultMonospacedFontName(), 15.0f, Font::plain));

    // set resize limits for this plug-in
    setResizeLimits (400, 200, 1024, 700);

    lastUIWidth .referTo (owner.state.state.getChildWithName ("uiState").getPropertyAsValue ("width",  nullptr));
    lastUIHeight.referTo (owner.state.state.getChildWithName ("uiState").getPropertyAsValue ("height", nullptr));

    // set our component's initial size to be the last one that was stored in the filter's settings
    setSize (lastUIWidth.getValue(), lastUIHeight.getValue());

    lastUIWidth. addListener (this);
    lastUIHeight.addListener (this);

    updateTrackProperties();

    // start a timer which will keep our timecode display updated
    startTimerHz (30);
}

JuceDemoPluginAudioProcessorEditor::~JuceDemoPluginAudioProcessorEditor()
{
}

//==============================================================================
void JuceDemoPluginAudioProcessorEditor::paint (Graphics& g)
{
    g.setColour (backgroundColour);
    g.fillAll();
}

void JuceDemoPluginAudioProcessorEditor::resized()
{
    // This lays out our child components...

    auto r = getLocalBounds().reduced (8);

    timecodeDisplayLabel.setBounds (r.removeFromTop (26));
    midiKeyboard        .setBounds (r.removeFromBottom (70));

    r.removeFromTop (20);
    auto sliderArea = r.removeFromTop (60);
    gainSlider.setBounds  (sliderArea.removeFromLeft (jmin (180, sliderArea.getWidth() / 3)));
    delaySlider.setBounds (sliderArea.removeFromLeft (jmin (180, sliderArea.getWidth() * 2 / 3)));
    modSlider.setBounds (sliderArea.removeFromLeft (jmin (180, sliderArea.getWidth())));
    iNumBox.setBounds (sliderArea.removeFromLeft (jmin (180, sliderArea.getWidth())));
    iDenBox.setBounds (sliderArea.removeFromLeft (jmin (180, sliderArea.getWidth())));
    //attackSlider.setBounds  (sliderArea.removeFromLeft (jmin (180, sliderArea.getWidth() / 2)));

    lastUIWidth  = getWidth();
    lastUIHeight = getHeight();
}

void JuceDemoPluginAudioProcessorEditor::timerCallback()
{
    updateTimecodeDisplay (getProcessor().lastPosInfo.get());
}

void JuceDemoPluginAudioProcessorEditor::hostMIDIControllerIsAvailable (bool controllerIsAvailable)
{
    midiKeyboard.setVisible (! controllerIsAvailable);
}

int JuceDemoPluginAudioProcessorEditor::getControlParameterIndex (Component& control)
{
    if (&control == &gainSlider)
        return 0;

    if (&control == &delaySlider)
        return 1;

    return -1;
}

void JuceDemoPluginAudioProcessorEditor::updateTrackProperties()
{
    auto trackColour = getProcessor().getTrackProperties().colour;
    auto& lf = getLookAndFeel();

    backgroundColour = (trackColour == Colour() ? lf.findColour (ResizableWindow::backgroundColourId)
                                                : trackColour.withAlpha (1.0f).withBrightness (0.266f));
    repaint();
}
    


//==============================================================================
JuceDemoPluginAudioProcessor& JuceDemoPluginAudioProcessorEditor::getProcessor() const
{
    return static_cast<JuceDemoPluginAudioProcessor&> (processor);
}

//==============================================================================
// quick-and-dirty function to format a timecode string
String JuceDemoPluginAudioProcessorEditor::timeToTimecodeString (double seconds)
{
    auto millisecs = roundToInt (seconds * 1000.0);
    auto absMillisecs = std::abs (millisecs);

    return String::formatted ("%02d:%02d:%02d.%03d",
                              millisecs / 3600000,
                              (absMillisecs / 60000) % 60,
                              (absMillisecs / 1000)  % 60,
                              absMillisecs % 1000);
}

// quick-and-dirty function to format a bars/beats string
String JuceDemoPluginAudioProcessorEditor::quarterNotePositionToBarsBeatsString (double quarterNotes, int numerator, int denominator)
{
    if (numerator == 0 || denominator == 0)
        return "1|1|000";

    auto quarterNotesPerBar = (numerator * 4 / denominator);
    auto beats  = (fmod (quarterNotes, quarterNotesPerBar) / quarterNotesPerBar) * numerator;

    auto bar    = ((int) quarterNotes) / quarterNotesPerBar + 1;
    auto beat   = ((int) beats) + 1;
    auto ticks  = ((int) (fmod (beats, 1.0) * 960.0 + 0.5));

    return String::formatted ("%d|%d|%03d", bar, beat, ticks);
}

// Updates the text in our position label.
void JuceDemoPluginAudioProcessorEditor::updateTimecodeDisplay (AudioPlayHead::CurrentPositionInfo pos)
{
    MemoryOutputStream displayText;

    displayText << "[" << SystemStats::getJUCEVersion() << "]   "
    << String (pos.bpm, 2) << " bpm, "
    << pos.timeSigNumerator << '/' << pos.timeSigDenominator
    << "  -  " << timeToTimecodeString (pos.timeInSeconds)
    << "  -  " << quarterNotePositionToBarsBeatsString (pos.ppqPosition,
                                                        pos.timeSigNumerator,
                                                        pos.timeSigDenominator);

    if (pos.isRecording)
        displayText << "  (recording)";
    else if (pos.isPlaying)
        displayText << "  (playing)";

    timecodeDisplayLabel.setText (displayText.toString(), dontSendNotification);
}

// called when the stored window size changes
void JuceDemoPluginAudioProcessorEditor::valueChanged (Value&)
{
    setSize (lastUIWidth.getValue(), lastUIHeight.getValue());
}
