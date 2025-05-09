#include <iostream>
#include <memory>
#include <vector>
#include <cassert>
#include <cmath>
#include <iomanip>
#include <chrono>
#include <thread>

#include "libs/audioHelper.hpp"
#include "libs/channel.hpp"
#include "libs/chainFX.hpp"
#include "libs/resourceManager.hpp"
#include "libs/engine.hpp"

// Mock Engine class for testing (since it's forward-declared in channel.hpp)
class Engine {
public:
    Engine() {}
    ~Engine() {}
};

// Test helper to generate sine wave data
void generateSineWave(Audio::sample_t* buffer, uint32_t length, uint32_t sampleRate, 
                     float frequency, float amplitude = 0.8f) {
    for (uint32_t i = 0; i < length; ++i) {
        float time = static_cast<float>(i) / sampleRate;
        float value = std::sin(2.0f * M_PI * frequency * time) * amplitude;
        buffer[i] = static_cast<int16_t>(value * 32767.0f);
    }
}

// Helper to create an automation profile
Audio::AutomationProfile createAutomationProfile(
    uint32_t sampleRate, 
    float startTimeSeconds, 
    float endTimeSeconds,
    const std::vector<uint32_t>& valuePoints) {
    
    Audio::AutomationProfile profile;
    
    // Calculate duration in samples
    uint32_t durationSamples = static_cast<uint32_t>((endTimeSeconds - startTimeSeconds) * sampleRate);
    
    // Set up times array - evenly distributed
    profile.times.resize(valuePoints.size());
    for (size_t i = 0; i < valuePoints.size(); ++i) {
        profile.times[i] = static_cast<uint32_t>(i * durationSamples / (valuePoints.size() - 1));
    }
    
    // Set up values array
    profile.values = valuePoints;
    
    // Set remaining properties
    profile.length = static_cast<uint32_t>(valuePoints.size());
    profile.sampleRate = sampleRate;
    profile.startSample = static_cast<uint32_t>(startTimeSeconds * sampleRate);
    profile.endSample = static_cast<uint32_t>(endTimeSeconds * sampleRate);
    
    return profile;
}

// Simple RMS calculator for audio buffers
float calculateRMS(const float* buffer, uint32_t length) {
    float sum = 0.0f;
    for (uint32_t i = 0; i < length; ++i) {
        sum += buffer[i] * buffer[i];
    }
    return std::sqrt(sum / length);
}

// Convert Channel::State enum to string
std::string stateToString(Audio::Channel::State state) {
    switch (state) {
        case Audio::Channel::State::IDLE: return "IDLE";
        case Audio::Channel::State::PLAYING: return "PLAYING";
        case Audio::Channel::State::PAUSED: return "PAUSED";
        case Audio::Channel::State::STOPPING: return "STOPPING";
        case Audio::Channel::State::ERROR: return "ERROR";
        default: return "UNKNOWN";
    }
}

// Main test function
void testChannel() {
    std::cout << "===== CHANNEL CLASS TEST =====" << std::endl;
    
    // Constants for testing
    const uint32_t SAMPLE_RATE = 44100;
    const uint32_t BUFFER_SIZE = 512;  // Frames per processing block
    const float DURATION = 3.0f;       // Duration in seconds
    const uint32_t BUFFER_LENGTH = static_cast<uint32_t>(SAMPLE_RATE * DURATION);
    
    // Create a mock engine
    Engine* mockEngine;
    
    // Create channel
    Audio::Channel channel(1);
    
    // Create and prepare an audio buffer
    std::cout << "Creating test audio buffer..." << std::endl;
    auto buffer = std::make_shared<Audio::AudioBuffer>(1);
    buffer->makeBuffer(BUFFER_LENGTH, SAMPLE_RATE);
    
    // Fill left and right channels with different frequency sine waves
    generateSineWave(buffer->audioBuffer.data_L, BUFFER_LENGTH, SAMPLE_RATE, 440.0f);  // A4 note in left channel
    generateSineWave(buffer->audioBuffer.data_R, BUFFER_LENGTH, SAMPLE_RATE, 523.25f); // C5 note in right channel
    
    buffer->audioBuffer.isLoaded = true;
    
    // Setup automation profiles
    std::cout << "Setting up automation profiles..." << std::endl;
    
    // Volume automation - starts at 100%, dips to 50% at halfway, back to 100%
    auto volumeProfile = createAutomationProfile(
        SAMPLE_RATE,
        0.5f,  // Start at 0.5 seconds
        2.5f,  // End at 2.5 seconds
        {100, 50, 100}  // Values in percent
    );
    channel.setVolumeAutomation(volumeProfile);
    
    // Pan automation - starts at full left, moves to center, then to full right
    auto panProfile = createAutomationProfile(
        SAMPLE_RATE,
        1.0f,  // Start at 1.0 seconds
        2.0f,  // End at 2.0 seconds
        {0, 50, 100}  // 0=full left, 50=center, 100=full right
    );
    channel.setPanAutomation(panProfile);
    
    // Set the buffer on the channel
    channel.setBuffer(buffer);
    
    // Start playback
    std::cout << "Starting playback..." << std::endl;
    channel.play();
    
    // Process audio in blocks to simulate real-time audio
    std::cout << std::setw(5) << "Block" << " | "
              << std::setw(8) << "Time" << " | "
              << std::setw(10) << "State" << " | "
              << std::setw(8) << "Volume" << " | "
              << std::setw(8) << "Pan" << " | "
              << std::setw(8) << "RMS L" << " | "
              << std::setw(8) << "RMS R" << " | "
              << "Action" << std::endl;
    
    std::cout << std::string(80, '-') << std::endl;
    
    const uint32_t totalBlocks = BUFFER_LENGTH / BUFFER_SIZE + 1;
    
    for (uint32_t block = 0; block < totalBlocks; ++block) {
        // Create output buffer for this block
        float outputBuffer[BUFFER_SIZE * 2] = {0};  // Stereo interleaved
        
        // Process audio for this block
        channel.processAudio(outputBuffer, BUFFER_SIZE, SAMPLE_RATE);
        
        // Calculate RMS for left and right channels
        float rmsL = calculateRMS(&outputBuffer[0], BUFFER_SIZE);
        float rmsR = calculateRMS(&outputBuffer[1], BUFFER_SIZE);
        
        // Get current position and convert to time
        float currentTime = static_cast<float>(channel.getPosition()) / SAMPLE_RATE;
        
        // Get current state as string
        std::string stateStr = stateToString(channel.state);
        
        // Print info every 10 blocks or when something interesting happens
        std::string action = "";
        
        // Test pausing, resuming, seeking and fading at specific times
        if (block == 10) {
            action = "Pausing";
            channel.pause();
        } else if (block == 15) {
            action = "Resuming";
            channel.play();
        } else if (block == 25) {
            action = "Seeking to 1.5s";
            channel.seek(1.5f);
        } else if (block == 35) {
            action = "Stopping with fade";
            channel.stopWithFade(500);  // 500ms fade out
        }
        
        // Print every 5 blocks or when there's an action
        if (block % 5 == 0 || !action.empty()) {
            std::cout << std::setw(5) << block << " | "
                      << std::setw(7) << std::fixed << std::setprecision(2) << currentTime << "s | "
                      << std::setw(10) << stateStr << " | "
                      << std::setw(7) << std::fixed << std::setprecision(2) << channel.volume << " | "
                      << std::setw(7) << std::fixed << std::setprecision(2) << channel.pan << " | "
                      << std::setw(7) << std::fixed << std::setprecision(4) << rmsL << " | "
                      << std::setw(7) << std::fixed << std::setprecision(4) << rmsR << " | "
                      << action << std::endl;
        }
        
        // If the channel has stopped, no need to continue processing
        if (channel.state == Audio::Channel::State::IDLE) {
            break;
        }
        
        // In a real test, we might add small delays to simulate real-time processing
        // std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    
    // Test loop functionality
    std::cout << "\nTesting loop functionality..." << std::endl;
    
    // Reset position and set loop to true
    channel.stop();
    channel.loop = true;
    
    // Setup a callback to detect loops
    int loopCount = 0;
    channel.onLoopCallback = [&loopCount](uint32_t channelId) {
        loopCount++;
        std::cout << "Loop detected on channel " << channelId << 
                     " (loop count: " << loopCount << ")" << std::endl;
    };
    
    // Start playback
    channel.play();
    
    // Process until we've detected at least 2 loops or processed enough blocks
    for (uint32_t block = 0; block < 250 && loopCount < 2; ++block) {
        float outputBuffer[BUFFER_SIZE * 2] = {0};
        channel.processAudio(outputBuffer, BUFFER_SIZE, SAMPLE_RATE);
        
        // Get current position and convert to time
        float currentTime = static_cast<float>(channel.getPosition()) / SAMPLE_RATE;
        
        // Print every 20 blocks to avoid too much output
        if (block % 20 == 0) {
            std::cout << "Block " << block << ", Time: " << std::fixed << std::setprecision(2) 
                      << currentTime << "s, State: " << stateToString(channel.state) << std::endl;
        }
    }
    
    // Test effect application
    std::cout << "\nTesting effects..." << std::endl;
    
    // Initialize the effect bank if not already
    Audio::ChainFX::initializeEffectBank();
    
    // Add some effects
    channel.applyEffect(Audio::EffectType::REVERB);
    
    // Set some parameters
    channel.setEffectParameter(Audio::EffectType::REVERB, Audio::ReverbParam::ROOM_SIZE, 0.8f);
    channel.setEffectParameter(Audio::EffectType::REVERB, Audio::ReverbParam::MIX, 0.5f);
    
    // Process a few blocks to test the effects
    for (uint32_t block = 0; block < 10; ++block) {
        float outputBuffer[BUFFER_SIZE * 2] = {0};
        channel.processAudio(outputBuffer, BUFFER_SIZE, SAMPLE_RATE);
        
        // Calculate RMS to see the effect
        float rmsL = calculateRMS(&outputBuffer[0], BUFFER_SIZE);
        float rmsR = calculateRMS(&outputBuffer[1], BUFFER_SIZE);
        
        std::cout << "Effect Block " << block 
                  << ", RMS L: " << std::fixed << std::setprecision(4) << rmsL
                  << ", RMS R: " << rmsR << std::endl;
    }
    
    // Clean up
    channel.stop();
    channel.clearEffects();
    
    std::cout << "===== CHANNEL TEST COMPLETE =====" << std::endl;
}

int main() {
    try {
        // Initialize ChainFX bank
        Audio::ChainFX::initializeEffectBank();
        
        // Run the channel test
        testChannel();
        
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}