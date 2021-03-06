/*
  =================================================================

  This file is part of the Measuring LUFS program.
  Copyright (c) 2014 - Mathieu Pavageau - contact@repetito.com

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  Details of these licenses can be found at: www.gnu.org/licenses

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  =================================================================
*/

#ifndef __LUFS_CHOOSEFILESEPARATORCOMPONENT_H__
#define __LUFS_CHOOSEFILESEPARATORCOMPONENT_H__

class ChooseDigitSeparatorComponent 
    : public juce::Component
    , public juce::Button::Listener
{
public:

    ChooseDigitSeparatorComponent( juce::ApplicationProperties & _settings );
    virtual ~ChooseDigitSeparatorComponent();

    // juce::Component
    virtual void paint( juce::Graphics & g ) override;

    // juce::Button::Listener
    virtual void buttonClicked( juce::Button* ) override;

    inline bool useCommas() const { return m_useCommas; }

private:

    juce::ApplicationProperties & m_settings;
    juce::String m_useCommasString;
    juce::ToggleButton m_commaButton;
    juce::ToggleButton m_pointButton;
    juce::TextButton m_okButton;
    bool m_useCommas;
};


#endif  // __LUFS_CHOOSEFILESEPARATORCOMPONENT_H__
