// ======================================================================
// \title fprime-baremetal/Os/Baremetal/DefaultTask.cpp
// \brief sets default Os::Task to baremetal implementation via linker
// ======================================================================
#include "Os/Delegate.hpp"
#include "Os/Task.hpp"
#include "fprime-baremetal/Os/Baremetal/Task.hpp"

namespace Os {
TaskInterface* TaskInterface::getDelegate(TaskHandleStorage& aligned_new_memory) {
    return Os::Delegate::makeDelegate<TaskInterface, Os::Baremetal::BaremetalTask>(aligned_new_memory);
}

}  // namespace Os
