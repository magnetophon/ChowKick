#include "FilterViewer.h"

namespace
{
constexpr float lowFreq = 25.0f;
constexpr float highFreq = 750.0f;
constexpr float dbRange = 72.0f;
constexpr double fs = 48000.0;

float getFreqForX (float xNorm)
{
    return lowFreq * std::pow ((highFreq / lowFreq), xNorm);
}

float getXForFreq (float freq)
{
    return std::log (freq / lowFreq) / std::log (highFreq / lowFreq);
}

float getYForMagnitude (float mag, float height)
{
    auto magDB = jmin (Decibels::gainToDecibels (mag), 70.0f);
    auto normDB = magDB / dbRange;
    return height * (1.0f - 0.85f * normDB) - (height * 0.1f);
}
}

FilterViewer::FilterViewer (AudioProcessorValueTreeState& vts) :
    trigger (vts),
    resFilter (vts, trigger)
{
    setColour (backgroundColour, Colours::black);
    setColour (traceColour, Colours::lightblue);

    resFilter.reset (fs);
    startTimerHz (29);
}

void FilterViewer::resized()
{
    repaint();
}

void FilterViewer::paint (Graphics& g)
{
    g.fillAll (findColour (backgroundColour));

    const auto width = (float) getWidth();
    const auto height = (float) getHeight();
    const auto left = (float) getX();
    const auto top = (float) getY();

    Path tracePath;
    float traceMagnitude = resFilter.getMagForFreq (getFreqForX (0.0f));
    float yPos = getYForMagnitude (traceMagnitude, height);
    tracePath.startNewSubPath (left, top + yPos);

    constexpr float xInc = 0.25f;
    bool beforeRes = false;
    for (float xPos = 1.0f; xPos < width; xPos += xInc)
    {
        traceMagnitude = resFilter.getMagForFreq (getFreqForX (xPos / width));
        yPos = getYForMagnitude (traceMagnitude, height);
        tracePath.lineTo (left + xPos, top + yPos);

        if (beforeRes && getFreqForX ((xPos + xInc) / width) >= resFilter.getFrequencyHz())
        {
            auto resFreq = resFilter.getFrequencyHz();
            traceMagnitude = resFilter.getMagForFreq (resFreq);
            auto xP = width * getXForFreq (resFreq);
            yPos = getYForMagnitude (traceMagnitude, height);
            tracePath.lineTo (left + xP, top + yPos);

            beforeRes = true;
        }
    }

    g.setColour (findColour (traceColour));
    g.strokePath (tracePath, PathStrokeType (2.0f, PathStrokeType::JointStyle::curved));
}

void FilterViewer::timerCallback()
{
    repaint();
}
