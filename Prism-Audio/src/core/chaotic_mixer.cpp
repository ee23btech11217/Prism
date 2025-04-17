// #include <random>
// #include <vector>
// #include <cmath>
// #include <chrono>
// #include <curl/curl.h>
// #include <nlohmann/json.hpp>

// class ChaoticMixer {
// private:
//     std::random_device hw_rand;
//     std::mt19937 gen;
//     float prevOutput;
//     std::vector<float> filterState;
    
//     // Callback for CURL
//     static size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* userp) {
//         userp->append((char*)contents, size * nmemb);
//         return size * nmemb;
//     }

// public:
//     ChaoticMixer() : gen(std::chrono::system_clock::now().time_since_epoch().count()), prevOutput(0.0f) {
//         // Warm up hardware random generator
//         for(int i = 0; i < 100; i++) {
//             hw_rand();
//         }
//     }

//     float process(const std::vector<float>& inputs) {
//         // Get fresh random values
//         std::uniform_real_distribution<float> coef_dist(0.0f, 0.3f); // Coefficients won't sum to 1
//         std::uniform_int_distribution<> channel_dist(0, inputs.size() - 1);
//         std::uniform_real_distribution<float> prob_dist(0.0f, 1.0f);

//         // Randomly decide number of channels to mix (1 to 3)
//         int num_channels = (hw_rand() % 3) + 1;
        
//         float output = 0.0f;
        
//         // Mix random channels
//         for(int i = 0; i < num_channels; i++) {
//             int channel = channel_dist(gen);
//             float coef = coef_dist(gen);
//             output += inputs[channel] * coef;
//         }

//         // Maybe add previous output
//         if(prob_dist(gen) > 0.7f) {
//             output += prevOutput * coef_dist(gen);
//         }

//         // Random unstable filter (1-3 poles in right half-plane)
//         int num_poles = (hw_rand() % 3) + 1;
//         for(int i = 0; i < num_poles; i++) {
//             float pole = 0.1f + (prob_dist(gen) * 0.4f); // Poles between 0.1 and 0.5
//             output = output + pole * output;
//         }

//         // Send to ChatGPT
//         if(prob_dist(gen) > 0.95f) { // 5% chance to send to ChatGPT
//             CURL* curl = curl_easy_init();
//             if(curl) {
//                 std::string response;
//                 std::string prompt = "You are terrible audio engineer/mixer, give the worst advice";
                
//                 // Note: You would need to add your actual API key and endpoint here
//                 curl_easy_setopt(curl, CURLOPT_URL, "https://api.openai.com/v1/chat/completions");
//                 curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
//                 curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
                
//                 curl_easy_cleanup(curl);
//             }
//         }

//         prevOutput = output;
//         return output;
//     }
// };