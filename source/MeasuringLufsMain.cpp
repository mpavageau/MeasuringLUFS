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
#include "MeasuringLufsComponent.h"

const juce::String g_windowStateString( "windowState" );


class MainWindow : public juce::DocumentWindow
{
public:
    //==============================================================================
    MainWindow() : juce::DocumentWindow( LufsAudioProcessor::makeAppNameWithVersion(),
                          juce::Colours::lightgrey,
                          juce::DocumentWindow::allButtons,
                          true)
    {
        MeasuringLufsComponent * component = new MeasuringLufsComponent( true );
        setContentOwned( component, true );

        // Centre the window on the screen
        centreWithSize( getWidth(), getHeight() );

        setResizable( true, true );
        setResizeLimits( getWidth(), getHeight(), 3000, 2000 );

        // And show it!
        setVisible (true);

        juce::String windowState = component->getProcessor()->m_settings.getUserSettings()->getValue( g_windowStateString );
        restoreWindowStateFromString( windowState );

        setColour( MainWindow::backgroundColourId, LUFS_COLOR_BACKGROUND );
        setColour( MainWindow::textColourId, LUFS_COLOR_FONT );
    }

    ~MainWindow()
    {
        if ( getContentComponent() )
        {
            const juce::String windowState = getWindowStateAsString();
            MeasuringLufsComponent * component = (MeasuringLufsComponent*)getContentComponent();
            component->getProcessor()->m_settings.getUserSettings()->setValue( g_windowStateString, windowState );
        }
    }

    //==============================================================================
    void closeButtonPressed()
    {
        // When the user presses the close button, we'll tell the app to quit. This
        // MainWindow object will be deleted by the Application class.
        juce::JUCEApplication::quit();
    }
};

//==============================================================================
/** This is the application object that is started up when Juce starts. It handles
    the initialisation and shutdown of the whole application.
*/
class Application : public juce::JUCEApplication
{
public:
    //==============================================================================
    Application() {}

    //==============================================================================
    void initialise (const juce::String& /*commandLine*/ )
    {
        // For this demo, we'll just create the main window...
        m_mainWindow = new MainWindow();

        /*  ..and now return, which will fall into to the main event
            dispatch loop, and this will run until something calls
            JUCEAppliction::quit().

            In this case, JUCEAppliction::quit() will be called by the
            hello world window being clicked.
        */
    }

    void shutdown()
    {
        // This method is where you should clear-up your app's resources..

        // The m_mainWindow variable is a ScopedPointer, so setting it to a null
        // pointer will delete the window.
        m_mainWindow = nullptr;
    }

    //==============================================================================
    const juce::String getApplicationName()
    {
        return "Measuring LUFS";
    }

    const juce::String getApplicationVersion()
    {
        // The ProjectInfo::versionString value is automatically updated by the Jucer, and
        // can be found in the JuceHeader.h file that it generates for our project.
        return "Version 0";//ProjectInfo::versionString;
    }

    bool moreThanOneInstanceAllowed()
    {
        return true;
    }

    void anotherInstanceStarted (const juce::String& /*commandLine*/)
    {
    }

private:
    juce::ScopedPointer<MainWindow> m_mainWindow;
};


#ifdef MEASURING_LUFS_APPLICATION
//==============================================================================
// This macro creates the application's main() function..
START_JUCE_APPLICATION (Application)
#endif
