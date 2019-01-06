#include <iostream>
#include <thread>
#include "../include/Task.h"

int main(int argc, char *argv[]) {
    //TODO: rule of 5

    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " host port" << std::endl << std::endl;
        return -1;
    }

    std::string host = argv[1];
    short port = atoi(argv[2]);
    ConnectionHandler connHandler(host,port);
    connHandler.connect();

    std::mutex mutex;
    ReadFromstdinTask stdinTask(0,mutex,connHandler);
    ReadFromSocketTask socketTask(1,mutex,connHandler);
    //stdinTask.getMsg();
    //stdinTask.sendMsg();
    std::thread stdin_th(&Task::run, &stdinTask);
    std::thread socket_th(&Task::run, &socketTask);
    stdin_th.join();
    socket_th.join();
    std::cout << "dolav: try to join threads" << std::endl;
    return 0;
}