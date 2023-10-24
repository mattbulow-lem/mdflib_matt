
#include "readCanKingTxt.h"

bool containsHexValue(const std::string& str) {
    for (char c : str) {
        if ((c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f')) {
            return true;
        }
    }
    return false;
}


std::vector<DataFrame> read_txt_file(std::string file_path) {
    std::ifstream file(file_path);
    if (!file.is_open()) {
        std::cerr << "Error opening file." << std::endl;
        return {};
    }

    std::vector<DataFrame> dataFrames; // Vector to store instances of the DataFrame class

    std::string line;
    bool skipFirstLine = true; // Flag to skip the first line
    int index{ 0 };
    while (std::getline(file, line)) {
        index++;
        if (skipFirstLine) {
            skipFirstLine = false;
            continue; // Skip the first line
        }
        // Check for the last line
        if (file.peek() == EOF) {
            break; // Exit the loop if this is the last line
        }

        // Assuming the fields have fixed lengths of 10, 10, and 4 characters
        DataFrame df;

        // If DLC is zero then skip line, tell operator
        try {
            df.DLC = static_cast<uint8_t>(std::stoul(line.substr(23, 1)));
        }
        catch (...) {
            std::cout << "Line " << index << " skipped due to empty DLC" << std::endl;
            continue;
        }
        //std::string substr(size_t pos, size_t len) const;
        df.Timestamp = std::stod(line.substr(57, 14));
        df.BusChannel = static_cast<uint8_t>(std::stoul(line.substr(0, 2))+1);
        df.ID_str = line.substr(5, 9);
        df.IDE_str = line.substr(15, 1);
        df.DataLength = df.DLC;
        df.DataBytes_str.push_back(line.substr(25, 3));
        df.DataBytes_str.push_back(line.substr(29, 3));
        df.DataBytes_str.push_back(line.substr(33, 3));
        df.DataBytes_str.push_back(line.substr(37, 3));
        df.DataBytes_str.push_back(line.substr(41, 3));
        df.DataBytes_str.push_back(line.substr(45, 3));
        df.DataBytes_str.push_back(line.substr(49, 3));
        df.DataBytes_str.push_back(line.substr(53, 3));
        df.Dir_str = line.substr(72, 1);

        dataFrames.push_back(df); // Store the parsed data in the vector
    }

    // Now you have the data stored in instances of the DataFrame class

    bool hex_encoded{ false };

    // now need to scan ID_str to find out if any are HEX
    index = 0;
    for (const DataFrame& df : dataFrames) {
        if (containsHexValue(df.ID_str)) {
            hex_encoded = true;
            std::cout << "ID contains a hex character (A-F)" << std::endl;
            break;
        }
        // only scan first 500 lines of trace file for HEX data
        if (index > 500) {
            break;
        }
        index++;
    }

   
    // convert string data to numbers
    for (DataFrame& df : dataFrames) {
        // Convert direction to boolean -> 0:Rx, 1 : Tx
        if (df.Dir_str == "R") {
            df.Dir = false;
        }
        else {
            df.Dir = true;
        }
        if (hex_encoded) {
            // convert hex strings to decimal numbers
            df.ID = std::stoul(df.ID_str, nullptr, 16);
            for (int i = 0; i < df.DataBytes_str.size(); i++) {
                try {
                    df.DataBytes.push_back(static_cast<uint8_t>(std::stoul(df.DataBytes_str[i], nullptr, 16)));
                }
                catch (...) {
                    df.DataBytes.push_back(uint8_t(0));
                }
            }
        }
        else {
            // convert dec strings to real numbers
            df.ID = std::stoul(df.ID_str);
            for (int i = 0; i < df.DataBytes_str.size(); i++) {
                try {
                    df.DataBytes.push_back(static_cast<uint8_t>(std::stoul(df.DataBytes_str[i])));
                }
                catch (...) {
                    df.DataBytes.push_back(uint8_t(0));
                }
            }
        }
        // Convert extended ID flag 'X' to a boolean
        if (df.IDE_str == "X") {
            df.IDE = true;
            // Set the 31st bit to true
            df.ID |= uint32_t(0x80000000);
        }
        else {
            df.IDE = false;
        }
    }

    std::cout << "Successfully read input file into vector of DataFrame objects" << std::endl;

    // with large files, printing to screen takes forever.
    bool verbose{ false };
    if (verbose) {
        std::cout << "Time   Chn   ID      IDE DLC DataLength    DataBytes            Dir" << std::endl;
        // You can iterate through the vector to access and display the data
        for (const DataFrame& df : dataFrames) {
            std::cout << df.Timestamp << "  ";
            std::cout << static_cast<unsigned>(df.BusChannel) << "  ";
            std::cout << std::left << std::setw(int(10)) << df.ID << "  ";
            std::cout << df.IDE << "  ";
            std::cout << static_cast<unsigned>(df.DLC) << "  ";
            std::cout << static_cast<unsigned>(df.DataLength) << "  ";
            // Display the DataBytes array elements
            for (int i = 0; i < df.DataBytes.size(); i++) {
                std::cout << std::left << std::setw(int(3)) << static_cast<unsigned>(df.DataBytes[i]) << " ";
            }
            std::cout << " " << df.Dir << std::endl;
        }
    }

    file.close(); // The file is automatically closed when 'file' goes out of scope
    return dataFrames;
}
