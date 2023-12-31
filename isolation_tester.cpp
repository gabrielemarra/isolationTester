#include <sys/types.h>
#include <sys/capability.h>

#include <ifaddrs.h>
#include <netdb.h>
#include <unistd.h>
#include <cstring>

#include <iostream>
#include <filesystem>
#include <map>
#include <iomanip> // for std::setw
#include <fstream>
#include <sstream>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cstring>
#include <iostream>
#include <sstream>


namespace fs = std::filesystem;

namespace {
    void print_namespace_info(pid_t pid) {
        std::cout << "\n=== Namespace Information ===" << std::endl;
        std::string ns_types[] = {"mnt", "pid", "net", "ipc", "uts", "user", "cgroup"};

        for (const auto &type: ns_types) {
            std::stringstream ss;
            ss << "/proc/" << pid << "/ns/" << type;
            std::string ns_path = ss.str();

            struct stat ns_stat;
            if (lstat(ns_path.c_str(), &ns_stat) != -1) {
                std::cout << type << " namespace inode: " << ns_stat.st_ino << std::endl;
            } else {
                std::cerr << "Unable to access " << type << " namespace information for PID " << pid << ": " <<
                        strerror(errno) << std::endl;
            }
        }
    }

    void print_environment_variables() {
        std::cout << "\n=== Environment Variables ===" << std::endl;
        for (char **env = environ; *env != nullptr; env++) {
            std::cout << *env << std::endl;
        }
    }

    void print_caps() {
        std::cout << "\n=== Capabilities ===" << std::endl;
        cap_t caps = cap_get_proc();

        // Using NULL for the length parameter as it is not used
        auto str = cap_to_text(caps, NULL);

        // Check if the capabilities string is just "="
        if (str != nullptr && std::string(str) == "=") {
            std::cout << "No capabilities" << std::endl;
        } else {
            std::cout << str << std::endl;
        }

        cap_free(str);
        cap_free(caps);
    }

    void print_pid_and_user() {
        std::cout << "\n=== User/PID info ===" << std::endl;
        pid_t pid = getpid();
        uid_t uid = geteuid();
        uid_t gid = getegid();

        std::cout << "uid: " << uid << std::endl;
        std::cout << "gid: " << gid << std::endl;
        std::cout << "pid: " << pid << std::endl;
    }

    bool can_access(const fs::path &path) {
        return fs::exists(path) && (fs::status(path).permissions() & fs::perms::owner_read) != fs::perms::none;
    }

    void print_directory_tree(const fs::path &path, int level, int depth) {
        if (depth < 0 || !can_access(path)) {
            return; // depth limit reached or cannot access
        }

        try {
            for (const auto &entry: fs::directory_iterator(path)) {
                fs::path entry_path = entry.path();
                std::string entry_name = entry_path.filename().string();

                // Applying indentation for tree structure
                std::cout << std::setw(level * 4) << "";

                if (fs::is_directory(entry_path)) {
                    std::cout << "[Dir] " << entry_name << std::endl;
                    print_directory_tree(entry_path, level + 1, depth - 1);
                } else if (fs::is_regular_file(entry_path) && (
                               fs::status(entry_path).permissions() & fs::perms::owner_exec) != fs::perms::none) {
                    std::cout << "[Exec] " << entry_name << std::endl;
                } else if (fs::is_regular_file(entry_path)) {
                    std::cout << "[File] " << entry_name << std::endl;
                }
            }
        } catch (const fs::filesystem_error &e) {
            std::cerr << "Error accessing " << path << ": " << e.what() << std::endl;
        }
    }

    void print_files_and_directories(fs::path path, int depth) {
        std::cout << "\n=== Files & Directories Tree ===" << std::endl;
        std::cout << "Max depth: " << depth << std::endl;
        print_directory_tree(path, 0, depth);
    }

    void print_ip(ifaddrs *interface) {
        // Check if ifa_addr is not null
        if (!interface->ifa_addr) {
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
        if (result != 0) {
            std::cerr << "getnameinfo() failed: " << gai_strerror(result) << std::endl;
            return;
        }

        std::cout << "\t" << host << std::endl;
    }

    void print_interfaces(std::multimap<std::string, ifaddrs *> const &if_map) {
        std::string interface_name;

        for (auto const &interface: if_map) {
            if (interface.first != interface_name) {
                std::cout << interface.first << std::endl;
                interface_name = interface.first;
            }

            print_ip(interface.second);
        }
    }

    void print_network_interfaces() {
        std::cout << "\n=== Network interfaces ===" << std::endl;
        ifaddrs *interfaces;
        if (getifaddrs(&interfaces) == -1) {
            std::cerr << "Failed getting interfaces" << std::endl;
            return;
        }

        std::multimap<std::string, ifaddrs *> if_map;
        for (ifaddrs *interface = interfaces; interface != nullptr; interface = interface->ifa_next) {
            if_map.insert(std::make_pair(std::string(interface->ifa_name), interface));
        }

        print_interfaces(if_map);
        freeifaddrs(interfaces);
    }

    void list_processes() {
        std::cout << "\n=== Running processes ===" << std::endl;
        int process_count = 0;
        int total_process_count = 0;

        for (auto const &p: fs::directory_iterator("/proc")) {
            auto filename = p.path().filename();
            try {
                int pid = std::stoi(filename);
                if (process_count < 10) {
                    std::cout << pid << std::endl;
                }
                process_count++;
            } catch (std::invalid_argument) {
            }
        }

        total_process_count = process_count;
        if (process_count > 10) {
            std::cout << "... and " << process_count - 10 << " more processes" << std::endl;
        }
        std::cout << "Total processes: " << total_process_count << std::endl;
    }
}

int main(int argc, char **argv) {
    std::cerr << "Running Isolation Tester" << std::endl;
    pid_t pid = getpid();

    print_pid_and_user();
    print_files_and_directories("/", 0);
    print_caps();
    print_network_interfaces();
    list_processes();
    print_environment_variables();
    print_namespace_info(pid); // Print namespace information
}
