#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <regex>

struct Coordinate {
    double latitude;
    double longitude;
    std::string original_text;
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
            
            if (coord.latitude >= -90.0 && coord.latitude <= 90.0 &&
                coord.longitude >= -180.0 && coord.longitude <= 180.0) {
                coordinates.push_back(coord);
            }
            
        } catch (const std::exception& e) {

        }
        
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
            
            if (coord.latitude >= -90.0 && coord.latitude <= 90.0 &&
                coord.longitude >= -180.0 && coord.longitude <= 180.0) {
                coordinates.push_back(coord);
            }
            
        } catch (const std::exception& e) {
            
        }
        
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
    
    std::cout << "=== TEXT CONTENT ===" << std::endl;
    std::cout << text << std::endl;
    std::cout << "====================" << std::endl;
    std::cout << "Text length: " << text.length() << " characters" << std::endl;
    
    auto basic_coordinates = findBasicCoordinates(text);
    auto direction_coordinates = findDirectionCoordinates(text);
    
    std::vector<Coordinate> all_coordinates;
    all_coordinates.insert(all_coordinates.end(), basic_coordinates.begin(), basic_coordinates.end());
    all_coordinates.insert(all_coordinates.end(), direction_coordinates.begin(), direction_coordinates.end());
    
    std::cout << "\n=== FOUND COORDINATES ===" << std::endl;
    if (all_coordinates.empty()) {
        std::cout << "No coordinates found" << std::endl;
    } else {
        for (size_t i = 0; i < all_coordinates.size(); i++) {
            std::cout << (i + 1) << ". Lat: " << all_coordinates[i].latitude << ", Lon: " << all_coordinates[i].longitude << std::endl;
        }
    }
    
    std::cout << "\n=== SUMMARY ===" << std::endl;
    std::cout << "Basic format: " << basic_coordinates.size() << " coordinates" << std::endl;
    std::cout << "Direction format: " << direction_coordinates.size() << " coordinates" << std::endl;
    std::cout << "Total: " << all_coordinates.size() << " coordinates" << std::endl;
    
    return 0;
}