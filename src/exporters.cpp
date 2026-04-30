#include "exporters.h"
#include <fstream>
#include <cmath>
#include <sstream>
#include <algorithm>

static unsigned int crc32(const unsigned char* data, size_t len) {
    unsigned int crc = 0xFFFFFFFF;
    for (size_t i = 0; i < len; i++) {
        crc ^= data[i];
        for (int j = 0; j < 8; j++) crc = (crc >> 1) ^ (0xEDB88320 & (-(crc & 1)));
    }
    return ~crc;
}

static void write32(std::ostream& os, unsigned int val) {
    unsigned char b[4] = {(unsigned char)(val >> 24), (unsigned char)(val >> 16), (unsigned char)(val >> 8), (unsigned char)val};
    os.write((char*)b, 4);
}

static void adler32(const unsigned char* data, size_t len, unsigned int& a, unsigned int& b) {
    for (size_t i = 0; i < len; i++) { a = (a + data[i]) % 65521; b = (b + a) % 65521; }
}

void generateMIDI(const std::vector<BrainFrame>& frames) {
    std::ofstream ofs("brain.mid", std::ios::binary);
    unsigned char header[] = {'M','T','h','d', 0,0,0,6, 0,1, 0,1, 0,96};
    ofs.write((char*)header, 14);
    std::vector<unsigned char> track = {'M','T','r','k'};
    std::vector<unsigned char> events;
    for(const auto& f : frames) {
        for(const auto& r : f.regions) {
            unsigned char pitch = 60 + (unsigned char)(r.intensity * 24);
            unsigned char vel = (unsigned char)(r.intensity * 127);
            events.push_back(0);
            events.push_back(0x90); events.push_back(pitch); events.push_back(vel);
            events.push_back(48);
            events.push_back(0x80); events.push_back(pitch); events.push_back(0);
        }
    }
    events.push_back(0); events.push_back(0xFF); events.push_back(0x2F); events.push_back(0);
    unsigned int len = events.size();
    unsigned char len_b[4] = {(unsigned char)(len>>24), (unsigned char)(len>>16), (unsigned char)(len>>8), (unsigned char)len};
    track.insert(track.end(), len_b, len_b+4);
    track.insert(track.end(), events.begin(), events.end());
    ofs.write((char*)track.data(), track.size());
}

std::string exportToSVG(const std::vector<BrainFrame>& frames) {
    std::ostringstream oss;
    oss << "<svg xmlns='http://www.w3.org/2000/svg' viewBox='0 0 500 500'>\n";
    for (const auto& f : frames) {
        for (const auto& r : f.regions) {
            oss << "<circle cx='" << (r.x * 400 + 50) << "' cy='" << (r.y * 400 + 50)
                << "' r='" << (r.intensity * 20 + 5) << "' fill='red' opacity='0.5'/>\n";
        }
    }
    oss << "</svg>";
    return oss.str();
}

void exportToPNG(const std::vector<BrainFrame>& frames, const AppConfig& config) {
    int w = 256, h = 256;
    for (size_t fIdx = 0; fIdx < frames.size(); ++fIdx) {
        std::string filename = "frame_" + std::to_string(fIdx) + ".png";
        std::ofstream ofs(filename, std::ios::binary);
        ofs.write("\x89PNG\r\n\x1a\n", 8);
        std::vector<unsigned char> ihdr = {0,0,1,0, 0,0,1,0, 8,2,0,0,0};
        write32(ofs, 13); ofs.write("IHDR", 4); ofs.write((char*)ihdr.data(), 13);
        std::vector<unsigned char> ihdr_chunk = {'I','H','D','R'}; ihdr_chunk.insert(ihdr_chunk.end(), ihdr.begin(), ihdr.end());
        write32(ofs, crc32(ihdr_chunk.data(), ihdr_chunk.size()));
        std::vector<unsigned char> pixels((w * 3 + 1) * h, 0);
        for(int y=0; y<h; y++) {
            pixels[y*(w*3+1)] = 0;
            for(const auto& r : frames[fIdx].regions) {
                int cx = (int)(r.x * 255), cy = (int)(r.y * 255);
                if(std::abs(cy-y) < 10) {
                    for(int x=std::max(0,cx-10); x<std::min(256,cx+10); x++) {
                        pixels[y*(w*3+1) + 1 + x*3] = (unsigned char)(r.intensity*255);
                    }
                }
            }
        }
        std::vector<unsigned char> zlib = {0x78, 0x01, 0x01};
        unsigned int a = 1, b = 0; adler32(pixels.data(), pixels.size(), a, b);
        unsigned short len = (unsigned short)pixels.size(), nlen = ~len;
        zlib.push_back(len & 0xFF); zlib.push_back(len >> 8);
        zlib.push_back(nlen & 0xFF); zlib.push_back(nlen >> 8);
        zlib.insert(zlib.end(), pixels.begin(), pixels.end());
        zlib.push_back(b >> 8); zlib.push_back(b & 0xFF); zlib.push_back(a >> 8); zlib.push_back(a & 0xFF);
        std::vector<unsigned char> idat_chunk = {'I','D','A','T'}; idat_chunk.insert(idat_chunk.end(), zlib.begin(), zlib.end());
        write32(ofs, zlib.size()); ofs.write((char*)idat_chunk.data(), idat_chunk.size());
        write32(ofs, crc32(idat_chunk.data(), idat_chunk.size()));
        write32(ofs, 0); ofs.write("IEND", 4); write32(ofs, 0xAE426082);
    }
}

void exportToBMP(const std::vector<BrainFrame>& frames, const AppConfig& config) {
    int w = 256, h = 256;
    int frameIdx = 0;
    for (const auto& f : frames) {
        std::string filename = "frame_" + std::to_string(frameIdx++) + ".bmp";
        std::ofstream ofs(filename, std::ios::binary);
        unsigned char fileHeader[14] = {'B','M', 0,0,0,0, 0,0, 0,0, 54,0,0,0};
        unsigned char infoHeader[40] = {40,0,0,0, 0,0,0,0, 0,0,0,0, 1,0, 24,0};
        int size = 54 + 3*w*h;
        fileHeader[2] = (unsigned char)(size); fileHeader[3] = (unsigned char)(size >> 8);
        fileHeader[4] = (unsigned char)(size >> 16); fileHeader[5] = (unsigned char)(size >> 24);
        infoHeader[4] = (unsigned char)(w); infoHeader[5] = (unsigned char)(w >> 8);
        infoHeader[6] = (unsigned char)(w >> 16); infoHeader[7] = (unsigned char)(w >> 24);
        infoHeader[8] = (unsigned char)(h); infoHeader[9] = (unsigned char)(h >> 8);
        infoHeader[10] = (unsigned char)(h >> 16); infoHeader[11] = (unsigned char)(h >> 24);
        ofs.write((char*)fileHeader, 14); ofs.write((char*)infoHeader, 40);
        std::vector<unsigned char> pixels(3*w*h, 0);
        for (const auto& r : f.regions) {
            int cx = (int)(r.x * 255), cy = (int)(r.y * 255);
            for(int dy=-5; dy<=5; ++dy) for(int dx=-5; dx<=5; ++dx) {
                int nx = cx+dx, ny = cy+dy;
                if(nx>=0 && nx<w && ny>=0 && ny<h) {
                    int i = (ny*w + nx)*3;
                    pixels[i+2] = (unsigned char)(r.intensity*255);
                }
            }
        }
        ofs.write((char*)pixels.data(), pixels.size());
    }
}

void exportToGIF(const std::vector<BrainFrame>& frames, const AppConfig& /* config */) {
    std::ofstream ofs("brain.gif", std::ios::binary);
    if (!ofs.is_open()) return;
    ofs.write("GIF89a", 6);
    unsigned short w = 256, h = 256;
    ofs.write((char*)&w, 2); ofs.write((char*)&h, 2);
    unsigned char fields = 0xF7; ofs.write((char*)&fields, 1);
    unsigned char zero = 0; ofs.write((char*)&zero, 1); ofs.write((char*)&zero, 1);
    for(int i=0; i<256; ++i) {
        unsigned char c = (unsigned char)i;
        ofs.write((char*)&c, 1); ofs.write((char*)&c, 1); ofs.write((char*)&c, 1);
    }
    ofs.write("\x21\xFF\x0BNetscape2.0\x03\x01\x00\x00\x00", 19);
    for (const auto& f : frames) {
        ofs.write("\x21\xF9\x04\x04\x0A\x00\x00\x00", 8);
        ofs.write("\x2C\x00\x00\x00\x00", 5);
        ofs.write((char*)&w, 2); ofs.write((char*)&h, 2);
        unsigned char loc = 0; ofs.write((char*)&loc, 1);
        unsigned char lzw_min = 8; ofs.write((char*)&lzw_min, 1);
        std::vector<unsigned char> lzw;
        auto pack = [&](int code, int size) {
            static int buf=0, bits=0;
            buf |= (code << bits); bits += size;
            while(bits >= 8) { lzw.push_back(buf & 0xFF); buf >>= 8; bits -= 8; }
        };
        pack(0x80, 9);
        for(int i=0; i<w*h; i++) {
            unsigned char p = 0;
            for(const auto& r : f.regions) if(std::abs(r.x*255-(i%256))<10 && std::abs(r.y*255-(i/256))<10) p=(unsigned char)(r.intensity*255);
            pack(p, 9);
        }
        pack(0x81, 9);
        for(size_t j=0; j<lzw.size(); j+=255) {
            unsigned char len = (unsigned char)std::min((size_t)255, lzw.size()-j);
            ofs.write((char*)&len, 1); ofs.write((char*)lzw.data()+j, len);
        }
        ofs.write("\x00", 1);
    }
    ofs.write("\x3B", 1);
}
