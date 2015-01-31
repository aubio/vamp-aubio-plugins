/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*-  vi:set ts=8 sts=4 sw=4: */

/*
    Vamp feature extraction plugins using Paul Brossier's Aubio library.

    Centre for Digital Music, Queen Mary, University of London.
    This file copyright 2006-2008 Chris Cannam and QMUL.
    
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
#include "Notes.h"

#include <algorithm>

using std::string;
using std::vector;
using std::cerr;
using std::endl;

Notes::Notes(float inputSampleRate) :
    Plugin(inputSampleRate),
    m_ibuf(0),
    m_onset(0),
    m_pitch(0),
    m_onsetdet(0),
    m_onsettype(OnsetComplex),
    m_pitchdet(0),
    m_pitchtype(PitchYinFFT),
    m_threshold(0.3),
    m_silence(-70),
    m_minioi(4),
    m_median(6),
    m_minpitch(32),
    m_maxpitch(95),
    m_wrapRange(false),
    m_avoidLeaps(false),
    m_prevPitch(-1)
{
}

Notes::~Notes()
{
    if (m_onsetdet) del_aubio_onset(m_onsetdet);
    if (m_pitchdet) del_aubio_pitch(m_pitchdet);
    if (m_ibuf) del_fvec(m_ibuf);
    if (m_onset) del_fvec(m_onset);
    if (m_pitch) del_fvec(m_pitch);
}

string
Notes::getIdentifier() const
{
    return "aubionotes";
}

string
Notes::getName() const
{
    return "Aubio Note Tracker";
}

string
Notes::getDescription() const
{
    return "Estimate note onset positions, pitches and durations";
}

string
Notes::getMaker() const
{
    return "Paul Brossier (plugin by Chris Cannam)";
}

int
Notes::getPluginVersion() const
{
    return 4;
}

string
Notes::getCopyright() const
{
    return "GPL";
}

bool
Notes::initialise(size_t channels, size_t stepSize, size_t blockSize)
{
    if (channels != 1) {
        std::cerr << "Notes::initialise: channels must be 1" << std::endl;
        return false;
    }

    m_stepSize = stepSize;
    m_blockSize = blockSize;

    m_ibuf = new_fvec(stepSize);
    m_onset = new_fvec(1);
    m_pitch = new_fvec(1);

    reset();

    return true;
}

void
Notes::reset()
{
    if (m_onsetdet) del_aubio_onset(m_onsetdet);
    if (m_pitchdet) del_aubio_pitch(m_pitchdet);

    m_onsetdet = new_aubio_onset
        (const_cast<char *>(getAubioNameForOnsetType(m_onsettype)),
         m_blockSize,
         m_stepSize,
         lrintf(m_inputSampleRate));
    
    aubio_onset_set_threshold(m_onsetdet, m_threshold);
    aubio_onset_set_silence(m_onsetdet, m_silence);
    aubio_onset_set_minioi(m_onsetdet, m_minioi);

    m_pitchdet = new_aubio_pitch
        (const_cast<char *>(getAubioNameForPitchType(m_pitchtype)),
         m_blockSize,
         m_stepSize,
         lrintf(m_inputSampleRate));

    aubio_pitch_set_unit(m_pitchdet, const_cast<char *>("freq"));

    m_count = 0;
    m_delay = Vamp::RealTime::frame2RealTime((4 + m_median) * m_stepSize,
                                       lrintf(m_inputSampleRate));
    m_currentOnset = Vamp::RealTime::zeroTime;
    m_haveCurrent = false;
    m_prevPitch = -1;
}

size_t
Notes::getPreferredStepSize() const
{
    return 512;
}

size_t
Notes::getPreferredBlockSize() const
{
    return 4 * getPreferredStepSize();
}

Notes::ParameterList
Notes::getParameterDescriptors() const
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
    desc.identifier = "minpitch";
    desc.name = "Minimum Pitch";
    desc.description = "Lowest pitch value to look for";
    desc.minValue = 0;
    desc.maxValue = 127;
    desc.defaultValue = 32;
    desc.unit = "MIDI units";
    desc.isQuantized = true;
    desc.quantizeStep = 1;
    list.push_back(desc);

    desc = ParameterDescriptor();
    desc.identifier = "maxpitch";
    desc.name = "Maximum Pitch";
    desc.description = "Highest pitch value to look for";
    desc.minValue = 0;
    desc.maxValue = 127;
    desc.defaultValue = 95;
    desc.unit = "MIDI units";
    desc.isQuantized = true;
    desc.quantizeStep = 1;
    list.push_back(desc);

    desc = ParameterDescriptor();
    desc.identifier = "wraprange";
    desc.name = "Fold Higher or Lower Notes into Range";
    desc.description = "Notes detected outside the range will be transposed to higher or lower octaves";
    desc.minValue = 0;
    desc.maxValue = 1;
    desc.defaultValue = 0;
    desc.isQuantized = true;
    desc.quantizeStep = 1;
    list.push_back(desc);

    desc = ParameterDescriptor();
    desc.identifier = "avoidleaps";
    desc.name = "Avoid Multi-Octave Jumps";
    desc.description = "Minimize octave jumps by transposing to the octave of the previously detected note";
    desc.minValue = 0;
    desc.maxValue = 1;
    desc.defaultValue = 0;
    desc.isQuantized = true;
    desc.quantizeStep = 1;
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
Notes::getParameter(std::string param) const
{
    if (param == "onsettype") {
        return m_onsettype;
    } else if (param == "pitchtype") {
        return m_pitchtype;
    } else if (param == "peakpickthreshold") {
        return m_threshold;
    } else if (param == "silencethreshold") {
        return m_silence;
    } else if (param == "minpitch") {
        return m_minpitch;
    } else if (param == "maxpitch") {
        return m_maxpitch;
    } else if (param == "wraprange") {
        return m_wrapRange ? 1.0 : 0.0;
    } else if (param == "avoidleaps") {
        return m_avoidLeaps ? 1.0 : 0.0;
    } else if (param == "minioi") {
        return m_minioi;
    } else {
        return 0.0;
    }
}

void
Notes::setParameter(std::string param, float value)
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
    } else if (param == "pitchtype") {
        switch (lrintf(value)) {
        case 0: m_pitchtype = PitchYin; break;
        case 1: m_pitchtype = PitchMComb; break;
        case 2: m_pitchtype = PitchSchmitt; break;
        case 3: m_pitchtype = PitchFComb; break;
        case 4: m_pitchtype = PitchYinFFT; break;
        }
    } else if (param == "peakpickthreshold") {
        m_threshold = value;
    } else if (param == "silencethreshold") {
        m_silence = value;
    } else if (param == "minpitch") {
        m_minpitch = lrintf(value);
    } else if (param == "maxpitch") {
        m_maxpitch = lrintf(value);
    } else if (param == "wraprange") {
        m_wrapRange = (value > 0.5);
    } else if (param == "avoidleaps") {
        m_avoidLeaps = (value > 0.5);
    } else if (param == "minioi") {
        m_minioi = value;
    }
}

Notes::OutputList
Notes::getOutputDescriptors() const
{
    OutputList list;

    OutputDescriptor d;
    d.identifier = "notes";
    d.name = "Notes";
    d.description = "List of notes detected, with their frequency and velocity";
    d.unit = "Hz";
    d.hasFixedBinCount = true;

    d.binCount = 2;
    d.binNames.push_back("Frequency");
    d.binNames.push_back("Velocity");
    d.hasDuration = true;

    d.hasKnownExtents = false;
    d.isQuantized = false;
    d.sampleType = OutputDescriptor::VariableSampleRate;
    d.sampleRate = 0;
    list.push_back(d);

    return list;
}

Notes::FeatureSet
Notes::process(const float *const *inputBuffers, Vamp::RealTime timestamp)
{
    for (size_t i = 0; i < m_stepSize; ++i) {
        fvec_set_sample(m_ibuf, inputBuffers[0][i], i);
    }

    aubio_onset_do(m_onsetdet, m_ibuf, m_onset);
    aubio_pitch_do(m_pitchdet, m_ibuf, m_pitch);

    bool isonset = m_onset->data[0];
    float frequency = m_pitch->data[0];

    m_notebuf.push_back(frequency);
    if (m_notebuf.size() > m_median) m_notebuf.pop_front();

    float level = aubio_level_detection(m_ibuf, m_silence);

    FeatureSet returnFeatures;

    if (isonset) {
        if (level == 1.) {
            isonset = false;
            m_count = 0;
            if (m_haveCurrent) pushNote(returnFeatures, timestamp);
        } else {
            m_count = 1;
        }
    } else {
        if (m_count > 0) ++m_count;
        if (m_count == m_median) {
            if (m_haveCurrent) pushNote(returnFeatures, timestamp);
            m_currentOnset = timestamp;
            m_currentLevel = level;
            m_haveCurrent = true;
        }
    }

    m_lastTimeStamp = timestamp;
    return returnFeatures;
}

Notes::FeatureSet
Notes::getRemainingFeatures()
{
    FeatureSet returnFeatures;
    if (m_haveCurrent) pushNote(returnFeatures, m_lastTimeStamp);
    return returnFeatures;
}

void
Notes::pushNote(FeatureSet &fs, const Vamp::RealTime &offTime)
{
    std::deque<float> toSort = m_notebuf;
    std::sort(toSort.begin(), toSort.end());
    float median = toSort[toSort.size()/2];
    if (median < 45.0) return;

    float freq = median;
    int midiPitch = (int)floor(aubio_freqtomidi(freq) + 0.5);
    
    if (m_avoidLeaps) {
        if (m_prevPitch >= 0) {
            while (midiPitch < m_prevPitch - 12) {
                midiPitch += 12;
                freq *= 2;
            }
            while (midiPitch > m_prevPitch + 12) {
                midiPitch -= 12;
                freq /= 2;
            }
        }
    }

    while (midiPitch < m_minpitch) {
        if (!m_wrapRange) return;
        midiPitch += 12;
        freq *= 2;
    }

    while (midiPitch > m_maxpitch) {
        if (!m_wrapRange) return;
        midiPitch -= 12;
        freq /= 2;
    }

    m_prevPitch = midiPitch;

    Feature feature;
    feature.hasTimestamp = true;
    if (m_currentOnset < m_delay) m_currentOnset = m_delay;
    feature.timestamp = m_currentOnset - m_delay;
    feature.values.push_back(freq);

    feature.values.push_back
        (Vamp::RealTime::realTime2Frame
         (offTime, lrintf(m_inputSampleRate)) -
         Vamp::RealTime::realTime2Frame
         (m_currentOnset, lrintf(m_inputSampleRate)));
    feature.hasDuration = false;

    feature.values.push_back(m_currentLevel);
    fs[0].push_back(feature);
}
    
