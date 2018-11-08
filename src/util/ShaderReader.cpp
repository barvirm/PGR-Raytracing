#include <util/ShaderReader.h>
#include <fstream>
#include <iostream>


std::string util::loadFile(const std::string &filePath) {
    std::ifstream f(filePath.c_str());
    if (!f.is_open()) {
        std::cerr << "Cannot open file " << filePath << std::endl;
        return "";
    }
    std::string str((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
    f.close();
    return str;
}