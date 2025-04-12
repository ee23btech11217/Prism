/*
    This code is AI generated, and is there for testing purposes. Apparently, my PAB writer (in Python) works as expected, which I didn't expect.
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
#include <portaudio.h>

struct PabEntry {
    uint8_t category;
    uint8_t subcategory;
    uint16_t index;
    uint32_t offset;
    uint32_t length;
    uint32_t frames;
    uint32_t sampleRate;
    float duration;
    std::string id;
};

// Audio data structure for playback
typedef struct {
    int16_t* samples;
    uint32_t frameCount;
    uint32_t position;
    uint32_t channels;
} AudioData;


static int paCallback(const void* inputBuffer, void* outputBuffer,
                     unsigned long framesPerBuffer,
                     const PaStreamCallbackTimeInfo* timeInfo,
                     PaStreamCallbackFlags statusFlags,
                     void* userData) {
    AudioData* data = (AudioData*)userData;
    int16_t* out = (int16_t*)outputBuffer;
    
    memset(out, 0, framesPerBuffer * data->channels * sizeof(int16_t));
    
    for (unsigned int i = 0; i < framesPerBuffer; i++) {
        if (data->position >= data->frameCount) {
            return paComplete;
        }
        
        for (unsigned int c = 0; c < data->channels; c++) {
            *out++ = data->samples[data->position * data->channels + c];
        }
        
        data->position++;
    }
    
    return paContinue;
}

class PabParser {
public:
    PabParser() : version(0), entryCount(0), headerSize(0), totalSize(0) {}

    bool parse(const std::string& filename) {
        std::ifstream file(filename, std::ios::binary);
        if (!file.is_open()) {
            std::cerr << "Error: Could not open file " << filename << std::endl;
            return false;
        }

        this->filename = filename;

        char magicBuf[10] = {0};
        file.read(magicBuf, 9);
        magic = std::string(magicBuf);
        
        if (magic != "PRSMAUBNK") {
            std::cerr << "Invalid magic identifier: " << magic << std::endl;
            return false;
        }

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

            entry.duration = static_cast<float>(entry.frames) / entry.sampleRate;

            entry.id = std::to_string(entry.category) + "." + 
                       std::to_string(entry.subcategory) + "." + 
                       std::to_string(entry.index);

            entries.push_back(entry);
        }

        return true;
    }

    void printEntries(int limit = -1) {
        int count = (limit < 0 || limit > static_cast<int>(entries.size())) ? 
                    entries.size() : limit;

        std::cout << "\nEntries (" << count << " of " << entries.size() << "):" << std::endl;
        std::cout << std::string(80, '-') << std::endl;
        std::cout << std::left << std::setw(15) << "ID" 
                  << std::setw(10) << "Offset" 
                  << std::setw(10) << "Length" 
                  << std::setw(10) << "Frames" 
                  << std::setw(12) << "Sample Rate"
                  << std::setw(8) << "Duration" << std::endl;
        std::cout << std::string(80, '-') << std::endl;

        for (int i = 0; i < count; i++) {
            const auto& entry = entries[i];
            std::cout << std::left << std::setw(15) << entry.id 
                      << std::setw(10) << entry.offset 
                      << std::setw(10) << entry.length 
                      << std::setw(10) << entry.frames 
                      << std::setw(12) << entry.sampleRate
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

            // Write it to the output file
            std::ofstream outFile(outputPath, std::ios::binary);
            if (!outFile) {
                std::cerr << "Error: Could not create output file " << outputPath << std::endl;
                return false;
            }
            
            outFile.write(audioData.data(), entry->length);
            
            std::cout << "Extracted " << identifier << " to " << outputPath 
                      << " (" << audioData.size() << " bytes)" << std::endl;
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
            std::vector<int16_t> samples(entry->frames * 2); // Assuming stereo
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
            audioData.channels = 2; // Assuming stereo

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
                                        ("extracted_" + id + ".raw");
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