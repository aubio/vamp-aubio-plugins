/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*-  vi:set ts=8 sts=4 sw=4: */

/*
    Vamp feature extraction plugins using Paul Brossier's Aubio library.

    Centre for Digital Music, Queen Mary, University of London.
    This file copyright 2006 Chris Cannam.
    
    This file is part of vamp-aubio-plugins.

    vamp-aubio is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    vamp-aubio is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with aubio.  If not, see <http://www.gnu.org/licenses/>.

*/

#include <vamp/vamp.h>
#include <vamp-sdk/PluginAdapter.h>

#include "plugins/Onset.h"
#include "plugins/Pitch.h"
#include "plugins/Notes.h"
#include "plugins/Tempo.h"
#include "plugins/Silence.h"
#include "plugins/Mfcc.h"
#include "plugins/MelEnergy.h"
#include "plugins/SpecDesc.h"

static Vamp::PluginAdapter<Onset> onsetAdapter;
static Vamp::PluginAdapter<Pitch> pitchAdapter;
static Vamp::PluginAdapter<Notes> notesAdapter;
static Vamp::PluginAdapter<Tempo> tempoAdapter;
static Vamp::PluginAdapter<Silence> silenceAdapter;
static Vamp::PluginAdapter<Mfcc> mfccAdapter;
static Vamp::PluginAdapter<MelEnergy> melenergyAdapter;
static Vamp::PluginAdapter<SpecDesc> specdescAdapter;

const VampPluginDescriptor *vampGetPluginDescriptor(unsigned int vampApiVersion,
                                                    unsigned int index)
{
    if (vampApiVersion < 2) return 0;

    switch (index) {
    case  0: return onsetAdapter.getDescriptor();
    case  1: return pitchAdapter.getDescriptor();
    case  2: return notesAdapter.getDescriptor();
    case  3: return tempoAdapter.getDescriptor();
    case  4: return silenceAdapter.getDescriptor();
    case  5: return mfccAdapter.getDescriptor();
    case  6: return melenergyAdapter.getDescriptor();
    case  7: return specdescAdapter.getDescriptor();
    default: return 0;
    }
}

