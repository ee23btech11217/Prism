/*
    AI Generated
*/

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <iomanip>
#include <cstring>
#include <map>
#include <filesystem>
#include <sstream>
#include <algorithm>
#include <portaudio.h>

struct PabEntry {
    uint8_t category;
    uint8_t subcategory;
    uint16_t index;
    uint32_t offset;
    uint32_t length;
    uint32_t frames;
    uint32_t sampleRate;
    uint32_t loopStart;
    uint32_t loopEnd;
    uint16_t loopCount;
    float duration;
    std::string id;
};

// Audio data structure for playback
typedef struct {
    int16_t* samples;
    uint32_t frameCount;
    uint32_t position;
    uint32_t channels;
    uint32_t loopStart;
    uint32_t loopEnd;
    uint16_t loopCount;
    uint16_t currentLoop;
    bool looping;
} AudioData;

static int paCallback(const void* inputBuffer, void* outputBuffer,
                     unsigned long framesPerBuffer,
                     const PaStreamCallbackTimeInfo* timeInfo,
                     PaStreamCallbackFlags statusFlags,
                     void* userData) {
    AudioData* data = (AudioData*)userData;
    int16_t* out = (int16_t*)outputBuffer;
    
    memset(out, 0, framesPerBuffer * 2 * sizeof(int16_t)); // Always stereo
    
    for (unsigned int i = 0; i < framesPerBuffer; i++) {
        // Handle looping logic
        if (data->looping && data->loopStart < data->loopEnd && 
            data->position >= data->loopEnd) {
            
            // Reset to loop start
            data->position = data->loopStart;
            data->currentLoop++;
            
            // Check if we've reached loop count limit (0 means infinite)
            if (data->loopCount > 0 && data->currentLoop >= data->loopCount) {
                data->looping = false;
            }
        }
        
        // Check for end of audio
        if (data->position >= data->frameCount) {
            return paComplete;
        }
        
        // Copy audio data
        for (unsigned int c = 0; c < data->channels; c++) {
            *out++ = data->samples[data->position * data->channels + c];
        }
        
        data->position++;
    }
    
    return paContinue;
}

class PabParser 
{
public:
    PabParser() : version(0), entryCount(0), headerSize(0), totalSize(0) {}

    bool parse(const std::string& filename) {
        std::ifstream file(filename, std::ios::binary);
        if (!file.is_open()) {
            std::cerr << "Error: Could not open file " << filename << std::endl;
            return false;
        }

        this->filename = filename;

        // Read and verify magic header
        char magicBuf[10] = {0};
        file.read(magicBuf, 9);
        magic = std::string(magicBuf);
        
        if (magic != "PRSMAUBNK") {
            std::cerr << "Invalid magic identifier: " << magic << std::endl;
            return false;
        }

        // Read header information
        file.read(reinterpret_cast<char*>(&version), 1);
        file.read(reinterpret_cast<char*>(&entryCount), 4);
        file.read(reinterpret_cast<char*>(&headerSize), 4);
        file.read(reinterpret_cast<char*>(&totalSize), 4);

        std::cout << "File: " << filename << std::endl;
        std::cout << "Magic: " << magic << std::endl;
        std::cout << "Version: " << static_cast<int>(version) << std::endl;
        std::cout << "Entry Count: " << entryCount << std::endl;
        std::cout << "Header Size: " << headerSize << std::endl;
        std::cout << "Total Size: " << totalSize << std::endl;

        auto actualSize = std::filesystem::file_size(filename);
        if (actualSize != totalSize) {
            std::cerr << "Warning: File size mismatch. Header: " << totalSize 
                    << ", Actual: " << actualSize << std::endl;
        }

        // Skip any padding after header metadata
        uint32_t currentPos = 22; // 9 (magic) + 1 (version) + 4*3 (counts/sizes)
        uint32_t paddingSize = 0;
        if (headerSize > entryCount * 30 + currentPos) {
            paddingSize = headerSize - (entryCount * 30 + currentPos);
            file.seekg(paddingSize, std::ios::cur);
        }

        // Read all entries
        entries.clear();
        for (uint32_t i = 0; i < entryCount; i++) {
            PabEntry entry;
            file.read(reinterpret_cast<char*>(&entry.category), 1);
            file.read(reinterpret_cast<char*>(&entry.subcategory), 1);
            file.read(reinterpret_cast<char*>(&entry.index), 2);
            file.read(reinterpret_cast<char*>(&entry.offset), 4);
            file.read(reinterpret_cast<char*>(&entry.length), 4);
            file.read(reinterpret_cast<char*>(&entry.frames), 4);
            file.read(reinterpret_cast<char*>(&entry.sampleRate), 4);
            file.read(reinterpret_cast<char*>(&entry.loopStart), 4);
            file.read(reinterpret_cast<char*>(&entry.loopEnd), 4);
            file.read(reinterpret_cast<char*>(&entry.loopCount), 2);

            entry.duration = static_cast<float>(entry.frames) / entry.sampleRate;

            // Create identifier string
            entry.id = std::to_string(entry.category) + "." + 
                    std::to_string(entry.subcategory) + "." + 
                    std::to_string(entry.index);

            entries.push_back(entry);
        }

        return true;
    }

    void printEntries(int limit = -1) {
        int count = (limit < 0 || limit > static_cast<int>(entries.size())) ? entries.size() : limit;

        std::cout << "\nEntries (" << count << " of " << entries.size() << "):" << std::endl;
        std::cout << std::string(120, '-') << std::endl;
        std::cout << std::left << std::setw(15) << "ID" 
                << std::setw(10) << "Offset" 
                << std::setw(10) << "Length" 
                << std::setw(10) << "Frames" 
                << std::setw(12) << "Sample Rate"
                << std::setw(12) << "Loop Start"
                << std::setw(12) << "Loop End"
                << std::setw(12) << "Loop Count"
                << std::setw(8) << "Duration" << std::endl;
        std::cout << std::string(120, '-') << std::endl;

        for (int i = 0; i < count; i++) {
            const auto& entry = entries[i];
            std::cout << std::left << std::setw(15) << entry.id 
                    << std::setw(10) << entry.offset 
                    << std::setw(10) << entry.length 
                    << std::setw(10) << entry.frames 
                    << std::setw(12) << entry.sampleRate
                    << std::setw(12) << entry.loopStart
                    << std::setw(12) << entry.loopEnd
                    << std::setw(12) << entry.loopCount
                    << std::fixed << std::setprecision(2) << entry.duration << "s" << std::endl;
        }
    }

    void checkAlignment() {
        std::vector<std::string> misaligned;

        for (const auto& entry : entries) {
            if (entry.offset % 4 != 0) {
                misaligned.push_back(entry.id);
            }
        }

        if (!misaligned.empty()) {
            std::cout << "\nWarning: " << misaligned.size() << " entries have misaligned offsets:" << std::endl;
            for (size_t i = 0; i < std::min(misaligned.size(), size_t(10)); i++) {
                std::cout << "  - " << misaligned[i] << std::endl;
            }
            if (misaligned.size() > 10) {
                std::cout << "  ... and " << (misaligned.size() - 10) << " more" << std::endl;
            }
        } else {
            std::cout << "\nAll data blocks are properly 4-byte aligned" << std::endl;
        }
    }

    int countLoopingSounds() {
        int count = 0;
        for (const auto& entry : entries) {
            if (entry.loopStart > 0 || entry.loopCount > 0) {
                count++;
            }
        }
        return count;
    }

    bool extractAudio(const std::string& identifier, const std::string& outputPath) {
        // Find entry with matching identifier
        PabEntry* entry = nullptr;
        for (auto& e : entries) {
            if (e.id == identifier) {
                entry = &e;
                break;
            }
        }

        if (!entry) {
            std::cerr << "Error: Entry " << identifier << " not found" << std::endl;
            return false;
        }

        try {
            std::ifstream inFile(filename, std::ios::binary);
            if (!inFile) {
                std::cerr << "Error: Could not open file " << filename << std::endl;
                return false;
            }

            // Seek to the audio data
            inFile.seekg(entry->offset);

            // Read the audio data
            std::vector<char> audioData(entry->length);
            inFile.read(audioData.data(), entry->length);

            // Output as WAV file
            std::string outFilePath = outputPath;
            if (!outputPath.empty() && outputPath.find('.') == std::string::npos) {
                outFilePath += ".wav"; // Add .wav extension if none provided
            }

            // Create WAV file
            std::ofstream outFile(outFilePath, std::ios::binary);
            if (!outFile) {
                std::cerr << "Error: Could not create output file " << outFilePath << std::endl;
                return false;
            }

            // Write WAV header
            // RIFF header
            outFile.write("RIFF", 4);
            uint32_t fileSize = 36 + entry->length; // 36 + data size
            outFile.write(reinterpret_cast<char*>(&fileSize), 4);
            outFile.write("WAVE", 4);

            // Format chunk
            outFile.write("fmt ", 4);
            uint32_t fmtChunkSize = 16;
            outFile.write(reinterpret_cast<char*>(&fmtChunkSize), 4);
            uint16_t audioFormat = 1; // PCM
            outFile.write(reinterpret_cast<char*>(&audioFormat), 2);
            uint16_t numChannels = 2; // Stereo
            outFile.write(reinterpret_cast<char*>(&numChannels), 2);
            uint32_t sampleRate = entry->sampleRate;
            outFile.write(reinterpret_cast<char*>(&sampleRate), 4);
            uint32_t byteRate = sampleRate * numChannels * 2; // Sample rate * channels * bytes per sample
            outFile.write(reinterpret_cast<char*>(&byteRate), 4);
            uint16_t blockAlign = numChannels * 2; // Channels * bytes per sample
            outFile.write(reinterpret_cast<char*>(&blockAlign), 2);
            uint16_t bitsPerSample = 16;
            outFile.write(reinterpret_cast<char*>(&bitsPerSample), 2);

            // Data chunk
            outFile.write("data", 4);
            uint32_t dataSize = entry->length;
            outFile.write(reinterpret_cast<char*>(&dataSize), 4);
            outFile.write(audioData.data(), dataSize);
            
            std::cout << "Extracted " << identifier << " to " << outFilePath 
                    << " (" << audioData.size() << " bytes)" << std::endl;

            if (entry->loopStart > 0 || entry->loopCount > 0) {
                std::cout << "  Loop Start: " << entry->loopStart 
                        << " samples, Loop End: " << entry->loopEnd 
                        << " samples, Loop Count: " << entry->loopCount << std::endl;
            }

            return true;
        } catch (const std::exception& e) {
            std::cerr << "Error extracting " << identifier << ": " << e.what() << std::endl;
            return false;
        }
    }

    void analyzeDataDistribution() {
        uint32_t headerBytes = headerSize;
        uint32_t dataBytes = totalSize - headerSize;

        float headerPercent = (static_cast<float>(headerBytes) / totalSize) * 100;
        float dataPercent = (static_cast<float>(dataBytes) / totalSize) * 100;

        std::cout << "\nData Distribution:" << std::endl;
        std::cout << "Header: " << headerBytes << " bytes (" 
                << std::fixed << std::setprecision(1) << headerPercent << "%)" << std::endl;
        std::cout << "Audio Data: " << dataBytes << " bytes (" 
                << std::fixed << std::setprecision(1) << dataPercent << "%)" << std::endl;

        // Analyze by category
        std::map<int, std::pair<int, uint32_t>> categories; // cat -> {count, bytes}
        for (const auto& entry : entries) {
            int cat = entry.category;
            if (categories.find(cat) == categories.end()) {
                categories[cat] = {0, 0};
            }
            
            categories[cat].first++;
            categories[cat].second += entry.length;
        }

        std::cout << "\nBreakdown by Category:" << std::endl;
        std::cout << std::left << std::setw(10) << "Category" 
                << std::setw(10) << "Count" 
                << std::setw(15) << "Size" 
                << std::setw(10) << "Percentage" << std::endl;
        std::cout << std::string(50, '-') << std::endl;

        for (const auto& [cat, data] : categories) {
            float percent = (static_cast<float>(data.second) / dataBytes) * 100;
            std::stringstream sizeStr;
            sizeStr << data.second << " bytes";
            
            std::cout << std::left << std::setw(10) << cat 
                    << std::setw(10) << data.first 
                    << std::setw(15) << sizeStr.str() 
                    << std::fixed << std::setprecision(1) << percent << "%" << std::endl;
        }

        // Show looping sound stats
        int loopCount = countLoopingSounds();
        if (loopCount > 0) {
            std::cout << "\nLooping Sounds: " << loopCount << " (" 
                    << std::fixed << std::setprecision(1) 
                    << (loopCount * 100.0f / entries.size()) << "%)" << std::endl;
        }
    }

    bool playSound(const std::string& identifier) {
        // Find entry with matching identifier
        PabEntry* entry = nullptr;
        for (auto& e : entries) {
            if (e.id == identifier) {
                entry = &e;
                break;
            }
        }

        if (!entry) {
            std::cerr << "Error: Entry " << identifier << " not found" << std::endl;
            return false;
        }

        try {
            // Read audio data directly from bank file
            std::ifstream file(filename, std::ios::binary);
            if (!file) {
                std::cerr << "Error: Could not open file " << filename << std::endl;
                return false;
            }

            // Seek to the audio data
            file.seekg(entry->offset);

            // Read the audio data
            std::vector<int16_t> samples(entry->frames * 2); // Stereo
            file.read(reinterpret_cast<char*>(samples.data()), entry->length);

            // Initialize PortAudio
            PaError err = Pa_Initialize();
            if (err != paNoError) {
                std::cerr << "PortAudio error: " << Pa_GetErrorText(err) << std::endl;
                return false;
            }

            // Setup our audio data structure
            AudioData audioData;
            audioData.samples = samples.data();
            audioData.frameCount = entry->frames;
            audioData.position = 0;
            audioData.channels = 2; // Stereo
            audioData.loopStart = entry->loopStart;
            audioData.loopEnd = entry->loopEnd;
            audioData.loopCount = entry->loopCount;
            audioData.currentLoop = 0;
            audioData.looping = (entry->loopStart > 0) && (entry->loopCount > 0 || entry->loopCount == 0);

            // Open an audio stream
            PaStream* stream;
            err = Pa_OpenDefaultStream(&stream,
                                     0,                // No input channels
                                     audioData.channels, // Output channels
                                     paInt16,          // 16-bit output
                                     entry->sampleRate, // Sample rate
                                     256,              // Frames per buffer
                                     paCallback,       // Callback function
                                     &audioData);      // Data structure passed to callback

            if (err != paNoError) {
                std::cerr << "PortAudio error: " << Pa_GetErrorText(err) << std::endl;
                Pa_Terminate();
                return false;
            }

            // Start the audio stream
            err = Pa_StartStream(stream);
            if (err != paNoError) {
                std::cerr << "PortAudio error: " << Pa_GetErrorText(err) << std::endl;
                Pa_CloseStream(stream);
                Pa_Terminate();
                return false;
            }

            std::cout << "Playing sound " << identifier << " (" 
                    << std::fixed << std::setprecision(2) << entry->duration 
                    << " seconds)" << std::endl;
            
            if (audioData.looping) {
                std::cout << "Looping: Start=" << entry->loopStart 
                          << ", End=" << entry->loopEnd 
                          << ", Count=" << (entry->loopCount == 0 ? "infinite" : std::to_string(entry->loopCount)) 
                          << std::endl;
            }
            
            std::cout << "Press Enter to stop playback..." << std::endl;

            // Wait for user to press Enter or for playback to complete
            std::cin.get();

            // Stop and close the stream
            Pa_StopStream(stream);
            Pa_CloseStream(stream);
            Pa_Terminate();

            return true;
        } catch (const std::exception& e) {
            std::cerr << "Error playing " << identifier << ": " << e.what() << std::endl;
            return false;
        }
    }

private:
    std::string filename;
    std::string magic;
    uint8_t version;
    uint32_t entryCount;
    uint32_t headerSize;
    uint32_t totalSize;
    std::vector<PabEntry> entries;
};

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <pab_file>" << std::endl;
        std::cerr << "   or: " << argv[0] << " <pab_file> extract <id> [output_file]" << std::endl;
        std::cerr << "   or: " << argv[0] << " <pab_file> play <id>" << std::endl;
        return 1;
    }

    PabParser parser;
    if (parser.parse(argv[1])) {
        parser.printEntries(10);  // Show first 10 entries
        parser.checkAlignment();
        parser.analyzeDataDistribution();
        
        if (argc >= 4) {
            std::string command = argv[2];
            std::string id = argv[3];
            
            if (command == "extract") {
                std::string outputPath = (argc >= 5) ? argv[4] : 
                                        ("extracted_" + id + ".wav");
                parser.extractAudio(id, outputPath);
            }
            else if (command == "play") {
                parser.playSound(id);
            }
            else {
                std::cerr << "Unknown command: " << command << std::endl;
                return 1;
            }
        }
    }
    
    return 0;
}