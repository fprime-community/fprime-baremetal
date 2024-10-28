// ======================================================================
// \title Os/Stub/Task.hpp
// \brief stub definitions for Os::Task
// ======================================================================
#include "Os/Task.hpp"

#ifndef OS_BAREMETAL_TASK_HPP
#define OS_BAREMETAL_TASK_HPP
namespace Os {
namespace Baremetal {

//! Test task handle
struct BaremetalTaskHandle : public TaskHandle {
    bool m_enabled;                            //!< Is this task enabled or not
    Os::TaskInterface::taskRoutine m_routine;  //!< Function passed into the task
    void* m_argument;                          //!< Argument input pointer
};

//! Implementation of task
class BaremetalTask : public TaskInterface {
  public:
    //! Constructor
    BaremetalTask();

    //! Destructor
    ~BaremetalTask() override = default;

    //! \brief perform required task start actions
    void onStart() override;

    //! \brief block until the task has ended
    //!
    //! Blocks the current (calling) task until this task execution has ended. Callers should ensure that any
    //! signals required to stop this task have already been emitted or will be emitted by another task.
    //!
    //! \return status of the block
    Status join() override;

    //! \brief suspend the task given the suspension type
    //!
    //! Suspends the task. Some implementations track if the suspension of a task was intentional or
    //! unintentional. The supplied `suspensionType` parameter indicates that this was intentional or
    //! unintentional. The type of suspension is also returned when calling `isSuspended`.
    //!
    //! \param suspensionType intentionality of the suspension
    void suspend(SuspensionType suspensionType) override;

    //! \brief resume a suspended task
    //!
    //! Resumes this task. Not started, running, and exited tasks take no action.
    //!
    void resume() override;

    //! \brief delay the current task
    //!
    //! Delays, or sleeps, the current task by the supplied time interval. In non-preempting os implementations
    //! the task will resume no earlier than expected but an exact wake-up time is not guaranteed.
    //!
    //! \param interval: delay time
    //! \return status of the delay
    Status _delay(Fw::TimeInterval interval) override;

    //! \brief return the underlying task handle (implementation specific)
    //! \return internal task handle representation
    TaskHandle* getHandle() override;

    //! \brief determine if the task requires cooperative multitasking
    //!
    //! Some task implementations require cooperative multitasking where the task execution is run by a user
    //! defined task scheduler and not the operating system task scheduler. These tasks cooperatively on
    //! multitask by doing one unit of work and return from the function.
    //!
    //! This function indicates if the task requires cooperative support.
    //! The default implementation returns false.
    //!
    //! \return true when the task expects cooperation, false otherwise
    bool isCooperative() override;

    //! \brief start the task
    //!
    //! Starts the task given the supplied arguments.
    //!
    //! \param arguments: arguments supplied to the task start call
    //! \return status of the task start
    Status start(const Arguments& arguments) override;

  private:
    BaremetalTaskHandle m_handle;
};

}  // namespace Baremetal
}  // namespace Os
#endif  // OS_BAREMETAL_TASK_HPP
