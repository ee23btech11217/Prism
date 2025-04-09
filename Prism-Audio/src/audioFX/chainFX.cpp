#include "libs/chainFX.hpp"
#include <iostream>
#include <algorithm>
#include <cmath>

namespace Audio
{
    // Static member initialization
    std::array<std::shared_ptr<Effect>, MAX_EFFECTS> ChainFX::effectBank;
    bool ChainFX::bankInitialized = false;
    
    /**
     * @brief Delay effect implementation
     */
    class DelayEffect : public Effect
    {
    public:
        DelayEffect() {
            // Initialize buffers with a reasonable size at construction time
            bufferSize = 88200; // Max 2 seconds at 48kHz
            bufferL.resize(bufferSize, 0.0f);
            bufferR.resize(bufferSize, 0.0f);
            writePos = 0;
        }

        void process(float& left, float& right, const std::vector<float>& params) override {
            float delayTime = params.size() > 0 ? std::clamp(params[0], 0.01f, 2.0f) : 0.5f;
            float feedback = params.size() > 1 ? std::clamp(params[1], 0.0f, 0.99f) : 0.5f;
            float mix = params.size() > 2 ? std::clamp(params[2], 0.0f, 1.0f) : 0.5f;
            
            // Calculate read position
            uint32_t sampleRate = params.size() > 3 ? static_cast<uint32_t>(params[3]) : 44100;
            int readPos = writePos - static_cast<int>(delayTime * sampleRate);
            if (readPos < 0) readPos += bufferSize;
            
            // Apply delay effect
            float delayedL = bufferL[readPos];
            float delayedR = bufferR[readPos];
            
            bufferL[writePos] = left + delayedL * feedback;
            bufferR[writePos] = right + delayedR * feedback;
            
            left = left * (1.0f - mix) + delayedL * mix;
            right = right * (1.0f - mix) + delayedR * mix;
            
            writePos = (writePos + 1) % bufferSize;
        }
        
        std::vector<float> getDefaultParameters() const override {
            return {0.5f, 0.5f, 0.5f, 44100.0f}; // delayTime, feedback, mix, sampleRate
        }
        
        uint32_t getParameterCount() const override {
            return 4; // Including sample rate
        }
        
        EffectType getType() const override {
            return EffectType::DELAY;
        }
        
    private:
        // Instance variables instead of static variables
        std::vector<float> bufferL;
        std::vector<float> bufferR;
        int bufferSize;
        int writePos;
    };
    
    /**
     * @brief Distortion effect implementation
     */
    class DistortionEffect : public Effect
    {
    public:
    
        void process(float& left, float& right, const std::vector<float>& params) override {
            float drive = params.size() > 0 ? std::clamp(params[0], 0.0f, 1.0f) : 0.5f;
            float tone = params.size() > 1 ? std::clamp(params[1], 0.0f, 1.0f) : 0.5f;
            float level = params.size() > 2 ? std::clamp(params[2], 0.0f, 1.0f) : 0.5f;
            
            left *= 1.0f + drive * 9.0f;
            right *= 1.0f + drive * 9.0f;
            
            left = std::tanh(left);
            right = std::tanh(right);
            
            float toneCoef = tone * 0.99f;
            left = left * (1.0f - toneCoef) + prevL * toneCoef;
            right = right * (1.0f - toneCoef) + prevR * toneCoef;
            prevL = left;
            prevR = right;
            
            left *= level;
            right *= level;
        }
        
        std::vector<float> getDefaultParameters() const override {
            return {0.5f, 0.5f, 0.5f}; // drive, tone, level
        }
        
        uint32_t getParameterCount() const override {
            return 3;
        }
        
        EffectType getType() const override {
            return EffectType::DISTORTION;
        }
        
        float prevL = 0.0f;
        float prevR = 0.0f;
    };
    
    /**
     * @brief Reverb effect implementation
     */
    class ReverbEffect : public Effect
    {
    public:
        ReverbEffect() {
            // Initialize all buffers and state variables at construction
            static const int combDelays[4] = {1557, 1617, 1491, 1422};
            static const int allpassDelays[2] = {225, 341};
            
            // Allocate the buffers
            for (int i = 0; i < 4; i++) {
                combL[i].resize(combDelays[i], 0.0f);
                combR[i].resize(combDelays[i], 0.0f);
                combPos[i] = 0;
                filterStore[i] = 0.0f;
            }
            
            for (int i = 0; i < 2; i++) {
                allpassL[i].resize(allpassDelays[i], 0.0f);
                allpassR[i].resize(allpassDelays[i], 0.0f);
                allpassPos[i] = 0;
            }
        }
        
        /**
         * @brief Process audio through reverb effect
         */
        void process(float& left, float& right, const std::vector<float>& params) override {
            float roomSize = params.size() > 0 ? std::clamp(params[0], 0.0f, 1.0f) : 0.5f;
            float damping = params.size() > 1 ? std::clamp(params[1], 0.0f, 1.0f) : 0.5f;
            float mix = params.size() > 2 ? std::clamp(params[2], 0.0f, 1.0f) : 0.3f;
            
            static const float combFeedback = 0.84f;
            static const float allpassFeedback = 0.5f;
            
            float dryL = left;
            float dryR = right;
            
            float wetL = 0.0f;
            float wetR = 0.0f;
            
            for (int i = 0; i < 4; i++) {
                wetL += processComb(left, combL[i], combL[i].size(), combFeedback * roomSize, damping, i);
                wetR += processComb(right, combR[i], combR[i].size(), combFeedback * roomSize, damping, i);
            }
            
            for (int i = 0; i < 2; i++) {
                wetL = processAllpass(wetL, allpassL[i], allpassL[i].size(), allpassFeedback, i);
                wetR = processAllpass(wetR, allpassR[i], allpassR[i].size(), allpassFeedback, i);
            }
            
            left = dryL * (1.0f - mix) + wetL * mix;
            right = dryR * (1.0f - mix) + wetR * mix;
        }
        
        std::vector<float> getDefaultParameters() const override {
            return {0.5f, 0.5f, 0.3f}; // roomSize, damping, mix
        }
        
        uint32_t getParameterCount() const override {
            return 3;
        }
        
        EffectType getType() const override {
            return EffectType::REVERB;
        }
        
        /**
         * @brief Process a single comb filter
         */
        float processComb(float input, std::vector<float>& buffer, int size, float feedback, float damp, int index) {
            float output = buffer[combPos[index]];
            
            filterStore[index] = output * (1.0f - damp) + filterStore[index] * damp;
            
            buffer[combPos[index]] = input + filterStore[index] * feedback;
            
            combPos[index] = (combPos[index] + 1) % size;
            
            return output;
        }
        
        /**
         * @brief Process a single allpass filter
         */
        float processAllpass(float input, std::vector<float>& buffer, int size, float feedback, int index) {
            float output = buffer[allpassPos[index]];
            
            buffer[allpassPos[index]] = input + output * feedback;
            
            allpassPos[index] = (allpassPos[index] + 1) % size;
            
            return output - input * feedback;
        }
        
    private:
        // Use instance variables instead of static variables
        std::vector<float> combL[4];
        std::vector<float> combR[4];
        std::vector<float> allpassL[2];
        std::vector<float> allpassR[2];
        
        // State variables
        int combPos[4];
        int allpassPos[2];
        float filterStore[4];
    };
    
    /**
     * @brief Initialize a new effect chain
     */
    ChainFX::ChainFX() {
        if (!bankInitialized) {
            initializeEffectBank();
        }
    }
    
    ChainFX::~ChainFX() {
        clearEffects();
    }
    
    /**
     * @brief Initialize the global effect bank with built-in effects
     */
    void ChainFX::initializeEffectBank() {
        if (bankInitialized) return;
        
        // IDs 0-9: Basic effects
        effectBank[static_cast<uint32_t>(EffectType::DELAY)] = std::make_shared<DelayEffect>();
        effectBank[static_cast<uint32_t>(EffectType::DISTORTION)] = std::make_shared<DistortionEffect>();
        effectBank[static_cast<uint32_t>(EffectType::REVERB)] = std::make_shared<ReverbEffect>();
        
        // IDs 10-19: Reserved for modulation effects
        // IDs 20-29: Reserved for filter effects
        // IDs 30-39: Reserved for dynamics effects
        // IDs 40-49: Reserved for special/custom effects
        
        bankInitialized = true;
    }
    
    /**
     * @brief Apply an effect to the chain
     */
    void ChainFX::applyEffect(uint32_t effectId) {
        if (effectId >= MAX_EFFECTS || !effectBank[effectId]) {
            std::cerr << "Error: Invalid effect ID: " << effectId << std::endl;
            return;
        }
        
        if (isEffectApplied(effectId)) {
            return;
        }
        
        activeEffects.push_back(effectId);
        
        if (effectParameters.find(effectId) == effectParameters.end()) {
            effectParameters[effectId] = effectBank[effectId]->getDefaultParameters();
        }
    }
    
    /**
     * @brief Replace all effects with new ones
     */
    void ChainFX::applyEffects(const std::vector<uint32_t>& effectIds) {
        clearEffects();
        
        for (uint32_t id : effectIds) {
            applyEffect(id);
        }
    }
    
    /**
     * @brief Remove an effect from the chain
     */
    void ChainFX::removeEffect(uint32_t effectId) {
        auto it = std::find(activeEffects.begin(), activeEffects.end(), effectId);
        if (it != activeEffects.end()) {
            activeEffects.erase(it);
        }
    }
    
    /**
     * @brief Remove all effects from the chain
     */
    void ChainFX::clearEffects() {
        activeEffects.clear();
    }
    
    /**
     * @brief Check if an effect is already in the chain
     */
    bool ChainFX::isEffectApplied(uint32_t effectId) const {
        return std::find(activeEffects.begin(), activeEffects.end(), effectId) != activeEffects.end();
    }
    
    /**
     * @brief Get current parameters for an effect
     */
    std::vector<float> ChainFX::getParameters(uint32_t effectId) {
        if (effectId >= MAX_EFFECTS || !effectBank[effectId]) {
            return {};
        }
        
        auto it = effectParameters.find(effectId);
        if (it != effectParameters.end()) {
            return it->second;
        }
        
        return effectBank[effectId]->getDefaultParameters();
    }
    
    /**
     * @brief Set effect parameter by index
     */
    void ChainFX::setParameter(uint32_t effectId, uint32_t paramIndex, float value) {
        if (effectId >= MAX_EFFECTS || !effectBank[effectId]) {
            return;
        }
        
        auto& params = effectParameters[effectId];
        if (params.empty()) {
            params = effectBank[effectId]->getDefaultParameters();
        }
        
        if (paramIndex < params.size()) {
            params[paramIndex] = value;
        }
    }
    
    /**
     * @brief Process audio through all active effects
     */
    void ChainFX::process(float& left, float& right) {
        for (uint32_t effectId : activeEffects) {
            if (effectId < MAX_EFFECTS && effectBank[effectId]) {
                auto& params = effectParameters[effectId];
                effectBank[effectId]->process(left, right, params);
            }
        }
    }
    
    /**
     * @brief Get effect type
     */
    EffectType ChainFX::getEffectType(uint32_t effectId) const {
        if (effectId < MAX_EFFECTS && effectBank[effectId]) {
            return effectBank[effectId]->getType();
        }
        return EffectType::DELAY; // Default
    }
    
    /**
     * @brief Get parameter count for an effect
     */
    uint32_t ChainFX::getParameterCount(uint32_t effectId) const {
        if (effectId < MAX_EFFECTS && effectBank[effectId]) {
            return effectBank[effectId]->getParameterCount();
        }
        return 0;
    }
    
    /**
     * @brief Create a new effect instance by type
     */
    std::shared_ptr<Effect> createEffect(EffectType type) {
        switch (type) {
            case EffectType::DELAY:
                return std::make_shared<DelayEffect>();
            case EffectType::DISTORTION:
                return std::make_shared<DistortionEffect>();
            case EffectType::REVERB:
                return std::make_shared<ReverbEffect>();
            default:
                std::cerr << "Unknown effect type: " << static_cast<int>(type) << std::endl;
                return nullptr;
        }
    }
}