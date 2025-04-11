#pragma once

#include <cstdint>
#include <vector>
#include <memory>
#include <string>
#include <functional>
#include <atomic>
#include <queue>
#include <iostream>
#include "libs/audioHelper.hpp"
#include "libs/chainFX.hpp"

namespace Audio
{
    // Forward declarations
    class Engine;
    
    class Channel 
    {
        public:
            // Channel state management
            enum class State 
            {
                IDLE,      
                PLAYING,   
                PAUSED,    
                STOPPING,  
                ERROR      
            };

            // Channel priority levels for resource management
            enum class Priority
            {
                LOW,
                MEDIUM,
                HIGH,
                CRITICAL
            };

            // Command structure for the ChannelQueue
            struct Command
            {
                enum class Type
                {
                    PLAY,
                    PAUSE,
                    STOP,
                    FADE_STOP,
                    SET_VOLUME,
                    SET_PAN,
                    SET_SPEED,
                    SET_POSITION,
                    SET_BUFFER,
                    SET_LOOP,
                    ADD_EFFECT,
                    REMOVE_EFFECT,
                    CLEAR_EFFECTS,
                    SET_EFFECT_PARAM
                };

                Type type;
                union
                {
                    float floatValue;
                    uint32_t uintValue;
                    bool boolValue;
                };
                uint32_t uintValue2;     // Parameter index
                std::vector<float> params; // Effect parameters
            };

            /**
             * @brief Creates a new audio channel
             * @param id Unique channel identifier
             * @param engine Parent engine instance
             */
            Channel(uint32_t id, Engine* engine = nullptr);
            ~Channel();

            // Public state and parameters
            State state;
            bool loop;
            Priority priority;
            float volume;
            float pan;
            float playbackSpeed;
            std::function<void(uint32_t)> onCompleteCallback;
            std::function<void(uint32_t)> onLoopCallback;
            std::shared_ptr<AudioBuffer> buffer;

            // Buffer management
            void setBuffer(std::shared_ptr<AudioBuffer> buffer);
            bool loadBufferFromFile(const std::string& filePath);
            void clearBuffer();
            void applyAutomation(
                uint32_t currentSample,
                const AutomationProfile& profile,
                float baseValue,
                float& resultValue,
                float minValue,
                float maxValue,
                bool isFactor = true
            );
            
            // Automation control
            void setVolumeAutomation(const AutomationProfile& profile);
            void setPanAutomation(const AutomationProfile& profile);
            void setSpeedAutomation(const AutomationProfile& profile);
            void clearAutomation(bool volume = true, bool pan = true, bool speed = true);

            // Playback control
            void play();
            void pause();
            void stop();
            void stopWithFade(uint32_t fadeDurationMs);
            
            // Position control
            uint32_t getPosition() const;
            uint32_t getDuration() const;
            void setPosition(uint32_t samplePosition);
            void seek(float positionInSeconds);
            
            // Effects management
            void applyEffect(EffectType effectType);
            void applyEffects(const std::vector<EffectType>& effectTypes);
            void removeEffect(EffectType effectType);
            void clearEffects();
            
            // Effect parameter control with specific parameter enums
            template<typename ParamEnum>
            void setEffectParameter(EffectType effectType, ParamEnum param, float value) {
                setEffectParameter(static_cast<uint32_t>(effectType), static_cast<uint32_t>(param), value);
            }
            
            // Generic effect parameter control
            void setEffectParameter(uint32_t effectId, uint32_t paramIndex, float value);
            
            // Audio processing
            void processAudio(float* outputBuffer, uint32_t numFrames, uint32_t sampleRate);
            
            // Command queue
            std::queue<Command> commandQueue;
            bool hasCommands;
            
        protected:
            
            // Internal processing
            void processCommands();
            void processSample(float& outLeft, float& outRight, uint32_t position);
            void notifyComplete();
            void notifyLoop();
            
            // Automation processing
            void applyVolumeAutomation(uint32_t currentSample);
            void applyPanAutomation(uint32_t currentSample);
            void applyPlaybackSpeedAutomation(uint32_t currentSample);
            
            // Internal state
            Engine* parentEngine;
            uint32_t channelID;
            std::shared_ptr<ChainFX> fx;
            uint32_t currentPosition;
            float fadeVolume;
            float fadeStep;
            
            // Automation profiles
            AutomationProfile volumeProfile;
            AutomationProfile panProfile;
            AutomationProfile speedProfile;
            bool hasVolumeAutomation;
            bool hasPanAutomation;
            bool hasSpeedAutomation;
    };
}