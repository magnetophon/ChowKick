#include "ChowKick.h"
#include "gui/CustomLNFs.h"
#include "gui/DisabledSlider.h"
#include "gui/FilterViewer.h"
#include "gui/PulseViewer.h"

ChowKick::ChowKick() : trigger (vts),
                       resFilter (vts, trigger),
                       outFilter (vts)
{
    scope = magicState.createAndAddObject<foleys::MagicOscilloscope> ("scope");
}

void ChowKick::addParameters (Parameters& params)
{
    Trigger::addParameters (params);
    PulseShaper::addParameters (params);
    ResonantFilter::addParameters (params);
    OutputFilter::addParameters (params);
}

void ChowKick::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    monoBuffer.setSize (1, samplesPerBlock);
    fourVoiceBuffer = dsp::AudioBlock<Vec> (fourVoiceData, 1, (size_t) samplesPerBlock);

    pulseShaper = std::make_unique<PulseShaper> (vts, sampleRate);
    resFilter.reset (sampleRate);
    outFilter.reset (sampleRate);

    dcBlocker.prepare ({ sampleRate, (uint32) samplesPerBlock, 1 });
    dcBlocker.setCutoffFrequency (10.0f);

    scope->prepareToPlay (sampleRate, samplesPerBlock);
}

void ChowKick::releaseResources()
{
}

void reduceBlock (const dsp::AudioBlock<Vec>& block4, AudioBuffer<float>& buffer)
{
    auto* x = block4.getChannelPointer (0);
    auto* y = buffer.getWritePointer (0);
    for (int i = 0; i < buffer.getNumSamples(); ++i)
        y[i] = x[i].sum();
}

void ChowKick::processSynth (AudioBuffer<float>& buffer, MidiBuffer& midi)
{
    ScopedNoDenormals noDenormals;
    const auto numSamples = buffer.getNumSamples();

    // set up buffers
    buffer.clear();
    monoBuffer.setSize (1, numSamples, false, false, true);
    monoBuffer.clear();
    fourVoiceBuffer.clear();

    magicState.processMidiBuffer (midi, numSamples);

    trigger.processBlock (fourVoiceBuffer, numSamples, midi);
    pulseShaper->processBlock (fourVoiceBuffer, numSamples);
    resFilter.processBlock (fourVoiceBuffer, numSamples);
    reduceBlock (fourVoiceBuffer, monoBuffer);
    outFilter.processBlock (monoBuffer.getWritePointer (0), numSamples);

    dsp::AudioBlock<float> monoBlock (monoBuffer);
    dsp::ProcessContextReplacing<float> monoContext (monoBlock);
    dcBlocker.process<dsp::ProcessContextReplacing<float>, chowdsp::StateVariableFilterType::Highpass> (monoContext);

    // copy monoBuffer to other channels
    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
        buffer.copyFrom (ch, 0, monoBuffer.getReadPointer (0), numSamples);

    scope->pushSamples (buffer);
}

AudioProcessorEditor* ChowKick::createEditor()
{
    auto builder = chowdsp::createGUIBuilder (magicState);
    builder->registerFactory ("PulseViewer", &PulseViewerItem::factory);
    builder->registerFactory ("FilterViewer", &FilterViewerItem::factory);
    builder->registerFactory ("DisabledSlider", &DisabledSlider::factory);
    builder->registerLookAndFeel ("SliderLNF", std::make_unique<SliderLNF>());
    builder->registerLookAndFeel ("ComboBoxLNF", std::make_unique<ComboBoxLNF>());

    auto editor = new foleys::MagicPluginEditor (magicState, BinaryData::gui_xml, BinaryData::gui_xmlSize, std::move (builder));
    editor->setResizeLimits (10, 10, 2000, 2000);

    return editor;
}

// This creates new instances of the plugin
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ChowKick();
}
