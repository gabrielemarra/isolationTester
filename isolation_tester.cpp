#include <sys/types.h>
#include <sys/capability.h>

#include <ifaddrs.h>
#include <netdb.h>
#include <unistd.h>
#include <cstring>
#include <errno.h>

#include <iostream>
#include <filesystem>
#include <map>
#include <cstdlib>


namespace fs = std::filesystem;

namespace
{
    void print_environment_variables()
    {
        std::cout << "\n=== Environment Variables ===" << std::endl;
        for (char **env = environ; *env != nullptr; env++)
        {
            std::cout << *env << std::endl;
        }
    }

    void print_caps()
    {
        std::cout << "\n=== Capabilities ===" << std::endl;
        cap_t caps = cap_get_proc();

        // Using NULL for the length parameter as it is not used
        auto str = cap_to_text(caps, NULL);

        // Check if the capabilities string is just "="
        if (str != nullptr && std::string(str) == "=")
        {
            std::cout << "No capabilities" << std::endl;
        }
        else
        {
            std::cout << str << std::endl;
        }

        cap_free(str);
        cap_free(caps);
    }

    void print_pid_and_user()
    {
        std::cout << "\n=== User/PID info ===" << std::endl;
        pid_t pid = getpid();
        uid_t uid = geteuid();
        uid_t gid = getegid();

        std::cout << "uid: " << uid << std::endl;
        std::cout << "gid: " << gid << std::endl;
        std::cout << "pid: " << pid << std::endl;
    }

    void print_directories(fs::path path)
    {
        std::cout << "\n=== Directories ===" << std::endl;
        for (auto const &p : fs::directory_iterator(path))
            std::cout << p << std::endl;
    }

    void print_ip(ifaddrs *interface)
    {
        // Check if ifa_addr is not null
        if (!interface->ifa_addr)
        {
            // std::cout << "\tAvoided segfault" << std::endl;
            return;
        }

        int family = interface->ifa_addr->sa_family;
        if (family != AF_INET && family != AF_INET6)
            return;

        char host[NI_MAXHOST];
        int result = getnameinfo(interface->ifa_addr,
                                 (family == AF_INET) ? sizeof(sockaddr_in) : sizeof(sockaddr_in6),
                                 host, NI_MAXHOST,
                                 NULL, 0, NI_NUMERICHOST);
        if (result != 0)
        {
            std::cerr << "getnameinfo() failed: " << gai_strerror(result) << std::endl;
            return;
        }

        std::cout << "\t" << host << std::endl;
    }

    void print_interfaces(std::multimap<std::string, ifaddrs *> const &if_map)
    {
        std::string interface_name;

        for (auto const &interface : if_map)
        {
            if (interface.first != interface_name)
            {
                std::cout << interface.first << std::endl;
                interface_name = interface.first;
            }

            print_ip(interface.second);
        }
    }

    void print_network_interfaces()
    {
        std::cout << "\n=== Network interfaces ===" << std::endl;
        ifaddrs *interfaces;
        if (getifaddrs(&interfaces) == -1)
        {
            std::cerr << "Failed getting interfaces" << std::endl;
            return;
        }

        std::multimap<std::string, ifaddrs *> if_map;
        for (ifaddrs *interface = interfaces; interface != nullptr; interface = interface->ifa_next)
        {
            if_map.insert(std::make_pair(std::string(interface->ifa_name), interface));
        }

        print_interfaces(if_map);
        freeifaddrs(interfaces);
    }

    void list_processes()
    {
        std::cout << "\n=== Running processes ===" << std::endl;
        int process_count = 0;
        int total_process_count = 0;

        for (auto const &p : fs::directory_iterator("/proc"))
        {
            auto filename = p.path().filename();
            try
            {
                int pid = std::stoi(filename);
                if (process_count < 10)
                {
                    std::cout << pid << std::endl;
                }
                process_count++;
            }
            catch (std::invalid_argument)
            {
            }
        }

        total_process_count = process_count;
        if (process_count > 10)
        {
            std::cout << "... and " << process_count - 10 << " more processes" << std::endl;
        }
        std::cout << "Total processes: " << total_process_count << std::endl;
    }
}

int main(int argc, char **argv)
{
    std::cerr << "Running Isolation Tester" << std::endl;
    print_pid_and_user();
    print_directories("/");
    print_caps();
    print_network_interfaces();
    list_processes();
    print_environment_variables();
}
