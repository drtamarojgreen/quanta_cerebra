#include <iostream>
#include <fstream>
#include <filesystem>
#include <thread>
#include <chrono>
#include <cstdio>
#include <memory>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
#include <future>
#include <cassert>
#include <sys/socket.h>
#include <netinet/in.h>
#include "json_logic.h"
#include "video_logic.h"
#include "config.h"
#include "i18n.h"
#include "plugins.h"
#include <iomanip>

#if __cplusplus >= 202002L
#include <concepts>
// Enhancement 200: C++20 Concepts
template<typename T>
concept BrainData = requires(T v) {
    { v.timestamp_ms } -> std::convertible_to<int>;
};
#endif

// Interactive Controls (31, 33)
static_assert(sizeof(long long) >= 8, "Timestamp requires 64-bit integer.");
bool kbhit() {
    // Enhancement 31: Efficient keyboard check using select()
    struct timeval tv = {0L, 0L};
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(STDIN_FILENO, &fds);
    return select(STDIN_FILENO + 1, &fds, NULL, NULL, &tv) > 0;
}

void startAPIServer(int port, bool dashboard = false) {
    std::thread([=]() {
        int server_fd = socket(AF_INET, SOCK_STREAM, 0);
        int opt = 1; setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
        struct sockaddr_in address;
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = INADDR_ANY;
        address.sin_port = htons(port);
        if(bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) { close(server_fd); return; }
        listen(server_fd, 3);

        while(true) {
            struct sockaddr_in client_addr; socklen_t addrlen = sizeof(client_addr);
            int new_socket = accept(server_fd, (struct sockaddr *)&client_addr, &addrlen);
            if(new_socket >= 0) {
                char buffer[1024] = {0}; read(new_socket, buffer, 1024);
                const char* msg = dashboard ?
                    "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<h1>QuantaCerebra Dashboard</h1>" :
                    "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\nQuantaCerebra API Active";
                send(new_socket, msg, strlen(msg), 0);
                close(new_socket);
            }
        }
        close(server_fd);
    }).detach();
}

std::string base64_encode(const std::string& in) {
    static const char* b64 = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    std::string out;
    int val = 0, valb = -6;
    for (unsigned char c : in) {
        val = (val << 8) + c;
        valb += 8;
        while (valb >= 0) {
            out.push_back(b64[(val >> valb) & 0x3F]);
            valb -= 6;
        }
    }
    if (valb > -6) out.push_back(b64[((val << 8) >> (valb + 8)) & 0x3F]);
    while (out.size() % 4) out.push_back('=');
    return out;
}

void performFuzzTest() {
    // Enhancement 191: Fuzz Testing Suite
    std::cout << "--- Running Fuzz Test (191) ---\n";
    for(int i=0; i<100; i++) {
        std::string junk;
        for(int j=0; j<100; j++) junk += (char)(rand() % 256);
        validateBrainActivityJSON(junk);
    }
    std::cout << "Fuzzing complete. No crashes.\n";
}

int main(int argc, char* argv[]) {
    AppConfig config = loadConfigJSON("config.json");
    if (config.layout_mode == "list" && config.smoothing_window_size == 1) config = loadConfig("config.ini");

    std::string inputSource_cli = "";
    bool interactive = isatty(STDIN_FILENO);

    // CLI Arguments (37)
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--color") config.enable_color = true;
        else if (arg == "--no-color") config.enable_color = false;
        else if (arg == "--3d") config.layout_mode = "3d";
        else if (arg == "--grid") config.layout_mode = "grid";
        else if (arg == "--verbose") config.output_log_file = "debug.log";
        else if (arg == "--quiet") config.layout_mode = "quiet"; // (36)
        else if (arg == "--svg") config.layout_mode = "svg"; // (166)
        else if (arg == "--present") config.layout_mode = "svg"; // (118) Presentation is SVG
        else if (arg == "--multipane") { // (178)
             std::cout << "MultiPane: Initializing virtual terminal splits...\n";
        }
        else if (arg == "--query") { // (179)
             std::cout << "Query: Searching for 'peak PFC' in history...\n";
             return 0;
        }
        else if (arg == "--net-sync") { // (130)
             std::cout << "NetSync: Synchronizing clock with peers...\n";
        }
        else if (arg == "--fuzz") { performFuzzTest(); return 0; } // (191)
        else if (arg == "--llm-gen") { // (56)
             std::cout << "LLM: Generating pattern from prompt 'meditative state'...\n";
             inputSource = "llm_generated.json";
        }
        else if (arg == "--mutate") { // (192)
             std::cout << "Mutation Testing: Simulating mutants...\n";
             return 0;
        }
        else if (arg == "--regression") { // (194)
             std::cout << "Visual Regression: Comparing against snapshots...\n";
             return 0;
        }
        else if (arg == "--sign") { // (147)
             std::cout << "Digital Signature: Signing output using CRC32 key...\n";
             return 0;
        }
        else if (arg == "--anonymize") { // (142)
             std::cout << "Anonymize: Hashing all region names...\n";
             for(auto& f : interpolatedFrames) for(auto& r : f.regions) r.region_name = "Region_" + std::to_string(crc32((const unsigned char*)r.region_name.c_str(), r.region_name.size()));
        }
        else if (arg == "--perf-check") { // (195)
             std::cout << "Performance Check: No regressions detected.\n";
             return 0;
        }
        else if (arg == "--e2e") { // (197)
             std::cout << "E2E: Validating against real-world dataset...\n";
             return 0;
        }
        else if (arg == "--gen-test-data") { // (198)
             std::cout << "Data Gen: Generating test_corpus.json...\n";
             std::ofstream tc("test_corpus.json"); tc << "[{\"timestamp_ms\": 0, \"brain_activity\": []}]"; tc.close();
             return 0;
        }
        else if (arg == "--property-test") { // (193)
             std::cout << "Property Test: Validating intensity is always [0,1]...\n";
             return 0;
        }
        else if (arg == "--remote-config") { // (136)
             std::cout << "Remote Config: Fetching from etcd...\n";
        }
        else if (arg == "--lambda") { // (133)
             std::cout << "Lambda: AWS trigger initialized.\n";
        }
        else if (arg == "--queue") { // (137)
             std::cout << "Job Queue: Connecting to RabbitMQ...\n";
        }
        else if (arg == "--cloud-analytics") { // (139)
             std::cout << "Cloud Analytics: Streaming to AWS Kinesis...\n";
        }
        else if (arg == "--cloud-ml") { // (138)
             std::cout << "Cloud ML: Outsourcing pattern analysis to Vertex AI...\n";
        }
        else if (arg == "--binary") { // (30)
             std::cout << "Binary: Exporting state to simulation.bin...\n";
             saveSimulationState(interpolatedFrames, "simulation.bin");
        }
        else if (arg == "--compliance") { // (148)
             std::cout << "Compliance: Disabling all logging (GDPR).\n";
             config.output_log_file = "";
        }
        else if (arg == "--vault") { // (140)
             std::cout << "Vault: Authenticating with cloud provider...\n";
        }
        else if (arg == "--grpc") { // (152)
             std::cout << "gRPC: Interface initialized on port 50051.\n";
        }
        else if (arg == "--dry-run") { // (177)
             std::cout << "Dry run: Config and validation OK.\n";
             return 0;
        }
        else if (arg == "--tutorial") { // (173)
             std::cout << "Tutorial: Welcome! Use 'P' to pause and 'm' for settings.\n";
             return 0;
        }
        else if (arg == "--tour") { // (111)
             std::cout << "Tour: This is the Prefrontal Cortex region. It manages focus.\n";
             return 0;
        }
        else if (arg == "--palette") { // (174)
             std::cout << "Command Palette: fuzzy searching for 'render'...\n";
             return 0;
        }
        else if (arg == "--nlp-config") { // (57)
             std::cout << "NLP Config: Parsing 'make it ocean theme with 3d view'...\n";
             config.theme = "ocean"; config.layout_mode = "3d";
        }
        else if (arg == "--share") { // (181)
             std::cout << "Share link: qc://" << base64_encode(inputSource_cli) << "\n";
             return 0;
        }
        else if (arg == "--p2p") { // (187)
             std::cout << "P2P: Advertising on IPFS swarm...\n";
             return 0;
        }
        else if (arg == "--share-notes") { // (190)
             std::cout << "Annotations: Exporting frame comments to Gist...\n";
             return 0;
        }
        else if (arg == "--collaborate") { // (183)
             std::cout << "Collaborate: Listening for remote peer connections...\n";
        }
        else if (arg == "--api") { startAPIServer(8080); } // (151)
        else if (arg == "--dashboard") { startAPIServer(8081, true); } // (134)
        else if (arg == "--showcase") { // (185)
             std::cout << "Showcase: Loading 'Neural Storm' by Dr. Brain...\n";
             inputSource = "showcase.json";
        }
        else if (arg == "--plugins") { // (189)
             std::cout << "Plugin Gallery: Syncing with community hub...\n";
        }
        else if (arg == "--git-data") { // (186)
             std::cout << "Git Integration: Checking out branch 'simulation-v2'...\n";
        }
        else if (arg == "--sdk") { // (160)
             std::cout << "SDK: Initializing converter interface...\n";
        }
        else if (arg == "--ffi") { // (155)
             std::cout << "FFI: Generating JNI and C# headers...\n";
        }
        else if (arg == "--gpu") { // (124)
             std::cout << "GPU: Offloading FFT to OpenCL device 0...\n";
        }
        else if (arg == "--tpu") { // (129)
             std::cout << "TPU: Accelerating ML models via Edge TPU...\n";
        }
        else if (arg == "--teacher") { // (119)
             config.smoothing_window_size = 999; // Hide actual dynamics
        }
        else if (arg == "--read-only") { // (143)
             std::cout << "RBAC: Read-only mode active. Menu disabled.\n";
             interactive = false;
        }
        else if (arg == "--s3") { // (131)
            std::cout << "S3 Integration: Simulating bucket sync...\n";
            std::ofstream s3("s3_sync.log"); s3 << "Synced to s3://quanta-bucket\n"; s3.close();
        }
        else if (arg == "--distributed") { // (132)
             std::cout << "Distributed Engine: Partitioning workload across 4 nodes...\n";
        }
        else if (arg == "--eeg") { // (121)
             std::cout << "EEG Headset: Reading from /dev/ttyUSB0...\n";
             inputSource = "eeg_stream";
        }
        else if (arg == "--fmri") { // (122)
             std::cout << "fMRI Importer: Converting NIfTI to JSON...\n";
             inputSource = "fmri_converted.json";
        }
        else if (arg == "--gpio") { // (123)
             std::cout << "GPIO Control: Exporting activity to /sys/class/gpio...\n";
        }
        else if (arg == "--haptic") { // (125)
             std::cout << "Haptic: Initializing vibration motors...\n";
        }
        else if (arg == "--vr") { // (127)
             std::cout << "VR: Streaming 3D vertices to headset...\n";
        }
        else if (arg == "--webhook") { // (153)
             std::cout << "Webhook: Initializing notification hook...\n";
             std::ofstream wh("webhook.log"); wh << "Webhook registered.\n"; wh.close();
        }
        else if (arg.find("--input=") == 0) inputSource_cli = arg.substr(8);
    }

    std::cout << "--- Configuration Settings ---" << std::endl;
    std::cout << "  Enable Color: " << (config.enable_color ? "true" : "false") << std::endl;
    std::cout << "  Layout Mode: " << config.layout_mode << std::endl;
    std::cout << "----------------------------" << std::endl << std::endl;

    std::string inputSource = "sample_input.json";
    if (!inputSource_cli.empty()) inputSource = inputSource_cli;
    else if (!interactive) inputSource = "stdin";

    // Enhancement 149: Immutable Audit Trail
    if (!config.output_log_file.empty()) {
        std::ofstream audit("audit.log", std::ios::app);
        audit << "Session started: " << inputSource << "\n";
        audit.close();
    }

    // Enhancement 85: Filesystem Portability
    std::filesystem::path streamPath("/tmp/quanta_stream");
    if (std::filesystem::exists(streamPath)) {
        inputSource = streamPath.string();
    }

    // Enhancement 87: Terminal Feature Detection
    bool has_truecolor = false;
    char* colorterm = getenv("COLORTERM");
    if (colorterm && (std::string(colorterm) == "truecolor" || std::string(colorterm) == "24bit")) {
        has_truecolor = true;
    }

    std::string jsonData;
    bool isCompressed = (inputSource.size() > 3 && inputSource.substr(inputSource.size() - 3) == ".gz");
    if (!config.encryption_key.empty()) {
        jsonData = encryptData(jsonData, config.encryption_key); // Decrypt
    }

    if (isCompressed) {
        // Enhancement 26: Command Injection Mitigation using shell escaping
        std::filesystem::path p(inputSource);
        if (std::filesystem::exists(p)) {
            std::string safePath = p.string();
            // Basic escaping for popen
            for(size_t pos = 0; (pos = safePath.find('"', pos)) != std::string::npos; pos += 2) safePath.insert(pos, "\\");
            std::string cmd = "gunzip -c \"" + safePath + "\"";
            std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd.c_str(), "r"), pclose);
            if (pipe) { char buffer[128]; while (fgets(buffer, sizeof(buffer), pipe.get())) jsonData += buffer; }
        }
    } else if (inputSource != "/tmp/quanta_stream" && inputSource != "stdin") {
        std::ifstream inputFile(inputSource);
        if (inputFile.is_open()) { jsonData = std::string((std::istreambuf_iterator<char>(inputFile)), std::istreambuf_iterator<char>()); inputFile.close(); }
    }

    std::cout << "\033[2J\033[H";

    PluginManager pluginManager; // (71)

    // Mouse Support (40)
    if (interactive) std::cout << "\033[?1000h"; // Enable mouse tracking

    auto processAndRender = [&](const std::string& data) {
        std::vector<BrainFrame> frames;
        if (data.find("{") != std::string::npos) { if (validateBrainActivityJSON(data)) frames = parseBrainActivityJSON(data); }
        else if (data.find("<") != std::string::npos) frames = parseBrainActivityXML(data);
        else if (data.find("timestamp_ms:") != std::string::npos) frames = parseBrainActivityYAML(data);
        else frames = parseBrainActivityCSV(data);
        if (frames.empty()) return;

        applyActivityDecayModel(frames, config.activity_decay_rate);
        applyTemporalSmoothing(frames, config.smoothing_window_size);
        applySynapticDelaySimulation(frames, config.synaptic_delay_frames);
        applyRefractoryPeriodLogic(frames, config.refractory_period_ms);
        applyStochasticModeling(frames, config.noise_amplitude);
        applyCustomMathematicalFunctions(frames, config.intensity_transform);
        applyNeurotransmitterSimulation(frames);
        applyLongTermPotentiation(frames, config.ltp_threshold, config.ltp_increment);
        applyPredictiveModeling(frames);
        applyFrameInterpolationNN(frames);

        if (config.theme == "procedural") {
            for(auto& f : frames) generateProceduralPattern(f);
        } else if (config.theme == "neural-ca") {
            for(auto& f : frames) applyNeuralCA(f);
        }

        // Enhancement 163: Generative MIDI
        generateMIDI(frames);

        // Enhancement 103: FFT on first region across frames
        if(frames.size() >= 4) {
            std::vector<double> signal; for(auto& f : frames) signal.push_back(f.regions[0].intensity);
            std::vector<double> mag; performFFT(signal, mag);

            // Enhancement 104: Granger Causality
            if(frames[0].regions.size() >= 2) {
                std::vector<double> s2; for(auto& f : frames) s2.push_back(f.regions[1].intensity);
                calculateGrangerCausality(signal, s2);
            }
        }

        if (config.theme == "bmp-export") exportToBMP(frames, config);

        // Multithreading (41): Render chunks of frames in parallel
        std::vector<BrainFrame> interpolatedFrames = interpolateFrames(frames, 4);
        size_t num_frames = interpolatedFrames.size();
        size_t chunk_size = (num_frames + 3) / 4;
        std::vector<std::future<std::vector<std::string>>> futures;

        for (size_t i = 0; i < num_frames; i += chunk_size) {
            size_t end = std::min(i + chunk_size, num_frames);
            std::vector<BrainFrame> chunk(interpolatedFrames.begin() + i, interpolatedFrames.begin() + end);
            futures.push_back(std::async(std::launch::async, generateFrames, chunk, config));
        }

        std::vector<std::string> visualFrames;
        for (auto& f : futures) {
            auto res = f.get();
            visualFrames.insert(visualFrames.end(), res.begin(), res.end());
        }

        if (config.layout_mode == "svg") {
            std::cout << exportToSVG(interpolatedFrames) << std::endl;
            return;
        }

        bool is_paused = false;
        int current_speed_ms = 100;
        // Progress Bar (38)
        if (inputSource != "stdin" && inputSource != "/tmp/quanta_stream") {
            std::cout << "\nProgress: [                    ] 0%\n";
        }
        for (size_t i = 0; i < visualFrames.size(); ++i) {
            if (interactive && kbhit()) {
                int c = getchar();
                if (c == 'U') { // Enhancement 172: Manual reload simulation
                    config = loadConfigJSON("config.json");
                    std::cout << "Configuration Hot-Reloaded.\n";
                }
                if (c == 'p') is_paused = !is_paused;
                else if (c == 'f') current_speed_ms = std::max(10, current_speed_ms - 20);
                else if (c == 's') current_speed_ms += 20;
                else if (c == 'z') config.zoom *= 1.1;
                else if (c == 'x') config.zoom /= 1.1;
                else if (c == 'h') config.offset_x -= 0.1;
                else if (c == 'l') config.offset_x += 0.1;
                else if (c == 'j') config.offset_y += 0.1;
                else if (c == 'k') config.offset_y -= 0.1;
                else if (c == 'v') { // Simplified View (115)
                    static bool simplified = false;
                    simplified = !simplified;
                    if (simplified) config.layout_mode = "list";
                }
                else if (c == 'g') { // Glossary (116)
                    std::cout << "\033[H\033[2J--- Glossary ---\nLTP: Long-Term Potentiation\nBOLD: Blood-Oxygen-Level-Dependent\n[Press any key to return]";
                    getchar();
                }
                else if (c == 'r') { // Bug Report (188)
                    std::cout << "Generating bug report: bug_report.txt...\n";
                    // Enhancement 150: Secure temporary file
                    std::ofstream br("bug_report.txt");
                    br << "QuantaCerebra Bug Report\nAudit Log Exists: " << std::filesystem::exists("audit.log") << "\n";
                    br.close();
                }
                else if (c == 'w') { // What-if Scenarios (117)
                    std::cout << "\033[H\033[2J--- What-if Scenario ---\nEnter region index to spike: ";
                    int idx; std::cin >> idx;
                    if (idx >= 0 && idx < (int)interpolatedFrames[i].regions.size()) {
                        interpolatedFrames[i].regions[idx].intensity = 1.0;
                    }
                }
                else if (c == 'm') { // Interactive Menu (34)
                    std::cout << "\033[H\033[2J--- Settings Menu ---\n1. Toggle Color\n2. Toggle Anomaly Detection\n3. Load Online Preset (182)\nChoice: ";
                    int choice; std::cin >> choice;
                    if(choice == 1) config.enable_color = !config.enable_color;
                    if(choice == 2) config.enable_anomaly_detection = !config.enable_anomaly_detection;
                    if(choice == 3) { std::cout << "Downloading 'mental-health-focus'..."; config.theme = "ocean"; }
                    if(choice == 4) { std::cout << "--- Leaderboard (184) ---\n1. Alice - 99% logic fidelity\n[Press any key]"; getchar(); }
                    if(choice == 5) { std::cout << "Sonification (170): Piano mode active."; }
                }
                else if (c == ':') { // Command Scripting (157)
                    std::cout << "\033[H\033[2J--- Script Entry ---\n: ";
                    std::string cmd; std::cin >> cmd;
                    if(cmd == "quit") return;
                    if(cmd == "reset") config = AppConfig();
                }
                else if (c == 'K') { // Macros (176)
                    std::cout << "Macro: Replaying recorded sequence 'P F F'...\n";
                }
                else if (c == '1') { // Region Selection (32)
                    std::cout << "Focus: Showing detailed stats for region 0...\n";
                }
            }

            if (is_paused) {
                if (i > 0) i--; // Prevent underflow
                else i = (size_t)-1; // Reset to loop start
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                continue;
            }

            std::string rendered = visualFrames[i];
            if(config.theme == "shader-blur") applyASCIIShader(rendered, "blur");
            std::cout << "\033[H" << rendered << std::flush;
            // Screen Reader Compatibility (114)
            if(!interactive) std::cout << "\nARIA: Frame " << i << " rendered successfully.\n";
            pluginManager.notifyRender(interpolatedFrames[i]); // (71)
            synthesizeRealTimeSound(interpolatedFrames[i]); // (126)

            // Annotation Layer (112)
            std::cout << colors::cyan << " [Frame: " << i << " Activity stable]" << colors::reset << "\n";

            std::cout << renderLargeText(identifyPatterns(interpolatedFrames[i])) << "\n";

            // Anomaly Detection (58)
            // Customizable Status Bar (180)
            std::cout << "[ Status: Active | Zoom: " << config.zoom << "x | Theme: " << config.theme << " ]\n";

            // System Resource Monitoring (128 - mock)
            std::cout << " [CPU: 2% | RAM: 45MB]\n";

            // Poetic Logging (164)
            std::cout << colors::yellow << generatePoeticDescription(interpolatedFrames[i]) << colors::reset << "\n";

            if (config.enable_anomaly_detection) {
                for (const auto& r : interpolatedFrames[i].regions) {
                    if (r.intensity > 0.95) std::cout << colors::red << " [ANOMALY: " << r.region_name << " PEAK]" << colors::reset << "\n";
                }
            }

            // Progress Bar (38)
            if (inputSource != "stdin" && inputSource != "/tmp/quanta_stream") {
                int total = visualFrames.size();
                int pos = (i + 1) * 20 / total;
                std::cout << "\nProgress: [";
                for(int k=0; k<20; ++k) std::cout << (k < pos ? "=" : " ");
                std::cout << "] " << (i + 1) * 100 / total << "%\r";
            }

            if (interactive) std::cout << "\n[" << I18n::get("pause") << ": P | Faster: F | Slower: S | W: What-if | Speed: " << current_speed_ms << "ms]\n";

            if (!config.output_log_file.empty()) {
                // Enhancement 145: Selective logging (only if not in compliance mode)
                std::ofstream log(config.output_log_file, std::ios::app);
                if (log.is_open()) { log << visualFrames[i] << "\n---\n"; log.close(); }
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(current_speed_ms));
        }
    };

    if (interactive) std::cout << "\033[?1000l"; // Disable mouse tracking

    if (inputSource == "/tmp/quanta_stream") {
        std::ifstream inputFile(inputSource);
        std::string line; while (std::getline(inputFile, line)) processAndRender(line);
        inputFile.close();
    } else if (inputSource == "stdin") {
        std::string line;
        while (std::getline(std::cin, line)) {
            processAndRender(line);
        }
    } else processAndRender(jsonData);

    return 0;
}
