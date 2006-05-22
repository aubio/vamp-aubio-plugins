/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*-  vi:set ts=8 sts=4 sw=4: */

/*
    Vamp feature extraction plugins using Paul Brossier's Aubio library.

    Centre for Digital Music, Queen Mary, University of London.
    This file copyright 2006 Chris Cannam.
    
    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.  See the file
    COPYING included with this distribution for more information.

*/

#include <vamp/vamp.h>
#include <vamp-sdk/PluginAdapter.h>

#include "plugins/Onset.h"
#include "plugins/Pitch.h"
#include "plugins/Notes.h"
#include "plugins/Tempo.h"

static Vamp::PluginAdapter<Onset> onsetAdapter;
static Vamp::PluginAdapter<Pitch> pitchAdapter;
static Vamp::PluginAdapter<Notes> notesAdapter;
static Vamp::PluginAdapter<Tempo> tempoAdapter;

const VampPluginDescriptor *vampGetPluginDescriptor(unsigned int index)
{
    switch (index) {
    case  0: return onsetAdapter.getDescriptor();
    case  1: return pitchAdapter.getDescriptor();
    case  2: return notesAdapter.getDescriptor();
    case  3: return tempoAdapter.getDescriptor();
    default: return 0;
    }
}

