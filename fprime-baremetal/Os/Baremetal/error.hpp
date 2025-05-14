// ======================================================================
// \title Os/Baremetal/error.hpp
// \brief header for baremetal errno conversion
// ======================================================================
#ifndef OS_BAREMETAL_ERRNO_HPP
#define OS_BAREMETAL_ERRNO_HPP
#include "Os/Condition.hpp"
#include "Os/Directory.hpp"
#include "Os/File.hpp"
#include "Os/FileSystem.hpp"
#include "Os/RawTime.hpp"
#include "Os/Task.hpp"

namespace Os {
namespace Baremetal {

//! Convert an errno representation of an error to the Os::File::Status representation.
//! \param errno_input: errno representation of the error
//! \return: Os::File::Status representation of the error
//!
Os::File::Status errno_to_file_status(int errno_input);

//! Convert an errno representation of an error to the Os::FileSystem::Status representation.
//! \param errno_input: errno representation of the error
//! \return: Os::FileSystem::Status representation of the error
//!
Os::FileSystem::Status errno_to_filesystem_status(int errno_input);

//! Convert an errno representation of an error to the Os::FileSystem::Status representation.
//! \param errno_input: errno representation of the error
//! \return: Os::Directory::Status representation of the error
//!
Os::Directory::Status errno_to_directory_status(int errno_input);

//! Convert an errno representation of an error to the Os::RawTime::Status representation.
//! \param errno_input: errno representation of the error
//! \return: Os::RawTime::Status representation of the error
//!
Os::RawTime::Status errno_to_rawtime_status(int errno_input);

//! Convert an baremetal task representation of an error to the Os::Task::Status representation.
//! \param baremetal_status: errno representation of the error
//! \return: Os::Task::Status representation of the error
//!
Os::Task::Status baremetal_status_to_task_status(int baremetal_status);

}  // namespace Baremetal
}  // namespace Os
#endif
