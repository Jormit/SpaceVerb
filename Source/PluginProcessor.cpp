/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
Reverb1AudioProcessor::Reverb1AudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
}

Reverb1AudioProcessor::~Reverb1AudioProcessor()
{
}

//==============================================================================
const String Reverb1AudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool Reverb1AudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool Reverb1AudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool Reverb1AudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double Reverb1AudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int Reverb1AudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int Reverb1AudioProcessor::getCurrentProgram()
{
    return 0;
}

void Reverb1AudioProcessor::setCurrentProgram (int index)
{
}

const String Reverb1AudioProcessor::getProgramName (int index)
{
    return {};
}

void Reverb1AudioProcessor::changeProgramName (int index, const String& newName)
{
}

//==============================================================================
void Reverb1AudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{    
    // Setup left and right delay lines.
    for (int channel = 0; channel < 2; channel++) 
    {
        // Setup delay line for early taps.
        delayLine[channel].updateSize(mDelay, sampleRate);
        delayLine[channel].clear();
        
        late[channel].updateSize(mPreDelay, sampleRate);
        late[channel].clear();
        
        for (int num = 0; num < 6; num++) 
        {
            combs[channel][num].updateSize((float)combTimes[num], sampleRate);
            combs[channel][num].clear();
        }
    }
}

void Reverb1AudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool Reverb1AudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    if (layouts.getMainOutputChannelSet() != AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

//============================================================================================
// MAIN LOOP.
//============================================================================================
void Reverb1AudioProcessor::processBlock (AudioBuffer<float>& buffer, MidiBuffer& midiMessages)
{
    ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer (channel);        

        for (int sample = 0; sample < buffer.getNumSamples(); sample++) 
        {
            float raw = channelData[sample];
            
            delayLine[channel].push(channelData[sample]);
            
            // Early reflections.
            channelData[sample] = 0.0f;
            for (int tap = 0; tap < 18; tap++)
            {
                channelData[sample] += delayLine[channel].get((float)tapTimes[tap]) * tapGain[tap];
            }        
            
            float early = channelData[sample];
            
            // Late reflections.
            for (int comb = 0; comb < 6; comb++)
            {
                float out = combs[channel][comb].get();                
                combs[channel][comb].push(0.3 * channelData[sample] + 0.7 * out);               
                
                channelData[sample] = combs[channel][comb].get();                
            }
            
            late[channel].push(channelData[sample]);
            
            channelData[sample] = raw * 0.7 + 0.2 * late[channel].get() + 0.1 * early;           
     
        }
    }
}

//==============================================================================
bool Reverb1AudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

AudioProcessorEditor* Reverb1AudioProcessor::createEditor()
{
    return new Reverb1AudioProcessorEditor (*this);
}

//==============================================================================
void Reverb1AudioProcessor::getStateInformation (MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void Reverb1AudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new Reverb1AudioProcessor();
}


//===============================================================================
// Delay line implementation.
//===============================================================================

void tappedDelayLine::push(float value) 
{
    delayLine[readOffset] = value;
    readOffset = (readOffset + 1) % delayLine.size();
}

// Get fully delayed sample.
float tappedDelayLine::get()
{
    return delayLine[readOffset];
}

// Get sample at n samples delay.
float tappedDelayLine::get(int n)
{
    jassert(n < delayLine.size());
    return delayLine[(readOffset + delayLine.size() - n) % delayLine.size()];
}

// Fractional access.
float tappedDelayLine::get(float frac)
{
    jassert(frac < 1.0f);
    return tappedDelayLine::get((int)(frac * delayLine.size()));
}

// Set all samples to 0.0f.
void tappedDelayLine::clear()
{
    for (int i = 0; i < delayLine.size(); i++)
    {
        delayLine[i] = 0.0f;
    }
}

// Update size by number of samples.
void tappedDelayLine::updateSize(long int size) 
{
    delayLine.resize(size);
    readOffset = readOffset % delayLine.size();
}

// Update size by time in milliseconds.
void tappedDelayLine::updateSize(float ms, double sampleRate) 
{
    this->updateSize((int) ((ms / 1000.0f) * (sampleRate)));
}
