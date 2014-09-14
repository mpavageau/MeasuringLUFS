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

#ifndef __LUFS_CHANNELSELECTOR_H__
#define __LUFS_CHANNELSELECTOR_H__

#include "LufsAudioProcessor.h"

class ChannelSelector 
    : public juce::Component
    , public juce::ComboBox::Listener
{
public:

    ChannelSelector( const juce::Colour _backgroundColor, const juce::Colour _textColor, const juce::String & _notAssignedText );

    virtual ~ChannelSelector();

    // juce::Component
    virtual void paint( juce::Graphics & g );

    // juce::ComboBox::Listener
    virtual void comboBoxChanged( juce::ComboBox* comboBoxThatHasChanged );

    // sets array for left column labels 
    void setIdentifierArray( const juce::StringArray & _identifierArray );

    // sets array for contents of right column combo boxes 
    void setChoiceArray( const juce::StringArray & _choiceArray );

private:

    // deletes all 
    void clear();

    // recreates combo boxes and labels according to identifier and choice arrays
    void create();

    // assign all choices to "not assigned" 
    void assignNotAssigned();

    // assign all choices to identifier on a one to one basis  
    void assignIncremental();

    juce::StringArray m_identifierArray;
    juce::StringArray m_choiceArray;
    juce::Array<juce::ComboBox*> m_comboBoxArray;
    juce::Array<juce::Label*> m_labelArray;

    juce::ScopedPointer<juce::XmlElement> m_settings;

    juce::Colour m_backgroundColor;
    juce::Colour m_textColor;
    juce::String m_notAssignedText;
};

#endif  // __LUFS_CHANNELSELECTOR_H__
