#include "testFramework.hpp"
#include "libs/channel.hpp"

// Helper functions and mocks for Channel tests
namespace {
    // Mock AudioBuffer for testing
    std::shared_ptr<Audio::AudioBuffer> createMockBuffer(uint32_t id, uint32_t length, uint32_t sampleRate) {
        auto buffer = std::make_shared<Audio::AudioBuffer>(id);
        buffer->makeBuffer(length, sampleRate);
        
        // Fill with sample data
        for (uint32_t i = 0; i < length; i++) {
            buffer->audioBuffer.data_L[i] = static_cast<int32_t>(i * 1000);
            buffer->audioBuffer.data_R[i] = static_cast<int32_t>(i * 2000);
        }
        
        return buffer;
    }
    
    // Helper to call processCommands (which is protected)
    class TestableChannel : public Audio::Channel {
    public:
        TestableChannel(uint32_t id) : Audio::Channel(id) {}
        
        void testProcessCommands() {
            processCommands();
        }
        
        void testProcessSample(float& outLeft, float& outRight, uint32_t position) {
            processSample(outLeft, outRight, position);
        }
        
        using Audio::Channel::commandQueue;
        using Audio::Channel::hasCommands;
    };
}

// Test suite for Channel class
void register_channel_tests(TestFramework& framework) {
    // Test constructor initialization
    framework.add_test("Channel Constructor Initialization", []() {
        Audio::Channel channel(1);
        
        // Check initial state
        TEST_ASSERT_EQUAL(channel.state, Audio::Channel::State::IDLE);
        TEST_ASSERT_EQUAL(channel.loop, false);
        TEST_ASSERT_EQUAL(channel.priority, Audio::Channel::Priority::MEDIUM);
        TEST_ASSERT_EQUAL(channel.volume, 1.0f);
        TEST_ASSERT_EQUAL(channel.pan, 0.0f);
        TEST_ASSERT_EQUAL(channel.playbackSpeed, 1.0f);
        TEST_ASSERT_EQUAL(channel.buffer, nullptr);
    });
    
    // Test setBuffer
    framework.add_test("Channel setBuffer", []() {
        Audio::Channel channel(1);
        
        auto buffer = createMockBuffer(5, 1000, 44100);
        channel.setBuffer(buffer);
        
        // Check buffer was set
        TEST_ASSERT_NOT_EQUAL(channel.buffer, nullptr);
        TEST_ASSERT_EQUAL(channel.buffer->audioBuffer.id, 5);
    });
    
    // Test command queue (play/pause/stop)
    framework.add_test("Channel Command Queue (Play/Pause/Stop)", []() {
        TestableChannel channel(1);
        
        // Queue commands
        channel.play();
        
        // Check command was queued
        TEST_ASSERT_EQUAL(channel.hasCommands, true);
        TEST_ASSERT_EQUAL(channel.commandQueue.size(), 1);
        
        // Process commands
        channel.testProcessCommands();
        
        // Check command was processed
        TEST_ASSERT_EQUAL(channel.hasCommands, false);
        TEST_ASSERT_EQUAL(channel.commandQueue.size(), 0);
        TEST_ASSERT_EQUAL(channel.state, Audio::Channel::State::PLAYING);
        
        // Test pause
        channel.pause();
        channel.testProcessCommands();
        TEST_ASSERT_EQUAL(channel.state, Audio::Channel::State::PAUSED);
        
        // Test stop
        channel.stop();
        channel.testProcessCommands();
        TEST_ASSERT_EQUAL(channel.state, Audio::Channel::State::IDLE);
    });
    
    // Test position control
    framework.add_test("Channel Position Control", []() {
        TestableChannel channel(1);
        
        // Set buffer to have a known length
        auto buffer = createMockBuffer(5, 1000, 44100);
        channel.setBuffer(buffer);
        
        // Check initial position
        TEST_ASSERT_EQUAL(channel.getPosition(), 0);
        TEST_ASSERT_EQUAL(channel.getDuration(), 1000);
        
        // Set position
        channel.setPosition(500);
        channel.testProcessCommands();
        TEST_ASSERT_EQUAL(channel.getPosition(), 500);
        
        // Try setting beyond length
        channel.setPosition(2000);
        channel.testProcessCommands();
        TEST_ASSERT_EQUAL(channel.getPosition(), 1000); // Clamped to buffer length
    });
    
    // Test seek functionality
    framework.add_test("Channel Seek", []() {
        TestableChannel channel(1);
        
        // Set buffer with 2 seconds of audio at 44100 Hz
        auto buffer = createMockBuffer(6, 88200, 44100);
        channel.setBuffer(buffer);
        
        // Seek to 1 second
        channel.seek(1.0f);
        channel.testProcessCommands();
        TEST_ASSERT_EQUAL(channel.getPosition(), 44100);
    });
    
    // Test effect application
    framework.add_test("Channel Effect Application", []() {
        TestableChannel channel(1);
        
        // Apply an effect
        channel.applyEffect(Audio::EffectType::DELAY);
        channel.testProcessCommands();
        
        // Apply multiple effects and check clear
        channel.clearEffects();
        channel.testProcessCommands();
        
        std::vector<Audio::EffectType> effects = {
            Audio::EffectType::DELAY,
            Audio::EffectType::REVERB
        };
        
        channel.applyEffects(effects);
        
        // This should have implicitly cleared effects and applied new ones
        // We can't easily check internal state, but this tests the API works without crashing
    });
    
    // Test sample processing
    framework.add_test("Channel Sample Processing", []() {
        TestableChannel channel(1);
        
        auto buffer = createMockBuffer(7, 1000, 44100);
        channel.setBuffer(buffer);
        
        float leftSample = 0.0f;
        float rightSample = 0.0f;
        
        // Process a sample
        channel.testProcessSample(leftSample, rightSample, 100);
        
        // With default pan (0), both samples should be processed identically except for the original values
        const float normalizationFactor = 1.0f / 2147483648.0f;
        float expectedLeft = buffer->audioBuffer.data_L[100] * normalizationFactor;
        float expectedRight = buffer->audioBuffer.data_R[100] * normalizationFactor;
        
        TEST_ASSERT_FLOAT_EQUAL(leftSample, expectedLeft, 0.0001f);
        TEST_ASSERT_FLOAT_EQUAL(rightSample, expectedRight, 0.0001f);
    });
    
    // Test panning
    framework.add_test("Channel Panning", []() {
        TestableChannel channel(1);
        
        auto buffer = createMockBuffer(8, 1000, 44100);
        channel.setBuffer(buffer);
        
        // Set pan hard right
        channel.pan = 1.0f;
        
        float leftSample = 0.0f;
        float rightSample = 0.0f;
        
        // Process a sample
        channel.testProcessSample(leftSample, rightSample, 100);
        
        // Left channel should be scaled to 0, right unchanged
        const float normalizationFactor = 1.0f / 2147483648.0f;
        float expectedLeft = 0.0f;
        float expectedRight = buffer->audioBuffer.data_R[100] * normalizationFactor;
        
        TEST_ASSERT_FLOAT_EQUAL(leftSample, expectedLeft, 0.0001f);
        TEST_ASSERT_FLOAT_EQUAL(rightSample, expectedRight, 0.0001f);
    });
    
    // Test volume
    framework.add_test("Channel Volume", []() {
        Audio::Channel channel(1);
        
        // Set volume to 50%
        channel.volume = 0.5f;
        TEST_ASSERT_EQUAL(channel.volume, 0.5f);
        
        // Set volume beyond range (should be clamped by the Channel::Command)
        TestableChannel::Command cmd;
        cmd.type = TestableChannel::Command::Type::SET_VOLUME;
        cmd.floatValue = 1.5f;
        channel.commandQueue.push(cmd);
        channel.hasCommands = true;
    });
    
    // Test automation
    framework.add_test("Channel Automation", []() {
        Audio::Channel channel(1);
        
        // Create automation profile
        Audio::AutomationProfile volumeProfile;
        volumeProfile.times = {0, 1000, 2000};
        volumeProfile.values = {100, 50, 100}; // 100% -> 50% -> 100%
        volumeProfile.length = 3;
        volumeProfile.sampleRate = 44100;
        volumeProfile.startSample = 0;
        volumeProfile.endSample = 2000;
        
        // Apply automation
        channel.setVolumeAutomation(volumeProfile);
        
        // Since we can't directly test the automation application, we at least verify the API works
        channel.clearAutomation(true, false, false);
    });
} 