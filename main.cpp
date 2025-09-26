#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <regex>
#include <algorithm>

struct Coordinate {
    double latitude;
    double longitude;
    std::string original_text;
    std::string sentence;
};

std::string readTextFromFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        return "";
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

std::string findSentence(const std::string& text, const std::string& coordinate, size_t max_length = 200) {
    size_t pos = text.find(coordinate);
    if (pos == std::string::npos) {
        return coordinate;
    }
    
    size_t start = 0;
    size_t dot_pos = text.rfind('.', pos);
    if (dot_pos != std::string::npos && dot_pos > 0) {
        start = dot_pos + 1;
    }
    
    size_t end = text.find('.', pos);
    if (end == std::string::npos) {
        end = text.length();
    } else {
        end += 1;
    }
    
    std::string sentence = text.substr(start, end - start);
    
    sentence.erase(0, sentence.find_first_not_of(" \n\r\t"));
    sentence.erase(sentence.find_last_not_of(" \n\r\t") + 1);
    
    if (sentence.length() > max_length) {
        sentence = sentence.substr(0, max_length) + "...";
    }
    
    return sentence;
}

std::vector<Coordinate> findBasicCoordinates(const std::string& text) {
    std::vector<Coordinate> coordinates;
    std::regex pattern(R"((-?\d+\.\d+)\s+(-?\d+\.\d+))");
    std::smatch matches;
    std::string::const_iterator searchStart(text.cbegin());
    
    while (std::regex_search(searchStart, text.cend(), matches, pattern)) {
        try {
            Coordinate coord;
            coord.latitude = std::stod(matches[1]);
            coord.longitude = std::stod(matches[2]);
            coord.original_text = matches[0];
            coord.sentence = findSentence(text, coord.original_text);
            
            if (coord.latitude >= -90.0 && coord.latitude <= 90.0 &&
                coord.longitude >= -180.0 && coord.longitude <= 180.0) {
                coordinates.push_back(coord);
            }
        } catch (const std::exception& e) {}
        searchStart = matches.suffix().first;
    }
    return coordinates;
}

std::vector<Coordinate> findDirectionCoordinates(const std::string& text) {
    std::vector<Coordinate> coordinates;
    std::regex pattern(R"(\b([NS])(\d+\.\d+)\s*([WE])(\d+\.\d+)\b)", std::regex::icase);
    std::smatch matches;
    std::string::const_iterator searchStart(text.cbegin());
    
    while (std::regex_search(searchStart, text.cend(), matches, pattern)) {
        try {
            Coordinate coord;
            std::string lat_dir = matches[1];
            std::string lon_dir = matches[3];
            
            double lat = std::stod(matches[2]);
            double lon = std::stod(matches[4]);
            
            coord.latitude = (std::toupper(lat_dir[0]) == 'N') ? lat : -lat;
            coord.longitude = (std::toupper(lon_dir[0]) == 'E') ? lon : -lon;
            coord.original_text = matches[0];
            coord.sentence = findSentence(text, coord.original_text);
            
            if (coord.latitude >= -90.0 && coord.latitude <= 90.0 &&
                coord.longitude >= -180.0 && coord.longitude <= 180.0) {
                coordinates.push_back(coord);
            }
        } catch (const std::exception& e) {}
        searchStart = matches.suffix().first;
    }
    return coordinates;
}

std::vector<Coordinate> findCommaCoordinates(const std::string& text) {
    std::vector<Coordinate> coordinates;
    std::regex pattern(R"((-?\d+,\d+)°?[\s,]+(-?\d+,\d+)°?)");
    std::smatch matches;
    std::string::const_iterator searchStart(text.cbegin());
    
    while (std::regex_search(searchStart, text.cend(), matches, pattern)) {
        try {
            Coordinate coord;
            
            std::string lat_str = matches[1];
            std::string lon_str = matches[2];
            std::replace(lat_str.begin(), lat_str.end(), ',', '.');
            std::replace(lon_str.begin(), lon_str.end(), ',', '.');
            
            coord.latitude = std::stod(lat_str);
            coord.longitude = std::stod(lon_str);
            coord.original_text = matches[0];
            coord.sentence = findSentence(text, coord.original_text);
            
            if (coord.latitude >= -90.0 && coord.latitude <= 90.0 &&
                coord.longitude >= -180.0 && coord.longitude <= 180.0) {
                coordinates.push_back(coord);
            }
        } catch (const std::exception& e) {}
        searchStart = matches.suffix().first;
    }
    return coordinates;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cout << "Usage: " << argv[0] << " <text_file>" << std::endl;
        return 1;
    }
    
    std::string filename = argv[1];
    std::string text = readTextFromFile(filename);
    
    if (text.empty()) {
        std::cout << "Failed to read file or file is empty: " << filename << std::endl;
        return 1;
    }
    
    auto basic_coordinates = findBasicCoordinates(text);
    auto direction_coordinates = findDirectionCoordinates(text);
    auto comma_coordinates = findCommaCoordinates(text);
    
    std::vector<Coordinate> all_coordinates;
    all_coordinates.insert(all_coordinates.end(), basic_coordinates.begin(), basic_coordinates.end());
    all_coordinates.insert(all_coordinates.end(), direction_coordinates.begin(), direction_coordinates.end());
    all_coordinates.insert(all_coordinates.end(), comma_coordinates.begin(), comma_coordinates.end());
    
    std::cout << "=== FOUND COORDINATES ===" << std::endl;
    if (all_coordinates.empty()) {
        std::cout << "No coordinates found" << std::endl;
    } else {
        for (size_t i = 0; i < all_coordinates.size(); i++) {
            std::cout << (i + 1) << ". Lat: " << all_coordinates[i].latitude << ", Lon: " << all_coordinates[i].longitude << "\n   Original: '" << all_coordinates[i].original_text << "'"<< "\n   Sentence: '" << all_coordinates[i].sentence << "'"<< "\n" << std::endl;
        }
    }
    
    std::cout << "=== SUMMARY ===" << std::endl;
    std::cout << "Basic format: " << basic_coordinates.size() << " coordinates" << std::endl;
    std::cout << "Direction format: " << direction_coordinates.size() << " coordinates" << std::endl;
    std::cout << "Comma format: " << comma_coordinates.size() << " coordinates" << std::endl;
    std::cout << "Total: " << all_coordinates.size() << " coordinates" << std::endl;
    
    return 0;
}