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

#ifndef __LUFS_AUDIOPROCESSING_H__
#define __LUFS_AUDIOPROCESSING_H__

class AudioProcessing
{
public:

    // oversamples by 4 a wave file using polyphase4 and saves new file to disk
    static void TestOversampling( const juce::File & input );

    // TruePeak class calculates True Peak linear volume for buffer

    class TruePeak
    {
    public:
        TruePeak();

        // processAndGetTruePeak method uses previous values stored in internal buffer as beginning of current buffer
        float processAndGetTruePeak( const juce::AudioSampleBuffer & buffer );

        // resets internal buffers 
        void reset();

    private:

        float getPolyphase4AbsMax( const juce::AudioSampleBuffer & buffer );

        juce::AudioSampleBuffer m_inputs;
        juce::AudioSampleBuffer m_outputs;
    };

private:

    // b signal must be mono
    static void convolution( const juce::AudioSampleBuffer & a, const juce::AudioSampleBuffer & b, juce::AudioSampleBuffer & result );

    static void polyphase4( const juce::AudioSampleBuffer & source, juce::AudioSampleBuffer & result );
    static float polyphase4ComputeSum( const float * input, int offset, int maxOffset, const float* coefficients, int numCoeff );

};

#endif  // __LUFS_AUDIOPROCESSING_H__
