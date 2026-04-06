#include <iostream>
#include <string>

#include <unistd.h>

#include <mdp/Broker.h>

void help() { std::cout << "broker -a broker_address" << std::endl; }

int main(int argc, char *const argv[])
{
    std::string address;

    for (int c; -1 != (c = ::getopt(argc, argv, "ha:"));)
    {
        switch (c)
        {
        case 'h':
            help();
            return EXIT_SUCCESS;
            break;
        case 'a': address = optarg; break;
        case ':':
        case '?':
        default: return EXIT_FAILURE; break;
        }
    }

    if (address.empty())
    {
        help();
        return EXIT_FAILURE;
    }

    try
    {
        Broker broker;

        broker.exec(address);
    }
    catch (const std::exception &except)
    {
        TRACE(TraceLevel::Error, except.what());
        return EXIT_FAILURE;
    }
    catch (...)
    {
        TRACE(TraceLevel::Error, "unsupported exception");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
