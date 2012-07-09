/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*-  vi:set ts=8 sts=4 sw=4: */

/*
    Vamp feature extraction plugins using Paul Brossier's Aubio library.

    Centre for Digital Music, Queen Mary, University of London.
    This file copyright 2012 Queen Mary, University of London.
    
    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.  See the file
    COPYING included with this distribution for more information.

*/

#include "Types.h"

const char *getAubioNameForOnsetType(OnsetType t)
{
    // In the same order as the enum elements in the header
    static const char *const names[] = {
        "energy", "specdiff", "hfc", "complex", "phase", "kl", "mkl", "specflux"
    };
    return names[(int)t];
}

const char *getAubioNameForPitchType(PitchType t)
{
    // In the same order as the enum elements in the header
    static const char *const names[] = {
        "yin", "mcomb", "schmitt", "fcomb", "yinfft"
    };
    return names[(int)t];
}

