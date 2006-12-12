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
#include "Tempo.h"

using std::string;
using std::vector;
using std::cerr;
using std::endl;

//#define HAVE_AUBIO_LOCKED_TEMPO_HACK

Tempo::Tempo(float inputSampleRate) :
    Plugin(inputSampleRate),
    m_ibuf(0),
    m_fftgrain(0),
    m_onset(0),
    m_pv(0),
    m_peakpick(0),
    m_onsetdet(0),
    m_onsettype(aubio_onset_specdiff),
    m_beattracking(0),
    m_dfframe(0),
    m_btout(0),
    m_btcounter(0),
    m_threshold(0.3),
    m_silence(-90),
    m_channelCount(1)
{
}

Tempo::~Tempo()
{
    if (m_onsetdet) aubio_onsetdetection_free(m_onsetdet);
    if (m_ibuf) del_fvec(m_ibuf);
    if (m_onset) del_fvec(m_onset);
    if (m_fftgrain) del_cvec(m_fftgrain);
    if (m_pv) del_aubio_pvoc(m_pv);
    if (m_peakpick) del_aubio_peakpicker(m_peakpick);
    if (m_beattracking) del_aubio_beattracking(m_beattracking);
    if (m_dfframe) del_fvec(m_dfframe);
    if (m_btout) del_fvec(m_btout);
}

string
Tempo::getName() const
{
    return "aubiotempo";
}

string
Tempo::getDescription() const
{
    return "Aubio Tempo Detector";
}

string
Tempo::getMaker() const
{
    return "Paul Brossier (plugin by Chris Cannam)";
}

int
Tempo::getPluginVersion() const
{
    return 1;
}

string
Tempo::getCopyright() const
{
    return "GPL";
}

bool
Tempo::initialise(size_t channels, size_t stepSize, size_t blockSize)
{
    m_channelCount = channels;
    m_stepSize = stepSize;
    m_blockSize = blockSize;

    m_ibuf = new_fvec(stepSize, channels);
    m_onset = new_fvec(1, channels);
    m_fftgrain = new_cvec(blockSize, channels);
    m_pv = new_aubio_pvoc(blockSize, stepSize, channels);
    m_peakpick = new_aubio_peakpicker(m_threshold);

    m_onsetdet = new_aubio_onsetdetection(m_onsettype, blockSize, channels);
    
    m_delay = Vamp::RealTime::frame2RealTime(3 * stepSize,
                                             lrintf(m_inputSampleRate));

    m_lastBeat = Vamp::RealTime::zeroTime - m_delay - m_delay;

    m_winlen = 512*512/stepSize;
    m_dfframe = new_fvec(m_winlen,channels);
    m_btstep = m_winlen/4;
    m_btout = new_fvec(m_btstep,channels);
    m_beattracking = new_aubio_beattracking(m_winlen,channels);

    return true;
}

void
Tempo::reset()
{
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
        case 0: m_onsettype = aubio_onset_energy; break;
        case 1: m_onsettype = aubio_onset_specdiff; break;
        case 2: m_onsettype = aubio_onset_hfc; break;
        case 3: m_onsettype = aubio_onset_complex; break;
        case 4: m_onsettype = aubio_onset_phase; break;
        case 5: m_onsettype = aubio_onset_kl; break;
        case 6: m_onsettype = aubio_onset_mkl; break;
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
    d.name = "beats";
    d.unit = "";
    d.description = "Beats";
    d.hasFixedBinCount = true;
    d.binCount = 0;
    d.sampleType = OutputDescriptor::VariableSampleRate;
    d.sampleRate = 0;
    list.push_back(d);

#ifdef HAVE_AUBIO_LOCKED_TEMPO_HACK
    d.name = "tempo";
    d.unit = "bpm";
    d.description = "Tempo";
    d.hasFixedBinCount = true;
    d.binCount = 1;
    d.hasKnownExtents = false;
    d.isQuantized = false;
    d.sampleType = OutputDescriptor::OneSamplePerStep;
    list.push_back(d);
#endif

    return list;
}

Tempo::FeatureSet
Tempo::process(const float *const *inputBuffers, Vamp::RealTime timestamp)
{
    for (size_t i = 0; i < m_stepSize; ++i) {
        for (size_t j = 0; j < m_channelCount; ++j) {
            fvec_write_sample(m_ibuf, inputBuffers[j][i], j, i);
        }
    }

    aubio_pvoc_do(m_pv, m_ibuf, m_fftgrain);
    aubio_onsetdetection(m_onsetdet, m_fftgrain, m_onset);

#ifdef HAVE_AUBIO_LOCKED_TEMPO_HACK
    float locked_tempo = 0;
#endif

    if ( m_btcounter == m_btstep - 1 ) {
#ifdef HAVE_AUBIO_LOCKED_TEMPO_HACK
        aubio_beattracking_do(m_beattracking,m_dfframe,m_btout,&locked_tempo);
#else
        aubio_beattracking_do(m_beattracking,m_dfframe,m_btout);
#endif
        /* rotate dfframe */
        for (size_t i = 0 ; i < m_winlen - m_btstep; i++ ) 
                m_dfframe->data[0][i] = m_dfframe->data[0][i+m_btstep];
        for (size_t i = m_winlen - m_btstep ; i < m_winlen; i++ ) 
                m_dfframe->data[0][i] = 0.;
                
        m_btcounter = -1;
    }
    m_btcounter++;
    bool isonset = aubio_peakpick_pimrt_wt( m_onset, m_peakpick, 
        &(m_dfframe->data[0][m_winlen - m_btstep + m_btcounter]));
    bool istactus = 0;

    /* check if any of the predicted beat correspond to the current time */
    for (size_t i = 1; i < m_btout->data[0][0]; i++ ) { 
            if (m_btcounter == m_btout->data[0][i]) {
                    if (aubio_silence_detection(m_ibuf, m_silence)) {
                            isonset  = false;
                            istactus = false;
                    } else {
                            istactus = true;
                    }
            }
    }

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

#ifdef HAVE_AUBIO_LOCKED_TEMPO_HACK
    if (locked_tempo >= 30 && locked_tempo <= 206) {
        if (locked_tempo > 145) locked_tempo /= 2;
        std::cerr << "Locked tempo: " << locked_tempo << std::endl;
        Feature tempo;
        tempo.hasTimestamp = false;
        tempo.values.push_back(locked_tempo);
        returnFeatures[1].push_back(tempo);
    }
#endif

    return returnFeatures;
}

Tempo::FeatureSet
Tempo::getRemainingFeatures()
{
    return FeatureSet();
}

