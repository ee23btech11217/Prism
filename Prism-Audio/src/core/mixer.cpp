// #include "libs/mixer.hpp"
// #include <algorithm>
// #include <cmath>
// #include <iostream>
// #include <vector>
// #include <numeric>

// namespace Audio
// {
//     // RMS window size for AGC calculations (in samples)
//     constexpr size_t AGC_WINDOW_SIZE = 1024;
    
//     // AGC parameters
//     constexpr float AGC_TARGET_LEVEL = 0.707f;  // -3dB RMS target level
//     constexpr float AGC_ATTACK_TIME = 0.01f;    // 10ms attack time
//     constexpr float AGC_RELEASE_TIME = 0.5f;    // 500ms release time
//     constexpr float AGC_THRESHOLD = 0.1f;       // Threshold below which AGC is inactive
//     constexpr float AGC_MAX_GAIN = 10.0f;       // Maximum gain factor
//     constexpr float AGC_MIN_GAIN = 0.1f;        // Minimum gain factor
    
//     /**
//      * @brief Constructor for the mixer
//      * @param engine Reference to the parent engine
//      * @param channels Shared pointer to the vector of audio channels
//      */
//     Mixer::Mixer(Engine& engine, std::shared_ptr<std::vector<Channel>> channels) :
//         masterVolume(1.0f),
//         channels(channels),
//         parentEngine(&engine),
//         agcEnabled(true),
//         agcGain(1.0f),
//         limitThreshold(0.95f),
//         limitReleaseTime(0.05f),
//         currentLimit(1.0f),
//         sampleRate(44100)  // Default sample rate, should be updated by engine
//     {
//         // Initialize RMS history buffer with zeros
//         rmsHistory.resize(AGC_WINDOW_SIZE, 0.0f);
//         rmsHistoryIndex = 0;
        
//         // Initialize channel volume matrix (for surround mixing)
//         channelVolumes.resize(channels->size(), 1.0f);
        
//         // Initialize channel send matrix for auxiliary effects
//         channelSends.resize(channels->size());
//         for (auto& sends : channelSends) {
//             sends.resize(MAX_AUX_SENDS, 0.0f);
//         }
        
//         // Initialize auxiliary effect busses
//         for (int i = 0; i < MAX_AUX_SENDS; i++) {
//             auxEffects[i] = std::make_shared<ChainFX>();
//             auxReturns[i] = 1.0f;
//         }
        
//         // Create master limiter and equalizer
//         masterLimiter = std::make_shared<ChainFX>();
//         masterEqualizer = std::make_shared<ChainFX>();
        
//         // Create spectrum analyzer
//         spectrumAnalyzer.resize(SPECTRUM_BANDS, 0.0f);
        
//         // Initialize AGC coefficients based on sample rate
//         updateAGCCoefficients();
//     }
    
//     /**
//      * @brief Destructor for the mixer
//      */
//     Mixer::~Mixer()
//     {
//         // Cleanup is handled by smart pointers
//     }
    
//     /**
//      * @brief Updates AGC coefficients based on current sample rate
//      */
//     void Mixer::updateAGCCoefficients()
//     {
//         // Calculate attack and release coefficients
//         agcAttackCoef = std::exp(-1.0f / (AGC_ATTACK_TIME * sampleRate));
//         agcReleaseCoef = std::exp(-1.0f / (AGC_RELEASE_TIME * sampleRate));
//         limitReleaseCoef = std::exp(-1.0f / (limitReleaseTime * sampleRate));
//     }
    
//     /**
//      * @brief Set the sample rate for the mixer
//      * @param rate New sample rate in Hz
//      */
//     void Mixer::setSampleRate(uint32_t rate)
//     {
//         sampleRate = rate;
//         updateAGCCoefficients();
//     }
    
//     /**
//      * @brief Enable or disable automatic gain control
//      * @param enabled Whether AGC should be enabled
//      */
//     void Mixer::enableAGC(bool enabled)
//     {
//         agcEnabled = enabled;
//     }
    
//     /**
//      * @brief Set the master volume level
//      * @param volume Master volume (0.0 to 1.0)
//      */
//     void Mixer::setMasterVolume(float volume)
//     {
//         masterVolume = std::clamp(volume, 0.0f, 1.0f);
//     }
    
//     /**
//      * @brief Set the aux send level for a channel
//      * @param channelIdx Channel index
//      * @param auxIdx Aux send index
//      * @param level Send level (0.0 to 1.0)
//      */
//     void Mixer::setAuxSendLevel(size_t channelIdx, size_t auxIdx, float level)
//     {
//         if (channelIdx < channelSends.size() && auxIdx < MAX_AUX_SENDS) {
//             channelSends[channelIdx][auxIdx] = std::clamp(level, 0.0f, 1.0f);
//         }
//     }
    
//     /**
//      * @brief Set the aux return level
//      * @param auxIdx Aux return index
//      * @param level Return level (0.0 to 1.0)
//      */
//     void Mixer::setAuxReturnLevel(size_t auxIdx, float level)
//     {
//         if (auxIdx < MAX_AUX_SENDS) {
//             auxReturns[auxIdx] = std::clamp(level, 0.0f, 1.0f);
//         }
//     }
    
//     /**
//      * @brief Get a reference to an aux effect chain
//      * @param auxIdx Aux effect index
//      * @return Shared pointer to the ChainFX
//      */
//     std::shared_ptr<ChainFX> Mixer::getAuxEffectChain(size_t auxIdx)
//     {
//         if (auxIdx < MAX_AUX_SENDS) {
//             return auxEffects[auxIdx];
//         }
//         return nullptr;
//     }
    
//     /**
//      * @brief Get a reference to the master limiter
//      * @return Shared pointer to the ChainFX
//      */
//     std::shared_ptr<ChainFX> Mixer::getMasterLimiter()
//     {
//         return masterLimiter;
//     }
    
//     /**
//      * @brief Get a reference to the master equalizer
//      * @return Shared pointer to the ChainFX
//      */
//     std::shared_ptr<ChainFX> Mixer::getMasterEqualizer()
//     {
//         return masterEqualizer;
//     }
    
//     /**
//      * @brief Set the limiter threshold
//      * @param threshold Threshold level (0.0 to 1.0)
//      */
//     void Mixer::setLimiterThreshold(float threshold)
//     {
//         limitThreshold = std::clamp(threshold, 0.1f, 1.0f);
//     }
    
//     /**
//      * @brief Set the limiter release time
//      * @param releaseTime Release time in seconds
//      */
//     void Mixer::setLimiterReleaseTime(float releaseTime)
//     {
//         limitReleaseTime = std::clamp(releaseTime, 0.01f, 1.0f);
//         updateAGCCoefficients();
//     }
    
//     /**
//      * @brief Get the current spectrum analysis
//      * @return Reference to the spectrum analyzer data
//      */
//     const std::vector<float>& Mixer::getSpectrumAnalysis() const
//     {
//         return spectrumAnalyzer;
//     }
    
//     /**
//      * @brief Get the current RMS level
//      * @return The current RMS level of the mix
//      */
//     float Mixer::getCurrentRMSLevel() const
//     {
//         return currentRMS;
//     }
    
//     /**
//      * @brief Get the current peak level
//      * @return The current peak level of the mix
//      */
//     float Mixer::getCurrentPeakLevel() const
//     {
//         return currentPeak;
//     }
    
//     /**
//      * @brief Get the current AGC gain
//      * @return The current gain applied by the AGC
//      */
//     float Mixer::getCurrentAGCGain() const
//     {
//         return agcGain;
//     }
    
//     /**
//      * @brief Apply soft clipping to a sample
//      * @param sample The audio sample to process
//      * @return The soft-clipped sample
//      */
//     float Mixer::softClip(float sample)
//     {
//         // Soft clipping function based on arctangent
//         constexpr float softness = 0.6f;
//         if (std::abs(sample) <= 1.0f) {
//             return sample;
//         }
//         return std::copysign(1.0f, sample) * (1.0f + std::tanh((std::abs(sample) - 1.0f) * softness));
//     }
    
//     /**
//      * @brief Calculate RMS level of a buffer
//      * @param buffer The audio buffer to measure
//      * @param length The length of the buffer in samples
//      * @return The RMS level
//      */
//     float Mixer::calculateRMS(const float* buffer, size_t length)
//     {
//         float sumSquares = 0.0f;
//         for (size_t i = 0; i < length; ++i) {
//             sumSquares += buffer[i] * buffer[i];
//         }
//         return std::sqrt(sumSquares / length);
//     }
    
//     /**
//      * @brief Process and mix all channels
//      * @param outputBuffer The output buffer to fill
//      * @param numFrames The number of frames to process
//      */
//     void Mixer::mixChannels(float* outputBuffer, uint32_t numFrames)
//     {
//         // Clear the output buffer
//         std::fill(outputBuffer, outputBuffer + numFrames * 2, 0.0f);
        
//         // Clear aux send buffers
//         float auxBuffers[MAX_AUX_SENDS][2048]; // Assuming max frame count of 1024 (stereo)
//         for (int i = 0; i < MAX_AUX_SENDS; i++) {
//             std::fill(auxBuffers[i], auxBuffers[i] + numFrames * 2, 0.0f);
//         }
        
//         // Process all active channels
//         for (size_t i = 0; i < channels->size(); ++i) {
//             Channel& channel = (*channels)[i];
            
//             // Skip inactive channels
//             if (channel.state != Channel::State::PLAYING && 
//                 channel.state != Channel::State::STOPPING) {
//                 continue;
//             }
            
//             // Process audio for this channel
//             float channelBuffer[2048]; // Assuming max frame count of 1024 (stereo)
//             std::fill(channelBuffer, channelBuffer + numFrames * 2, 0.0f);
            
//             // Let the channel fill its buffer
//             channel.processAudio(channelBuffer, numFrames, sampleRate);
            
//             // Apply channel volume
//             float chanVolume = channelVolumes[i];
//             for (uint32_t j = 0; j < numFrames * 2; ++j) {
//                 channelBuffer[j] *= chanVolume;
//             }
            
//             // Add to main output buffer
//             for (uint32_t j = 0; j < numFrames * 2; ++j) {
//                 outputBuffer[j] += channelBuffer[j];
//             }
            
//             // Process aux sends
//             for (size_t auxIdx = 0; auxIdx < MAX_AUX_SENDS; ++auxIdx) {
//                 float sendLevel = channelSends[i][auxIdx];
//                 if (sendLevel > 0.0f) {
//                     for (uint32_t j = 0; j < numFrames * 2; ++j) {
//                         auxBuffers[auxIdx][j] += channelBuffer[j] * sendLevel;
//                     }
//                 }
//             }
//         }
        
//         // Process aux effect returns
//         for (size_t auxIdx = 0; auxIdx < MAX_AUX_SENDS; ++auxIdx) {
//             // Apply effects to aux buffer
//             for (uint32_t j = 0; j < numFrames; ++j) {
//                 float& left = auxBuffers[auxIdx][j * 2];
//                 float& right = auxBuffers[auxIdx][j * 2 + 1];
//                 auxEffects[auxIdx]->process(left, right);
//             }
            
//             // Mix aux return to main output
//             float returnLevel = auxReturns[auxIdx];
//             if (returnLevel > 0.0f) {
//                 for (uint32_t j = 0; j < numFrames * 2; ++j) {
//                     outputBuffer[j] += auxBuffers[auxIdx][j] * returnLevel;
//                 }
//             }
//         }
        
//         // Apply master equalizer
//         for (uint32_t i = 0; i < numFrames; ++i) {
//             float& left = outputBuffer[i * 2];
//             float& right = outputBuffer[i * 2 + 1];
//             masterEqualizer->process(left, right);
//         }
        
//         // Calculate current mix RMS and peak levels
//         float currentFrameRMS = calculateRMS(outputBuffer, numFrames * 2);
//         float currentFramePeak = 0.0f;
//         for (uint32_t i = 0; i < numFrames * 2; ++i) {
//             currentFramePeak = std::max(currentFramePeak, std::abs(outputBuffer[i]));
//         }
        
//         // Update sliding RMS window
//         rmsHistory[rmsHistoryIndex] = currentFrameRMS;
//         rmsHistoryIndex = (rmsHistoryIndex + 1) % AGC_WINDOW_SIZE;
        
//         // Calculate average RMS over window
//         float sumRMS = std::accumulate(rmsHistory.begin(), rmsHistory.end(), 0.0f);
//         currentRMS = sumRMS / AGC_WINDOW_SIZE;
//         currentPeak = currentFramePeak;
        
//         // Apply AGC if enabled
//         if (agcEnabled && currentRMS > AGC_THRESHOLD) {
//             // Calculate target gain
//             float targetGain = AGC_TARGET_LEVEL / currentRMS;
//             targetGain = std::clamp(targetGain, AGC_MIN_GAIN, AGC_MAX_GAIN);
            
//             // Smoothly adjust gain (fast attack, slow release)
//             if (targetGain < agcGain) {
//                 // Attack phase - gain reduction
//                 agcGain = agcAttackCoef * agcGain + (1.0f - agcAttackCoef) * targetGain;
//             } else {
//                 // Release phase - gain increase
//                 agcGain = agcReleaseCoef * agcGain + (1.0f - agcReleaseCoef) * targetGain;
//             }
            
//             // Apply AGC gain to the mix
//             for (uint32_t i = 0; i < numFrames * 2; ++i) {
//                 outputBuffer[i] *= agcGain;
//             }
//         }
        
//         // Apply master volume
//         for (uint32_t i = 0; i < numFrames * 2; ++i) {
//             outputBuffer[i] *= masterVolume;
//         }
        
//         // Apply brick-wall limiter
//         for (uint32_t i = 0; i < numFrames * 2; ++i) {
//             float absValue = std::abs(outputBuffer[i]);
            
//             // Check if we need to adjust the limiter threshold
//             if (absValue > limitThreshold) {
//                 // Calculate the new limit factor
//                 float newLimit = limitThreshold / absValue;
//                 // Apply fast attack
//                 currentLimit = std::min(currentLimit, newLimit);
//             } else {
//                 // Gradual release
//                 currentLimit = limitReleaseCoef * currentLimit + (1.0f - limitReleaseCoef);
//                 currentLimit = std::min(currentLimit, 1.0f);
//             }
            
//             // Apply the limiter
//             outputBuffer[i] *= currentLimit;
            
//             // Apply soft clipping for extra protection
//             outputBuffer[i] = softClip(outputBuffer[i]);
//         }
        
//         // Apply master limiter effects chain
//         for (uint32_t i = 0; i < numFrames; ++i) {
//             float& left = outputBuffer[i * 2];
//             float& right = outputBuffer[i * 2 + 1];
//             masterLimiter->process(left, right);
//         }
        
//         // Update spectrum analyzer (simple FFT approximation)
//         if (numFrames >= SPECTRUM_BANDS * 2) {
//             updateSpectrumAnalysis(outputBuffer, numFrames);
//         }
//     }
    
//     /**
//      * @brief Update the spectrum analyzer
//      * @param buffer The audio buffer to analyze
//      * @param numFrames The number of frames in the buffer
//      */
//     void Mixer::updateSpectrumAnalysis(const float* buffer, uint32_t numFrames)
//     {
//         // Simple band-power spectrum analysis
//         // In a real implementation, this would use FFT
        
//         constexpr float DECAY_FACTOR = 0.7f;
        
//         // Decay current spectrum values
//         for (size_t i = 0; i < SPECTRUM_BANDS; ++i) {
//             spectrumAnalyzer[i] *= DECAY_FACTOR;
//         }
        
//         // Simplified band analysis (just for demonstration)
//         for (size_t band = 0; band < SPECTRUM_BANDS; ++band) {
//             size_t startIdx = (band * numFrames) / SPECTRUM_BANDS;
//             size_t endIdx = ((band + 1) * numFrames) / SPECTRUM_BANDS;
            
//             float bandSum = 0.0f;
//             for (size_t i = startIdx; i < endIdx && i < numFrames; ++i) {
//                 // Calculate average power in this frequency band
//                 // (We're using the left channel only for simplicity)
//                 bandSum += buffer[i * 2] * buffer[i * 2];
//             }
            
//             float bandAvg = bandSum / (endIdx - startIdx);
//             float bandRMS = std::sqrt(bandAvg);
            
//             // Update the spectrum value with peak tracking
//             spectrumAnalyzer[band] = std::max(spectrumAnalyzer[band], bandRMS);
//         }
//     }
    
//     /**
//      * @brief Get the number of active channels
//      * @return The count of currently active channels
//      */
//     size_t Mixer::getActiveChannelCount() const
//     {
//         size_t count = 0;
//         for (const auto& channel : *channels) {
//             if (channel.state == Channel::State::PLAYING || 
//                 channel.state == Channel::State::STOPPING) {
//                 count++;
//             }
//         }
//         return count;
//     }
    
//     /**
//      * @brief Mute or unmute a specific channel
//      * @param channelIdx The index of the channel to mute
//      * @param muted Whether the channel should be muted
//      */
//     void Mixer::setChannelMute(size_t channelIdx, bool muted)
//     {
//         if (channelIdx < channelVolumes.size()) {
//             // Store the original volume if we're muting
//             if (muted && channelVolumes[channelIdx] > 0.0f) {
//                 channelMuteStates[channelIdx] = channelVolumes[channelIdx];
//                 channelVolumes[channelIdx] = 0.0f;
//             } 
//             // Restore the original volume if we're unmuting
//             else if (!muted && channelVolumes[channelIdx] == 0.0f) {
//                 auto it = channelMuteStates.find(channelIdx);
//                 if (it != channelMuteStates.end()) {
//                     channelVolumes[channelIdx] = it->second;
//                     channelMuteStates.erase(it);
//                 } else {
//                     channelVolumes[channelIdx] = 1.0f; // Default volume if no stored state
//                 }
//             }
//         }
//     }
    
//     /**
//      * @brief Set the volume for a specific channel
//      * @param channelIdx The index of the channel
//      * @param volume The volume level (0.0 to 1.0)
//      */
//     void Mixer::setChannelVolume(size_t channelIdx, float volume)
//     {
//         if (channelIdx < channelVolumes.size()) {
//             channelVolumes[channelIdx] = std::clamp(volume, 0.0f, 1.0f);
            
//             // Update mute state if necessary
//             if (volume > 0.0f) {
//                 channelMuteStates.erase(channelIdx);
//             }
//         }
//     }
// }
