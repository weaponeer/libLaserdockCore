/**
    libLaserdockCore
    Copyright(c) 2018 Wicked Lasers

    This file is part of libLaserdockCore.

    libLaserdockCore is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    libLaserdockCore is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with libLaserdockCore.  If not, see <https://www.gnu.org/licenses/>.
**/

#include "ldCore/Visualizations/MusicManager/ldMusicManager.h"

#include <QtCore/QDebug>

#include <ldCore/Sound/ldSoundInterface.h>

#include "ldCore/Helpers/Audio/ldAppakBpmSelector.h"
#include "ldCore/Helpers/Audio/ldAppakPeaks.h"
#include "ldCore/Helpers/Audio/ldAppakSpectrum.h"
#include "ldCore/Helpers/Audio/ldBpmBeatDetector.h"
#include "ldCore/Helpers/Audio/ldHybridReactor.h"
#include "ldCore/Helpers/Audio/ldManualBpm.h"
#include "ldCore/Helpers/Audio/ldTempoAC.h"
#include "ldCore/Helpers/Audio/ldTempoTracker.h"

// initial state
ldMusicManager::ldMusicManager(QObject* parent)
    : QObject(parent)
    , m_manualBpm(new ldManualBpm(this))
{
    qDebug() << __FUNCTION__;

    // soundgate
    if (isFakeSound) qDebug() << "WARNING!!! ldMusicManager isFakeSound is true! ";
    
    // spect
    // TODO: fix memory leak in spectFrame
    spectrogram.reset(new ldSpectrogram);
    spectrogram2.reset(new ldSpectrogram);

    spectAdvanced.reset(new ldSpectAdvanced(0.808f));
    spectAdvanced2.reset(new ldSpectAdvanced(0.303f));

    // tempo AC
    tempoACSlower.reset(new ldTempoAC(80/2.0/2.0, 5.00, 0.25, false));
    tempoACSlow.reset(new ldTempoAC(80/2.0/1.0, 4.00, 0.25, false));
    tempoACFast.reset(new ldTempoAC(90*1.3*1.0, 0.25, 0.75, false));
    tempoACFaster.reset(new ldTempoAC(90*1.3f*1.5f, 0.20f, 0.75f, false));


    // style detectors
    musicFeature1.reset(new MusicFeature1());

    m_tempoTrackerFast.reset(new ldTempoTracker());
    m_tempoTrackerSlow.reset(new ldTempoTracker(false, true));

    // music reactor
    mrSlowBass.reset(new ldMusicReactor());
    mrFastBass.reset(new ldMusicReactor());
    mrSlowTreb.reset(new ldMusicReactor());
    mrFastTreb.reset(new ldMusicReactor());
    mrVolume.reset(new ldMusicReactor());
    //        p2[MRPARAMS] = {0.00f,0.00f,0.20f,0.00f,0.20f,0.83f,1.00f,0.63f,0.20f,0.49f,0.40f,0.09f,1.00f,0.75f,0.80f,0.62f,0.52f,0.09f,0.98f,0.00f};
    float   t_p2[MRPARAMS] = {0.50f,0.00f,0.20f,0.00f,0.20f,0.83f,1.00f,0.63f,0.20f,0.49f,0.40f,0.09f,0.98f,0.75f,1.00f,0.58f,0.50f,0.25f,0.50f,0.25f};

    //        fa[MRPARAMS] = {0.51f,0.00f,0.22f,0.00f,0.20f,1.00f,1.00f,0.26f,0.39f,0.21f,0.42f,0.66f,1.00f,0.77f,0.80f,0.48f,0.47f,0.46f,0.98f,0.00f};
    float   t_fa[MRPARAMS] = {0.75f,0.00f,0.34f,0.31f,0.14f,1.00f,1.00f,0.25f,0.56f,0.14f,0.40f,0.66f,0.98f,0.60f,0.75f,0.58f,0.37f,0.25f,1.00f,0.25f};

    //      t1fa[MRPARAMS] = {0.50f,0.79f,0.99f,0.26f,0.00f,0.21f,0.75f,0.49f,0.20f,0.25f,0.12f,0.26f,0.27f,0.85f,0.85f,0.25f,0.25f,0.25f,0.25f,0.25f};
    float t_t1fa[MRPARAMS] = {0.50f,0.79f,0.99f,0.26f,0.00f,0.21f,0.75f,0.49f,0.20f,0.25f,0.12f,0.26f,0.98f,0.40f,0.60f,0.58f,0.50f,0.25f,0.25f,0.25f};

    //       tp0[MRPARAMS] = {0.60f,0.00f,0.00f,0.75f,0.00f,0.50f,0.00f,0.27f,0.55f,0.15f,0.40f,0.15f,1.00f,1.00f,1.00f,0.46f,0.46f,0.46f,1.00f,1.00f};
    float  t_tp0[MRPARAMS] = {0.60f,0.00f,0.00f,0.75f,0.00f,0.50f,0.00f,0.27f,0.55f,0.15f,0.40f,0.15f,0.98f,0.20f,0.40f,0.58f,0.37f,0.25f,0.75f,0.25f};
    float pv[MRPARAMS] = {0.50f,0.36f,1.00f,0.25f,0.50f,1.00f,0.00f,0.93f,0.81f,0.05f,0.63f,0.18f,1.00f,0.75f,0.80f,0.46f,0.46f,0.46f,0.98f,0.00f};
    

    mrSlowBass->setParams(t_p2);
    mrFastBass->setParams(t_fa);
    mrSlowTreb->setParams(t_t1fa);
    mrFastTreb->setParams(t_tp0);
    mrVolume->setParams(pv);

    /*
    // tweak for real time
    mrSlowBass->params[19] = 0;
    mrFastBass->params[19] = 0;
    mrSlowTreb->params[19] = 0;
    mrFastTreb->params[19] = 0;
    mrVolume->params[19] = 0;*/  // no

    // appaka
    appakaBeat.reset(new ldAppakaBeat());
    m_peaks.reset(new ldAppakPeaks());
    appakaGate.reset(new ldAppakGate());
    appakaSpectrum.reset(new ldAppakSpectrum());
    appakaBpmSelector.reset(new ldAppakBpmSelector());

    // audioBasic
    audioBasic.reset(new ldAudioBasic);
    
    // soundGate
    soundGate.reset(new ldSoundGate);
    silentThree.reset(new ldSilentThree);

    // drop detectors
    beatWarm.reset(new ldBeatWarm);
    beatFresh.reset(new ldBeatFresh);

    // pitch
//    pitchTracker = new PitchTracker();
    
    // hybrid
    hybridAnima.reset(new ldHybridAnima);
    hybridFlash.reset(new ldHybridFlash);
    hybridAutoColor2.reset(new ldHybridAutoColor2);
    hybridColorPalette.reset(new ldHybridColorPalette);

    m_bpmBeatDetector.reset(new ldBpmBeatDetector());
    m_bpmBeatDetector->setDuration(0.7f);
}


ldMusicManager::~ldMusicManager() {}

// process all algorithms
void ldMusicManager::updateWith(std::shared_ptr<ldSoundData> psd, float delta) {

    // first measurements
    m_psd = psd;
    audioBasic->process(psd.get());

    // basic music reactors
    mrVolume->process(psd.get());
    mrSlowBass->process(psd.get());
    mrFastBass->process(psd.get());
    mrSlowTreb->process(psd.get());
    mrFastTreb->process(psd.get());

    // Appak
    appakaBeat->process(psd.get());
    m_peaks->process(psd.get());
    appakaSpectrum->process(psd.get());

    // sound gate and silent
    soundGate->process(psd.get());
    silentThree->process(psd.get());
    appakaGate->basicMono(audioBasic->mono);

#ifndef LD_CORE_REDUCE_ANALYZER_SUPPORT
    // ******************
    // spectrum and spectrogram processing
    spectFrame.update(psd.get());
    spectrogram->addFrame(spectFrame);
    {
        ldSpectrumFrame t = spectrogram->getNoisedFrame(10, sqrtf(2.f), sqrtf(2.f), 1, 0);
        spectrogram2->addFrame(t);
    }
    spectFrame = spectrogram->currentFrame();
    //*spectFrame = spectrogram2->currentFrame;

    // spect advanced
    spectAdvanced->update(spectrogram2->currentFrame());
    spectAdvanced2->update(spectrogram2->currentFrame());

    // autocorrelative tempo processing
    spectrogram2->calculateS();
    bool ismusic = ((isSilent()?0:1) + (isSilent2()?0:1) + (silentThree?0:1)) > 0;
    tempoACSlower->update(spectrogram2.get(), ismusic, delta);
    tempoACSlow->update(spectrogram2.get(), ismusic, delta);
    tempoACFast->update(spectrogram2.get(), ismusic, delta);
    tempoACFaster->update(spectrogram2.get(), ismusic, delta);

    // aubio library tempo trackers
    // test fix
    m_tempoTrackerFast->process(psd.get(), delta);
    m_tempoTrackerSlow->process(psd.get(), delta);

    // feature metaprocessing
    musicFeature1->update(spectFrame, psd.get());

    // *****************
    // drop detectors
    float ff4 = 0;
    ff4 += (sqrt(tempoACFast->phaseSmooth) + sqrt(tempoACSlow->phaseSmooth))/2;
    ff4 += MAX(mrFastBass->statOutput, mrFastTreb->statOutput);
    ff4 += MAX(mrSlowBass->statOutput, mrSlowTreb->statOutput);
    ff4 /= 3;
    ff4 = ff4*ff4*ff4;
    onsetLargeBeat1 = ff4;

    onsetLargeBeat2 = spectAdvanced2->peak2 - 1;
    onsetLargeBeat2 /= 2.717f;
    onsetLargeBeat2 = powf(onsetLargeBeat2, 1.0/2.0);
    clampfp(onsetLargeBeat2, 0, 1);

    beatWarm->update(spectAdvanced2.get(), spectAdvanced2.get(), tempoACFast->freqSmooth*1, tempoACSlow->freqSmooth*1);
    onsetBeatWarm = beatWarm->output;
    clampfp(onsetBeatWarm, 0, 1);

    beatFresh->update(spectrogram2->currentFrame().sls, static_cast<int>((1/tempoACSlow->freqSmooth)/4));
    onsetBeatFresh = beatFresh->outputVar / 12;
    onsetBeatFresh = powf(onsetBeatFresh, 1/2.414f);
    clampfp(onsetBeatFresh, 0, 1);

    //if (onsetLargeBeat2 > 0.125) {
        //if (onsetBeatFresh > 0.0125 || onsetBeatWarm > 0.0125) {
            //wfr = onsetBeatFresh / (onsetBeatFresh + onsetBeatWarm);
            wfr = 0.5f + 2.f*(onsetLargeBeat2+1)*(onsetBeatFresh - onsetBeatWarm);
            dsewfr.rho = 0.212f/(1+onsetLargeBeat2);
            //extern bool ABTEST;
            //if (ABTEST) dsewfr.rho = 0.0202/(1+onsetLargeBeat2);
            dsewfr.add(wfr);
            //wfr = dsewfr.mean;
        //}
    //}

    // pitch trackers
    //pitchTracker->update(psd.get(), onsetLargeBeat1, (onsetLargeBeat1 > 0.30), tempoACSlow->freqSmooth);
#endif

    // hybrid algos
    hybridAnima->process(this, delta);
    hybridFlash->process(this);
    hybridAutoColor2->process(this);
    hybridColorPalette->process(this);

    // drop detector
    float dropDetectValue = 0;

#ifndef LD_CORE_REDUCE_ANALYZER_SUPPORT
    {
        float tf2 = (0.5f-mrSlowBass->walkerOutput);
        tf2 = tf2*tf2 / 0.25f;
        float tf3 = (0.5f-mrFastBass->walkerOutput);
        tf3 = tf3*tf3 / 0.25f;

        float dropDetectValue2;
        dropDetectValue2 = 0;
        dropDetectValue2 += clampf(mrSlowBass->output * 1.25f - 0.25f, 0, 1);
        dropDetectValue2 += clampf(mrSlowTreb->output * 1.25f - 0.25f, 0, 1);
        dropDetectValue2 += tf2 / 2;
        dropDetectValue2 += tf3 / 2;
        dropDetectValue2 *= 1.0f/3.0f;
        clampfp(dropDetectValue2, 0, 1);
        dropDetectValue2 *= clampf(soundLevel() / 100.0f, 0.125f, 0.5f) / 0.5f;
        clampfp(dropDetectValue2, 0, 1);

        dropDetectValue = dropDetectValue2;
    }
#else
    {
        float dropDetectValue3;
        dropDetectValue3 = 0;
        float f = 0.20f;
        dropDetectValue3 += clampf((onsetBeatFresh - 0.88f + f) * 3, 0, 1);
        dropDetectValue3 += clampf((onsetBeatWarm - 0.84f + f) * 3, 0, 1);
        dropDetectValue3 += clampf((onsetLargeBeat2 - 0.92f + f) * 3, 0, 1);
        dropDetectValue3 += clampf((onsetBeatFresh - 0.83f + f) * 2, 0, 1);
        dropDetectValue3 += clampf((onsetBeatWarm - 0.74f + f) * 2, 0, 1);
        dropDetectValue3 += clampf((onsetLargeBeat2 - 0.85f + f) * 2, 0, 1);
        dropDetectValue3 /= 5;
        clampfp(dropDetectValue3, 0, 1);
        dropDetectValue3 = powf(dropDetectValue3, 1.0f/3.0f);
        clampfp(dropDetectValue3, 0, 1);
        dropDetectValue = dropDetectValue3;
    }
#endif
    // apply drop detect
    {
        bool change = false;
        if (m_dropDetectorLockout > 0) {
            m_dropDetectorLockout -= delta;
        } else if (m_dropDetectorEnabled) {
            if (dropDetectValue > (1.1f - m_dropDetectorSens)) change = true;
            if (change) {
                float lockout = 10;
                float bpm = 120; // todo
                float secondsPerBeat = 60.0f / bpm;
                lockout = 3.75f * secondsPerBeat;
                clampfp(lockout, 0.5f, 4.0f);
                m_dropDetectorLockout = lockout;
                qDebug() << "drop detected, value " << int(dropDetectValue*100) << " cooldown is " << m_dropDetectorLockout;
            }
            if (change) emit dropDetected();
        }
    }

    // appak bpm selector
    appakaBpmSelector->process(m_tempoTrackerFast->bpm(), appakaBeat->bpm, m_peaks->lastBpmApproximation());

    m_bpmBeatDetector->process(appakaBpmSelector->bestBpm(), m_peaks->output(), delta);
    emit updated();
}

float ldMusicManager::bass() const
{
    return m_psd ? m_psd->GetBass() : 0;
}

float ldMusicManager::mids() const
{
    return m_psd ? m_psd->GetMids() : 0;
}

float ldMusicManager::high() const
{
    return m_psd ? m_psd->GetHigh() : 0;
}

float ldMusicManager::volumePowerPre() const
{
    return m_psd ? m_psd->volumePowerPre : 0;
}

float ldMusicManager::volumePowerPost() const
{
    return m_psd ? m_psd->volumePowerPost : 0;
}

int ldMusicManager::soundLevel() const
{
    return m_psd ? m_psd->GetSoundLevel() : 0;
}

void ldMusicManager::setRealSoundLevel(int value)
{
    m_realSoundLevel = value;
}

int ldMusicManager::realSoundLevel() const
{
    return m_realSoundLevel;
}

const ldAppakPeaks *ldMusicManager::peaks() const
{
    return m_peaks.get();
}

ldManualBpm *ldMusicManager::manualBpm() const
{
    return m_manualBpm;
}

const ldTempoTracker *ldMusicManager::tempoTrackerFast() const
{
    return m_tempoTrackerFast.get();
}

const ldTempoTracker *ldMusicManager::tempoTrackerSlow() const
{
    return m_tempoTrackerSlow.get();
}

const ldBpmBeatDetector *ldMusicManager::bpmBeatDetector() const
{
    return m_bpmBeatDetector.get();
}

bool ldMusicManager::isSilent() const
{
    if(isFakeSound)
        return false;

    return soundGate->isSilent;
}

bool ldMusicManager::isSilent2() const
{
    if(isFakeSound)
        return false;

    return mrVolume->isSilent2;
}

float ldMusicManager::isSilent2float() const
{
    if(isFakeSound)
        return 0;

    return mrVolume->isSilent2float;
}

bool ldMusicManager::isSilent3() const
{
    if(isFakeSound)
        return false;

    return silentThree->isSilent;
}

float ldMusicManager::bestBpm() const
{
    if(m_manualBpm->isEnabled())
        return m_manualBpm->bpm();
    else
        return appakaBpmSelector->bestBpm();
}

float ldMusicManager::slowBpm() const
{
    if(m_manualBpm->isEnabled())
        return m_manualBpm->bpm();
    else
        return m_tempoTrackerSlow->bpm();
}

void ldMusicManager::setDropDetectEnabledValue(bool value) { m_dropDetectorEnabled = value; }
void ldMusicManager::setDropDetectSensValue(int value) { m_dropDetectorSens = clampf(value / 100.0f, 0, 1);}
