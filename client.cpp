#include <unistd.h>

#include <fstream>

#include <nlohmann/json.hpp>

#include "Client.h"

using json = nlohmann::json;

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
            payloadSeq.emplace_back(i.dump());
        }

        Client client;

        const auto reply = client.exec(address, serviceName, std::move(payloadSeq));

        for(const auto &i : reply) std::cout << i << std::endl;
    }
    catch(const std::exception &except)
    {
        TRACE(TraceLevel::Error, except.what());
        return EXIT_FAILURE;
    }
    catch(...)
    {
        TRACE(TraceLevel::Error, "unsupported exception");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
