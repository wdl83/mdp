#include "Ensure.h"
#include "Except.h"
#include "MDP.h"
#include "WorkerTask.h"
#include "utils.h"

WorkerTask::MasterGuard::~MasterGuard()
{
    TRACE(TraceLevel::Info, this, " exit");
    send(socket_, zmqpp::message{"exit"}, IOMode::Blocking);
}

WorkerTask::SlaveGuard::~SlaveGuard()
{
    TRACE(TraceLevel::Info, this, " exited");
    send(socket_, zmqpp::message{"exited"}, IOMode::Blocking);
}

void WorkerTask::operator()(zmqpp::socket &socket)
{
    SlaveGuard guard{socket};

    for(;;)
    {
        zmqpp::message request;

        {
            const auto status = socket.receive(request, false /* dont_block */);

            ENSURE(status, RecvFailed);

            if(1 == request.parts() && "exit" == request.get(0)) return;
        }

        ASSERT(5 <= request.parts());

        const auto clientAddress = request.get(3);

        /* Frame 0: empty */
        request.pop_front();
        /* Frame 1: six byte signature (worker) */
        request.pop_front();
        /* Frame 2: one byte signature (worker request) */
        request.pop_front();
        /* Frame 3: Client address (envelope stack) */
        request.pop_front();
        /* Frame 4: Empty (zero bytes, envelope delimiter) */
        request.pop_front();

        auto reply = transform_(std::move(request));

        /* Frame 4: Empty (zero bytes, envelope delimiter) */
        reply.push_front(nullptr, 0);
        /* Frame 3: Client address (envelope stack) */
        reply.push_front(clientAddress);
        /* Frame 2: one byte signature (worker request) */
        reply.push_front(MDP::Worker::Signature::reply);
        /* Frame 1: six byte signature (worker) */
        reply.push_front(MDP::Worker::Signature::self);
        /* Frame 0: empty */
        reply.push_front(nullptr, 0);

        {
            const auto status = socket.send(reply, false /* dont block */);

            ENSURE(status, SendFailed);
        }
    }
}
