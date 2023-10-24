#pragma once

#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <vector>

class DataFrame {
public:
    double Timestamp{ 0 };
    uint8_t BusChannel{ 0 };
    uint32_t ID{ 0 };
    std::string ID_str;
    bool IDE{ false };
    std::string IDE_str;
    uint8_t DLC{ 0 };
    uint8_t DataLength{ 0 };
    std::vector<uint8_t> DataBytes;
    std::vector<std::string> DataBytes_str;
    bool Dir{ false };
    std::string Dir_str;
};

bool containsHexValue(const std::string& str);

std::vector<DataFrame> read_txt_file(std::string file_path);

