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
	virtual ~Task();
    Task(int id,std::mutex& mutex,ConnectionHandler &connectionHandler);
    void run();
    virtual bool getMsg()=0;
    virtual bool sendMsg()=0;
	virtual void terminate()=0;
	short bytesTOShort(char* bytesArr);
	void shortToBytes(short num, char* bytesArr);
	int addBytesToVec(std::string str, std::vector<char>& charVec);
private:
	const int _id;
protected:
	std::mutex &_mutex;
	static std::condition_variable cv;
	static bool should_terminate; //static because the two threads work together
	ConnectionHandler &_connectionHandler;
	std::string _host;
	short _port;


};

class ReadFromstdinTask : public Task{
public:
	ReadFromstdinTask(int id,std::mutex& mutex,ConnectionHandler &connectionHandler);
	~ReadFromstdinTask();
    bool getMsg();
    bool sendMsg();
	void terminate();
private:
    void set_msg(std::vector<char> command);
    int sizeOfMsg;
	char msg2server[];

};

class ReadFromSocketTask : public Task{
public:
	ReadFromSocketTask(int id,std::mutex& mutex,ConnectionHandler &connectionHandler);
	~ReadFromSocketTask();
    bool getMsg();
    bool sendMsg();
	void terminate();
private:
	short opcode;
};

#endif //TASK_H
