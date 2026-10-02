# Os::Baremetal::TaskRunner

## 1. Introduction

The `Os::Baremetal::TaskRunner` runs cooperative tasks on systems without an operating system scheduler. A cooperative
task does one unit of work each time it is run and then returns, letting the task runner run the next task. Baremetal
`Os::Task` implementations register themselves with the task runner when they are started, and the system's main loop
calls `TaskRunner::run()` repeatedly.

## 2. Requirements

The requirements for `Os::Baremetal::TaskRunner` are as follows:

Requirement | Description | Verification Method
----------- | ----------- | -------------------
TR-001 | The `Os::Baremetal::TaskRunner` shall register each started task exactly once | Unit Test
TR-002 | The `Os::Baremetal::TaskRunner` shall run tasks in descending priority order, and tasks of equal priority in the order they were started | Unit Test
TR-003 | The `Os::Baremetal::TaskRunner` shall run one unit of work of one task per call to `run()`, giving each registered task one turn per round | Unit Test
TR-004 | The `Os::Baremetal::TaskRunner` shall run one unit of work of each registered task per call to `runAll()` | Unit Test
TR-005 | The `Os::Baremetal::TaskRunner` shall register tasks started after it has begun running without dropping or duplicating registered tasks | Unit Test
TR-006 | The `Os::Baremetal::TaskRunner` shall stop running a task once the task is removed | Unit Test

## 3. Design

### 3.1 Interfaces

Method | Usage
------ | -----
`addTask(Task*)` | Called by `Os::Task::start()` to register a task
`removeTask(Task*)` | Called by `Os::Task::~Task()`, and by the task runner when a task exits, to unregister a task
`run()` | Run one unit of work of the next running task
`runAll()` | Run one unit of work of each running task
`stop()` | Stop running tasks
`getSingleton()` | Get the task runner that baremetal tasks register with

### 3.2 Functional Description

Registered tasks are kept in a task table sorted by descending priority. Only running tasks are run: a task is running
when its `Os::Task` state is `STARTING` or `RUNNING`. Tasks in other states, such as suspended or joined tasks, stay in
the table but are skipped. `run()` walks the table in a round-robin, starting from where the previous call left off, and
runs the next running task. `runAll()` walks the table from the start and runs each running task once. A task is removed
from the table when its `Os::Task` is destroyed, or when its state becomes `EXITED` while its routine is being run.

### 3.3 Algorithms

The task table is a fixed array of `TASK_CAPACITY` task pointers. Registered tasks are packed at the start of the array
and followed by `nullptr` entries, so the number of registered tasks is the index of the first `nullptr` entry.

`addTask()` finds the first entry with a lower priority than the new task, shifts that entry and the ones after it one
slot to the right, and places the new task there. This keeps the table sorted, and keeps tasks of equal priority in the
order they were started.

`removeTask()` shifts every entry after the removed task one slot to the left.
