#ifndef TUNE_FILE_UTILS_H
#define TUNE_FILE_UTILS_H

#include <filesystem>
#include <fstream>

template<typename T>
void writeFileAtomically(const std::filesystem::path& filePath, const T& value) {
    std::filesystem::path tempPath = filePath;
    tempPath += ".tmp";

    try {
        std::ofstream outFile(tempPath, std::ios::binary | std::ios::trunc);
        if(!outFile)
            throw std::ios_base::failure("Failed to open temporary file");

        outFile << value;
        outFile.flush();

        if(!outFile)
            throw std::ios_base::failure("Failed to write temporary file");

        outFile.close();

        std::filesystem::rename(tempPath, filePath);
    } catch(...) {
        std::error_code ignore;
        std::filesystem::remove(tempPath, ignore);
        throw;
    }
}

template<typename T>
bool tryReadFile(const std::filesystem::path& filePath, T& value) {
    std::ifstream inputFile(filePath, std::ios::binary);
    if(!inputFile)
        return false;

    T candidate{};
    inputFile >> candidate;
    if(!inputFile)
        return false;

    value = std::move(candidate);
    return true;
}

template<typename T>
bool readFileWithTempFallback(const std::filesystem::path& filePath, T& value) {
    if(tryReadFile(filePath, value))
        return true;

    std::filesystem::path tempPath = filePath;
    tempPath += ".tmp";
    return tryReadFile(tempPath, value);
}

#endif