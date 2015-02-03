/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*-  vi:set ts=8 sts=4 sw=4: */

/*
    Vamp feature extraction plugins using Paul Brossier's Aubio library.

    Copyright (C) 2006-2015 Paul Brossier <piem@aubio.org>

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

#include <math.h>
#include "Mfcc.h"

using std::string;
using std::vector;
using std::cerr;
using std::endl;

Mfcc::Mfcc(float inputSampleRate) :
    Plugin(inputSampleRate),
    m_ibuf(0),      // input fvec_t, set in initialise
    m_pvoc(0),      // aubio_pvoc_t, set in reset
    m_ispec(0),     // cvec_t, set in initialise
    m_mfcc(0),      // aubio_mfcc_t, set in reset
    m_ovec(0),      // output fvec_t, set in initialise
    m_nfilters(40), // parameter
    m_ncoeffs(13),  // parameter
    m_stepSize(0),  // host parameter
    m_blockSize(0)  // host parameter
{
}

Mfcc::~Mfcc()
{
    if (m_mfcc) del_aubio_mfcc(m_mfcc);
    if (m_pvoc) del_aubio_pvoc(m_pvoc);
    if (m_ibuf) del_fvec(m_ibuf);
    if (m_ispec) del_cvec(m_ispec);
    if (m_ovec) del_fvec(m_ovec);
}

string
Mfcc::getIdentifier() const
{
    return "aubiomfcc";
}

string
Mfcc::getName() const
{
    return "Aubio Mfcc Extractor";
}

string
Mfcc::getDescription() const
{
    return "Extract Mel-Frequency Cepstrum Coefficients";
}

string
Mfcc::getMaker() const
{
    return "Paul Brossier";
}

int
Mfcc::getPluginVersion() const
{
    return 3;
}

string
Mfcc::getCopyright() const
{
    return "GPL";
}

bool
Mfcc::initialise(size_t channels, size_t stepSize, size_t blockSize)
{
    if (channels != 1) {
        std::cerr << "Mfcc::initialise: channels must be 1" << std::endl;
        return false;
    }

    m_stepSize = stepSize;
    m_blockSize = blockSize;

    m_ibuf = new_fvec(stepSize);
    m_ispec = new_cvec(blockSize);
    m_ovec = new_fvec(m_ncoeffs);

    reset();

    return true;
}

void
Mfcc::reset()
{
    if (m_pvoc) del_aubio_pvoc(m_pvoc);
    if (m_mfcc) del_aubio_mfcc(m_mfcc);

    m_pvoc = new_aubio_pvoc(m_blockSize, m_stepSize);

    m_mfcc = new_aubio_mfcc(m_blockSize, m_nfilters, m_ncoeffs,
            lrintf(m_inputSampleRate));

}

size_t
Mfcc::getPreferredStepSize() const
{
    return 128;
}

size_t
Mfcc::getPreferredBlockSize() const
{
    return 512;
}

Mfcc::ParameterList
Mfcc::getParameterDescriptors() const
{
    ParameterList list;

    ParameterDescriptor desc;
    desc.identifier = "nfilters";
    desc.name = "Number of filters";
    desc.description = "Size of mel filterbank used to compute MFCCs (fixed to 40 for now)";
    desc.minValue = 40;
    desc.maxValue = 40;
    desc.defaultValue = 40;
    desc.isQuantized = true;
    desc.quantizeStep = 1;
    list.push_back(desc);

    desc = ParameterDescriptor();
    desc.identifier = "ncoeffs";
    desc.name = "Number of coefficients";
    desc.description = "Number of output coefficients to compute";
    desc.minValue = 1;
    desc.maxValue = 100;
    desc.defaultValue = 13;
    desc.isQuantized = true;
    desc.quantizeStep = 1;
    list.push_back(desc);

    return list;
}

float
Mfcc::getParameter(std::string param) const
{
    if (param == "ncoeffs") {
        return m_ncoeffs;
    } else if (param == "nfilters") {
        return m_nfilters;
    } else {
        return 0.0;
    }
}

void
Mfcc::setParameter(std::string param, float value)
{
    if (param == "nfilters") {
        m_nfilters = lrintf(value);
    } else if (param == "ncoeffs") {
        m_ncoeffs = lrintf(value);
    }
}

Mfcc::OutputList
Mfcc::getOutputDescriptors() const
{
    OutputList list;

    OutputDescriptor d;
    d.identifier = "mfcc";
    d.name = "Mel-Frequency Cepstrum Coefficients";
    d.description = "List of detected Mel-Frequency Cepstrum Coefficients";
    d.unit = "";
    d.hasFixedBinCount = true;
    d.binCount = m_ncoeffs;
    d.isQuantized = true;
    d.sampleType = OutputDescriptor::FixedSampleRate;
    d.sampleRate = OutputDescriptor::OneSamplePerStep;
    list.push_back(d);

    return list;
}

Mfcc::FeatureSet
Mfcc::process(const float *const *inputBuffers,
               __attribute__((unused)) Vamp::RealTime timestamp)
{
    FeatureSet returnFeatures;

    if (m_stepSize == 0) {
        std::cerr << "Mfcc::process: Mfcc plugin not initialised" << std::endl;
        return returnFeatures;
    }
    if (m_ncoeffs == 0) {
        std::cerr << "Mfcc::process: Mfcc plugin not initialised" << std::endl;
        return returnFeatures;
    }

    for (size_t i = 0; i < m_stepSize; ++i) {
        fvec_set_sample(m_ibuf, inputBuffers[0][i], i);
    }

    aubio_pvoc_do(m_pvoc, m_ibuf, m_ispec);
    aubio_mfcc_do(m_mfcc, m_ispec, m_ovec);

    Feature feature;
    for (uint_t i = 0; i < m_ovec->length; i++) {
        float value = m_ovec->data[i];
        feature.values.push_back(value);
    }

    returnFeatures[0].push_back(feature);
    return returnFeatures;
}

Mfcc::FeatureSet
Mfcc::getRemainingFeatures()
{
    return FeatureSet();
}

