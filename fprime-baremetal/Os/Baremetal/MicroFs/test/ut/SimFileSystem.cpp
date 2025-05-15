#include <random>
#include <vector>
#include <algorithm>
#include <iostream>
#include "SimFileSystem.h"
#include <map>


const int SimFileSystem::max_files_per_bin = 10;
const int SimFileSystem::max_bins = 10;

SimFileSystem::SimFileSystem(int n_bins, int n_files_per_bin, std::size_t max_file_size)
    : max_file_size(max_file_size) {

    for (int i = 0; i < n_bins; ++i) {
        bins[i] = std::vector<std::string>();
        for (int j = 0; j < n_files_per_bin; ++j) {
            bins[i].push_back("file" + std::to_string(j));
            file_states["/bin" + std::to_string(i) + "/file" + std::to_string(j)] = FileState::DOES_NOT_EXIST;
        }
    }
}

bool SimFileSystem::canOpenFile() const {
    // Check all bins for nonexistent or closed files
    for (const auto& bin_pair : bins) {
        for (const auto& file : bin_pair.second) {
            FileState state = file_states.at("/bin" + std::to_string(bin_pair.first) + "/" + file);
            if (state == FileState::DOES_NOT_EXIST || state == FileState::CLOSED) {
                return true;  // Found a file that can be opened
            }
        }
    }

    return false;  // No files that can be opened were found
}


std::string SimFileSystem::openFile() {
    std::vector<std::string> available_files;

    // Find all nonexistent or closed files
    for (const auto& bin_pair : bins) {
        for (const auto& file : bin_pair.second) {
            FileState state = file_states["/bin" + std::to_string(bin_pair.first) + "/" + file];
            if (state == FileState::DOES_NOT_EXIST || state == FileState::CLOSED) {
                available_files.push_back("/bin" + std::to_string(bin_pair.first) + "/" + file);
            }
        }
    }

    // If there are no available files, return
    if (available_files.empty()) {
        std::cout << "No available files to open.\n";
        return "";
    }

    // Randomly select an available file to open
    std::random_device rd;
    std::mt19937 generator(rd());
    std::uniform_int_distribution<> distribution(0, available_files.size() - 1);
    std::string file_to_open = available_files[distribution(generator)];
    file_states[file_to_open] = FileState::OPENED;

     // Reset the position pointer
    file_positions[file_to_open] = 0;

    return file_to_open;
}


bool SimFileSystem::canCloseFile() const {
    // Check all bins for opened files
    for (const auto& bin_pair : bins) {
        for (const auto& file : bin_pair.second) {
            FileState state = file_states.at("/bin" + std::to_string(bin_pair.first) + "/" + file);
            if (state == FileState::OPENED) {
                return true;  // Found a file that can be closed
            }
        }
    }

    return false;  // No files that can be closed were found
}


std::string SimFileSystem::closeFile() {
    std::vector<std::string> open_files;

    // Find all open files
    for (const auto& bin_pair : bins) {
        for (const auto& file : bin_pair.second) {
            if (file_states["/bin" + std::to_string(bin_pair.first) + "/" + file] == FileState::OPENED) {
                open_files.push_back("/bin" + std::to_string(bin_pair.first) + "/" + file);
            }
        }
    }

    // If there are no open files, return
    if (open_files.empty()) {
        std::cout << "No open files available to close.\n";
        return "";
    }

    // Randomly select an open file to close
    std::random_device rd;
    std::mt19937 generator(rd());
    std::uniform_int_distribution<> distribution(0, open_files.size() - 1);
    std::string file_to_close = open_files[distribution(generator)];
    file_states[file_to_close] = FileState::CLOSED;

    return file_to_close;
}

std::string SimFileSystem::writeToFile(const std::vector<uint8_t>& data) {
    std::vector<std::string> openFiles;

    for (const auto& file_state_pair : file_states) {
        if (file_state_pair.second == FileState::OPENED) {
            openFiles.push_back(file_state_pair.first);
        }
    }

    // Check if there are any open files
    if (openFiles.empty()) {
        std::cout << "No open files to write to" << std::endl;
        return "";
    }

    // Select a random open file
    int random_file_idx = rand() % openFiles.size();
    std::string selected_file = openFiles[random_file_idx];

     // Append data to the existing file content
    file_contents[selected_file] += std::string(data.begin(), data.end());

    // Update the position pointer
    file_positions[selected_file] += data.size();

    return selected_file;
}


std::unordered_map<std::string, SimFileSystem::FileState> SimFileSystem::getAllFileStates() const {
    return file_states;
}

Os::File *SimFileSystem::getFileDesc(const std::string filename) {
    return &this->file_desc[filename];
}

std::size_t SimFileSystem::getFilePos(const std::string filename) {
    return file_positions[filename];
}



bool SimFileSystem::canWriteFile() const {
    for (const auto& file_state_pair : file_states) {
        if (file_state_pair.second == FileState::OPENED) {
            return true;
        }
    }
    return false;
}




