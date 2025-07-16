// ======================================================================
// \title Os/Baremetal/FileSystem.hpp
// \brief Baremetal FileSystem definitions for Os::FileSystem
// ======================================================================
#ifndef OS_BAREMETAL_FILESYSTEM_HPP
#define OS_BAREMETAL_FILESYSTEM_HPP

#include "Os/FileSystem.hpp"

namespace Os {
namespace Baremetal {
namespace FileSystem {

struct BaremetalFileSystemHandle : public FileSystemHandle {};

//! \brief Baremetal implementation of Os::FileSystem
//!
//! Baremetal implementation of `FileSystemInterface` for use as a delegate class handling error-only fileSystem
//! operations.
//!
class BaremetalFileSystem : public FileSystemInterface {
  public:
    //! \brief constructor
    BaremetalFileSystem() = default;

    //! \brief destructor
    ~BaremetalFileSystem() override = default;

    // ------------------------------------------------------------
    // Implementation-specific member functions - overrides
    // ------------------------------------------------------------

    //! \brief Remove a directory at the specified path
    //!
    //! It is invalid to pass `nullptr` as the path.
    //!
    //! \param path The path of the directory to remove
    //! \return Status of the operation
    Status _removeDirectory(const char* path) override;

    //! \brief Remove a file at the specified path
    //!
    //! It is invalid to pass `nullptr` as the path.
    //!
    //! \param path The path of the file to remove
    //! \return Status of the operation
    Status _removeFile(const char* path) override;

    //! \brief Rename a file from source to destination
    //!
    //! If the rename fails due to a cross-device operation, this function should return EXDEV_ERROR
    //! and moveFile can be used instead to force a copy-and-remove.
    //!
    //! It is invalid to pass `nullptr` as sourcePath or destPath.
    //!
    //! \param sourcePath The path of the source file
    //! \param destPath The path of the destination file
    //! \return Status of the operation
    Status _rename(const char* sourcePath, const char* destPath) override;

    //! \brief Get filesystem free and total space in bytes on the filesystem containing the specified path
    //!
    //! It is invalid to pass `nullptr` as the path.
    //!
    //! \param path The path on the filesystem to query
    //! \param totalBytes Reference to store the total bytes on the filesystem
    //! \param freeBytes Reference to store the free bytes on the filesystem
    //! \return Status of the operation
    Status _getFreeSpace(const char* path, FwSizeType& totalBytes, FwSizeType& freeBytes) override;

    BaremetalFileSystem::Status _getPathType(const char* path, PathType& pathType) override;

    //! \brief Get the current working directory
    //!
    //! Writes the current working directory path to the provided buffer of size bufferSize.
    //! If the buffer is too small to hold the full path, the function will return BUFFER_TOO_SMALL.
    //!
    //! It is invalid to pass `nullptr` as the path.
    //! It is invalid to pass a bufferSize of 0.
    //!
    //! \param path Buffer to store the current working directory path
    //! \param bufferSize Size of the buffer
    //! \return Status of the operation
    Status _getWorkingDirectory(char* path, FwSizeType bufferSize) override;

    //! \brief Change the current working directory to the specified path
    //!
    //! It is invalid to pass `nullptr` as the path.
    //!
    //! \param path The path of the new working directory
    //! \return Status of the operation
    Status _changeWorkingDirectory(const char* path) override;

    //! \brief Get the raw FileSystem handle
    //!
    //! Gets the raw FileSystem handle from the implementation. Note: users must include the implementation specific
    //! header to make any real use of this handle. Otherwise it *must* be passed as an opaque type.
    //!
    //! \return raw fileSystem handle
    FileSystemHandle* getHandle() override;

  private:
    //! FileSystem handle for BaremetalFileSystem
    BaremetalFileSystemHandle m_handle;
};  // class BaremetalFileSystem

}  // namespace FileSystem
}  // namespace Baremetal
}  // namespace Os
#endif  // OS_BAREMETAL_FILESYSTEM_HPP
