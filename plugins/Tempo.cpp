/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*-  vi:set ts=8 sts=4 sw=4: */

/*
    Vamp feature extraction plugins using Paul Brossier's Aubio library.

    Centre for Digital Music, Queen Mary, University of London.
    This file copyright 2006 Chris Cannam.
    
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
#include "Tempo.h"

using std::string;
using std::vector;
using std::cerr;
using std::endl;

Tempo::Tempo(float inputSampleRate) :
    Plugin(inputSampleRate),
    m_ibuf(0),
    m_beat(0),
    m_bpm(0),
    m_onsettype(OnsetComplex),
    m_tempo(0),
    m_threshold(0.3),
    m_silence(-70)
{
}

Tempo::~Tempo()
{
    if (m_ibuf) del_fvec(m_ibuf);
    if (m_beat) del_fvec(m_beat);
    if (m_tempo) del_aubio_tempo(m_tempo);
}

string
Tempo::getIdentifier() const
{
    return "aubiotempo";
}

string
Tempo::getName() const
{
    return "Aubio Beat Tracker";
}

string
Tempo::getDescription() const
{
    return "Estimate the musical tempo and track beat positions";
}

string
Tempo::getMaker() const
{
    return "Paul Brossier (method by Matthew Davies, plugin by Chris Cannam)";
}

int
Tempo::getPluginVersion() const
{
    return 2;
}

string
Tempo::getCopyright() const
{
    return "GPL";
}

bool
Tempo::initialise(size_t channels, size_t stepSize, size_t blockSize)
{
    if (channels != 1) {
        std::cerr << "Tempo::initialise: channels must be 1" << std::endl;
        return false;
    }

    m_stepSize = stepSize;
    m_blockSize = blockSize;

    m_ibuf = new_fvec(stepSize);
    m_beat = new_fvec(2);
    
    m_delay = Vamp::RealTime::frame2RealTime(3 * stepSize,
                                             lrintf(m_inputSampleRate));

    reset();

    return true;
}

void
Tempo::reset()
{
    if (m_tempo) del_aubio_tempo(m_tempo);

    m_lastBeat = Vamp::RealTime::zeroTime - m_delay - m_delay;

    m_tempo = new_aubio_tempo
        (const_cast<char *>(getAubioNameForOnsetType(m_onsettype)),
         m_blockSize,
         m_stepSize,
         lrintf(m_inputSampleRate));

    aubio_tempo_set_silence(m_tempo, m_silence);
    aubio_tempo_set_threshold(m_tempo, m_threshold);
}

size_t
Tempo::getPreferredStepSize() const
{
    return 512;
}

size_t
Tempo::getPreferredBlockSize() const
{
    return 2 * getPreferredStepSize();
}

Tempo::ParameterList
Tempo::getParameterDescriptors() const
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
    desc.description = "Peak picking threshold, the higher the least detection";
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

    return list;
}

float
Tempo::getParameter(std::string param) const
{
    if (param == "onsettype") {
        return m_onsettype;
    } else if (param == "peakpickthreshold") {
        return m_threshold;
    } else if (param == "silencethreshold") {
        return m_silence;
    } else {
        return 0.0;
    }
}

void
Tempo::setParameter(std::string param, float value)
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
    }
}

Tempo::OutputList
Tempo::getOutputDescriptors() const
{
    OutputList list;

    OutputDescriptor d;
    d.identifier = "beats";
    d.name = "Beats";
    d.description = "List of times at which a beat was detected";
    d.unit = "";
    d.hasFixedBinCount = true;
    d.binCount = 0;
    d.sampleType = OutputDescriptor::VariableSampleRate;
    d.sampleRate = 0;
    list.push_back(d);

    d.identifier = "tempo";
    d.name = "Tempo";
    d.description = "Overall estimated tempo";
    d.unit = "bpm";
    d.hasFixedBinCount = true;
    d.binCount = 1;
    d.hasKnownExtents = false;
    d.isQuantized = false;
    d.sampleType = OutputDescriptor::OneSamplePerStep;
    list.push_back(d);

    return list;
}

Tempo::FeatureSet
Tempo::process(const float *const *inputBuffers, Vamp::RealTime timestamp)
{
    for (size_t i = 0; i < m_stepSize; ++i) {
        fvec_set_sample(m_ibuf, inputBuffers[0][i], i);
    }

    aubio_tempo_do(m_tempo, m_ibuf, m_beat);

    bool istactus = m_beat->data[0];

    m_bpm = aubio_tempo_get_bpm(m_tempo);

    FeatureSet returnFeatures;

    if (istactus == true) {
        if (timestamp - m_lastBeat >= m_delay) {
            Feature onsettime;
            onsettime.hasTimestamp = true;
            if (timestamp < m_delay) timestamp = m_delay;
            onsettime.timestamp = timestamp - m_delay;
            returnFeatures[0].push_back(onsettime);
            m_lastBeat = timestamp;
        }
    }

    if (m_bpm >= 30 && m_bpm <= 206) {
        Feature tempo;
        tempo.hasTimestamp = false;
        tempo.values.push_back(m_bpm);
        returnFeatures[1].push_back(tempo);
    }

    return returnFeatures;
}

Tempo::FeatureSet
Tempo::getRemainingFeatures()
{
    return FeatureSet();
}

