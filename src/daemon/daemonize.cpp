/* This file is part of libesf.
 * 
 * Copyright (c) 2019, Alexandre Monti
 * 
 * libesf is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * libesf is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with libesf.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "lesf/daemon/daemonize.h"
#include "lesf/daemon/defaults.h"

#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <cstring>
#include <csignal>
#include <cerrno>
#include <syslog.h>

#include <iostream>

using namespace lesf;
using namespace daemon;

static struct
{
    std::function<Service*()> ctor;

    int lockfile_fd;
    int lockfile_fn;

    int pid;
    Service* service;
} daemon_data;

static void daemon_try_cleanup()
{
    lockf(daemon_data.lockfile_fd, F_ULOCK, 0);
    close(daemon_data.lockfile_fd);
    unlink(Defaults::LockFile);
}

static void daemon_signal_handler(int signo)
{
    if (signo == SIGINT || signo == SIGTERM)
    {
        daemon_try_cleanup();

        // Stop service
        daemon_data.service->stop();

        // Reset signal handling to default behavior (and terminate program)
        signal(signo, SIG_DFL);
    }
    else if (signo == SIGHUP)
    {
        daemon_data.service->restart();
    }
    else if (signo == SIGSEGV || signo == SIGABRT)
    {
        signal(signo, SIG_DFL);
        daemon_try_cleanup();
        
        //TODO: save stacktrace
        
        raise(SIGABRT);
    }
}

void lesf::daemon::daemonize(std::function<Service*()> const& ctor)
{
    // Save daemon information
    daemon_data.ctor = ctor;
    daemon_data.service = 0;

    pid_t pid = fork();

    // fork() failed
    if (pid < 0)
    {
        std::cerr << "lesf::daemon::daemonize: first fork() failed (" << strerror(errno) << ")" << std::endl;
        exit(EXIT_FAILURE);
    }
    // fork() succeeded : let the parent terminate
    else if (pid > 0)
    {
        exit(EXIT_SUCCESS);
    }

    // Child becomes sessions leader
    if (setsid() < 0)
    {
        std::cerr << "lesf::daemon::daemonize: setsid() failed (" << strerror(errno);
        std::cout << ")" << std::endl;
        exit(EXIT_FAILURE);
    }

    // Ignore signals sent from chld to parent process
    signal(SIGCHLD, SIG_IGN);

    pid = fork();

    // Second fork() failed
    if (pid < 0)
    {
        std::cerr << "lesf::daemon::daemonize: second fork() failed (" << strerror(errno);
        std::cout << ")" << std::endl;
        exit(EXIT_FAILURE);
    }
    // fork() succeeded : let the parent terminate
    else if (pid > 0)
    {
        exit(EXIT_SUCCESS);
    }

    // Set file permissions, change directory
    umask(0);
    chdir("/");

    // Get current pid
    daemon_data.pid = getpid();

    // Create log file name with pid suffix
    char pid_str[256];
    sprintf(pid_str, "%d\n", daemon_data.pid);

    std::size_t logfile_len = strlen(Defaults::LogFilePrefix) + strlen(pid_str);

    char* logfile_fn = (char*) malloc(logfile_len+1);
    if (!logfile_fn)
        exit(EXIT_FAILURE);

    strcpy(logfile_fn, Defaults::LogFilePrefix);
    strcpy(logfile_fn + strlen(Defaults::LogFilePrefix), pid_str);
    logfile_fn[logfile_len-1] = '\0';

    // Redirect stdios
    close(0);
    close(1);
    close(2);
    open("/dev/null", O_RDONLY);
    open(logfile_fn, O_WRONLY | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    dup(1);

    free(logfile_fn);

    // Create lockfile
    daemon_data.lockfile_fd = open(Defaults::LockFile, O_RDWR | O_CREAT, 0640);
    if (daemon_data.lockfile_fd < 0)
    {
        std::cout << "lesf::daemon::daemonize: failed to create pid lockfile (";
        std::cout << strerror(errno) << ")" << std::endl;
        exit(EXIT_FAILURE);
    }

    // Lock it file
    if (lockf(daemon_data.lockfile_fd, F_TLOCK, 0) < 0)
    {
        std::cout << "lesf::daemon::daemonize: failed to lock lockfile (";
        std::cout << strerror(errno) << ")" << std::endl;
        exit(EXIT_FAILURE);
    }

    // Write pid in lockfile
    write(daemon_data.lockfile_fd, pid_str, strlen(pid_str));

    // Install various signal handlers
    signal(SIGINT, &daemon_signal_handler);
    signal(SIGTERM, &daemon_signal_handler);
    signal(SIGHUP, &daemon_signal_handler);
    signal(SIGSEGV, &daemon_signal_handler);
    signal(SIGABRT, &daemon_signal_handler);

    // Start service
    daemon_data.service = daemon_data.ctor();
    daemon_data.service->run();
    daemon_data.service->stop();
    delete daemon_data.service;

    daemon_try_cleanup();
    exit(EXIT_SUCCESS);
}
