#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

std::string readTextFromFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        return "";
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
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
    
    return 0;
}