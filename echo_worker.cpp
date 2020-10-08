#include <unistd.h>

#include "Worker.h"

void help()
{
    std::cout
        << "worker -a broker_address -s service_name"
        << std::endl;
}

int main(int argc, char *const argv[])
{
    std::string address;
    std::string serviceName;

    for(int c; -1 != (c = ::getopt(argc, argv, "ha:s:"));)
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
            case ':':
            case '?':
            default:
                return EXIT_FAILURE;
                break;
        }
    }

    if(address.empty() || serviceName.empty())
    {
        help();
        return EXIT_FAILURE;
    }

    try
    {
        Worker worker;

        worker.exec(
            address,
            serviceName,
            [](zmqpp::message message)
            {
                /* echo */
                return message;
            });
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
