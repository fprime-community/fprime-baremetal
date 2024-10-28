// ======================================================================
// \title fprime-baremetal/Os/TaskRunner/TaskRunner.hpp
// \brief TaskRunner definitions
// ======================================================================
#ifndef FPRIME_BAREMETAL_TASKRUNNER_TASKRUNNER_HPP_
#define FPRIME_BAREMETAL_TASKRUNNER_TASKRUNNER_HPP_
#include <Os/Task.hpp>

namespace Os {
namespace Baremetal {

constexpr FwSizeType TASK_CAPACITY = 100;  // TODO: configurize this

//! \brief a synthetic task runner that will invoke cooperative tasks
//!
//! The task runner is designed to synthesize task switching by running a time slice of a given task before moving to
//! the next. This is done via a technique known as cooperative multitasking. Each of the tasks in the system is
//! expected to cooperate in multitasking by performing one unit of work before returning. The scheduler leverages this
//! behavior to switch to the next task's unit of work. Thus showing synthetic task switching.
//!
//! The task runner handles `priority` a bit differently from standard RTOSes. The task runner will walk a pass of the
//! cooperative tasks in priority order. This means that priority tasks with multiple units of work will wait for lower
//! priority tasks initial unit of works to run.
//!
class TaskRunner : TaskRegistry {

  public:
    //!< Nothing constructor
    TaskRunner();
    //!< Nothing destructor
    ~TaskRunner();

    //! \brief add a task to this runner
    //!
    //! This will add a task to the set of tasks that will be run by this task runner. This call will perform the
    //! initial sort of the tasks into priority order.
    //!
    //! It is invalid to pass an nullptr to this function.
    //!
    //! \param task: pointer task to add to this runner
    void addTask(Task* task);

    //! \brief remove a task from this runner
    //!
    //! This will remove a task from this runner. This task will no longer be run.
    //!
    //! It is invalid to pass an nullptr to this function.
    //!
    //! \param task: pointer task to remove to this runner
    void removeTask(Task* task);

    //! \brief stop this task runner
    //!
    //! Stop this task runner. No addition tasks work will be run.
    void stop();

    //! \brief run a single unit of work
    //!
    //! This will run a single unit of work of a single task. The task runner will walk the task list and choose the
    //! next task in the list to run.
    //!
    //! This method should be called within the run-loop of the system:
    //!
    //! ```c++
    //! while (true) {
    //!     taskRunner.run();
    //! }
    //! ```
    void run();

    //! \brief get a singleton instance of the global task runner
    static TaskRunner& getSingleton();
  private:
    //! \brief helper to run a single unit of work, from this task
    void runOne(Task& task);
    Task* m_task_table[TASK_CAPACITY]; //!< Task table of registered tasks
    FwSizeType m_index = 0; //!< Index of current task
    bool m_cycling = true; //!< Is the task runner cycling
};
} // End namespace Baremetal
} // End Namespace Os
#endif /* OS_BAREMETAL_TASKRUNNER_TASKRUNNER_HPP_ */
