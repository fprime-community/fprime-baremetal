#ifndef SimFileSystem_H
#define SimFileSystem_H

#include <vector>
#include <string>
#include <unordered_map>
#include <map>
#include <Os/File.hpp>

class SimFileSystem {
public:
    enum class FileState {
        DOES_NOT_EXIST,
        OPENED,
        CLOSED
    };

    static const int max_files_per_bin;
    static const int max_bins;

    SimFileSystem(int n_bins, int n_files_per_bin, std::size_t max_file_size);

    bool canOpenFile() const; 
    std::string openFile();

    bool canCloseFile() const;
    std::string closeFile();

    std::string writeToFile(const std::vector<uint8_t>& data);

    Os::File *getFileDesc(const std::string filename);

    std::size_t getFilePos(const std::string filename);

    std::unordered_map<std::string, SimFileSystem::FileState> getAllFileStates() const;

    bool canWriteFile() const;

private:
    std::unordered_map<int, std::vector<std::string>> bins;
    std::unordered_map<std::string, FileState> file_states;
    std::unordered_map<std::string, Os::File> file_desc;
    std::unordered_map<std::string, std::string> file_contents;
    std::unordered_map<std::string, std::size_t> file_positions;
    std::size_t max_file_size;  // The maximum size of a file
};

#endif // SimFileSystem_H
