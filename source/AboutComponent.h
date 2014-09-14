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

#ifndef __LUFS_ABOUTCOMPONENT_H__
#define __LUFS_ABOUTCOMPONENT_H__

class AboutComponent : public juce::Component
{
public:

    AboutComponent();
    virtual ~AboutComponent();

    // juce::Component
    virtual void paint( juce::Graphics & g );

};


#endif  // __LUFS_ABOUTCOMPONENT_H__
