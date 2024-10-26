// ======================================================================
// \title Os/Baremetal/Task.cpp
// \brief baremetal implementations for Os::Baremetal::Task
// ======================================================================
#include "fprime-baremetal/Os/Baremetal/Task.hpp"
#include "fprime-baremetal/Os/TaskRunner/TaskRunner.hpp" // Required for task execution
namespace Os {
namespace Baremetal {

BaremetalTask::BaremetalTask() {
    (void) TaskRunner::getSingleton(); // Force the task runner into existence
}

void BaremetalTask::onStart() {

}

Os::TaskInterface::Status BaremetalTask::join() {
    return Os::TaskInterface::Status::OP_OK;
}

void BaremetalTask::suspend(Os::TaskInterface::SuspensionType suspensionType) {}

void BaremetalTask::resume() {}

Os::TaskHandle *BaremetalTask::getHandle() {
    return &m_handle;
}

Os::TaskInterface::Status BaremetalTask::start(const Os::TaskInterface::Arguments &arguments) {
    return Os::TaskInterface::Status::OP_OK;
}

bool BaremetalTask::isCooperative() {
    return true;
}

Os::Task::Status BaremetalTask::_delay(Fw::TimeInterval interval) {
    // It is an error to attempt to delay a batemetal task because this would amount to
    FW_ASSERT(0);
    return Os::Task::Status::UNKNOWN_ERROR;
}

}
}
