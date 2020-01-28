/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"


// Simple delay line with fractional access.
class tappedDelayLine 
{
public:  
    // Initialisation.
    void updateSize(long int size);
    void updateSize(float ms, double sampleRate);
    void clear();

    // Add new sample to buffer and remove last sample.
    void push(float value);
    
    // Get sample at specified delay.
    float get(); // Get the last sample.
    float get(int n); // Get nth sample from newest sample.
    float get(float frac); // Get sample fraction way through.
    
private:
    int readOffset = 0;
    std::vector <float> delayLine;
};

class Reverb1AudioProcessor  : public AudioProcessor
{
public:
    //==============================================================================
    Reverb1AudioProcessor();
    ~Reverb1AudioProcessor();

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (AudioBuffer<float>&, MidiBuffer&) override;

    //==============================================================================
    AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const String getProgramName (int index) override;
    void changeProgramName (int index, const String& newName) override;

    //==============================================================================
    void getStateInformation (MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    float mDelay = 2000.0f; // Reverb length in ms.
    float mPreDelay = 6.0f;
    
    
private:
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Reverb1AudioProcessor)
    
    tappedDelayLine delayLine[2]; // Delay line for early taps.
    tappedDelayLine combs[2][6];     // Delay lines for comb filters.
    tappedDelayLine late[2];
    
    // Early reflection taps.
    double tapTimes [18] = {
        .0043, .0215, .0225,
        .0268, .0270, .0298,
        .0458, .0485, .0572,
        .0587, .0595, .0612,
        .0707, .0708, .0726, 
        .0741, .0753, .0797
    };
    
    double tapGain [18] = {
        .841, .504, .491,
        .379, .380, .346,
        .289, .272, .192,
        .193, .217, .181,
        .180, .181, .176,
        .142, .167, .134
    };
    
    double combTimes [6] = {
        0.050, 0.056, 0.061,
        0.068, 0.072, 0.078
    };
    
    float combGain = 0.7;
};
