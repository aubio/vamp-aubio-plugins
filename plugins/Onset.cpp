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

#include <math.h>
#include "Onset.h"

using std::string;
using std::vector;
using std::cerr;
using std::endl;

Onset::Onset(float inputSampleRate) :
    Plugin(inputSampleRate),
    m_ibuf(0),
    m_onset(0),
    m_onsetdet(0),
    m_onsettype(OnsetComplex),
    m_threshold(0.3),
    m_silence(-70),
    m_minioi(4)
{
}

Onset::~Onset()
{
    if (m_onsetdet) del_aubio_onset(m_onsetdet);
    if (m_ibuf) del_fvec(m_ibuf);
    if (m_onset) del_fvec(m_onset);
}

string
Onset::getIdentifier() const
{
    return "aubioonset";
}

string
Onset::getName() const
{
    return "Aubio Onset Detector";
}

string
Onset::getDescription() const
{
    return "Estimate note onset times";
}

string
Onset::getMaker() const
{
    return "Paul Brossier (plugin by Chris Cannam)";
}

int
Onset::getPluginVersion() const
{
    return 2;
}

string
Onset::getCopyright() const
{
    return "GPL";
}

bool
Onset::initialise(size_t channels, size_t stepSize, size_t blockSize)
{
    if (channels != 1) {
        std::cerr << "Onset::initialise: channels must be 1" << std::endl;
        return false;
    }

    m_stepSize = stepSize;
    m_blockSize = blockSize;

    m_ibuf = new_fvec(stepSize);
    m_onset = new_fvec(1);

    reset();

    return true;
}

void
Onset::reset()
{
    if (m_onsetdet) del_aubio_onset(m_onsetdet);

    m_onsetdet = new_aubio_onset
        (const_cast<char *>(getAubioNameForOnsetType(m_onsettype)),
         m_blockSize,
         m_stepSize,
         lrintf(m_inputSampleRate));
    
    aubio_onset_set_threshold(m_onsetdet, m_threshold);
    aubio_onset_set_silence(m_onsetdet, m_silence);
    aubio_onset_set_minioi(m_onsetdet, m_minioi);

    m_delay = Vamp::RealTime::frame2RealTime(4 * m_stepSize,
                                             lrintf(m_inputSampleRate));

    m_lastOnset = Vamp::RealTime::zeroTime - m_delay - m_delay;
}

size_t
Onset::getPreferredStepSize() const
{
    return 512;
}

size_t
Onset::getPreferredBlockSize() const
{
    return 2 * getPreferredStepSize();
}

Onset::ParameterList
Onset::getParameterDescriptors() const
{
    ParameterList list;
    
    ParameterDescriptor desc;
    desc.identifier = "onsettype";
    desc.name = "Onset Detection Function Type";
    desc.description = "Type of onset detection function to use";
    desc.minValue = 0;
    desc.maxValue = 7;
    desc.defaultValue = (int)OnsetComplex;
    desc.isQuantized = true;
    desc.quantizeStep = 1;
    desc.valueNames.push_back("Energy Based");
    desc.valueNames.push_back("Spectral Difference");
    desc.valueNames.push_back("High-Frequency Content");
    desc.valueNames.push_back("Complex Domain");
    desc.valueNames.push_back("Phase Deviation");
    desc.valueNames.push_back("Kullback-Liebler");
    desc.valueNames.push_back("Modified Kullback-Liebler");
    desc.valueNames.push_back("Spectral Flux");
    list.push_back(desc);

    desc = ParameterDescriptor();
    desc.identifier = "peakpickthreshold";
    desc.name = "Peak Picker Threshold";
    desc.description = "Threshold used for peak picking, the higher the more detections";
    desc.minValue = 0;
    desc.maxValue = 1;
    desc.defaultValue = 0.3;
    desc.isQuantized = false;
    list.push_back(desc);

    desc = ParameterDescriptor();
    desc.identifier = "silencethreshold";
    desc.name = "Silence Threshold";
    desc.description = "Silence threshold, the higher the least detection";
    desc.minValue = -120;
    desc.maxValue = 0;
    desc.defaultValue = -70;
    desc.unit = "dB";
    desc.isQuantized = false;
    list.push_back(desc);

    desc = ParameterDescriptor();
    desc.identifier = "minioi";
    desc.name = "Minimum Inter-Onset Interval";
    desc.description = "Time interval below which two consecutive onsets should be merged";
    desc.minValue = 0;
    desc.maxValue = 40;
    desc.defaultValue = 4;
    desc.unit = "ms";
    desc.isQuantized = true;
    desc.quantizeStep = 1;
    list.push_back(desc);

    return list;
}

float
Onset::getParameter(std::string param) const
{
    if (param == "onsettype") {
        return m_onsettype;
    } else if (param == "peakpickthreshold") {
        return m_threshold;
    } else if (param == "silencethreshold") {
        return m_silence;
    } else if (param == "minioi") {
        return m_minioi;
    } else {
        return 0.0;
    }
}

void
Onset::setParameter(std::string param, float value)
{
    if (param == "onsettype") {
        switch (lrintf(value)) {
        case 0: m_onsettype = OnsetEnergy; break;
        case 1: m_onsettype = OnsetSpecDiff; break;
        case 2: m_onsettype = OnsetHFC; break;
        case 3: m_onsettype = OnsetComplex; break;
        case 4: m_onsettype = OnsetPhase; break;
        case 5: m_onsettype = OnsetKL; break;
        case 6: m_onsettype = OnsetMKL; break;
        case 7: m_onsettype = OnsetSpecFlux; break;
        }
    } else if (param == "peakpickthreshold") {
        m_threshold = value;
    } else if (param == "silencethreshold") {
        m_silence = value;
    } else if (param == "minioi") {
        m_minioi = value;
    }
}

Onset::OutputList
Onset::getOutputDescriptors() const
{
    OutputList list;

    OutputDescriptor d;
    d.identifier = "onsets";
    d.name = "Onsets";
    d.description = "List of times at which a note onset was detected";
    d.unit = "";
    d.hasFixedBinCount = true;
    d.binCount = 0;
    d.sampleType = OutputDescriptor::VariableSampleRate;
    d.sampleRate = 0;
    list.push_back(d);

    return list;
}

Onset::FeatureSet
Onset::process(const float *const *inputBuffers,
               Vamp::RealTime timestamp)
{
    for (size_t i = 0; i < m_stepSize; ++i) {
        fvec_set_sample(m_ibuf, inputBuffers[0][i], i);
    }

    aubio_onset_do(m_onsetdet, m_ibuf, m_onset);

    bool isonset = m_onset->data[0];

    FeatureSet returnFeatures;

    if (isonset) {
        if (timestamp - m_lastOnset >= m_delay) {
            Feature onsettime;
            onsettime.hasTimestamp = true;
            if (timestamp < m_delay) timestamp = m_delay;
            onsettime.timestamp = timestamp - m_delay;
            returnFeatures[0].push_back(onsettime);
            m_lastOnset = timestamp;
        }
    }

    return returnFeatures;
}

Onset::FeatureSet
Onset::getRemainingFeatures()
{
    return FeatureSet();
}

