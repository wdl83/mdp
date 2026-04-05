#pragma once

#include <functional>

#include <zmqpp/zmqpp.hpp>


struct WorkerTask
{
    struct Guard
    {
        zmqpp::socket &socket_;
        explicit Guard(zmqpp::socket &socket): socket_{socket}
        {}

        Guard(const Guard &) = delete;
        Guard &operator=(const Guard &) = delete;
    };

    struct MasterGuard: public Guard
    {
        using Guard::Guard;
        ~MasterGuard();
    };

    struct SlaveGuard: public Guard
    {
        using Guard::Guard;
        ~SlaveGuard();
    };

    using Transform = std::function<zmqpp::message (zmqpp::message)>;

    Transform transform_;

    explicit WorkerTask(Transform transform):
        transform_{std::move(transform)}
    {}

    WorkerTask(const WorkerTask &) = delete;
    WorkerTask &operator=(const WorkerTask &) = delete;

    void operator()(zmqpp::socket &);
};
