#pragma once

#include <algorithm>
#include <map>
#include <string>
#include <vector>
#include <ostream>

#include "Ensure.h"
#include "Except.h"
#include "MutualHeartbeatMonitor.h"
#include "ZMQIdentity.h"


struct WorkerPool
{
    struct Worker
    {
        enum class State
        {
            begin, Idle = begin, Busy, end
        };

        friend
        std::ostream &operator<< (std::ostream &os, const State &s)
        {
            static const char* name[] =
            {
                "idle", "busy"
            };
            os << (State::begin <= s && State::end > s ? name[int(s)] : "?");
            return os;
        }

        std::string serviceName_;
        State state_;
        ZMQIdentity identity_;
        MutualHeartbeatMonitor monitor_;

        Worker(std::string serviceName, ZMQIdentity identity):
            serviceName_{std::move(serviceName)},
            state_{State::Idle},
            identity_{std::move(identity)}
        {}

        friend
        std::ostream &operator<< (std::ostream &os, const Worker &w)
        {
            os << w.identity_.asString() << ' ' << w.serviceName_ << ' ' << w.state_;
                return os;
        }
    };

    using WorkerSeq = std::list<Worker>;
    using ServiceName = std::string;
    using ServiceMap = std::map<std::string, WorkerSeq>;
    using ServiceLookup = std::map<ZMQIdentity, std::string>;
private:
    ServiceMap serviceMap_;
    ServiceLookup serviceLookup_;

    static
    WorkerSeq::iterator findWorker(WorkerSeq &workerSeq, const ZMQIdentity &identity)
    {
        return
            std::find_if(
                std::begin(workerSeq), std::end(workerSeq),
                [&](const Worker &worker)
                {
                    return worker.identity_ == identity;
                });
    }
public:
    template <typename F>
    void forEachWorker(F f)
    {
        for(auto &pair : serviceMap_)
        {
            for(auto &worker : pair.second) f(worker);
        }
    }

    bool valid(const ServiceName &serviceName) const
    {
        return
            0 < serviceMap_.count(serviceName)
            && !serviceMap_.at(serviceName).empty();
    }

    Worker *acquire(const ServiceName &serviceName)
    {
        ENSURE(valid(serviceName), ServiceUnsupported);

        auto &workerSeq = serviceMap_[serviceName];

        if(Worker::State::Idle != workerSeq.front().state_) return nullptr;

        /* round robin (if more then 1 worker in sequence) */
        if(std::end(workerSeq) != std::next(std::begin(workerSeq)))
        {
            std::rotate(
                std::begin(workerSeq),
                std::next(std::begin(workerSeq)),
                std::end(workerSeq));
        }

        return &workerSeq.back();
    }

    WorkerSeq::iterator findWorker(const ZMQIdentity &identity)
    {
        ENSURE(0 < serviceLookup_.count(identity), IdentityInvalid);

        const auto &serviceName = serviceLookup_.find(identity)->second;
        auto &workerSeq = serviceMap_[serviceName];

        return findWorker(workerSeq, identity);
    }

    size_t append(const std::string &serviceName, const ZMQIdentity &identity)
    {
        auto &workerSeq = serviceMap_[serviceName];

        ENSURE(workerSeq.end() == findWorker(workerSeq, identity), WorkerDuplicate);

        workerSeq.emplace_back(serviceName, identity);

        ENSURE(0 == serviceLookup_.count(identity), WorkerDuplicate);

        serviceLookup_[identity] = serviceName;
        return workerSeq.size();
    }

    size_t remove(const ZMQIdentity &identity)
    {
        ENSURE(0 < serviceLookup_.count(identity), IdentityInvalid);

        const auto &serviceName = serviceLookup_.find(identity)->second;
        auto &workerSeq = serviceMap_[serviceName];

        workerSeq.erase(findWorker(workerSeq, identity));
        const auto num = workerSeq.size();
        if(workerSeq.empty()) serviceMap_.erase(serviceName);
        // WARNING: workerSeq ref invalid
        serviceLookup_.erase(identity);
        return num;
    }
};
