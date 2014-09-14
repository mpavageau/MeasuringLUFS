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

#include "AppIncsAndDefs.h"

#include "ChannelSelector.h"

ChannelSelector::ChannelSelector( const juce::Colour _backgroundColor, const juce::Colour _textColor, const juce::String & _notAssignedText )
    : m_backgroundColor( _backgroundColor )
    , m_textColor( _textColor )
    , m_notAssignedText( _notAssignedText )
{
}

ChannelSelector::~ChannelSelector()
{
    clear();
}

#ifdef TESTCOLORS
void ChannelSelector::paint( juce::Graphics & g )
{
    g.fillAll( juce::Colours::purple );
}
#else
void ChannelSelector::paint( juce::Graphics & /*g*/ )
{
}
#endif

void ChannelSelector::comboBoxChanged( juce::ComboBox* comboBoxThatHasChanged )
{
    const int selectedId = comboBoxThatHasChanged->getSelectedId();

    for ( int i = 0 ; i < m_comboBoxArray.size() ; ++i )
    {
        if ( ( m_comboBoxArray[ i ] != comboBoxThatHasChanged )
            && ( m_comboBoxArray[ i ]->getSelectedId() == selectedId ) )
        {
            m_comboBoxArray[ i ]->setSelectedId( -1 );
        }
    }
}

void ChannelSelector::clear()
{
    for ( int i = 0 ; i < m_labelArray.size() ; ++i )
        delete ( m_labelArray.getUnchecked( i ) );

    for ( int i = 0 ; i < m_comboBoxArray.size() ; ++i )
        delete ( m_comboBoxArray.getUnchecked( i ) );
}

void ChannelSelector::create()
{
    jassert( m_identifierArray.size() && m_choiceArray.size() );

    const int width = 300;
    int offset = 10;
    int x = offset;
    int y = offset;
    int widthLeft = 80;
    int widthRight = width - widthLeft - 3 * offset;
    int height = 20;

    int i;
    for ( i = 0 ; i < m_identifierArray.size() ; ++i )
    {
        // label
        juce::Label * label = new juce::Label( m_identifierArray[ i ], m_identifierArray[ i ] );
        label->setBounds( x, y, widthLeft, height );
        addAndMakeVisible( label );
        label->setColour( juce::Label::backgroundColourId, m_backgroundColor );
        label->setColour( juce::Label::textColourId, m_textColor );
        label->setJustificationType( juce::Justification::centred );
        m_labelArray.add( label );

        // juce::ComboBox
        juce::ComboBox * comboBox = new juce::ComboBox;
        comboBox->addItem( m_notAssignedText, -1 );
        for ( int j = 0 ; j < m_choiceArray.size() ; ++j )
        {
            comboBox->addItem( m_choiceArray[ j ], j + 1 );
        }
        comboBox->addListener (this);
        comboBox->setBounds( x + offset + widthLeft, y, widthRight, height );
        addAndMakeVisible( comboBox );
        m_comboBoxArray.add( comboBox );

        y += ( offset + height );
    }

    setSize( width, y + offset );

    assignIncremental();
}

void ChannelSelector::setIdentifierArray( const juce::StringArray & _identifierArray )
{
    clear();

    m_identifierArray = _identifierArray;

    if ( m_choiceArray.size() && m_identifierArray.size() )
        create();
}

void ChannelSelector::setChoiceArray( const juce::StringArray & _choiceArray )
{
    clear();

    m_choiceArray = _choiceArray;

    if ( m_choiceArray.size() && m_identifierArray.size() )
        create();
}

void ChannelSelector::assignNotAssigned()
{
    for ( int i = 0 ; i < m_comboBoxArray.size() ; ++i )
    {
        m_comboBoxArray[ i ]->setSelectedId( -1 );
    }
}

void ChannelSelector::assignIncremental()
{
    for ( int i = 0 ; i < m_comboBoxArray.size() ; ++i )
    {
        if ( i < m_choiceArray.size() )
            m_comboBoxArray[ i ]->setSelectedId( i + 1 );
        else
            m_comboBoxArray[ i ]->setSelectedId( -1 );
    }
}
