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

#include "Pitch.h"

using std::string;
using std::vector;
using std::cerr;
using std::endl;

Pitch::Pitch(float inputSampleRate) :
    Plugin(inputSampleRate),
    m_ibuf(0),
    m_pitchdet(0),
    m_pitchtype(aubio_pitch_fcomb),
    m_pitchmode(aubio_pitchm_freq)
{
}

Pitch::~Pitch()
{
    if (m_pitchdet) del_aubio_pitchdetection(m_pitchdet);
    if (m_ibuf) del_fvec(m_ibuf);
}

string
Pitch::getName() const
{
    return "aubiopitch";
}

string
Pitch::getDescription() const
{
    return "Aubio Pitch Detector";
}

string
Pitch::getMaker() const
{
    return "Paul Brossier (plugin by Chris Cannam)";
}

int
Pitch::getPluginVersion() const
{
    return 1;
}

string
Pitch::getCopyright() const
{
    return "GPL";
}

bool
Pitch::initialise(size_t channels, size_t stepSize, size_t blockSize)
{
    m_channelCount = channels;
    m_stepSize = stepSize;
    m_blockSize = blockSize;

    m_ibuf = new_fvec(stepSize, channels);

    m_pitchdet = new_aubio_pitchdetection(blockSize * 4,
                                          stepSize,
                                          channels,
                                          lrintf(m_inputSampleRate),
                                          m_pitchtype,
                                          m_pitchmode);

    return true;
}

void
Pitch::reset()
{
}

size_t
Pitch::getPreferredStepSize() const
{
    return 512;
}

size_t
Pitch::getPreferredBlockSize() const
{
    return 1024;
}

Pitch::ParameterList
Pitch::getParameterDescriptors() const
{
    ParameterList list;
    
    ParameterDescriptor desc;
    desc.name = "pitchtype";
    desc.description = "Pitch Detection Function Type";
    desc.minValue = 0;
    desc.maxValue = 4;
    desc.defaultValue = (int)aubio_pitch_fcomb;
    desc.isQuantized = true;
    desc.quantizeStep = 1;
    desc.valueNames.push_back("YIN Frequency Estimator");
    desc.valueNames.push_back("Spectral Comb");
    desc.valueNames.push_back("Schmitt");
    desc.valueNames.push_back("Fast Harmonic Comb");
    desc.valueNames.push_back("YIN with FFT");
    list.push_back(desc);

    return list;
}

float
Pitch::getParameter(std::string param) const
{
    if (param == "pitchtype") {
        return m_pitchtype;
    } else {
        return 0.0;
    }
}

void
Pitch::setParameter(std::string param, float value)
{
    if (param == "pitchtype") {
        switch (lrintf(value)) {
        case 0: m_pitchtype = aubio_pitch_yin; break;
        case 1: m_pitchtype = aubio_pitch_mcomb; break;
        case 2: m_pitchtype = aubio_pitch_schmitt; break;
        case 3: m_pitchtype = aubio_pitch_fcomb; break;
        case 4: m_pitchtype = aubio_pitch_yinfft; break;
        }
    }
}

Pitch::OutputList
Pitch::getOutputDescriptors() const
{
    OutputList list;

    OutputDescriptor d;
    d.name = "frequency";
    d.unit = "Hz";
    d.description = "Frequency";
    d.hasFixedBinCount = true;
    d.binCount = 1;
    d.hasKnownExtents = false;
    d.isQuantized = false;
    d.sampleType = OutputDescriptor::OneSamplePerStep;
    list.push_back(d);

    return list;
}

Pitch::FeatureSet
Pitch::process(float **inputBuffers, Vamp::RealTime /* timestamp */)
{
    for (size_t i = 0; i < m_stepSize; ++i) {
        for (size_t j = 0; j < m_channelCount; ++j) {
            fvec_write_sample(m_ibuf, inputBuffers[j][i], j, i);
        }
    }

    float pitch = aubio_pitchdetection(m_pitchdet, m_ibuf);

    Feature feature;
    feature.hasTimestamp = false;
    feature.values.push_back(pitch);

    FeatureSet returnFeatures;
    returnFeatures[0].push_back(feature);
    return returnFeatures;
}

Pitch::FeatureSet
Pitch::getRemainingFeatures()
{
    return FeatureSet();
}

