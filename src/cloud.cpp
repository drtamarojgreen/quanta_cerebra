#include "cloud.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <filesystem>
#include <cstring>
#include <algorithm>

bool CloudSystem::syncToS3(const std::string& bucket, const std::string& data) {
    std::filesystem::create_directories("cloud/s3/" + bucket);
    std::ofstream ofs("cloud/s3/" + bucket + "/latest.json");
    ofs << data;
    return ofs.good();
}

bool CloudSystem::triggerLambda(const std::string& func, const std::string& payload) {
    std::filesystem::create_directories("cloud/lambda/logs");
    std::ofstream ofs("cloud/lambda/logs/" + func + ".log", std::ios::app);
    ofs << "Triggered with: " << payload << "\n";
    return true;
}

std::string CloudSystem::fetchRemoteConfig(const std::string& url) {
    return "{\"theme\": \"ocean\", \"layout_mode\": \"3d\"}";
}

void CloudSystem::streamToKinesis(const std::string& data) {
    std::filesystem::create_directories("cloud/kinesis");
    static int seq = 0;
    std::ofstream ofs("cloud/kinesis/stream_" + std::to_string(seq++) + ".dat");
    ofs << data;
}

void CloudSystem::publishToQueue(const std::string& queue, const std::string& msg) {
    std::filesystem::create_directories("cloud/queues/" + queue);
    static int id = 0;
    std::ofstream ofs("cloud/queues/" + queue + "/" + std::to_string(id++) + ".msg");
    ofs << msg;
}

bool CloudSystem::authenticateVault(const std::string& token) {
    return token == "vault-root-token";
}

void DBConnector::saveFrame(const BrainFrame& frame) {
    std::filesystem::create_directories("cloud/db");
    std::ofstream ofs("cloud/db/history.csv", std::ios::app);
    ofs << frame.timestamp_ms << "," << frame.regions.size() << "\n";
}

std::vector<BrainFrame> DBConnector::queryHistory(const std::string& filter) {
    std::vector<BrainFrame> res;
    std::ifstream ifs("cloud/db/history.csv");
    std::string line;
    while(std::getline(ifs, line)) {
        if(line.find(filter) != std::string::npos) {
            BrainFrame f; f.timestamp_ms = std::stoi(line.substr(0, line.find(',')));
            res.push_back(f);
        }
    }
    return res;
}

std::vector<unsigned char> GRPCInterface::serialize(const BrainFrame& frame) {
    std::vector<unsigned char> res;
    size_t sz = frame.regions.size();
    res.insert(res.end(), (unsigned char*)&frame.timestamp_ms, (unsigned char*)&frame.timestamp_ms + sizeof(int));
    res.insert(res.end(), (unsigned char*)&sz, (unsigned char*)&sz + sizeof(size_t));
    for(const auto& r : frame.regions) {
        double i = r.intensity;
        res.insert(res.end(), (unsigned char*)&i, (unsigned char*)&i + sizeof(double));
    }
    return res;
}

BrainFrame GRPCInterface::deserialize(const std::vector<unsigned char>& data) {
    BrainFrame f;
    if(data.size() < sizeof(int) + sizeof(size_t)) return f;
    std::memcpy(&f.timestamp_ms, data.data(), sizeof(int));
    size_t sz; std::memcpy(&sz, data.data() + sizeof(int), sizeof(size_t));
    size_t offset = sizeof(int) + sizeof(size_t);
    for(size_t i=0; i<sz && offset + sizeof(double) <= data.size(); i++) {
        BrainRegion r; std::memcpy(&r.intensity, data.data() + offset, sizeof(double));
        f.regions.push_back(r); offset += sizeof(double);
    }
    return f;
}

// Simulated P2P Swarm
void P2PSystem::broadcast(const std::string& data) {
    std::filesystem::create_directories("cloud/p2p");
    std::ofstream ofs("cloud/p2p/broadcast.dat", std::ios::app);
    ofs << data << "\n";
}
