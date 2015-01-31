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

#include <math.h>
#include "MelEnergy.h"

using std::string;
using std::vector;
using std::cerr;
using std::endl;

MelEnergy::MelEnergy(float inputSampleRate) :
    Plugin(inputSampleRate),
    m_ibuf(0),      // input fvec_t, set in initialise
    m_pvoc(0),      // aubio_pvoc_t, set in reset
    m_ispec(0),     // cvec_t, set in initialise
    m_melbank(0),   // aubio_filterbank_t, set in reset
    m_ovec(0),      // output fvec_t, set in initialise
    m_nfilters(40), // parameter
    m_stepSize(0),  // host parameter
    m_blockSize(0)  // host parameter
{
}

MelEnergy::~MelEnergy()
{
    if (m_melbank) del_aubio_filterbank(m_melbank);
    if (m_pvoc) del_aubio_pvoc(m_pvoc);
    if (m_ibuf) del_fvec(m_ibuf);
    if (m_ispec) del_cvec(m_ispec);
    if (m_ovec) del_fvec(m_ovec);
}

string
MelEnergy::getIdentifier() const
{
    return "aubiomelenergy";
}

string
MelEnergy::getName() const
{
    return "Aubio Mel Bands Energy Extractor";
}

string
MelEnergy::getDescription() const
{
    return "Extract energy in each Mel-frequency bands";
}

string
MelEnergy::getMaker() const
{
    return "Paul Brossier";
}

int
MelEnergy::getPluginVersion() const
{
    return 3;
}

string
MelEnergy::getCopyright() const
{
    return "GPL";
}

bool
MelEnergy::initialise(size_t channels, size_t stepSize, size_t blockSize)
{
    if (channels != 1) {
        std::cerr << "MelEnergy::initialise: channels must be 1" << std::endl;
        return false;
    }

    if (m_nfilters != 40) {
        std::cerr << "MelEnergy::initialise: number of filters must be 40" << std::endl;
        return false;
    }

    m_stepSize = stepSize;
    m_blockSize = blockSize;

    m_ibuf = new_fvec(stepSize);
    m_ispec = new_cvec(blockSize);
    m_ovec = new_fvec(m_nfilters);

    reset();

    return true;
}

void
MelEnergy::reset()
{
    if (m_pvoc) del_aubio_pvoc(m_pvoc);
    if (m_melbank) del_aubio_filterbank(m_melbank);

    m_pvoc = new_aubio_pvoc(m_blockSize, m_stepSize);

    m_melbank = new_aubio_filterbank(m_nfilters, m_blockSize);
    aubio_filterbank_set_mel_coeffs_slaney(m_melbank, lrintf(m_inputSampleRate));

}

size_t
MelEnergy::getPreferredStepSize() const
{
    return 128;
}

size_t
MelEnergy::getPreferredBlockSize() const
{
    return 512;
}

MelEnergy::ParameterList
MelEnergy::getParameterDescriptors() const
{
    ParameterList list;

    ParameterDescriptor desc;
    desc.identifier = "nfilters";
    desc.name = "Number of filters";
    desc.description = "Size of filterbank used to compute mel bands (fixed to 40 for now)";
    desc.minValue = 40;
    desc.maxValue = 40;
    desc.defaultValue = 40;
    desc.isQuantized = true;
    desc.quantizeStep = 1;
    list.push_back(desc);

    return list;
}

float
MelEnergy::getParameter(std::string param) const
{
    if (param == "nfilters") {
        return m_nfilters;
    } else {
        return 0.0;
    }
}

void
MelEnergy::setParameter(std::string param, float value)
{
    if (param == "nfilters") {
        m_nfilters = lrintf(value);
    }
}

MelEnergy::OutputList
MelEnergy::getOutputDescriptors() const
{
    OutputList list;

    OutputDescriptor d;
    d.identifier = "mfcc";
    d.name = "Mel-Frequency Energy per band";
    d.description = "List of computed Energies in each Mel-Frequency Band";
    d.unit = "";
    d.hasFixedBinCount = true;
    d.binCount = m_nfilters;
    d.isQuantized = true;
    d.sampleType = OutputDescriptor::FixedSampleRate;
    d.sampleRate = m_inputSampleRate / m_stepSize;
    list.push_back(d);

    return list;
}

MelEnergy::FeatureSet
MelEnergy::process(const float *const *inputBuffers,
               Vamp::RealTime timestamp)
{
    FeatureSet returnFeatures;

    if (m_stepSize == 0) {
        std::cerr << "MelEnergy::process: MelEnergy plugin not initialised" << std::endl;
        return returnFeatures;
    }
    if (m_nfilters == 0) {
        std::cerr << "MelEnergy::process: MelEnergy plugin not initialised" << std::endl;
        return returnFeatures;
    }

    for (size_t i = 0; i < m_stepSize; ++i) {
        fvec_set_sample(m_ibuf, inputBuffers[0][i], i);
    }

    aubio_pvoc_do(m_pvoc, m_ibuf, m_ispec);
    aubio_filterbank_do(m_melbank, m_ispec, m_ovec);

    Feature feature;
    //feature.hasTimestamp = false;
    feature.timestamp = timestamp;
    for (uint_t i = 0; i < m_ovec->length; i++) {
        float value = m_ovec->data[i];
        feature.values.push_back(value);
    }

    returnFeatures[0].push_back(feature);
    return returnFeatures;
}

MelEnergy::FeatureSet
MelEnergy::getRemainingFeatures()
{
    return FeatureSet();
}

