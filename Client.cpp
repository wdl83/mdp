#include "Client.h"
#include "Except.h"

#include <fstream>

#include <unistd.h>

#include <nlohmann/json.hpp>


using json = nlohmann::json;

auto Client::exec(
    const std::string &address,
    const std::string &serviceName,
    PayloadSeq payloadSeq) -> PayloadSeq
{
    auto zmqContext = ZMQContext{ZMQIdentity::unique(), address};

    try
    {
        TRACE(this, " sending request for ", serviceName);

        zmqContext.send(MDP::Client::makeReq(serviceName, payloadSeq));
        return onReply(onMessage(zmqContext.recv(), zmqContext, serviceName));
    }
    catch(const EnsureException &except)
    {
        TRACE(this, " EnsureException: ", except.toString(), " aborting");
        return {};
    }
    catch(const std::exception &except)
    {
        TRACE(this, " std::exception: ", except.what(), " aborting");
        return {};
    }
    catch(...)
    {
        TRACE(this, " Unsupported exception, aborting");
        return {};
    }

    return {};
}

auto Client::onMessage(
    Message msg,
    const ZMQContext &,
    const std::string &serviceName) -> Message
{
    TRACE(this, " ", msg);

    ENSURE(4 <= msg.parts(), MessageFormatInvalid);
    /* Frame 0: empty */
    ENSURE(0 == msg.size(0), MessageFormatInvalid);
    /* Frame 1: six byte signature (client) */
    ENSURE(6 == msg.size(1), MessageFormatInvalid);
    ENSURE(MDP::Client::Signature::self == msg.get(1), MessageFormatInvalid);
    /* Frame 2: service name */
    ENSURE(serviceName == msg.get(2), MessageFormatInvalid);
    /* Frame 3+: payload */
    ENSURE(0 < msg.size(3), MessageFormatInvalid);
    return msg;
}

auto Client::onReply(Message message) ->PayloadSeq
{
    TRACE(this, " ", message);

    PayloadSeq seq;

    for(size_t i = 3; i < message.parts(); ++i) seq.push_back(message.get(i));
    return seq;
}


void help()
{
    std::cout
        << "client -a broker_address -s service_name -p payload_string"
        << std::endl;
}

int main(int argc, char *const argv[])
{
    std::string address;
    std::string serviceName;
    std::string payloadName;

    for(char c; -1 != (c = ::getopt(argc, argv, "ha:s:p:"));)
    {
        switch(c)
        {
            case 'h':
                help();
                return EXIT_SUCCESS;
                break;
            case 'a':
                address = optarg;
                break;
            case 's':
                serviceName = optarg;
                break;
            case 'p':
                payloadName = optarg;
                break;
            case ':':
            case '?':
            default:
                return EXIT_FAILURE;
                break;
        }
    }

    if(address.empty() || serviceName.empty() || payloadName.empty())
    {
        help();
        return EXIT_FAILURE;
    }

    try
    {
        std::ifstream payloadFile{payloadName};
        Client::PayloadSeq payloadSeq;

        for(auto i : json::parse(payloadFile))
        {
            std::cout << "\t" << i.dump() << std::endl;
            payloadSeq.emplace_back(i.dump());
        }

        Client client;

        const auto reply = client.exec(address, serviceName, std::move(payloadSeq));

        for(const auto &i : reply) std::cout << i;
        std::cout << std::endl;
    }
    catch(const EnsureException &except)
    {
        std::cerr << "ensure exception " << except.toString() << std::endl;
        return EXIT_FAILURE;
    }
    catch(const std::exception &except)
    {
        std::cerr << "std exception " << except.what() << std::endl;
        return EXIT_FAILURE;
    }
    catch(...)
    {
        std::cerr << "unsupported exception" << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
