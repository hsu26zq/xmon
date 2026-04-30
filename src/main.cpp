#include <ncurses.h>
#include <dirent.h>
#include <unistd.h>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <map>
#include <algorithm>

struct Proc {
    int pid;
    std::string name;
    std::string state;
    long rss_kb;
    int threads;
};

std::string read_file(const std::string& path) {
    std::ifstream f(path);
    std::string s;
    std::getline(f, s);
    return s;
}

std::string get_name(int pid) {
    return read_file("/proc/" + std::to_string(pid) + "/comm");
}

std::string get_status(int pid, const std::string& key) {
    std::ifstream f("/proc/" + std::to_string(pid) + "/status");
    std::string line;
    while (std::getline(f, line)) {
        if (line.find(key) == 0) {
            std::istringstream iss(line);
            std::string k;
            iss >> k;
            return line.substr(k.size());
        }
    }
    return "";
}

int get_thread_count(int pid) {
    std::string path = "/proc/" + std::to_string(pid) + "/task";
    DIR* d = opendir(path.c_str());
    if (!d) return 0;

    int count = 0;
    dirent* e;
    while ((e = readdir(d))) {
        if (e->d_name[0] != '.')
            count++;
    }
    closedir(d);
    return count;
}

long parse_rss_kb(int pid) {
    std::ifstream f("/proc/" + std::to_string(pid) + "/status");
    std::string line;
    while (std::getline(f, line)) {
        if (line.find("VmRSS:") == 0) {
            std::istringstream iss(line);
            std::string k;
            long val;
            std::string unit;
            iss >> k >> val >> unit;
            return val;
        }
    }
    return 0;
}

std::vector<Proc> scan_procs() {
    std::vector<Proc> out;

    DIR* d = opendir("/proc");
    if (!d) return out;

    dirent* e;
    while ((e = readdir(d))) {
        std::string name = e->d_name;
        if (!std::all_of(name.begin(), name.end(), ::isdigit))
            continue;

        int pid = std::stoi(name);

        Proc p;
        p.pid = pid;
        p.name = get_name(pid);
        p.state = get_status(pid, "State:");
        p.rss_kb = parse_rss_kb(pid);
        p.threads = get_thread_count(pid);

        out.push_back(p);
    }

    closedir(d);
    return out;
}

void draw(const std::vector<Proc>& procs) {
    clear();

    mvprintw(0, 0, "xmon v0 - process monitor");

    mvprintw(2, 0, "PID   NAME              STATE   RSS(MB)   THREADS");

    int row = 3;
    for (auto& p : procs) {
        mvprintw(row++, 0,
            "%-5d %-16s %-7s %-8ld %-7d",
            p.pid,
            p.name.c_str(),
            p.state.c_str(),
            p.rss_kb / 1024,
            p.threads
        );
    }

    refresh();
}

int main() {
    initscr();
    noecho();
    curs_set(0);

    while (true) {
        auto procs = scan_procs();
        draw(procs);
        usleep(1000000); // 1 sec
    }

    endwin();
    return 0;
}
