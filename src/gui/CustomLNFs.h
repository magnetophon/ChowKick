#pragma once

#include <pch.h>

class SliderLNF : public chowdsp::ChowLNF
{
public:
    SliderLNF() = default;

    Label* createSliderTextBox (Slider& slider) override
    {
        auto l = chowdsp::ChowLNF::createSliderTextBox (slider);
        l->setFont ((float) slider.getTextBoxHeight() * 0.8f);

        return l;
    }
};

/** Custom LookAndFeel for ComboBoxes */
class ComboBoxLNF : public chowdsp::ChowLNF
{
public:
    ComboBoxLNF()
    {
        setColour (PopupMenu::backgroundColourId, Colour (0xFF1B2A33));
        setColour (PopupMenu::highlightedBackgroundColourId, Colour (0xFF425866));
    }

    ~ComboBoxLNF() override = default;

    void drawPopupMenuItem (Graphics& g, const Rectangle<int>& area, const bool isSeparator, const bool isActive, const bool isHighlighted, const bool /*isTicked*/, const bool hasSubMenu, const String& text, const String& shortcutKeyText, const Drawable* icon, const Colour* const textColourToUse) override
    {
        LookAndFeel_V4::drawPopupMenuItem (g, area, isSeparator, isActive, isHighlighted, false /*isTicked*/, hasSubMenu, text, shortcutKeyText, icon, textColourToUse);
    }

    Component* getParentComponentForMenuOptions (const PopupMenu::Options& options) override
    {
#if JUCE_IOS
        if (PluginHostType::getPluginLoadedAs() == AudioProcessor::wrapperType_AudioUnitv3)
        {
            if (options.getParentComponent() == nullptr && options.getTargetComponent() != nullptr)
                return options.getTargetComponent()->getTopLevelComponent();
        }
#endif
        return LookAndFeel_V2::getParentComponentForMenuOptions (options);
    }

    void drawPopupMenuBackground (Graphics& g, int width, int height) override
    {
        g.fillAll (findColour (PopupMenu::backgroundColourId));
        ignoreUnused (width, height);
    }

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ComboBoxLNF)
};
