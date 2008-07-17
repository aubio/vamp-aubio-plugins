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
#include "plugins/Silence.h"

template <typename P>
class VersionedPluginAdapter : public Vamp::PluginAdapterBase
{
public:
    VersionedPluginAdapter(unsigned int v) : PluginAdapterBase(), m_v(v) { }
    virtual ~VersionedPluginAdapter() { }

protected:
    Vamp::Plugin *createPlugin(float inputSampleRate) {
        P *p = new P(inputSampleRate, m_v);
        Vamp::Plugin *plugin = dynamic_cast<Vamp::Plugin *>(p);
        return plugin;
    }
    unsigned int m_v;
};

static Vamp::PluginAdapter<Onset> onsetAdapter;
static Vamp::PluginAdapter<Pitch> pitchAdapter;
static Vamp::PluginAdapter<Tempo> tempoAdapter;

// These two plugins both benefit from the Vamp v2 API if available
static VersionedPluginAdapter<Notes> *notesAdapter = 0;
static VersionedPluginAdapter<Silence> *silenceAdapter = 0;

struct Tidy
{
    ~Tidy() { delete notesAdapter; delete silenceAdapter; }
};
static Tidy tidy;

const VampPluginDescriptor *vampGetPluginDescriptor(unsigned int vampApiVersion,
                                                    unsigned int index)
{
    if (vampApiVersion < 1) return 0;

    switch (index) {
    case  0: return onsetAdapter.getDescriptor();
    case  1: return pitchAdapter.getDescriptor();
    case  3: return tempoAdapter.getDescriptor();

    case  2: 
        if (!notesAdapter) {
            notesAdapter = new VersionedPluginAdapter<Notes>(vampApiVersion);
        }
        return notesAdapter->getDescriptor();

    case  4:
        if (!silenceAdapter) {
            silenceAdapter = new VersionedPluginAdapter<Silence>(vampApiVersion);
        }
        return silenceAdapter->getDescriptor();

    default: return 0;
    }
}

