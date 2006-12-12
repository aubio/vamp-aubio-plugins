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
#include "Notes.h"

using std::string;
using std::vector;
using std::cerr;
using std::endl;

Notes::Notes(float inputSampleRate) :
    Plugin(inputSampleRate),
    m_ibuf(0),
    m_fftgrain(0),
    m_onset(0),
    m_pv(0),
    m_peakpick(0),
    m_onsetdet(0),
    m_onsettype(aubio_onset_complex),
    m_pitchdet(0),
    m_pitchtype(aubio_pitch_yinfft),
    m_pitchmode(aubio_pitchm_freq),
    m_threshold(0.3),
    m_silence(-90),
    m_median(6),
    m_minpitch(27),
    m_maxpitch(95),
    m_wrapRange(false),
    m_avoidLeaps(false),
    m_prevPitch(-1)
{
}

Notes::~Notes()
{
    if (m_onsetdet) aubio_onsetdetection_free(m_onsetdet);
    if (m_pitchdet) del_aubio_pitchdetection(m_pitchdet);
    if (m_ibuf) del_fvec(m_ibuf);
    if (m_onset) del_fvec(m_onset);
    if (m_fftgrain) del_cvec(m_fftgrain);
    if (m_pv) del_aubio_pvoc(m_pv);
    if (m_peakpick) del_aubio_peakpicker(m_peakpick);
}

string
Notes::getName() const
{
    return "aubionotes";
}

string
Notes::getDescription() const
{
    return "Aubio Note Tracker";
}

string
Notes::getMaker() const
{
    return "Paul Brossier (plugin by Chris Cannam)";
}

int
Notes::getPluginVersion() const
{
    return 1;
}

string
Notes::getCopyright() const
{
    return "GPL";
}

bool
Notes::initialise(size_t channels, size_t stepSize, size_t blockSize)
{
    m_channelCount = channels;
    m_stepSize = stepSize;
    m_blockSize = blockSize;

    size_t processingBlockSize;
    if (m_onsettype == aubio_onset_energy ||
        m_onsettype == aubio_onset_hfc) {
        processingBlockSize = stepSize * 2;
    } else {
        processingBlockSize = stepSize * 4;
    }

    m_ibuf = new_fvec(stepSize, channels);
    m_onset = new_fvec(1, channels);
    m_fftgrain = new_cvec(processingBlockSize, channels);
    m_pv = new_aubio_pvoc(processingBlockSize, stepSize, channels);
    m_peakpick = new_aubio_peakpicker(m_threshold);

    m_onsetdet = new_aubio_onsetdetection(m_onsettype, processingBlockSize, channels);

    m_pitchdet = new_aubio_pitchdetection(processingBlockSize * 4,
                                          stepSize,
                                          channels,
                                          lrintf(m_inputSampleRate),
                                          m_pitchtype,
                                          m_pitchmode);

    m_count = 0;
    m_delay = Vamp::RealTime::frame2RealTime((4 + m_median) * m_stepSize,
                                       lrintf(m_inputSampleRate));
    m_currentOnset = Vamp::RealTime::zeroTime;
    m_haveCurrent = false;
    m_prevPitch = -1;

    return true;
}

void
Notes::reset()
{
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
    desc.name = "onsettype";
    desc.description = "Onset Detection Function Type";
    desc.minValue = 0;
    desc.maxValue = 6;
    desc.defaultValue = (int)aubio_onset_complex;
    desc.isQuantized = true;
    desc.quantizeStep = 1;
    desc.valueNames.push_back("Energy Based");
    desc.valueNames.push_back("Spectral Difference");
    desc.valueNames.push_back("High-Frequency Content");
    desc.valueNames.push_back("Complex Domain");
    desc.valueNames.push_back("Phase Deviation");
    desc.valueNames.push_back("Kullback-Liebler");
    desc.valueNames.push_back("Modified Kullback-Liebler");
    list.push_back(desc);

    desc = ParameterDescriptor();
    desc.name = "pitchtype";
    desc.description = "Pitch Detection Function Type";
    desc.minValue = 0;
    desc.maxValue = 4;
    desc.defaultValue = (int)aubio_pitch_yinfft;
    desc.isQuantized = true;
    desc.quantizeStep = 1;
    desc.valueNames.push_back("YIN Frequency Estimator");
    desc.valueNames.push_back("Spectral Comb");
    desc.valueNames.push_back("Schmitt");
    desc.valueNames.push_back("Fast Harmonic Comb");
    desc.valueNames.push_back("YIN with FFT");
    list.push_back(desc);

    desc = ParameterDescriptor();
    desc.name = "minpitch";
    desc.description = "Minimum Pitch";
    desc.minValue = 0;
    desc.maxValue = 127;
    desc.defaultValue = 32;
    desc.unit = "MIDI units";
    desc.isQuantized = true;
    desc.quantizeStep = 1;
    list.push_back(desc);

    desc = ParameterDescriptor();
    desc.name = "maxpitch";
    desc.description = "Maximum Pitch";
    desc.minValue = 0;
    desc.maxValue = 127;
    desc.defaultValue = 95;
    desc.unit = "MIDI units";
    desc.isQuantized = true;
    desc.quantizeStep = 1;
    list.push_back(desc);

    desc = ParameterDescriptor();
    desc.name = "wraprange";
    desc.description = "Fold Higher or Lower Notes into Range";
    desc.minValue = 0;
    desc.maxValue = 1;
    desc.defaultValue = 0;
    desc.isQuantized = true;
    desc.quantizeStep = 1;
    list.push_back(desc);

    desc = ParameterDescriptor();
    desc.name = "avoidleaps";
    desc.description = "Avoid Multi-Octave Jumps";
    desc.minValue = 0;
    desc.maxValue = 1;
    desc.defaultValue = 0;
    desc.isQuantized = true;
    desc.quantizeStep = 1;
    list.push_back(desc);

    desc = ParameterDescriptor();
    desc.name = "peakpickthreshold";
    desc.description = "Peak Picker Threshold";
    desc.minValue = 0;
    desc.maxValue = 1;
    desc.defaultValue = 0.3;
    desc.isQuantized = false;
    list.push_back(desc);

    desc = ParameterDescriptor();
    desc.name = "silencethreshold";
    desc.description = "Silence Threshold";
    desc.minValue = -120;
    desc.maxValue = 0;
    desc.defaultValue = -90;
    desc.unit = "dB";
    desc.isQuantized = false;
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
    } else {
        return 0.0;
    }
}

void
Notes::setParameter(std::string param, float value)
{
    if (param == "onsettype") {
        switch (lrintf(value)) {
        case 0: m_onsettype = aubio_onset_energy; break;
        case 1: m_onsettype = aubio_onset_specdiff; break;
        case 2: m_onsettype = aubio_onset_hfc; break;
        case 3: m_onsettype = aubio_onset_complex; break;
        case 4: m_onsettype = aubio_onset_phase; break;
        case 5: m_onsettype = aubio_onset_kl; break;
        case 6: m_onsettype = aubio_onset_mkl; break;
        }
    } else if (param == "pitchtype") {
        switch (lrintf(value)) {
        case 0: m_pitchtype = aubio_pitch_yin; break;
        case 1: m_pitchtype = aubio_pitch_mcomb; break;
        case 2: m_pitchtype = aubio_pitch_schmitt; break;
        case 3: m_pitchtype = aubio_pitch_fcomb; break;
        case 4: m_pitchtype = aubio_pitch_yinfft; break;
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
    }
}

Notes::OutputList
Notes::getOutputDescriptors() const
{
    OutputList list;

    OutputDescriptor d;
    d.name = "notes";
    d.unit = "Hz";
    d.description = "Notes";
    d.hasFixedBinCount = true;
    d.binCount = 2;
    d.binNames.push_back("Frequency");
    d.binNames.push_back("Duration");
    d.binNames.push_back("Velocity");
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
        for (size_t j = 0; j < m_channelCount; ++j) {
            fvec_write_sample(m_ibuf, inputBuffers[j][i], j, i);
        }
    }

    aubio_pvoc_do(m_pv, m_ibuf, m_fftgrain);
    aubio_onsetdetection(m_onsetdet, m_fftgrain, m_onset);

    bool isonset = aubio_peakpick_pimrt(m_onset, m_peakpick);

    float frequency = aubio_pitchdetection(m_pitchdet, m_ibuf);

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
    int midiPitch = (int)FLOOR(aubio_freqtomidi(freq) + 0.5);
    
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
        (Vamp::RealTime::realTime2Frame(offTime, lrintf(m_inputSampleRate)) -
         Vamp::RealTime::realTime2Frame(m_currentOnset, lrintf(m_inputSampleRate)));
    feature.values.push_back(m_currentLevel);
    fs[0].push_back(feature);
}
    
