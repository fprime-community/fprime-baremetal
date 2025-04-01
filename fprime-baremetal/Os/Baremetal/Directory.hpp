// ======================================================================
// \title Os/Baremetal/Directory.hpp
// \brief Baremetal definitions for Os::Directory
// ======================================================================
#ifndef OS_BAREMETAL_DIRECTORY_HPP
#define OS_BAREMETAL_DIRECTORY_HPP
#include <Os/Directory.hpp>
#include <climits>

namespace Os {
namespace Baremetal {
namespace Directory {

struct BaremetalDirectoryHandle : public DirectoryHandle {
    static constexpr PlatformSizeType INVALID_DIR_DESCRIPTOR = std::numeric_limits<PlatformSizeType>::max();

    PlatformSizeType m_dir_index = INVALID_DIR_DESCRIPTOR;  // The current open directory
    PlatformSizeType m_file_index = 0;                      // Keep track of the last file index to read
};

//! \brief Baremetal implementation of Os::Directory
//!
//! Baremetal implementation of `DirectoryInterface` for use as a delegate class handling error-only file operations.
class BaremetalDirectory : public DirectoryInterface {
  public:
    //! \brief constructor
    BaremetalDirectory();

    //! \brief destructor
    ~BaremetalDirectory() = default;

    //! \brief return the underlying mutex handle (implementation specific)
    //! \return internal mutex handle representation
    DirectoryHandle* getHandle() override;

    // ------------------------------------------------------------
    // Implementation-specific Directory member functions
    // ------------------------------------------------------------

    //! \brief Open or create a directory
    //!
    //! This call will return success if the directory name (path) matches the file bin naming scheme.
    //! This does not affect the state of any files.
    //!
    //! It is invalid to pass `nullptr` as the path.
    //!
    //! \param path: path of directory to open
    //! \param mode: Unused
    //! \return status of the operation
    Status open(const char* path, OpenMode mode) override;

    //! \brief Rewind directory stream
    //!
    //! Each read operation moves the seek position forward. This function resets the seek position to the beginning.
    //!
    //! \return status of the operation
    Status rewind() override;

    //! \brief Get next filename from directory stream
    //!
    //! Writes at most buffSize characters of the file name to fileNameBuffer.
    //! This function skips the current directory (.) and parent directory (..) entries.
    //! Returns NO_MORE_FILES if there are no more files to read from the buffer.
    //!
    //! It is invalid to pass `nullptr` as fileNameBuffer.
    //!
    //! \param fileNameBuffer: buffer to store filename
    //! \param buffSize: size of fileNameBuffer
    //! \return status of the operation
    Status read(char* fileNameBuffer, FwSizeType buffSize) override;

    //! \brief Close directory
    void close() override;

  private:
    //! Handle for BaremetalDirectory
    BaremetalDirectoryHandle m_handle;
};

}  // namespace Directory
}  // namespace Baremetal
}  // namespace Os
#endif  // OS_BAREMETAL_DIRECTORY_HPP
