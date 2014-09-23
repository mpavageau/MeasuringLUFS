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


#include "AudioProcessing.h"


void AudioProcessing::TestOversampling( const juce::File & input )
{
    juce::AudioFormatManager audioFormatManager;
    audioFormatManager.registerBasicFormats();

    juce::AudioFormatReader * reader = audioFormatManager.createReaderFor( input );

    if ( reader != nullptr )
    {
        // read file
        juce::AudioSampleBuffer origin( (int)reader->numChannels, (int)reader->lengthInSamples );
        reader->read( &origin, 0, (int)reader->lengthInSamples, 0, true, true );

        int newSize = 4 * (int)reader->lengthInSamples;
        juce::AudioSampleBuffer processed( (int)reader->numChannels, newSize );

        for ( int ch = 0 ; ch < (int)reader->numChannels ; ++ch )
        {
            float const * src = origin.getReadPointer( ch );
            float * dest = processed.getWritePointer( ch );

            for ( int i = 0 ; i < (int)reader->lengthInSamples ; ++i )
            {
                *dest++ = *src++;
                *dest++ = 0.f;
                *dest++ = 0.f;
                *dest++ = 0.f;
            }
        }

        juce::StringPairArray emptyArray;
        juce::File outputFile( ( input.getFullPathName() + "_TMP.wav" ) );
        juce::FileOutputStream outputStream( outputFile );

        juce::WavAudioFormat wavAudioFormat;

        juce::AudioFormatWriter * writer = wavAudioFormat.createWriterFor( 
            &outputStream, reader->sampleRate, reader->numChannels, 24, emptyArray, 0 );

        //AudioFormatWriter* createWriterFor (OutputStream* streamToWriteTo,
                                        //double sampleRateToUse,
                                        //unsigned int numberOfChannels,
                                        //int bitsPerSample,
                                        //const StringPairArray& metadataValues,
                                        //int qualityOptionIndex) override;

        if ( writer != nullptr )
        {
            writer->writeFromAudioSampleBuffer( processed, 0, newSize );

//            delete writer; what's this crash about?
        }

        delete reader;
    }
}
        