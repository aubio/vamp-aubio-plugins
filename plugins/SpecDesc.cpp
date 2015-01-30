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
#include "SpecDesc.h"

using std::string;
using std::vector;
using std::cerr;
using std::endl;

SpecDesc::SpecDesc(float inputSampleRate) :
    Plugin(inputSampleRate),
    m_ibuf(0),
    m_pvoc(0),
    m_ispec(0),
    m_specdesc(0),
    m_out(0),
    m_specdesctype(SpecDescFlux)
{
}

SpecDesc::~SpecDesc()
{
    if (m_specdesc) del_aubio_specdesc(m_specdesc);
    if (m_pvoc) del_aubio_pvoc(m_pvoc);
    if (m_ibuf) del_fvec(m_ibuf);
    if (m_ispec) del_cvec(m_ispec);
    if (m_out) del_fvec(m_out);
}

string
SpecDesc::getIdentifier() const
{
    return "aubiospecdesc";
}

string
SpecDesc::getName() const
{
    return "Aubio Spectral Descriptor";
}

string
SpecDesc::getDescription() const
{
    return "Compute spectral descriptor";
}

string
SpecDesc::getMaker() const
{
    return "Paul Brossier";
}

int
SpecDesc::getPluginVersion() const
{
    return 2;
}

string
SpecDesc::getCopyright() const
{
    return "GPL";
}

bool
SpecDesc::initialise(size_t channels, size_t stepSize, size_t blockSize)
{
    if (channels != 1) {
        std::cerr << "SpecDesc::initialise: channels must be 1" << std::endl;
        return false;
    }

    m_stepSize = stepSize;
    m_blockSize = blockSize;

    m_ibuf = new_fvec(stepSize);
    m_ispec = new_cvec(blockSize);
    m_out = new_fvec(1);

    reset();

    return true;
}

void
SpecDesc::reset()
{
    if (m_pvoc) del_aubio_pvoc(m_pvoc);
    if (m_specdesc) del_aubio_specdesc(m_specdesc);

    m_specdesc = new_aubio_specdesc
        (const_cast<char *>(getAubioNameForSpecDescType(m_specdesctype)),
         m_blockSize);

    m_pvoc = new_aubio_pvoc(m_blockSize, m_stepSize);
}

size_t
SpecDesc::getPreferredStepSize() const
{
    return 512;
}

size_t
SpecDesc::getPreferredBlockSize() const
{
    return 2 * getPreferredStepSize();
}

SpecDesc::ParameterList
SpecDesc::getParameterDescriptors() const
{
    ParameterList list;

    ParameterDescriptor desc;
    desc.identifier = "specdesctype";
    desc.name = "Spectral Descriptor Type";
    desc.description = "Type of spectral descriptor to use";
    desc.minValue = 0;
    desc.maxValue = 7;
    desc.defaultValue = (int)SpecDescFlux;
    desc.isQuantized = true;
    desc.quantizeStep = 1;
    desc.valueNames.push_back("Spectral Flux");
    desc.valueNames.push_back("Spectral Centroid");
    desc.valueNames.push_back("Spectral Spread");
    desc.valueNames.push_back("Spectral Skewness");
    desc.valueNames.push_back("Spectral Kurtosis");
    desc.valueNames.push_back("Spectral Slope");
    desc.valueNames.push_back("Spectral Decrease");
    desc.valueNames.push_back("Spectral Rolloff");
    list.push_back(desc);

    return list;
}

float
SpecDesc::getParameter(std::string param) const
{
    if (param == "specdesctype") {
        return m_specdesctype;
    } else {
        return 0.0;
    }
}

void
SpecDesc::setParameter(std::string param, float value)
{
    if (param == "specdesctype") {
        switch (lrintf(value)) {
        case 0: m_specdesctype = SpecDescFlux; break;
        case 1: m_specdesctype = SpecDescCentroid; break;
        case 2: m_specdesctype = SpecDescSpread; break;
        case 3: m_specdesctype = SpecDescSkeweness; break;
        case 4: m_specdesctype = SpecDescKurtosis; break;
        case 5: m_specdesctype = SpecDescSlope; break;
        case 6: m_specdesctype = SpecDescDecrease; break;
        case 7: m_specdesctype = SpecDescRolloff; break;
        }
    }
}

SpecDesc::OutputList
SpecDesc::getOutputDescriptors() const
{
    OutputList list;

    OutputDescriptor d;
    d.identifier = "specdesc";
    d.name = "Spectral descriptor output";
    d.description = "Output of the spectral descpriptor";
    d.binCount = 1;
    d.isQuantized = true;
    d.quantizeStep = 1.0;
    d.sampleType = OutputDescriptor::OneSamplePerStep;
    list.push_back(d);

    return list;
}

SpecDesc::FeatureSet
SpecDesc::process(const float *const *inputBuffers,
               __attribute__((unused)) Vamp::RealTime timestamp)
{
    for (size_t i = 0; i < m_stepSize; ++i) {
        fvec_set_sample(m_ibuf, inputBuffers[0][i], i);
    }

    aubio_pvoc_do(m_pvoc, m_ibuf, m_ispec);
    aubio_specdesc_do(m_specdesc, m_ispec, m_out);

    FeatureSet returnFeatures;

    Feature specdesc;
    specdesc.hasTimestamp = false;
    specdesc.values.push_back(m_out->data[0]);
    returnFeatures[0].push_back(specdesc);

    return returnFeatures;
}

SpecDesc::FeatureSet
SpecDesc::getRemainingFeatures()
{
    return FeatureSet();
}

