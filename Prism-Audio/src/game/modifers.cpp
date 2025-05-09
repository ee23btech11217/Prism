#include "libs/modifiers.hpp"

namespace Audio
{
    Modifiers::Modifiers() = default;
    Modifiers::~Modifiers() = default;

    void Modifiers::processModifier(const uint8_t modID, const uint8_t sourceID, AudioBuffer& buffer)
    {
        static Listener listener;
        updateListener(listener);
    }
    void Modifiers::updateListener(Listener &listen_)
    {
        
    } 
    void Modifiers::updateSource(float pos[3], float orient[3], float vel[3], uint8_t sourceID)
    {
        ;
    }
    void Modifiers::updateTransferFunction()
    {
        ;
    }
} // namespace Audio
