// ======================================================================
// \title Os/Baremetal/DefaultFile.cpp
// \brief sets default Os::File to baremetal implementation via linker
// ======================================================================
#include "Os/Delegate.hpp"
#include "Os/Directory.hpp"
#include "Os/File.hpp"
#include "Os/FileSystem.hpp"
#include "fprime-baremetal/Os/Baremetal/Directory.hpp"
#include "fprime-baremetal/Os/Baremetal/File.hpp"
#include "fprime-baremetal/Os/Baremetal/FileSystem.hpp"

namespace Os {
FileInterface* FileInterface::getDelegate(FileHandleStorage& aligned_new_memory, const FileInterface* to_copy) {
    return Os::Delegate::makeDelegate<FileInterface, Os::Baremetal::File::BaremetalFile>(aligned_new_memory, to_copy);
}
FileSystemInterface* FileSystemInterface::getDelegate(FileSystemHandleStorage& aligned_new_memory) {
    return Os::Delegate::makeDelegate<FileSystemInterface, Os::Baremetal::FileSystem::BaremetalFileSystem>(
        aligned_new_memory);
}
DirectoryInterface* DirectoryInterface::getDelegate(DirectoryHandleStorage& aligned_new_memory) {
    return Os::Delegate::makeDelegate<DirectoryInterface, Os::Baremetal::Directory::BaremetalDirectory>(
        aligned_new_memory);
}
}  // namespace Os
