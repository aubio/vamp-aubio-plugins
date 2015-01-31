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
#include "Pitch.h"

using std::string;
using std::vector;
using std::cerr;
using std::endl;

Pitch::Pitch(float inputSampleRate) :
    Plugin(inputSampleRate),
    m_ibuf(0),
    m_obuf(0),
    m_pitchdet(0),
    m_pitchtype(PitchYinFFT),
    m_minfreq(aubio_miditofreq(32)),
    m_maxfreq(aubio_miditofreq(95)),
    m_silence(-90),
    m_wrapRange(false),
    m_stepSize(0),
    m_blockSize(0)
{
}

Pitch::~Pitch()
{
    if (m_pitchdet) del_aubio_pitch(m_pitchdet);
    if (m_ibuf) del_fvec(m_ibuf);
    if (m_obuf) del_fvec(m_obuf);
}

string
Pitch::getIdentifier() const
{
    return "aubiopitch";
}

string
Pitch::getName() const
{
    return "Aubio Pitch Detector";
}

string
Pitch::getDescription() const
{
    return "Track estimated note pitches";
}

string
Pitch::getMaker() const
{
    return "Paul Brossier (plugin by Chris Cannam)";
}

int
Pitch::getPluginVersion() const
{
    return 3;
}

string
Pitch::getCopyright() const
{
    return "GPL";
}

bool
Pitch::initialise(size_t channels, size_t stepSize, size_t blockSize)
{
    if (channels != 1) {
        std::cerr << "Pitch::initialise: channels must be 1" << std::endl;
        return false;
    }

    m_stepSize = stepSize;
    m_blockSize = blockSize;

    m_ibuf = new_fvec(stepSize);
    m_obuf = new_fvec(1);

    reset();

    return true;
}

void
Pitch::reset()
{
    if (m_pitchdet) del_aubio_pitch(m_pitchdet);

    m_pitchdet = new_aubio_pitch
        (const_cast<char *>(getAubioNameForPitchType(m_pitchtype)),
         m_blockSize,
         m_stepSize,
         lrintf(m_inputSampleRate));

    aubio_pitch_set_unit(m_pitchdet, const_cast<char *>("freq"));
}

size_t
Pitch::getPreferredStepSize() const
{
    return 512;
}

size_t
Pitch::getPreferredBlockSize() const
{
    return 2048;
}

Pitch::ParameterList
Pitch::getParameterDescriptors() const
{
    ParameterList list;
    
    ParameterDescriptor desc;
    desc.identifier = "pitchtype";
    desc.name = "Pitch Detection Function Type";
    desc.description = "Type of pitch detection function to use";
    desc.minValue = 0;
    desc.maxValue = 4;
    desc.defaultValue = (int)PitchYinFFT;
    desc.isQuantized = true;
    desc.quantizeStep = 1;
    desc.valueNames.push_back("YIN Frequency Estimator");
    desc.valueNames.push_back("Spectral Comb");
    desc.valueNames.push_back("Schmitt");
    desc.valueNames.push_back("Fast Harmonic Comb");
    desc.valueNames.push_back("YIN with FFT");
    list.push_back(desc);

    desc = ParameterDescriptor();
    desc.identifier = "minfreq";
    desc.name = "Minimum Fundamental Frequency";
    desc.description = "Lowest frequency to look for";
    desc.minValue = 1;
    desc.maxValue = m_inputSampleRate/2;
    desc.defaultValue = aubio_miditofreq(32);
    desc.unit = "Hz";
    desc.isQuantized = false;
    list.push_back(desc);

    desc = ParameterDescriptor();
    desc.identifier = "maxfreq";
    desc.name = "Maximum Fundamental Frequency";
    desc.description = "Highest frequency to look for";
    desc.minValue = 1;
    desc.maxValue = m_inputSampleRate/2;
    desc.defaultValue = aubio_miditofreq(95);
    desc.unit = "Hz";
    desc.isQuantized = false;
    list.push_back(desc);

    desc = ParameterDescriptor();
    desc.identifier = "wraprange";
    desc.name = "Fold Higher or Lower Frequencies into Range";
    desc.description = "Frequencies detected outside the range will be transposed to higher or lower octaves";
    desc.minValue = 0;
    desc.maxValue = 1;
    desc.defaultValue = 0;
    desc.isQuantized = true;
    desc.quantizeStep = 1;
    list.push_back(desc);

    desc = ParameterDescriptor();
    desc.identifier = "silencethreshold";
    desc.name = "Silence Threshold";
    desc.description = "Silence threshold, the higher the least detection";
    desc.minValue = -120;
    desc.maxValue = 0;
    desc.defaultValue = -90;
    desc.unit = "dB";
    desc.isQuantized = false;
    list.push_back(desc);

    return list;
}

float
Pitch::getParameter(std::string param) const
{
    if (param == "pitchtype") {
        return m_pitchtype;
    } else if (param == "minfreq") {
        return m_minfreq;
    } else if (param == "maxfreq") {
        return m_maxfreq;
    } else if (param == "wraprange") {
        return m_wrapRange ? 1.0 : 0.0;
    } else if (param == "silencethreshold") {
        return m_silence;
    } else {
        return 0.0;
    }
}

void
Pitch::setParameter(std::string param, float value)
{
    if (param == "pitchtype") {
        switch (lrintf(value)) {
        case 0: m_pitchtype = PitchYin; break;
        case 1: m_pitchtype = PitchMComb; break;
        case 2: m_pitchtype = PitchSchmitt; break;
        case 3: m_pitchtype = PitchFComb; break;
        case 4: m_pitchtype = PitchYinFFT; break;
        }
    } else if (param == "minfreq") {
        m_minfreq = value;
    } else if (param == "maxfreq") {
        m_maxfreq = value;
    } else if (param == "wraprange") {
        m_wrapRange = (value > 0.5);
    } else if (param == "silencethreshold") {
        m_silence = value;
    }
}

Pitch::OutputList
Pitch::getOutputDescriptors() const
{
    OutputList list;

    OutputDescriptor d;
    d.identifier = "frequency";
    d.name = "Fundamental Frequency";
    d.description = "List of detected frequencies";
    d.unit = "Hz";
    d.hasFixedBinCount = true;
    d.binCount = 1;
    d.hasKnownExtents = false;
    d.isQuantized = false;
    d.sampleType = OutputDescriptor::VariableSampleRate;
    d.sampleRate = 0;
    if (m_stepSize != 0) {
        d.sampleRate = m_inputSampleRate / m_stepSize;
    }
    list.push_back(d);

    return list;
}

Pitch::FeatureSet
Pitch::process(const float *const *inputBuffers,
               Vamp::RealTime timestamp)
{
    FeatureSet returnFeatures;

    if (m_stepSize == 0) {
        std::cerr << "Pitch::process: Pitch plugin not initialised" << std::endl;
        return returnFeatures;
    }

    for (size_t i = 0; i < m_stepSize; ++i) {
        fvec_set_sample(m_ibuf, inputBuffers[0][i], i);
    }

    aubio_pitch_do(m_pitchdet, m_ibuf, m_obuf);
    
    float freq = m_obuf->data[0];

    bool silent = aubio_silence_detection(m_ibuf, m_silence);
    if (silent) {
//        std::cerr << "(silent)" << std::endl;
        return returnFeatures;
    }

    if (m_wrapRange) {
        while (freq > 0 && freq < m_minfreq) {
            freq = freq * 2.0;
        }
        while (freq > m_maxfreq) {
            freq = freq / 2.0;
        }
    }

    if (freq < m_minfreq || freq > m_maxfreq) {
        return returnFeatures;
    }

    Feature feature;
    feature.hasTimestamp = true;
    feature.timestamp = timestamp;
    feature.values.push_back(freq);

    returnFeatures[0].push_back(feature);
    return returnFeatures;
}

Pitch::FeatureSet
Pitch::getRemainingFeatures()
{
    return FeatureSet();
}

