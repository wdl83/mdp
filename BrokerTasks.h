#pragma once

#include <map>

#include "Ensure.h"
#include "Except.h"
#include "WorkerPool.h"
#include "ZMQIdentity.h"


struct BrokerTasks
{
    using WorkerIterator = WorkerPool::WorkerSeq::iterator;

    struct TaskInfo
    {
        WorkerIterator workerIterator_;
        ZMQIdentity clientIdentity_;

        TaskInfo(WorkerIterator workerIterator, ZMQIdentity clientIdentity):
            workerIterator_{workerIterator},
            clientIdentity_{clientIdentity}
        {}

        TaskInfo(const TaskInfo &) = delete;
        TaskInfo &operator= (const TaskInfo &) = delete;

        TaskInfo(TaskInfo &&) = default;
        TaskInfo &operator= (TaskInfo &&) = default;
    };

    using TaskInfoMap = std::map<ZMQIdentity, TaskInfo>;
private:
    TaskInfoMap taskInfoMap_;
public:
    bool valid(const ZMQIdentity &identity) const
    {
        return taskInfoMap_.count(identity);
    }

    void append(WorkerIterator workerIterator, ZMQIdentity clientIdentity)
    {
        ENSURE(0 == taskInfoMap_.count(workerIterator->identity_), WorkerDuplicate);

        taskInfoMap_.emplace(
            std::make_pair(
                workerIterator->identity_,
                TaskInfo{workerIterator, clientIdentity}));
    }

    void remove(const ZMQIdentity &workerIdentity)
    {
        taskInfoMap_.erase(workerIdentity);
    }

    const TaskInfo &taskInfo(const ZMQIdentity &workerIdentity) const
    {
        ENSURE(valid(workerIdentity), IdentityInvalid);
        return taskInfoMap_.at(workerIdentity);
    }
};
