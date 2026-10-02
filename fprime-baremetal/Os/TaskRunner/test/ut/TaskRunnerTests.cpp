// ======================================================================
// \title fprime-baremetal/Os/TaskRunner/test/ut/TaskRunnerTests.cpp
// \brief tests for Os::Baremetal::TaskRunner
// ======================================================================
#include <gtest/gtest.h>
#include <Os/Task.hpp>
#include <STest/Pick/Pick.hpp>
#include <STest/Random/Random.hpp>
#include <algorithm>
#include <fprime-baremetal/Os/TaskRunner/TaskRunner.hpp>
#include <memory>
#include <string>
#include <vector>

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    STest::Random::seed();
    return RUN_ALL_TESTS();
}

namespace {

constexpr FwSizeType ITERATIONS = 100;  //!< Random task sets tried by each test
constexpr U32 MAX_TASKS = 40;           //!< Tasks started at once, so two sets fit within TASK_CAPACITY
constexpr U32 MAX_PRIORITY = 255;       //!< Largest priority picked, which fits any FwTaskPriorityType

//! Record of a task's runs, passed to the task routine as its argument
struct TaskRecord {
    FwSizeType id;
    std::vector<FwSizeType>* log;
};

void recordRun(void* argument) {
    TaskRecord* record = static_cast<TaskRecord*>(argument);
    record->log->push_back(record->id);
}

//! Starts baremetal tasks, which register themselves with the TaskRunner singleton, and records each run of their
//! routines. Task ids are assigned in start order. Tasks are destroyed, and so removed from the runner, by clearTasks()
//! and when the fixture is torn down.
class TaskRunnerTest : public ::testing::Test {
  protected:
    void TearDown() override { this->clearTasks(); }

    //! Start a task and return its id
    FwSizeType startTask(FwTaskPriorityType priority) {
        const FwSizeType id = this->m_tasks.size();
        this->m_records.emplace_back(new TaskRecord{id, &this->m_log});
        this->m_tasks.emplace_back(new Os::Task());
        const std::string name = "Task" + std::to_string(id);
        Os::Task::Status status = this->m_tasks.back()->start(
            Os::Task::Arguments(Fw::String(name.c_str()), recordRun, this->m_records.back().get(), priority));
        EXPECT_EQ(status, Os::Task::Status::OP_OK);
        // Starting a baremetal task runs its routine once, which is not a scheduled run
        this->m_log.clear();
        return id;
    }

    //! Start tasks with the given priorities, in order
    void startTasks(const std::vector<FwTaskPriorityType>& priorities) {
        for (FwTaskPriorityType priority : priorities) {
            this->startTask(priority);
        }
    }

    //! Destroy all tasks and clear the run log
    void clearTasks() {
        // Destroy tasks before their records
        this->m_tasks.clear();
        this->m_records.clear();
        this->m_log.clear();
    }

    //! Pick between 1 and MAX_TASKS random priorities. The priorities are drawn from a randomly sized range, so small
    //! ranges produce many tasks of equal priority.
    static std::vector<FwTaskPriorityType> pickPriorities() {
        const U32 count = STest::Pick::lowerUpper(1, MAX_TASKS);
        const U32 upper = STest::Pick::lowerUpper(0, MAX_PRIORITY);
        std::vector<FwTaskPriorityType> priorities;
        for (U32 i = 0; i < count; i++) {
            priorities.push_back(static_cast<FwTaskPriorityType>(STest::Pick::lowerUpper(0, upper)));
        }
        return priorities;
    }

    //! Ids of the given priorities sorted by descending priority, with ties kept in start order
    static std::vector<FwSizeType> expectedOrder(const std::vector<FwTaskPriorityType>& priorities) {
        std::vector<FwSizeType> order;
        for (FwSizeType id = 0; id < priorities.size(); id++) {
            order.push_back(id);
        }
        std::stable_sort(order.begin(), order.end(),
                         [&priorities](FwSizeType a, FwSizeType b) { return priorities[a] > priorities[b]; });
        return order;
    }

    static Os::Baremetal::TaskRunner& runner() { return Os::Baremetal::TaskRunner::getSingleton(); }

    std::vector<FwSizeType> m_log;
    std::vector<std::unique_ptr<TaskRecord>> m_records;
    std::vector<std::unique_ptr<Os::Task>> m_tasks;
};

}  // namespace

TEST_F(TaskRunnerTest, RunAllRunsEachTaskOnceInPriorityOrder) {
    for (FwSizeType iteration = 0; iteration < ITERATIONS; iteration++) {
        this->clearTasks();
        const std::vector<FwTaskPriorityType> priorities = pickPriorities();
        this->startTasks(priorities);
        runner().runAll();
        ASSERT_EQ(this->m_log, expectedOrder(priorities));
    }
}

TEST_F(TaskRunnerTest, RunGivesEachTaskOneTurnPerRound) {
    for (FwSizeType iteration = 0; iteration < ITERATIONS; iteration++) {
        this->clearTasks();
        const std::vector<FwTaskPriorityType> priorities = pickPriorities();
        this->startTasks(priorities);
        for (FwSizeType round = 0; round < 2; round++) {
            this->m_log.clear();
            for (FwSizeType i = 0; i < priorities.size(); i++) {
                runner().run();
            }
            ASSERT_EQ(this->m_log.size(), priorities.size());
            for (FwSizeType id = 0; id < priorities.size(); id++) {
                ASSERT_EQ(std::count(this->m_log.begin(), this->m_log.end(), id), 1) << "task " << id;
            }
        }
    }
}

TEST_F(TaskRunnerTest, AddTaskAfterRunKeepsExistingTasks) {
    for (FwSizeType iteration = 0; iteration < ITERATIONS; iteration++) {
        this->clearTasks();
        std::vector<FwTaskPriorityType> priorities = pickPriorities();
        this->startTasks(priorities);
        // Advance the run cursor, then start more tasks
        const U32 runs = STest::Pick::lowerUpper(1, static_cast<U32>(2 * priorities.size()));
        for (U32 i = 0; i < runs; i++) {
            runner().run();
        }
        const std::vector<FwTaskPriorityType> more = pickPriorities();
        this->startTasks(more);
        priorities.insert(priorities.end(), more.begin(), more.end());
        runner().runAll();
        ASSERT_EQ(this->m_log, expectedOrder(priorities));
    }
}

TEST_F(TaskRunnerTest, DestroyedTaskIsNoLongerRun) {
    for (FwSizeType iteration = 0; iteration < ITERATIONS; iteration++) {
        this->clearTasks();
        const std::vector<FwTaskPriorityType> priorities = pickPriorities();
        this->startTasks(priorities);
        const FwSizeType destroyed = STest::Pick::lowerUpper(0, static_cast<U32>(priorities.size() - 1));
        this->m_tasks[destroyed].reset();
        runner().runAll();
        std::vector<FwSizeType> expected = expectedOrder(priorities);
        expected.erase(std::find(expected.begin(), expected.end(), destroyed));
        ASSERT_EQ(this->m_log, expected);
    }
}
