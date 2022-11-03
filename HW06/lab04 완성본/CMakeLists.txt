cmake_minimum_required(VERSION 3.23)
project(lab04_ C)

set(CMAKE_C_STANDARD 11)

add_executable(lab04_
        cal_linux_cli.c
        cal_linux_server.c
        child_wait.c
        command.c
        daemonOSNW.c
        echo_client.c
        echo_server_fork.c
        exec_test.c
        fork_test.c
        myshell.c)
