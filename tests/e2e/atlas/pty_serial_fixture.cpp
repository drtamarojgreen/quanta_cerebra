// PTY-based end-to-end fixture for brain_modeler --serial.
//
// Usage: pty_serial_fixture <path-to-brain_modeler-binary>
//
// What it does:
//   1. Opens a POSIX pseudoterminal pair via posix_openpt.
//   2. Forks brain_modeler with --serial <slave-path>, --report,
//      --serial-frames 3, --theme mono. The child's stdout is piped back here.
//   3. Writes 3 JSON brain-activity frames over the master FD, simulating an
//      IoT device pushing frames at ~20 Hz.
//   4. Drains the child's stdout, waits for the child to exit, and verifies
//      the rendered report mentions every region we streamed and reports the
//      correct frame count.
//
// Exit codes:
//   0  success
//   2  bad usage
//   3  setup error (pty/pipe/fork)
//   4  child died abnormally
//   5  child exited non-zero (output dumped to stderr)
//   6  expected content not found in child's stdout (output dumped to stderr)

#include <cerrno>
#include <chrono>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <stdlib.h>
#include <string>
#include <sys/select.h>
#include <sys/wait.h>
#include <thread>
#include <unistd.h>
#include <vector>

namespace {

bool contains_all(const std::string& hay, std::initializer_list<const char*> needles) {
    for (const char* n : needles) {
        if (hay.find(n) == std::string::npos) return false;
    }
    return true;
}

ssize_t write_full(int fd, const std::string& s) {
    ssize_t off = 0;
    while (off < static_cast<ssize_t>(s.size())) {
        ssize_t n = ::write(fd, s.data() + off, s.size() - off);
        if (n < 0) {
            if (errno == EINTR) continue;
            return -1;
        }
        off += n;
    }
    return off;
}

}

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "usage: " << argv[0] << " <path-to-brain_modeler>\n";
        return 2;
    }
    const std::string binary = argv[1];

    // --- Open PTY pair ---
    int master = posix_openpt(O_RDWR | O_NOCTTY);
    if (master < 0)         { std::perror("posix_openpt");  return 3; }
    if (grantpt(master)  != 0) { std::perror("grantpt");    return 3; }
    if (unlockpt(master) != 0) { std::perror("unlockpt");   return 3; }
    const char* slave = ptsname(master);
    if (!slave) { std::perror("ptsname"); return 3; }
    const std::string slave_path = slave;

    // --- Pipe to capture child stdout ---
    int outpipe[2];
    if (pipe(outpipe) != 0) { std::perror("pipe"); return 3; }

    pid_t pid = fork();
    if (pid < 0) { std::perror("fork"); return 3; }
    if (pid == 0) {
        // Child: rewire stdout to the pipe, close everything else.
        ::close(master);
        ::close(outpipe[0]);
        if (dup2(outpipe[1], STDOUT_FILENO) < 0) _exit(126);
        ::close(outpipe[1]);

        std::vector<const char*> args = {
            binary.c_str(),
            "--report",
            "--serial",        slave_path.c_str(),
            "--serial-frames", "3",
            "--serial-baud",   "115200",
            "--theme",         "mono",
            nullptr,
        };
        execv(binary.c_str(), const_cast<char* const*>(args.data()));
        _exit(127);
    }

    // --- Parent: feed JSON frames over the master ---
    ::close(outpipe[1]);

    // Brief pause so the child has time to open the slave.
    std::this_thread::sleep_for(std::chrono::milliseconds(150));

    const std::string frames[3] = {
        R"({"brain_activity":[{"region":"amygdala","intensity":0.55}],"timestamp_ms":0})" "\n",
        R"({"brain_activity":[{"region":"thalamus","intensity":0.66}],"timestamp_ms":100})" "\n",
        R"({"brain_activity":[{"region":"hippocampus","intensity":0.77}],"timestamp_ms":200})" "\n",
    };
    for (const auto& f : frames) {
        if (write_full(master, f) < 0) { std::perror("write master"); }
        std::this_thread::sleep_for(std::chrono::milliseconds(30));
    }

    // --- Drain child stdout until EOF, then reap. ---
    std::string out;
    char buf[1024];
    while (true) {
        ssize_t n = ::read(outpipe[0], buf, sizeof(buf));
        if (n > 0) out.append(buf, n);
        else if (n == 0) break;
        else if (errno == EINTR) continue;
        else break;
    }
    ::close(outpipe[0]);
    ::close(master);

    int status = 0;
    if (waitpid(pid, &status, 0) < 0) { std::perror("waitpid"); return 4; }
    if (!WIFEXITED(status)) {
        std::cerr << "child did not exit normally\n";
        return 4;
    }
    if (WEXITSTATUS(status) != 0) {
        std::cerr << "child exit code: " << WEXITSTATUS(status) << "\n";
        std::cerr << "------ child stdout ------\n" << out << "\n--------\n";
        return 5;
    }

    if (!contains_all(out, {
            "Amygdala", "Thalamus", "Hippocampus",
            "frame 3/3", "theme=mono", "t=0.20s",
        }))
    {
        std::cerr << "expected content missing\n";
        std::cerr << "------ child stdout ------\n" << out << "\n--------\n";
        return 6;
    }
    return 0;
}
