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

#include "Chart.h"

 #include "LufsAudioProcessor.h"
 #include "LufsPluginEditor.h"


void OUTPUTDEBUG( const char * czText, ... )
{
    char formattedText[0x200];
    va_list args ;
    va_start (args, czText);
    vsprintf_s(formattedText, czText, args);
    va_end (args) ;

    DBG( formattedText ) ; 
}


int Chart::getVolumeY( const int height, const float decibels )
{
    if ( decibels > m_maxChartVolume )
        return 0;

    if ( decibels < m_minChartVolume )
        return height;

    const float offset = 25.f; // offset to avoid being too close to 0 in log function 
    float value = logf( offset - decibels );
    static const float valueMin = logf( offset - m_minChartVolume );
    static const float valueMax = logf( offset - m_maxChartVolume );

    return int( ( value - valueMax ) * (float) ( height ) / ( valueMin - valueMax ) );
}

void Chart::update()
{
    LufsAudioProcessor* processor = m_editor->getProcessor();

    bool cursorIsAtMaxRight = false;
    if ( m_chartView != nullptr )
    {
        const int viewPositionX = m_chartView->getViewPositionX();
        const int viewWidth = m_chartView->getViewWidth();
        cursorIsAtMaxRight = ( viewPositionX + viewWidth ) >= ( m_validSize - 2 );
    }
 
    m_validSize = processor->m_lufsProcessor.getValidSize();
    if ( m_validSize > getWidth() )
        setSize( m_validSize, getHeight() );

    if ( cursorIsAtMaxRight )
    {
        m_chartView->setViewPositionProportionately( 1.0, 1.0 );
    }

    repaint();
}

void Chart::paint(juce::Graphics& g)
{
    juce::Rectangle<int> clipBounds = g.getClipBounds();
/*    juce::String log("Chart::paint X");
    log += clipBounds.getX(); 
    log += " width ";
    log += clipBounds.getWidth();
    DBG( log );
    */
    LufsAudioProcessor* processor = m_editor->getProcessor();

    const int imageWidth = getWidth();
    const int imageHeight = getHeight();

    g.setColour( COLOR_BACKGROUND_GRAPH );
    g.fillRect( 0, 0, imageWidth, imageHeight );

    if ( m_validSize && clipBounds.getX() < ( m_validSize - 2 ) )
    {
        // range 
        g.setColour( COLOR_RANGE );
        int yRange = getVolumeY( imageHeight, processor->m_lufsProcessor.getRangeMaxVolume() );
        int hRange = getVolumeY( imageHeight, processor->m_lufsProcessor.getRangeMinVolume() ) - yRange;
        g.fillRect( clipBounds.getX(), yRange, clipBounds.getWidth(), hRange );

        // volume lines 
        juce::Font lufsFont( 12.f );
        lufsFont.setBold(true);
        g.setFont( lufsFont );
        g.setColour( juce::Colours::black );
        for ( float v = -3.f ; v > -70.f ; v -= 3.f )
        {
            if ( ( v >= m_minChartVolume ) && ( v <= ( m_maxChartVolume ) ) )
            {
                int y = getVolumeY( imageHeight, v );
                g.fillRect( 0, y, imageWidth, 1 );
            }
        }
/*
        juce::Font timeFont( 18.f );
        timeFont.setBold(true);
        g.setFont( timeFont );
        g.setColour( juce::Colours::black );
        */

        // time lines
        const int beginning = clipBounds.getX(); 
        for ( int i = beginning - 100 ; i < ( beginning + imageWidth ) ; ++i )
        {
            if ( !( i % 100 ) )
            {
                const int seconds = ( i / 10 ) % 60;
                const int minutes = ( i / 600 ) % 60;
                const int hours = ( i / 36000 );
                juce::String text;
                if ( hours )
                {
                    text += juce::String( hours );
                    text += ":";
                }
                if ( minutes < 10 ) text += "0";
                text += juce::String( minutes );
                text += ":";
                if ( seconds < 10 ) text += "0";
                text += juce::String( seconds );

                g.drawFittedText( text, i + 5, imageHeight - 12, 60, 10, juce::Justification::centredLeft, 1, 0.01f );

                g.fillRect( i, 0, 1, imageHeight );
            }
        }


        g.setColour( COLOR_INTEGRATED );
        g.fillRect( 0, getVolumeY( imageHeight, processor->m_lufsProcessor.getIntegratedVolume() ), imageWidth, 3 );

        int size = imageWidth;
        if ( beginning + size > m_validSize - 2 )
            size = m_validSize - 2 - beginning;

//        if ( beginning < m_validSize - 2 )
        {
//             paintValues( g, COLOR_MOMENTARY, processor->m_lufsProcessor.getMomentaryVolumeArray(), 1, beginning, m_validSize - 1 - beginning );
//             paintValues( g, COLOR_SHORTTERM, processor->m_lufsProcessor.getShortTermVolumeArray(), 1, beginning, m_validSize - 1 - beginning );

            const int factor = 1;
            paintValues( g, COLOR_MOMENTARY, processor->m_lufsProcessor.getMomentaryVolumeArray(), factor, beginning, size / factor );
            paintValues( g, COLOR_SHORTTERM, processor->m_lufsProcessor.getShortTermVolumeArray(), factor, beginning, size / factor );
        }
/*
        g.setColour( COLOR_MOMENTARY );

        for ( int i = beginning ; i < m_validSize - 2 ; ++i )
        {
            float vol1 = processor->m_lufsProcessor.getMomentaryVolumeArray()[ i ];
            float vol2 = processor->m_lufsProcessor.getMomentaryVolumeArray()[ i + 1];
            g.drawLine( (float)( i - beginning ), (float)getVolumeY( imageHeight, vol1 ), (float)(i + 1 - beginning), (float)getVolumeY( imageHeight, vol2 ), 3.f );
        }

        g.setColour( COLOR_SHORTTERM );
        for ( int i = beginning ; i < m_validSize - 2 ; ++i )
        {
            float vol1 = processor->m_lufsProcessor.getShortTermVolumeArray()[ i ];
            float vol2 = processor->m_lufsProcessor.getShortTermVolumeArray()[ i + 1];
            g.drawLine( (float)(i - beginning), (float)getVolumeY( imageHeight, vol1 ), (float)(i + 1 - beginning), (float)getVolumeY( imageHeight, vol2 ), 3.f );
        }*/
    }
    else
    {
        g.setColour( juce::Colours::black );
        for ( float v = -5.f ; v > -70.f ; v -= 5.f )
        {
            int y = getVolumeY( imageHeight, v );
            g.fillRect( 0, y, imageWidth, 1 );
        }
    }

/*    g.setColour( juce::Colours::black );
    juce::Font lufsFont( 12.f );
    lufsFont.setBold(true);
    g.setFont( lufsFont );
*/
    g.setColour( juce::Colours::black );

    // volume text
    for ( float v = -3.f ; v > -70.f ; v -= 3.f )
    {
        if ( ( v >= m_minChartVolume + 3.f ) && ( v <= m_maxChartVolume ) )
        {
            int y = getVolumeY( imageHeight, v );
            g.drawFittedText( juce::String( v, 0 ), 5, y - 30, 20, 30, juce::Justification::centredBottom, 1, 0.01f );
        }
    }

    // memory
    float memoryPercent = 100.f * (float)processor->m_lufsProcessor.getValidSize() / (float)processor->m_lufsProcessor.getMaxSize();

    if ( memoryPercent > 80.f )
        g.setColour( juce::Colours::red );

    juce::String memory( "Memory: ");
    memory << juce::String( memoryPercent, 2 );
    memory << " %";
    g.drawFittedText( memory, imageWidth - 125, imageHeight - 23, 120, 10, juce::Justification::bottomRight, 1, 0.01f );

}

void Chart::paintValues( juce::Graphics& g, const juce::Colour _color, const float * _data, const int _itemsPerPixel, const int _offset, const int _pixels )
{
    g.setColour( _color );

    const int imageHeight = getHeight();
    const int max = _offset + _pixels * _itemsPerPixel;

    jassert( _offset < m_validSize );
    jassert( max < m_validSize );

    _data += _offset;

    if ( _itemsPerPixel == 1 )
    {
/*
        float vol = *_data++;

        juce::Path p;
        p.startNewSubPath( (float)_offset , (float) getVolumeY( imageHeight, vol ) );

        juce::Array<float> volYArray;
        for ( int i = _offset + 1 ; i < _offset + _pixels ; ++i )
        {
            vol = *_data++;
            float volY = (float)getVolumeY( imageHeight, vol );
            volYArray.add( volY );
            p.lineTo( (float)i, volY );
        }

        for ( int i = _offset + _pixels - 1 ; i > _offset ; --i )
        {
            int arrayOffset = i - _offset - 1;
            float volY = volYArray[ arrayOffset ];
            p.lineTo( 4.f + (float)i, 4.f + volY );
        }

        p.closeSubPath();
        g.fillPath( p );
*/
        float vol1 = *_data++;
        float vol2 = *_data++;

        for ( int i = _offset ; i < _offset + _pixels - 2 ; ++i )
        {
            g.drawLine( (float)( i ), (float)getVolumeY( imageHeight, vol1 ), (float)( i + 1 ), (float)getVolumeY( imageHeight, vol2 ), 3.f );
            vol1 = vol2;
            vol2 = *_data++;
        }   
    }
    else
    {
        f32 min1 = *_data++;;
        f32 max1 = min1;
        for ( int j = 1 ; j < _itemsPerPixel ; ++j )
        {
            f32 val = *_data++;
            if ( val > max1 ) max1 = val;
            if ( val < min1 ) min1 = val;
        }

        for ( int i = 0 ; i < _pixels - 2; ++i )
        {
            f32 min2 = *_data++;;
            f32 max2 = min2;
            for ( int j = 1 ; j < _itemsPerPixel ; ++j )
            {
                f32 val = *_data++;
                if ( val > max2 ) max2 = val;
                if ( val < min2 ) min2 = val;
            }

//            g.drawLine( (float)( i ), (float)getVolumeY( imageHeight, max ), (float)( i + 1 ), (float)getVolumeY( imageHeight, max ), 3.f );
            g.drawLine( (float)( i ), (float)getVolumeY( imageHeight, max1 ), (float)( i + 1 ), (float)getVolumeY( imageHeight, max2 ), 3.f );
            max1 = max2;
        }

    }
}



ChartView::ChartView( float _minChartVolume, float _maxChartVolume ) 
    : m_chart (_minChartVolume, _maxChartVolume )
{
    m_chart.setSize( 10, 300 );
    m_chart.setChartView( this );

    setViewedComponent( &m_chart, false );
}
/*
void ChartView::visibleAreaChanged( const juce::Rectangle<int>& newVisibleArea )
{
    juce::String log("ChartView::visibleAreaChanged left ");
    log += newVisibleArea.getTopLeft().getX();
    log += " right ";
    log += newVisibleArea.getBottomRight().getX();
    log += " width ";
    log += m_chart.getWidth();
    DBG( log );
    juce::Viewport::visibleAreaChanged( newVisibleArea );
}*/

void ChartView::resized()
{
    m_chart.setSize( 1 + getWidth(), getHeight() - 35 );
}

void ChartView::mouseWheelMove( const juce::MouseEvent& /*event*/, const juce::MouseWheelDetails & wheel ) 
{
    juce::String text( wheel.deltaX );
    text += " y: ";
    text += juce::String( wheel.deltaY );
    DBG(text);
/*
    const int viewPositionX = getViewPositionX();
    const int viewPositionY = getViewPositionY();

    if ( wheel.deltaY > 0.f )
    {
        int newPositionX = viewPositionX + getViewWidth() / 2;
        if ( newPositionX + getWidth() > m_chart.getWidth() )
            newPositionX = m_chart.getWidth() - getWidth();
        
        setViewPosition( newPositionX, viewPositionY );
    }
    else if ( wheel.deltaY < 0.f )
    {
        int newPositionX = viewPositionX - getViewWidth() / 2;
        if ( newPositionX < 0 )
            newPositionX = 0;
        
        setViewPosition( newPositionX, viewPositionY );
    }

    OUTPUTDEBUG( "viewPositionX %d -> %d (getMaximumVisibleWidth() %d getViewWidth() %d chart width %d)", 
        viewPositionX, getViewPositionX(), getMaximumVisibleWidth(), getViewWidth(), m_chart.getWidth() );

    juce::Viewport::mouseWheelMove( event, wheel );*/

    const int viewPositionX = getViewPositionX();
    const int width = m_chart.getWidth() - getWidth();
    const float pos = (float)viewPositionX / (float)width;
    const float page = (float)getWidth() / (float)width;

    if ( wheel.deltaY > 0.f )
    {
        float newPositionX = pos + page / 2.f;
        if ( newPositionX > 1.f )
            newPositionX = 1.f;
        
        setViewPositionProportionately( newPositionX, 1.f );
    }
    else if ( wheel.deltaY < 0.f )
    {
        float newPositionX = pos - page / 2.f;
        if ( newPositionX < 0.f )
            newPositionX = 0.f;
        
        setViewPositionProportionately( newPositionX, 1.f );
    }
}

void ChartView::update()
{
    m_chart.update();
}
