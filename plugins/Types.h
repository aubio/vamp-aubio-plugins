/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*-  vi:set ts=8 sts=4 sw=4: */

/*
    Vamp feature extraction plugins using Paul Brossier's Aubio library.

    Centre for Digital Music, Queen Mary, University of London.
    This file copyright 2012 Queen Mary, University of London.
    
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

#ifndef _ONSET_TYPE_H_
#define _ONSET_TYPE_H_

/** silence unused parameter warning by adding an attribute */
#if defined(__GNUC__)
#define UNUSED __attribute__((unused))
#else
#define UNUSED
#endif

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

enum SpecDescType {
    SpecDescFlux,
    SpecDescCentroid,
    SpecDescSpread,
    SpecDescSkeweness,
    SpecDescKurtosis,
    SpecDescSlope,
    SpecDescDecrease,
    SpecDescRolloff
};

extern const char *getAubioNameForSpecDescType(SpecDescType t);

enum PitchType {
    PitchYin,
    PitchMComb,
    PitchSchmitt,
    PitchFComb,
    PitchYinFFT
};

extern const char *getAubioNameForPitchType(PitchType t);
    

#endif


