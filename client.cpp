#include <unistd.h>

#include <fstream>
#include <iterator>

#include <nlohmann/json.hpp>

#include "Client.h"

using json = nlohmann::json;

void help()
{
    std::cout
        << "client -a broker_address -s service_name -i [input.json|-] [-o output]"
        << std::endl;
}

int main(int argc, char *const argv[])
{
    std::string address;
    std::string serviceName;
    std::string iname;
    std::string oname;

    for(int c; -1 != (c = ::getopt(argc, argv, "ha:s:i:"));)
    {
        switch(c)
        {
            case 'h':
                help();
                return EXIT_SUCCESS;
                break;
            case 'a':
                address = optarg ? optarg : "";
                break;
            case 's':
                serviceName = optarg ? optarg : "";
                break;
            case 'i':
                iname = optarg ? optarg : "";
                break;
            case 'o':
                oname = optarg ? optarg : "";
                break;
            case ':':
            case '?':
            default:
                return EXIT_FAILURE;
                break;
        }
    }

    if(address.empty() || serviceName.empty() || iname.empty())
    {
        help();
        return EXIT_FAILURE;
    }

    try
    {
        json input;

        if("-" == iname) std::cin >> input;
        else std::ifstream{iname} >> input;

        Client::PayloadSeq payloadSeq;

        for(const auto &i : input) payloadSeq.emplace_back(i.dump());

        Client client;

        const auto reply = client.exec(address, serviceName, payloadSeq);

        ENSURE(!reply.empty(), RuntimeError);

        auto begin = std::begin(reply);

        ENSURE(MDP::Broker::Signature::statusSucess == *begin, RuntimeError);

        ++begin;

        if(std::end(reply) == begin) goto exit;

        if(oname.empty())
        {
            std::cout << '[';

            for(;begin != std::end(reply);)
            {
                std::cout << *begin;
                ++begin;

                if(begin != std::end(reply)) std::cout << ',';
            }
            std::cout << ']';
        }
        else
        {
            std::ofstream ofile{oname};

            ofile << '[';

            for(;begin != std::end(reply);)
            {
                ofile << *begin;
                if(begin != std::end(reply)) ofile << ',';
            }
            ofile << ']';
        }
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
exit:
    return EXIT_SUCCESS;
}
