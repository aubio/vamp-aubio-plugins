/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*-  vi:set ts=8 sts=4 sw=4: */

/*
    Vamp feature extraction plugins using Paul Brossier's Aubio library.

    Copyright (C) 2006-2015 Paul Brossier <piem@aubio.org>

    This file is part of vamp-aubio.

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

#ifndef _MFCC_PLUGIN_H_
#define _MFCC_PLUGIN_H_

#include <vamp-sdk/Plugin.h>
#include <aubio/aubio.h>

#include "Types.h"

class Mfcc : public Vamp::Plugin
{
public:
    Mfcc(float inputSampleRate);
    virtual ~Mfcc();

    bool initialise(size_t channels, size_t stepSize, size_t blockSize);
    void reset();

    InputDomain getInputDomain() const { return TimeDomain; }

    std::string getIdentifier() const;
    std::string getName() const;
    std::string getDescription() const;
    std::string getMaker() const;
    int getPluginVersion() const;
    std::string getCopyright() const;

    ParameterList getParameterDescriptors() const;
    float getParameter(std::string) const;
    void setParameter(std::string, float);

    size_t getPreferredStepSize() const;
    size_t getPreferredBlockSize() const;

    OutputList getOutputDescriptors() const;

    FeatureSet process(const float *const *inputBuffers,
                       Vamp::RealTime timestamp);

    FeatureSet getRemainingFeatures();

protected:
    fvec_t *m_ibuf;
    aubio_pvoc_t *m_pvoc;
    cvec_t *m_ispec;
    aubio_mfcc_t *m_mfcc;
    fvec_t *m_ovec;

    size_t m_nfilters;
    size_t m_ncoeffs;

    size_t m_stepSize;
    size_t m_blockSize;
};


#endif
