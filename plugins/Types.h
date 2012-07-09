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

#ifndef _ONSET_TYPE_H_
#define _ONSET_TYPE_H_

// Note: the enum values in this header are ordered to match the Vamp
// plugin parameter values in earlier versions of this plugin set, to
// avoid breaking stored parameter settings that use the parameter's
// numerical value. Any additional values must be added after all
// existing ones.

enum OnsetType {
    OnsetEnergy,
    OnsetSpecDiff,
    OnsetHFC,
    OnsetComplex,
    OnsetPhase,
    OnsetKL,
    OnsetMKL,
    OnsetSpecFlux // new in 0.4!
};

extern const char *getAubioNameForOnsetType(OnsetType t);

enum PitchType {
    PitchYin,
    PitchMComb,
    PitchSchmitt,
    PitchFComb,
    PitchYinFFT
};

extern const char *getAubioNameForPitchType(PitchType t);
    

#endif


