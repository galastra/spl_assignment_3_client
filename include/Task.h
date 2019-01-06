//
// Created by gal on 12/26/18.
//

#ifndef TASK_H
#define TASK_H

#include <mutex>
#include "ConnectionHandler.h"
#include <condition_variable>

class Task {
public:
	virtual ~Task() = default;
    Task(int id,std::mutex& mutex,std::condition_variable& cv, ConnectionHandler &connectionHandler);
    void run();
    virtual bool getMsg()=0;
    virtual bool sendMsg()=0;
	short bytesTOShort(char* bytesArr);
	void shortToBytes(short num, char* bytesArr);
	int addBytesToVec(std::string str, std::vector<char>& charVec);
private:
	const int _id;
protected:
	std::mutex &_mutex;
	std::condition_variable &_cv;
	static bool sent_LOGOUT; //static because the two threads work together
	static bool recv_ACK4LOGOUT;
	ConnectionHandler &_connectionHandler;


};

class ReadFromstdinTask : public Task{
public:
	ReadFromstdinTask(int id,std::mutex& mutex,std::condition_variable &cv, ConnectionHandler &connectionHandler);
	~ReadFromstdinTask() =default;
    bool getMsg();
    bool sendMsg();
private:
    void set_msg(std::vector<char> command);
    int sizeOfMsg;
	char msg2server[1024];

};

class ReadFromSocketTask : public Task{
public:
	ReadFromSocketTask(int id,std::mutex& mutex,std::condition_variable &cv,ConnectionHandler &connectionHandler);
	~ReadFromSocketTask() = default;
    bool getMsg();
    bool sendMsg();
private:
	short opcode;
};

#endif //TASK_H
