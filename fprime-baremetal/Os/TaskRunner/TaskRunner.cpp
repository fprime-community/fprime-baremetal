// ======================================================================
// \title fprime-baremetal/Os/TaskRunner/TaskRunner.cpp
// \brief TaskRunner implementations
// ======================================================================
#include <FpConfig.hpp>
#include <Fw/Types/Assert.hpp>
#include <fprime-baremetal/Os/Baremetal/Task.hpp>
#include <fprime-baremetal/Os/TaskRunner/TaskRunner.hpp>
namespace Os {
namespace Baremetal {

bool isRunning(Task& task) {
    return (task.getState() == Os::Task::State::STARTING || task.getState() == Os::Task::RUNNING);
}

TaskRunner::TaskRunner() {
    // Initialize all elements in the task table to nullptr
    for (U32 i = 0; i < Os::Baremetal::TASK_CAPACITY; i++) {
        this->m_task_table[i] = nullptr;
    }
    Task::registerTaskRegistry(this);
}

TaskRunner::~TaskRunner() {}

void TaskRunner::addTask(Task* task) {
    FW_ASSERT(task->isCooperative());  // Cannot register uncooperative tasks

    FW_ASSERT(this->m_index < Os::Baremetal::TASK_CAPACITY);
    this->m_task_table[this->m_index] = task;
    this->m_index++;

    // Sort by priority during insertion
    Task* sort_element = task;
    for (FwSizeType i = 0; (sort_element != nullptr) && (i < Os::Baremetal::TASK_CAPACITY); i++) {
        if ((this->m_task_table[i] == nullptr) or (sort_element->getPriority() >
        this->m_task_table[i]->getPriority())) {
            Task* temp = sort_element;
            sort_element = this->m_task_table[i];
            this->m_task_table[i] = temp;
        }
    }

    // The last sort element must be nullptr or the table overflowed
    FW_ASSERT(sort_element == nullptr);
}

void TaskRunner::removeTask(Task* task) {
    bool found = false;
    // Squash that existing task
    for (FwSizeType i = 0; i < Os::Baremetal::TASK_CAPACITY; i++) {
        found = found | (task == this->m_task_table[i]);
        // If not found, keep looking
        if (not found) {
            continue;
        }
        // If we are less than the end of the array, shift variables over
        else if (i < Os::Baremetal::TASK_CAPACITY - 1) {
            this->m_task_table[i] = this->m_task_table[i + 1];
        }
        // If the last element, mark NULL
        else {
            this->m_task_table[i] = nullptr;
        }
    }
}

TaskRunner& TaskRunner::getSingleton() {
    static TaskRunner runner;
    return runner;
}

void TaskRunner::stop() {
    this->m_cycling = false;
}

void TaskRunner::runOne(Task& task) {
    FW_ASSERT(task.isCooperative());
    // Downcast to baremetal task handle
    BaremetalTaskHandle& baremetal_handle = *static_cast<BaremetalTaskHandle*>(task.getHandle());

    // Run the cooperative task once
    baremetal_handle.m_routine(baremetal_handle.m_argument);
}

bool TaskRunner::runNext() {
    Task* task = this->m_task_table[this->m_index];
    // Run a single task
    if (task != nullptr && isRunning(*task)) {
        this->runOne(*task);
        // Check and remove exited task
        if (task->getState() == Os::Task::State::EXITED) {
            this->removeTask(task);
        }
        // Otherwise bump the next start task
        else {
            this->m_index = (this->m_index + 1) % Os::Baremetal::TASK_CAPACITY;
        }
        return true;
    }
    this->m_index = (this->m_index + 1) % Os::Baremetal::TASK_CAPACITY;
    return false;
}

void TaskRunner::run() {
    // While cycling run a task and increment to the next
    if (this->m_cycling) {
        // Start at the next task
        for (FwSizeType i = 0; i < Os::Baremetal::TASK_CAPACITY; i++) {
            // Run only one task.
            if (runNext()) {
                break;
            }
        }
    }
}

void TaskRunner::runAll() {
    if (this->m_cycling) {
        this->m_index = 0;
        for (FwSizeType i = 0; i < Os::Baremetal::TASK_CAPACITY; i++) {
            // Run each task exactly once.
            (void) runNext();
            if (this->m_index == 0) {
                break;
            }
        }
    }
}
}  // End namespace Baremetal
}  // End Namespace Os
