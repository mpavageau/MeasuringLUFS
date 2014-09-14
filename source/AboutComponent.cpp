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


#include "AboutComponent.h"


#include "LufsAudioProcessor.h"


AboutComponent::AboutComponent()
{
    setSize( 800, 630 );
}

AboutComponent::~AboutComponent()
{
}

void AboutComponent::paint( juce::Graphics & g )
{
    g.fillAll( LUFS_COLOR_BACKGROUND  );

    juce::Font font( 14.f );
    font.setBold(true);
    g.setFont (font);
    g.setColour( LUFS_COLOR_FONT );

    juce::String text;

    int xIndent = 60;
    int y = 60;
    int height = 16;
    int yIndent = 5;
    
    text = LufsAudioProcessor::makeAppNameWithVersion();
#ifdef MEASURING_LUFS_APPLICATION
    text += " is a program to measure LUFS volume loudness.";
#else // MEASURING_LUFS_APPLICATION
    text += " is a plugin to measure LUFS volume loudness.";
#endif // MEASURING_LUFS_APPLICATION
    g.drawFittedText( text, xIndent, y, getWidth() - 2 * xIndent, height, juce::Justification::horizontallyJustified | juce::Justification::verticallyCentred, 1, 0.01f );
    y += ( height + yIndent );

    y += ( height + yIndent );

    juce::Font font2( 14.f );
    g.setFont (font2);

    const int centredJustified = juce::Justification::horizontallyJustified | juce::Justification::centredLeft;

    text = "LUFS volume loudness is a subjective loudness, calculated as recommended by International Telecommunication";
    g.drawFittedText( text, xIndent, y, getWidth() - 2 * xIndent, height, centredJustified, 1, 0.01f );
    y += ( height + yIndent );

    text = "Union (ITU), the United Nations specialized agency for information and communication technologies.";
    g.drawFittedText( text, xIndent, y, getWidth() - 2 * xIndent, height, juce::Justification::centredLeft, 1, 0.01f );
    y += ( height + yIndent );

    y += ( height + yIndent );

    text = "The used algorithm is that specified by document BS 1770 from ITU:";
    g.drawFittedText( text, xIndent, y, getWidth() - 2 * xIndent, height, juce::Justification::centredLeft, 1, 0.01f );
    y += ( height + yIndent );

    text = "https://www.itu.int/rec/R-REC-BS.1770/";
    g.drawFittedText( text, xIndent, y, getWidth() - 2 * xIndent, height, juce::Justification::centredLeft, 1, 0.01f );
    y += ( height + yIndent );

    y += ( height + yIndent );

    text = "This program is free software; you can redistribute it and/or modify it under the terms of the GNU General";
    g.drawFittedText( text, xIndent, y, getWidth() - 2 * xIndent, height, centredJustified, 1, 0.01f );
    y += ( height + yIndent );
  
    text = "Public License as published by the Free Software Foundation; either version 2 of the License, or (at your";
    g.drawFittedText( text, xIndent, y, getWidth() - 2 * xIndent, height, centredJustified, 1, 0.01f );
    y += ( height + yIndent );
  
    text = "option) any later version.";
    g.drawFittedText( text, xIndent, y, getWidth() - 2 * xIndent, height, juce::Justification::centredLeft, 1, 0.01f );
    y += ( height + yIndent );
    
    y += ( height + yIndent );

    text = "Details of these licenses can be found at: www.gnu.org/licenses.";
    g.drawFittedText( text, xIndent, y, getWidth() - 2 * xIndent, height, juce::Justification::centredLeft, 1, 0.01f );
    y += ( height + yIndent );
    
    y += ( height + yIndent );

    text = "This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even";
    g.drawFittedText( text, xIndent, y, getWidth() - 2 * xIndent, height, juce::Justification::centredLeft, 1, 0.01f );
    y += ( height + yIndent );

    text = "the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General";
    g.drawFittedText( text, xIndent, y, getWidth() - 2 * xIndent, height, juce::Justification::centredLeft, 1, 0.01f );
    y += ( height + yIndent );

    text = "Public License for more details.";
    g.drawFittedText( text, xIndent, y, getWidth() - 2 * xIndent, height, juce::Justification::centredLeft, 1, 0.01f );
    y += ( height + yIndent );

    y += ( height + yIndent );

#ifdef MEASURING_LUFS_APPLICATION
    text = "The source code for this program can be found at https://github.com/mpavageau/MeasuringLUFS.";
#else // MEASURING_LUFS_APPLICATION
    text = "The source code for this plugin can be found at https://github.com/mpavageau/MeasuringLUFS.";
#endif // MEASURING_LUFS_APPLICATION
    g.drawFittedText( text, xIndent, y, getWidth() - 2 * xIndent, height, juce::Justification::centredLeft, 1, 0.01f );
    y += ( height + yIndent );

    y += ( height + yIndent );

#ifdef MEASURING_LUFS_APPLICATION
    text = "This program was developed using the JUCE framework (http://juce.com), and ASIO and VST technologies by";
#else // MEASURING_LUFS_APPLICATION
    text = "This plugin was developed using the JUCE framework (http://juce.com), and ASIO and VST technologies by";
#endif // MEASURING_LUFS_APPLICATION
    g.drawFittedText( text, xIndent, y, getWidth() - 2 * xIndent, height, juce::Justification::centredLeft, 1, 0.01f );
    y += ( height + yIndent );

    text = "Steinberg Media Technologies (http://www.steinberg.net/en/company/developers.html).";
    g.drawFittedText( text, xIndent, y, getWidth() - 2 * xIndent, height, juce::Justification::centredLeft, 1, 0.01f );
    y += ( height + yIndent );

    y += ( height + yIndent );

#ifdef MEASURING_LUFS_APPLICATION
    text = "This program was developed by Mathieu Pavageau (contact@repetito.com) - Copyright (c) 2014.";
#else // MEASURING_LUFS_APPLICATION
    text = "This plugin was developed by Mathieu Pavageau (contact@repetito.com) - Copyright (c) 2014.";
#endif // MEASURING_LUFS_APPLICATION
    g.drawFittedText( text, xIndent, y, getWidth() - 2 * xIndent, height, juce::Justification::centredLeft, 1, 0.01f );
    y += ( height + yIndent );

    text = "The latest version can be downloaded at http://www.repetito.com/index.php?page=content_measuring_lufs.";
    g.drawFittedText( text, xIndent, y, getWidth() - 2 * xIndent, height, juce::Justification::centredLeft, 1, 0.01f );
    y += ( height + yIndent );
/*  
    juce::HyperlinkButton 
        m_repetitoWebsiteButton = new juce::HyperlinkButton( juce::String( coded_repetitoWebsite ), juce::URL( coded_repetitoWebsite ) );

        m_repetitoWebsiteButton->setFont( juce::Font( 14.f, juce::Font::bold ), false );
        m_repetitoWebsiteButton->setColour( juce::HyperlinkButton::textColourId, GuiObjectManager::Singleton().GetInputColor( 0 ) );
 
        addAndMakeVisible( m_repetitoWebsiteButton );
*/
}

