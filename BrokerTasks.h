#pragma once

#include <map>

#include "Ensure.h"
#include "Except.h"
#include "WorkerPool.h"
#include "ZMQIdentity.h"


struct BrokerTasks
{
    using WorkerIterator = WorkerPool::WorkerSeq::iterator;
    using TaskMap = std::map<ZMQIdentity, WorkerIterator>;
private:
    TaskMap taskMap_;
public:
    bool valid(const ZMQIdentity &identity) const
    {
        return 0 < taskMap_.count(identity);
    }

    void append(WorkerIterator iterator)
    {
        ENSURE(0 == taskMap_.count(iterator->identity_), WorkerDuplicate);
        taskMap_[iterator->identity_] = iterator;
    }

    void remove(const ZMQIdentity &identity)
    {
        taskMap_.erase(identity);
    }

    WorkerIterator workerIterator(const ZMQIdentity &identity) const
    {
        ENSURE(valid(identity), IdentityInvalid);
        return taskMap_.at(identity);
    }
};
