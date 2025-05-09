#include "libs/audioHelper.hpp"

namespace Audio
{
    struct ImpulseResponse
    {
        uint16_t id;
        std::vector<uint16_t> coeffs;
    };

    struct Node
    {
        uint8_t id;
        uint8_t envType;

        bool isSource;
        volume_t sourceVolume;

        volume_t volumeL;
        volume_t volumeR;

        float globalPosition[3];
        float orientation[3];

        bool isListener;
    };

    struct Egde
    {
        uint16_t id;
        uint8_t srcNodeID;
        uint8_t dstNodeID;

        bool portalExists;
        float openPercent;
        ImpulseResponse response;
    };

    class Graph
    {
        public:
            Graph();
            ~Graph();
            
            void readGraph(const std::string& path);


        private:
            std::unordered_map<uint8_t, Node> nodes;
            std::unordered_map<uint16_t, Egde> edges;
            std::unordered_map<uint8_t, std::vector<uint16_t>> adjacencyList;
            std::unordered_map<uint8_t, std::vector<uint16_t>> reverseAdjacencyList;
    }
}