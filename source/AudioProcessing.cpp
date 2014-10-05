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

const float filterPhase0[] =
{
    0.0017089843750f, 0.0109863281250f, -0.0196533203125f, 0.0332031250000f,
    -0.0594482421875f, 0.1373291015625f, 0.9721679687500f, -0.1022949218750f, 
    0.0476074218750f, -0.0266113281250f, 0.0148925781250f, -0.0083007812500f 
};

const float filterPhase1[] =
{
    -0.0291748046875f, 0.0292968750000f, -0.0517578125000f, 0.0891113281250f, 
    -0.1665039062500f, 0.4650878906250f, 0.7797851562500f, -0.2003173828125f,
    0.1015625000000f, -0.0582275390625f, 0.0330810546875f, -0.0189208984375f 
};

const float filterPhase2[] =
{
    -0.0189208984375f, 0.0330810546875f, -0.0582275390625f, 0.1015625000000f, 
    -0.2003173828125f, 0.7797851562500f, 0.4650878906250f, -0.1665039062500f, 
    0.0891113281250f, -0.0517578125000f, 0.0292968750000f, -0.0291748046875f 
};

const float filterPhase3[] =
{
    -0.0083007812500f, 0.0148925781250f, -0.0266113281250f, 0.0476074218750f,
    -0.1022949218750f, 0.9721679687500f, 0.1373291015625f, -0.0594482421875f, 
    0.0332031250000f, -0.0196533203125f, 0.0109863281250f, 0.0017089843750f
};

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

        int withZeroesSize = 4 * (int)reader->lengthInSamples;
        juce::AudioSampleBuffer withZeroes( (int)reader->numChannels, withZeroesSize );

        for ( int ch = 0 ; ch < (int)reader->numChannels ; ++ch )
        {
            float const * src = origin.getReadPointer( ch );
            float * dest = withZeroes.getWritePointer( ch );

            for ( int i = 0 ; i < (int)reader->lengthInSamples ; ++i )
            {
                *dest++ = *src++;
                *dest++ = 0.f;
                *dest++ = 0.f;
                *dest++ = 0.f;
            }
        }

        // convolution

        float * filterPhase0Array[] = { (float*)filterPhase0 };
        juce::AudioSampleBuffer phaseBuffer0( filterPhase0Array, 1, sizeof(filterPhase0) / sizeof(float) );
        juce::AudioSampleBuffer convoluted0;
        convolution( withZeroes, phaseBuffer0, convoluted0 );

        float * filterPhase1Array[] = { (float*)filterPhase1 };
        juce::AudioSampleBuffer phaseBuffer1( filterPhase1Array, 1, sizeof(filterPhase1) / sizeof(float) );
        juce::AudioSampleBuffer convoluted1;
        convolution( convoluted0, phaseBuffer1, convoluted1 );

        float * filterPhase2Array[] = { (float*)filterPhase2 };
        juce::AudioSampleBuffer phaseBuffer2( filterPhase2Array, 1, sizeof(filterPhase2) / sizeof(float) );
        juce::AudioSampleBuffer convoluted2;
        convolution( convoluted1, phaseBuffer2, convoluted2 );

        float * filterPhase3Array[] = { (float*)filterPhase2 };
        juce::AudioSampleBuffer phaseBuffer3( filterPhase3Array, 1, sizeof(filterPhase3) / sizeof(float) );
        juce::AudioSampleBuffer convoluted3;
        convolution( convoluted2, phaseBuffer3, convoluted3 );

        juce::String outputName = input.getFullPathName().substring(0, input.getFullPathName().length() - 4);
        juce::File outputFile( outputName + "_CONVO_4.wav" );
        juce::FileOutputStream * outputStream = new juce::FileOutputStream( outputFile );

        juce::WavAudioFormat wavAudioFormat;

        juce::StringPairArray emptyArray;
        juce::AudioFormatWriter * writer = wavAudioFormat.createWriterFor( 
            outputStream, reader->sampleRate, reader->numChannels, 24, emptyArray, 0 );

        if ( writer != nullptr )
        {
            writer->writeFromAudioSampleBuffer( convoluted3, 0, convoluted3.getNumSamples() );

            delete writer;
        }

        delete reader;
    }
}

void AudioProcessing::convolution( const juce::AudioSampleBuffer & a, const juce::AudioSampleBuffer & b, juce::AudioSampleBuffer & result )
{
    jassert( b.getNumChannels() == 1 );

    int sampleSize = a.getNumSamples() + b.getNumSamples();

    result.setSize( a.getNumChannels(), sampleSize );

    const float * bSignal = b.getArrayOfReadPointers()[ 0 ];
    
    for ( int ch = 0 ; ch < a.getNumChannels() ; ++ch )
    {
        const float * aSignal = a.getArrayOfReadPointers()[ ch ];
        float * res = result.getArrayOfWritePointers()[ ch ];

        for ( int i = 0 ; i < sampleSize ; ++i )
        {
            float sum = 0.f;

            for ( int j = 0 ; j < b.getNumSamples() ; ++j )
            {
                int indexA = i - j;

                if ( ( indexA >= 0 ) && ( indexA < a.getNumSamples() ) )
                    sum += ( aSignal[indexA] * bSignal[j] );
            }

            jassert( sum < 1.f );
            jassert( sum > -1.f );

            res[i] = sum;
        }
    }

}
