#include "libs/audioHelper.hpp"
#include "libs/chainFX.hpp"
#include "libs/channel.hpp"

namespace Audio
{   
    struct Listener
    {
        uint8_t id;
        float position[3];
        float orientation[3];
        float velocity[3];
    };

    struct Source
    {
        uint8_t id;
        float position[3];
        float orientation[3];
        float velocity[3];
        bool isActive;
    };

    class Modifiers
    {
        public:
            Modifiers();
            ~Modifiers();
            void processModifier(const uint8_t modID, const uint8_t sourceID, AudioBuffer& buffer);

        private:
            std::vector<float> impulseResponse;
            std::vector<Source> sources;
            Listener listener;

            std::vector<Source> getActiveSources();

            void updateTransferFunction();
            void updateListener(Listener &listen);
            void updateSource(float pos[3], float orient[3], float vel[3], uint8_t sourceID);
    };
}
