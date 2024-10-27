// ======================================================================
// \title Os/Baremetal/Task.cpp
// \brief baremetal implementations for Os::Baremetal::Task
// ======================================================================
#include "fprime-baremetal/Os/Baremetal/Task.hpp"
#include "fprime-baremetal/Os/TaskRunner/TaskRunner.hpp"  // Required for task execution
namespace Os {
namespace Baremetal {

BaremetalTask::BaremetalTask() {
    (void)TaskRunner::getSingleton();  // Force the task runner into existence
}

void BaremetalTask::onStart() {}

Os::TaskInterface::Status BaremetalTask::join() {
    return Os::TaskInterface::Status::OP_OK;
}

void BaremetalTask::suspend(Os::TaskInterface::SuspensionType suspensionType) {
    this->m_handle.m_enabled = false;
}

void BaremetalTask::resume() {
    this->m_handle.m_enabled = true;
}

Os::TaskHandle* BaremetalTask::getHandle() {
    return &m_handle;
}

Os::TaskInterface::Status BaremetalTask::start(const Os::TaskInterface::Arguments& arguments) {
    // Set handle member variables
    this->m_handle.m_enabled = true;
    this->m_handle.m_priority = arguments.m_priority;
    this->m_handle.m_routine = arguments.m_routine;
    this->m_handle.m_argument = arguments.m_routine_argument;

    // Running the task the first time allows setup activities for the task
    this->m_handle.m_routine(this->m_handle.m_argument);

    return Os::TaskInterface::Status::OP_OK;
}

bool BaremetalTask::isCooperative() {
    return true;
}

Os::Task::Status BaremetalTask::_delay(Fw::TimeInterval interval) {
    // It is an error to attempt to delay a baremetal task because this would amount to [insert something bad here]
    FW_ASSERT(0);
    return Os::Task::Status::UNKNOWN_ERROR;
}

}  // namespace Baremetal
}  // namespace Os
