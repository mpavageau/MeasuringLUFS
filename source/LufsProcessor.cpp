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

#include "LufsProcessor.h"

#define LUFS_PROCESSOR_NB_MEMORY_VALUES 4

void DEBUGPLUGIN_output( const char * _text, ...);

double LufsProcessor::ms_log10 = log( 10.0 );

LufsProcessor::LufsProcessor( const int nbChannels )
    : m_memory( nbChannels, 0 )
    , m_block( nbChannels, 0 )
    , m_nbChannels( nbChannels )
    , m_sampleRate( 0.0 )
    , m_memorySize( 0 )
    , m_sampleSize100ms( 0 ) 
    , m_squaredInputArray( NULL )
    , m_momentaryVolumeArray( NULL )
    , m_shortTermVolumeArray( NULL )
    , m_integratedVolumeArray( NULL )
    , m_maxSize( 0 )
    , m_processSize( 0 )
    , m_validSize( 0 )
    , m_paused( false )
    , m_tempBlock( 1, 4096 )
{
    DEBUGPLUGIN_output("LufsProcessor::LufsProcessor %d channels", nbChannels);

    for ( int i = 0 ; i < nbChannels ; ++i )
    {
        m_shelveFilterArray.add( BiquadProcessor() );
        m_highPassFilterArray.add( BiquadProcessor() );
    }

    m_maxSize = 256 * 1024; // more than 7 hours
    jassert( m_maxSize * 4 < 0x80000000 );
    m_squaredInputArray = (float*)malloc( m_maxSize * sizeof( float ) );
    m_momentaryVolumeArray = (float*)malloc( m_maxSize * sizeof( float ) );
    m_shortTermVolumeArray = (float*)malloc( m_maxSize * sizeof( float ) );
    m_integratedVolumeArray = (float*)malloc( m_maxSize * sizeof( float ) );

    m_memArray = (float**)malloc( nbChannels * sizeof( float* ) );

    for ( int i = 0 ; i < nbChannels ; ++i )
    {
        m_memArray[ i ] = (float*)malloc( LUFS_PROCESSOR_NB_MEMORY_VALUES * sizeof( float ) );

        memset( m_memArray[ i ], 0, LUFS_PROCESSOR_NB_MEMORY_VALUES * sizeof( float ) );
    }
    m_maxLinArray = (float*)malloc( nbChannels * sizeof( float ) );

    reset();
}

LufsProcessor::~LufsProcessor()
{
    DEBUGPLUGIN_output("LufsProcessor::~LufsProcessor");

    free( m_squaredInputArray );
    free( m_momentaryVolumeArray );
    free( m_shortTermVolumeArray );
    free( m_integratedVolumeArray );

    for ( int i = 0 ; i < m_nbChannels ; ++i )
    {
        free( m_memArray[ i ] );
    }
    free( m_memArray );

    free( m_maxLinArray );
}

void LufsProcessor::reset()
{
    DEBUGPLUGIN_output("LufsProcessor::reset");

    const juce::SpinLock::ScopedLockType scopedLock( m_locker );

    m_processSize = 0;
    m_validSize = 0;
    m_memorySize = 0;

    m_integratedVolume = DEFAULT_MIN_VOLUME;
    m_rangeMin = DEFAULT_MIN_VOLUME;
    m_rangeMax = DEFAULT_MIN_VOLUME;

    m_sum400ms70.reset();
    m_sum3s70.reset();

    for ( int i = 0 ; i < m_nbChannels ; ++i )
    {
        m_maxLinArray[ i ]  = 0.f;
    }

#if 0
    // add points at beginning to debug view port
    for ( int i = 0 ; i < 4000 ; ++i )
    {
        addSquaredInput( 0.f );
    }
#endif 
}

void LufsProcessor::prepareToPlay(const double sampleRate, int samplesPerBlock)
{
    DEBUGPLUGIN_output("LufsProcessor::prepareToPlay sampleRate %.1f samplesPerBlock %d", (float)sampleRate, samplesPerBlock);

    for ( int i = 0 ; i < m_nbChannels ; ++i )
    {
        m_shelveFilterArray.getReference( i ).setFilterParams( (float)sampleRate, BiquadProcessor::HighShelf, 1500.f, 0.5f, 4.f );
        m_highPassFilterArray.getReference( i ).setFilterParams( (float)sampleRate, BiquadProcessor::HighPass, 60.f, 0.5f, 0.f );
    }

    m_block.setSize( m_nbChannels, 2 * samplesPerBlock );
    m_memory.setSize( m_nbChannels, 2 * (int)sampleRate );
    m_sampleRate = sampleRate;
    m_sampleSize100ms = (int)( m_sampleRate / 10.0 );
    reset();
}

void LufsProcessor::processBlock( juce::AudioSampleBuffer& buffer )
{
    jassert( buffer.getNumChannels() <= m_nbChannels );
    //DEBUGPLUGIN_output("LufsProcessor::processBlock buffer size %.d", buffer.getNumSamples());

    if ( m_paused )
        return;

    processPeak( buffer );

    const juce::SpinLock::ScopedLockType scopedLock( m_locker );

    m_block.setSize( m_nbChannels, buffer.getNumSamples(), false, false, true );

    for ( int i = 0 ; i < m_nbChannels ; ++i )
    {
        if ( i >= buffer.getNumChannels() )
            continue;

        // copy to internal buffer
        m_block.copyFrom( i, 0, buffer, i, 0, buffer.getNumSamples() );

        // apply high shelf
        m_shelveFilterArray.getReference( i ).process( m_block.getWritePointer( i ), m_block.getNumSamples() );
        
        // apply high pass
        m_highPassFilterArray.getReference( i ).process( m_block.getWritePointer( i ), m_block.getNumSamples() );

        // copy block at the end of memory
        m_memory.copyFrom( i, m_memorySize, m_block, i, 0, m_block.getNumSamples() );
    }

    m_memorySize += buffer.getNumSamples();

    if ( m_memorySize < m_sampleSize100ms )
        return;

    int sizeDone = 0 ;

    int nbChannels = m_nbChannels > 6 ? 6 : m_nbChannels;
    while ( sizeDone < m_memorySize )
    {
        float sum = 0.f;
        
        for ( int i = 0 ; i < nbChannels ; ++i )
        {
            if ( i >= buffer.getNumChannels() )
                continue;

            float weightingCoef = 0.f; 
            
            // kSpeakerArr51 is "L R C Lfe Ls Rs";
            switch ( i )
            {
            case 0: // L
            case 1: // R
            case 2: // C
                weightingCoef = 1.f;
                break;
            case 3: // Lfe
                weightingCoef = 0.f;
                break;
            case 4: // R
            case 5: // C
                weightingCoef = 1.414213f; // 1.41 (~ +1.5 dB) for left and right surround channels 
                break;
            }

            const float * data = &( m_memory.getReadPointer( i )[ sizeDone ] );

            for ( int s = 0 ; s < m_sampleSize100ms ; ++s )
            {
                const float value = *data;
                sum += value * value * weightingCoef;
                ++data;
            }
        }

        addSquaredInput( sum / m_sampleSize100ms );

        sizeDone += m_sampleSize100ms;

        if ( ( sizeDone + m_sampleSize100ms ) > m_memorySize )
            break;
    }

    // copy remaining samples to beginning of memory 

    const int remaining = m_memorySize - sizeDone;
    jassert( remaining >= 0 );

    for ( int i = 0 ; i < m_nbChannels ; ++i )
    {
        float * dest = m_memory.getWritePointer( i );
        float * src = &dest[ sizeDone ];

        for ( int s = 0 ; s < remaining ; ++s )
        {
            *dest++ = *src++;
        }
    }

    m_memorySize = remaining;
}

void LufsProcessor::addSquaredInput( const float squaredInput )
{
    if ( m_processSize < m_maxSize )
    {
        m_squaredInputArray[ m_processSize ] = squaredInput;
        ++m_processSize;
    }
}

void LufsProcessor::update()
{
    //DEBUGPLUGIN_output("LufsProcessor::update");

    int size = m_processSize;

    while ( m_validSize < size )
    {
        updatePosition( m_validSize );
        ++m_validSize;
    }
}

void LufsProcessor::updatePosition( int position )
{
    //DEBUGPLUGIN_output("LufsProcessor::updatePosition position %d", position);

    // m_momentaryVolume 
    m_integratedVolumeArray[ position ] = DEFAULT_MIN_VOLUME;

    if ( position >= 4 )
    {
        // momentary, abbreviated M (400 ms)

        float sum = 0.f;
        for ( int i = position - 4 ; i < position ; ++i )
        {
            sum += m_squaredInputArray[ i ];
        }
        sum /= 4;

        m_momentaryVolumeArray[ position ] = juce::jmax( float(-0.691 + 10.*std::log10( sum ) ), DEFAULT_MIN_VOLUME );
        
        if ( m_momentaryVolumeArray[ position ] > -70.f )
        {
            //DBG( juce::String( "Adding 1 " ) + juce::String( sum ) );
            m_sum400ms70.addLufs( sum );
        }

        if ( m_sum400ms70.size() )
        {
            const float absoluteSum = m_sum400ms70.getSum() / (float) m_sum400ms70.size();
            const float absoluteThresholdVolume = getLufsVolume( absoluteSum ) -10.f;
            const float thresholdSum = getLufsSum( absoluteThresholdVolume );

            const int index = m_sum400ms70.findIndexAfterValue( thresholdSum );

            float relativeSum = 0.f;
            for ( int i = index ; i < m_sum400ms70.size() ; ++i )
            {
                //DBG( juce::String( "relativeSum adding " ) + juce::String( m_sum400ms70.getUnchecked( i ) ) );
                relativeSum += m_sum400ms70.getUnchecked( i );
            }

            if ( m_sum400ms70.size() - index )
                relativeSum /= ( m_sum400ms70.size() - index );

            //DBG( juce::String( "relativeSum " ) + juce::String( relativeSum ) );
            
            m_integratedVolume = getLufsVolume( relativeSum );
            m_integratedVolumeArray[ position ] = m_integratedVolume;
        }
    }
    else
    {
        m_momentaryVolumeArray[ position ] = DEFAULT_MIN_VOLUME;
    }


    // short term, abbreviated S (3 s)

    if ( position >= 30 )
    {
        float sum = 0.f;
        for ( int i = position - 30 ; i < position ; ++i )
        {
            sum += m_squaredInputArray[ i ];
        }
        sum /= 30;
        m_shortTermVolumeArray[ position ] = juce::jmax( float(-0.691 + 10.*std::log10( sum ) ), DEFAULT_MIN_VOLUME );

        if ( m_shortTermVolumeArray[ position ] > -70.f )
        {
            m_sum3s70.addLufs( sum );
        }

        if ( m_sum3s70.size() )
        {
            const float absoluteSum = m_sum3s70.getSum() / (float) m_sum3s70.size();
            const float absoluteThresholdVolume = getLufsVolume( absoluteSum ) -20.f;
            const float thresholdSum = getLufsSum( absoluteThresholdVolume );

            const int index = m_sum3s70.findIndexAfterValue( thresholdSum );

            const float sumPercentile10 = m_sum3s70.getPercentileValue( index, 0.1f );
            const float sumPercentile95 = m_sum3s70.getPercentileValue( index, 0.95f );

            m_rangeMin = getLufsVolume( sumPercentile10 );
            m_rangeMax = getLufsVolume( sumPercentile95 );
        }

    }
    else
    {
        m_shortTermVolumeArray[ position ] = DEFAULT_MIN_VOLUME;
    }
}


struct FileArrayCont
{
    juce::Thread::ThreadID m_threadId;
    FILE * m_file;
};
static juce::Array<FileArrayCont> s_fileArray;


void DEBUGPLUGIN_output( const char * /*_text*/, ...)
{
/*
    char formattedText[0x200];
    va_list args ;
    va_start( args, _text );

    vsprintf_s( formattedText, _text, args );
    va_end (args) ;

    // show time 
    char formattedTextWithTime[0x200];
    static juce::Time timeBeginning = juce::Time::getCurrentTime();
    juce::RelativeTime duration = juce::Time::getCurrentTime() - timeBeginning; 

    juce::Thread::ThreadID threadID = juce::Thread::getCurrentThreadId();
    sprintf_s( formattedTextWithTime, 0x200, "(%.3f) (thread 0x%x) %s\n", 0.001f * (float)duration.inMilliseconds(), (int)threadID, formattedText);

    FILE * file = NULL;
    for ( int i = 0 ; i < s_fileArray.size() ; ++i )
    {
        if ( s_fileArray.getUnchecked( i ).m_threadId == threadID )
        {
            file = s_fileArray.getUnchecked( i ).m_file;
            break;
        }
    }

    if ( !file )
    {
        static int count = 0;
        char path[0x200];
        sprintf_s( path, 0x200, "c:/temp/LUFS_debug_%d.txt", count );
        ++count;

        fopen_s( &file, path, "a" );

        if ( file )
        {
            juce::String date = timeBeginning.toString( true, true, true );
            char dateText[0x200];
            sprintf_s( dateText, 0x200, "\n%s\n\n", date.getCharPointer());
            fwrite( dateText, 1, strlen( dateText ), file );

            FileArrayCont cont;
            cont.m_threadId = threadID;
            cont.m_file = file;
            s_fileArray.add( cont );
        }
    }

    if ( file )
    {
        fwrite( formattedTextWithTime, 1, strlen( formattedTextWithTime ), file );
        fflush( file );
    }
*/
}



// BiquadProcessor implementation 

BiquadProcessor::BiquadProcessor()
    : m_X_1( 0.f )
    , m_X_2( 0.f )
    , m_Y_1( 0.f )
    , m_Y_2( 0.f )
    , m_B0( 0.f )
    , m_B1( 0.f )
    , m_B2( 0.f )
    , m_A1( 0.f )
    , m_A2( 0.f )
{
}

void BiquadProcessor::process( float * _data, const int _sampleSize )
{
    float * data = _data;

    for ( int sample = 0 ; sample < _sampleSize ; ++sample )
    {
        float x = *data;

        float y = x * m_B0;
        y += m_X_1 * m_B1;
        y += m_X_2 * m_B2;
        y += m_Y_1 * m_A1;
        y += m_Y_2 * m_A2;

        m_X_2 = m_X_1;
        m_X_1 = x;

        m_Y_2 = m_Y_1;
        m_Y_1 = y;

        *data = y;

        ++data;
    }
}

void BiquadProcessor::setFilterParams( const float _samplingRate, const FilterType _filterType, const float _frequency, const float _quality, const float _decibelGain )
{
    m_X_2 = 0.f;
    m_X_1 = 0.f;

    m_Y_2 = 0.f;
    m_Y_1 = 0.f;

    jassert( _quality > 0.001f );
    jassert( _frequency > 1.f );
    jassert( _filterType != Off );

    float b0 = 0.f;
    float b1 = 0.f;
    float b2 = 0.f;
    float a0 = 1.f;
    float a1 = 0.f;
    float a2 = 0.f;
    const float bandwidth = 1.f / _quality;

    const float constant_log2 = 0.69314718055994530942f;
    float fA = powf( 10.f, _decibelGain / 40.f );
    float omega = 2.f * juce::float_Pi * _frequency / (float) _samplingRate;
    float sinus = sinf( omega );
    float cosinus = cosf( omega );
    float alpha = ( sinus * sinh( constant_log2 /2.f * bandwidth * omega / sinus ) );
    float beta = sqrtf(fA + fA);    

    switch ( _filterType ) 
    {
    case LowPass:
        b0 = (1.f - cosinus) /2.f;
        b1 = 1.f - cosinus;
        b2 = (1.f - cosinus) /2.f;
        a0 = 1.f + alpha;
        a1 = -2.f * cosinus;
        a2 = 1.f - alpha;
        break;

    case HighPass:
        b0 = (1.f + cosinus) /2.f;
        b1 = -(1.f + cosinus);
        b2 = (1.f + cosinus) /2.f;
        a0 = 1.f + alpha;
        a1 = -2.f * cosinus;
        a2 = 1.f - alpha;
        break;

    case BandPass:
        b0 = alpha;
        b1 = 0;
        b2 = -alpha;
        a0 = 1.f + alpha;
        a1 = -2.f * cosinus;
        a2 = 1.f - alpha;
        break;

    case Notch:
        b0 = 1.f;
        b1 = -2.f * cosinus;
        b2 = 1.f;
        a0 = 1.f + alpha;
        a1 = -2.f * cosinus;
        a2 = 1.f - alpha;
        break;

    case PeakBandEq:
        b0 = 1.f + (alpha * fA);
        b1 = -2.f * cosinus;
        b2 = 1.f - (alpha * fA);
        a0 = 1.f + (alpha /fA);
        a1 = -2.f * cosinus;
        a2 = 1.f - (alpha /fA);
        break;

    case LowShelf:
        b0 = fA * ((fA + 1.f) - (fA - 1.f) * cosinus + beta * sinus);
        b1 = 2.f * fA * ((fA - 1.f) - (fA + 1.f) * cosinus);
        b2 = fA * ((fA + 1.f) - (fA - 1.f) * cosinus - beta * sinus);
        a0 = (fA + 1.f) + (fA - 1.f) * cosinus + beta * sinus;
        a1 = -2.f * ((fA - 1.f) + (fA + 1.f) * cosinus);
        a2 = (fA + 1.f) + (fA - 1.f) * cosinus - beta * sinus;
        break;

    case HighShelf:
        b0 = fA * ((fA + 1.f) + (fA - 1.f) * cosinus + beta * sinus);
        b1 = -2.f * fA * ((fA - 1.f) + (fA + 1.f) * cosinus);
        b2 = fA * ((fA + 1.f) + (fA - 1.f) * cosinus - beta * sinus);
        a0 = (fA + 1.f) - (fA - 1.f) * cosinus + beta * sinus;
        a1 = 2.f * ((fA - 1.f) - (fA + 1.f) * cosinus);
        a2 = (fA + 1.f) - (fA - 1.f) * cosinus - beta * sinus;
        break;

    default:
        jassertfalse;

    }

    if ( a0 > 0.0 )
    {
        m_B0 = b0/a0;
        m_B1 = b1/a0;
        m_B2 = b2/a0;
        m_A1 = -a1/a0;
        m_A2 = -a2/a0;
    }
}

float getDecibelVolumeFromLinearVolume( f32 _linearVolume )
{
    static const float log10dividedBy20 = 0.1151292546497f;

    if ( _linearVolume < 0.00001f )
        return -100.f;

    return logf( _linearVolume ) / log10dividedBy20;
}

void LufsProcessor::processPeak( const juce::AudioSampleBuffer& buffer )
{
    static int sample = 0;

    for ( int ch = 0 ; ch < buffer.getNumChannels() ; ++ch )
    {
        // check size
        if ( m_tempBlock.getNumSamples() < buffer.getNumSamples() + 4 )
            m_tempBlock.setSize( 1, buffer.getNumSamples() + 4 );

        // copy channel buffer to temp buffer 
        memcpy( m_tempBlock.getArrayOfWritePointers()[0], m_memArray[ch], 4 * sizeof( float ) );
        memcpy( &m_tempBlock.getArrayOfWritePointers()[0][4], buffer.getArrayOfReadPointers()[ch], buffer.getNumSamples() * sizeof( float ) );

        const int sampleSize = 4 + buffer.getNumSamples();
        const float * data = m_tempBlock.getArrayOfReadPointers()[ 0 ];
        float prevPrev = *data++;
        float prev = *data++;
        float value = *data++;
        float next = *data++;
        float nextNext = *data++;

        for ( int i = 2 ; i < ( sampleSize - 2 ) ; ++i )
        {
            f32 maxValue = 0.f;

            if ( ( value > ( 0.7f * m_maxLinArray[ ch ] ) ) && ( value >= prev )  && ( value >= prevPrev ) && ( value >= next ) && ( value >= nextNext ) )
            {
                maxValue = value;

                LinInterpolationMinMaxEstimate estimate( data - 5, i, false );

                if ( estimate.isValid() && estimate.getMinMax() > maxValue )
                {
                    maxValue = estimate.getMinMax();
                }

            }
            else if ( ( -value > ( 0.7f * m_maxLinArray[ ch ] ) ) && ( value <= prev ) && ( value <= prevPrev ) && ( value <= next ) && ( value <= nextNext ) )
            {
                maxValue = -value;

                LinInterpolationMinMaxEstimate estimate( data - 5, i, true );

                if ( estimate.isValid() && ( -estimate.getMinMax() ) > maxValue )
                {
                    maxValue = -estimate.getMinMax();
                }
            }

            float decibels = getDecibelVolumeFromLinearVolume( maxValue );
            static float maxDecibels = decibels;
            if ( decibels > maxDecibels )
                maxDecibels = decibels;

            if ( decibels > -1.f )
            {
                //DBG("found value " + String(maxValue) + String(" at pos ") + String(sample + i) + String(" > ") + String( decibels ) + String(" dB - (max: ")  + String( maxDecibels ) + String(" dB) ") );
                DBG( juce::String("At pos ") + juce::String(sample + i) + juce::String(": ") + juce::String( decibels, 2 ) + juce::String(" dB - (max: ")  + juce::String( maxDecibels, 2 ) + juce::String(" dB) ") );
            }
            
            if ( maxValue > m_maxLinArray[ ch ] )
                m_maxLinArray[ ch ] = maxValue;

            prevPrev = prev;
            prev = value;
            value = next;
            next = nextNext;
            nextNext = *data++;
        }
    }

    sample += buffer.getNumSamples();
}





LinInterpolationMinMaxEstimate::LinInterpolationMinMaxEstimate( const f32 * _fiveValueArray, const u32 _xOffset, const bool _findMinimum ) 
    : m_y0( 0.f )
    , m_isValid( true )
    , m_xMiddle( 0.f )
    , m_xMiddleWithoutOffset( 0.f )
    , m_minMax( 0.f )
{
    // look at values to decide if we use the first four values or the last four values 
    if ( ( ( _fiveValueArray[ 2 ] - _fiveValueArray[ 0 ] ) * ( _fiveValueArray[ 4 ] - _fiveValueArray[ 2 ] ) ) > 0.f ) // these five values should constitute a min or a max
    {
        // there is a problem with the data given, don't calculate anything, use minimum if _findMinimum, else use maximum
        m_xMiddle = (f32) _xOffset + 2.f;

        if ( _findMinimum )
        {
            for ( u32 u = 1 ; u < 5 ; ++u )
            {
                if ( _fiveValueArray[ u ] < _fiveValueArray[ 0 ] )
                    m_xMiddle = (f32)( _xOffset + u );
            }
        }
        else
        {
            for ( u32 u = 1 ; u < 5 ; ++u )
            {
                if ( _fiveValueArray[ u ] > _fiveValueArray[ 0 ] )
                    m_xMiddle = (f32)( _xOffset + u );
            }
        }
        m_isValid = false;
        return;
    }

    f32 absolute_1_2 = fabs( _fiveValueArray[ 2 ] - _fiveValueArray[ 1 ] );
    f32 absolute_2_3 = fabs( _fiveValueArray[ 3 ] - _fiveValueArray[ 2 ] );
    bool useLastFour = absolute_2_3 < absolute_1_2;

    f32 origin;
    if ( useLastFour )
    {
        origin = _fiveValueArray[ 1 ];
        m_y1 = _fiveValueArray[ 2 ] - _fiveValueArray[ 1 ];
        m_y2 = _fiveValueArray[ 3 ] - _fiveValueArray[ 1 ];
        m_y3 = _fiveValueArray[ 4 ] - _fiveValueArray[ 1 ];
    }
    else
    {
        origin = _fiveValueArray[ 0 ];
        m_y1 = _fiveValueArray[ 1 ] - _fiveValueArray[ 0 ];
        m_y2 = _fiveValueArray[ 2 ] - _fiveValueArray[ 0 ];
        m_y3 = _fiveValueArray[ 3 ] - _fiveValueArray[ 0 ];
    }

    // zero division / validity check 

    if ( fabs( m_y1 + m_y2 - m_y3 ) > ( fabs( m_y1 ) / 1e6f ) )
    {
        m_xMiddleWithoutOffset = ( 3.f * m_y2 - 2.f * m_y3 ) / ( m_y1 + m_y2 - m_y3 );

        if ( 
            ( m_xMiddleWithoutOffset < 1.f ) 
            || ( m_xMiddleWithoutOffset > 2.f ) 
            || ( m_y1 * ( m_y3 - m_y2 ) >= 0.f ) 
            )
        {
            m_isValid = false;
        }
        else
        {

            m_minMax = origin + calculateMinOrMax();

            if ( _findMinimum )
            {
                if ( m_minMax >= _fiveValueArray[ 2 ] )
                {
                    m_xMiddleWithoutOffset = useLastFour ? 1.f : 2.f;
                    m_minMax = _fiveValueArray[ 2 ];
                }
                else
                {
                    for ( u32 u = 0 ; u < 5 ; ++u )
                    {
                        if ( _fiveValueArray[ u ] < m_minMax )
                        {
                            m_xMiddleWithoutOffset = 1.5f;
                            m_isValid = false;
                            break;
                        }
                    }
                }
            }
            else
            {
                if ( m_minMax <= _fiveValueArray[ 2 ] )
                {
                    m_xMiddleWithoutOffset = useLastFour ? 1.f : 2.f;
                    m_minMax = _fiveValueArray[ 2 ];
                }
                else
                {
                    for ( u32 u = 0 ; u < 5 ; ++u )
                    {
                        if ( _fiveValueArray[ u ] > m_minMax )
                        {
                            m_xMiddleWithoutOffset = 1.5f;
                            m_isValid = false;
                            break;
                        }
                    }
                }
            }
        }
    }
    else
    {
        // not valid :(

        m_xMiddleWithoutOffset = 1.5f;
        m_isValid = false;
    }

    m_xMiddle = m_xMiddleWithoutOffset;
    m_xMiddle += (f32) _xOffset;
    m_xMiddle += useLastFour ? 1.f : 0.f;
}

f32 LinInterpolationMinMaxEstimate::calculateMinOrMax()
{
    jassert( m_isValid );

    const f32 alpha = m_xMiddleWithoutOffset - 1.f;

    // left point
    const f32 leftX = 1.f + alpha * alpha;
    const f32 leftY = leftX * m_y1;

    // right point
    const f32 rightX = 1.f + 2.f * alpha - alpha * alpha;

    const f32 a = -m_y2 + m_y3;
    const f32 b = 3.f * m_y2 - 2.f * m_y3;
    const f32 rightY = a * rightX + b;

    return ( 1.f - alpha ) * leftY + alpha * rightY;
}


