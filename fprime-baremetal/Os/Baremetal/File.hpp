// ======================================================================
// \title Os/Baremetal/File.hpp
// \brief baremetal implementation for Os::File, header and test definitions
// ======================================================================
#ifndef OS_BAREMETAL_FILE_HPP
#define OS_BAREMETAL_FILE_HPP

#include <Os/File.hpp>

namespace Os {
namespace Baremetal {
namespace File {

//! FileHandle class definition for baremetal implementations.
//!
struct BaremetalFileHandle : public FileHandle {
    static constexpr PlatformIntType INVALID_FILE_DESCRIPTOR = -1;

    //! Baremetal file descriptor
    PlatformIntType m_file_descriptor = INVALID_FILE_DESCRIPTOR;
    //! File mode
    Os::FileInterface::Mode m_mode = Os::File::Mode::OPEN_NO_MODE;
};

//! \brief baremetal implementation of Os::File
//!
//! Baremetal implementation of `FileInterface` for use as a delegate class handling baremetal file operations.
//! Baremetal files use standard `open`, `read`, and `write` baremetal calls. The handle is represented as a
//! `BaremetalFileHandle` which wraps a single `int` type file descriptor used in those API calls.
//!
class BaremetalFile : public FileInterface {
  public:
    //! \brief constructor
    //!
    BaremetalFile() = default;

    //! \brief destructor
    //!
    ~BaremetalFile() override;

    // ------------------------------------
    // Functions overrides
    // ------------------------------------

    //! \brief open file with supplied path and mode
    //!
    //! Open the file passed in with the given mode. If overwrite is set to OVERWRITE, then opening files in
    //! OPEN_CREATE mode will clobber existing files. Set overwrite to NO_OVERWRITE to preserve existing files.
    //! The status of the open request is returned from the function call. Delegates to the chosen
    //! implementation's `open` function.
    //!
    //! It is invalid to send `nullptr` as the path.
    //! It is invalid to supply `mode` as a non-enumerated value.
    //! It is invalid to supply `overwrite` as a non-enumerated value.
    //!
    //! \param path: c-string of path to open
    //! \param mode: file operation mode
    //! \param overwrite: overwrite existing file on create
    //! \return: status of the open
    //!
    Os::FileInterface::Status open(const char* path, Mode mode, OverwriteType overwrite) override;

    //! \brief determine if the file is open
    //! \return true if file is open, false otherwise
    //!
    bool isOpen() const;

    //! \brief close the file, if not opened then do nothing
    //!
    //! Closes the file, if open. Otherwise this function does nothing. Delegates to the chosen implementation's
    //! `closeInternal` function. `mode` is set to `OPEN_NO_MODE`.
    //!
    void close() override;

    //! \brief get size of currently open file
    //!
    //! Get the size of the currently open file and fill the size parameter. Return status of the operation.
    //! \param size: output parameter for size.
    //! \return OP_OK on success otherwise error status
    //!
    Status size(FwSizeType& size_result) override;

    //! \brief get file pointer position of the currently open file
    //!
    //! Get the current position of the read/write pointer of the open file.
    //! \param position: output parameter for size.
    //! \return OP_OK on success otherwise error status
    //!
    Status position(FwSizeType& position_result) override;

    //! \brief pre-allocate file storage
    //!
    //! Pre-allocates file storage with at least `length` storage starting at `offset`. No-op on implementations
    //! that cannot pre-allocate.
    //!
    //! It is invalid to pass a negative `offset`.
    //! It is invalid to pass a negative `length`.
    //!
    //! \param offset: offset into file
    //! \param length: length after offset to preallocate
    //! \return OP_OK on success otherwise error status
    //!
    Status preallocate(FwSizeType offset, FwSizeType length) override;

    //! \brief seek the file pointer to the given offset
    //!
    //! Seek the file pointer to the given `offset`. If `seekType` is set to `ABSOLUTE` then the offset is calculated
    //! from the start of the file, and if it is set to `CURRENT` it is calculated from the current position.
    //!
    //! \param offset: offset to seek to
    //! \param seekType: `ABSOLUTE` for seeking from beginning of file, `CURRENT` to use current position.
    //! \return OP_OK on success otherwise error status
    //!
    Status seek(FwSignedSizeType offset, SeekType seekType) override;

    //! \brief flush file contents to storage
    //!
    //! Flushes the file contents to storage (i.e. out of the OS cache to disk). Does nothing in implementations
    //! that do not support flushing.
    //!
    //! \return OP_OK on success otherwise error status
    //!
    Status flush() override;

    //! \brief read data from this file into supplied buffer bounded by size
    //!
    //! Read data from this file up to the `size` and store it in `buffer`.  When `wait` is set to `WAIT`, this
    //! will block until the requested size has been read successfully read or the end of the file has been
    //! reached. When `wait` is set to `NO_WAIT` it will return whatever data is currently available.
    //!
    //! `size` will be updated to the count of bytes actually read. Status will reflect the success/failure of
    //! the read operation.
    //!
    //! It is invalid to pass `nullptr` to this function call.
    //! It is invalid to pass a negative `size`.
    //! It is invalid to supply wait as a non-enumerated value.
    //!
    //! \param buffer: memory location to store data read from file
    //! \param size: size of data to read
    //! \param wait: `WAIT` to wait for data, `NO_WAIT` to return what is currently available
    //! \return OP_OK on success otherwise error status
    //!
    Status read(U8* buffer, FwSizeType& size, WaitType wait) override;

    //! \brief read data from this file into supplied buffer bounded by size
    //!
    //! Write data to this file up to the `size` from the `buffer`.  When `wait` is set to `WAIT`, this
    //! will block until the requested size has been written successfully to disk. When `wait` is set to
    //! `NO_WAIT` it will return once the data is sent to the OS.
    //!
    //! `size` will be updated to the count of bytes actually written. Status will reflect the success/failure of
    //! the read operation.
    //!
    //! It is invalid to pass `nullptr` to this function call.
    //! It is invalid to pass a negative `size`.
    //! It is invalid to supply wait as a non-enumerated value.
    //!
    //! \param buffer: memory location to store data read from file
    //! \param size: size of data to read
    //! \param wait: `WAIT` to wait for data to write to disk, `NO_WAIT` to return what is currently available
    //! \return OP_OK on success otherwise error status
    //!
    Status write(const U8* buffer, FwSizeType& size, WaitType wait) override;

    //! \brief returns the raw file handle
    //!
    //! Gets the raw file handle from the implementation. Note: users must include the implementation specific
    //! header to make any real use of this handle. Otherwise it//!must* be passed as an opaque type.
    //!
    //! \return raw file handle
    //!
    FileHandle* getHandle() override;

  private:
    //! File handle for BaremetalFile
    BaremetalFileHandle m_handle;
};
}  // namespace File
}  // namespace Baremetal
}  // namespace Os

#endif  // OS_BAREMETAL_FILE_HPP
