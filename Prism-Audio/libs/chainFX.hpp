#pragma once

#include <cstdint>
#include <vector>
#include <string>
#include <memory>
#include <unordered_map>
#include <array>
#include <functional>

namespace Audio
{
    // Forward declaration
    class Effect;
    
    // Maximum number of effects in the bank
    constexpr size_t MAX_EFFECTS = 50;
    
    // Effect types
    enum class EffectType : uint32_t
    {
        DELAY = 0,
        DISTORTION = 1,
        REVERB = 2
    };
    
    // Parameter enums for each effect type
    enum class DelayParam : uint32_t
    {
        TIME = 0,
        FEEDBACK = 1,
        MIX = 2
    };
    
    enum class DistortionParam : uint32_t
    {
        DRIVE = 0,
        TONE = 1,
        LEVEL = 2
    };
    
    enum class ReverbParam : uint32_t
    {
        ROOM_SIZE = 0,
        DAMPING = 1,
        MIX = 2
    };
    
    /**
     * @brief Chain of audio effects
     */
    class ChainFX
    {
    public:
        ChainFX();
        ~ChainFX();
        
        // Effect management using numeric IDs
        void applyEffect(uint32_t effectId);
        void applyEffects(const std::vector<uint32_t>& effectIds);
        void removeEffect(uint32_t effectId);
        void clearEffects();
        
        // Check if an effect is applied
        bool isEffectApplied(uint32_t effectId) const;
        
        // Parameter control
        std::vector<float> getParameters(uint32_t effectId);
        void setParameter(uint32_t effectId, uint32_t paramIndex, float value);
        
        // Audio processing
        void process(float& left, float& right);
        
        // Get effect information
        EffectType getEffectType(uint32_t effectId) const;
        uint32_t getParameterCount(uint32_t effectId) const;
        
        // Static method to initialize the effect bank
        static void initializeEffectBank();
        
        // Active effects and their order
        std::vector<uint32_t> activeEffects;
        
        // Local cache of effect parameters
        std::unordered_map<uint32_t, std::vector<float>> effectParameters;
        
        // Static effect bank
        static std::array<std::shared_ptr<Effect>, MAX_EFFECTS> effectBank;
        static bool bankInitialized;
    };
    
    /**
     * @brief Base class for audio effects
     */
    class Effect
    {
    public:
        virtual ~Effect() = default;
        
        // Process audio
        virtual void process(float& left, float& right, const std::vector<float>& params) = 0;
        
        // Parameter handling
        virtual std::vector<float> getDefaultParameters() const = 0;
        virtual uint32_t getParameterCount() const = 0;
        
        // Effect info
        virtual EffectType getType() const = 0;
    };
    
    // Effect Factory
    std::shared_ptr<Effect> createEffect(EffectType type);
}