#pragma once

#include <functional>

#include <zmqpp/zmqpp.hpp>


struct WorkerTask
{
    struct Guard
    {
        zmqpp::socket &socket_;
        Guard(zmqpp::socket &socket): socket_{socket}
        {}
        ~Guard();
    };

    using Transform = std::function<zmqpp::message (zmqpp::message)>;

    Transform transform_;

    WorkerTask(Transform transform):
        transform_{std::move(transform)}
    {}

    void operator()(zmqpp::socket &);
};
