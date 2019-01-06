//
// Created by gal on 12/26/18.
//
#include "../include/Task.h"
#include "../include/ConnectionHandler.h"
#include "boost/algorithm/string.hpp"

//Papa Task
bool Task::should_terminate = false;
std::condition_variable Task::cv;
Task::Task(int id, std::mutex &mutex,ConnectionHandler &connectionHandler) : _id(id), _mutex(mutex), _connectionHandler(connectionHandler){
    Task::should_terminate = false;
    //Task::messages.clear();
}
void Task::run() {
    while(!Task::should_terminate) {
		getMsg();
		sendMsg();
	}
    terminate();
}

Task::~Task() {
    _connectionHandler.close();
}

void Task::shortToBytes(short num, char *bytesArr) {
	bytesArr[0] = ((num >> 8) & 0xFF);
	bytesArr[1] = (num & 0xFF);
}

short Task::bytesTOShort(char *bytesArr) {
	short result = (short)((bytesArr[0] & 0xff) << 8);
	result += (short)(bytesArr[1] & 0xff);
	return result;
}

int Task::addBytesToVec(std::string str, std::vector<char> &charVec) {
	try {
		char charArr[1024];
		strncpy(charArr, str.c_str(), str.size());
		charArr[sizeof(charArr) - 1] = 0;
		for (size_t i = 0; i < str.size(); i++) {
			char c = charArr[i];
			if (c == ' ') c = '\0';
			charVec.push_back(c);
		}
	}
	catch (...){std::cout<<"error while addBytesToVec\n";}
	return (int)str.size();
}

/**
 * ReadFromstdinTask
 */

ReadFromstdinTask::ReadFromstdinTask(int id, std::mutex &mutex,ConnectionHandler &connectionHandler) : Task(id,mutex,connectionHandler),sizeOfMsg(0){
	msg2server[1024];
}

bool ReadFromstdinTask::getMsg() {
	try {
    const short bufsize = 1024;
    char buf[bufsize];
    //read from stdin
    std::cout<<"i1\n";
    //_mutex.lock();
    std::cin.getline(buf, bufsize); //TODO: understand why this stuck
    //_mutex.unlock();
    std::cout<<"i2\n";
    std::string line(buf);
    std::vector<std::string> cmd;

    boost::split(cmd,line,[](char c){return c == ' ';});
    //check which command

        std::string typeofMessage(cmd[0]);
        if (typeofMessage == "REGISTER") {
            char opcode[2];
            Task::shortToBytes(short(1),opcode);
			std::vector<char> newMsg = {opcode[0],opcode[1]};
			addBytesToVec(cmd[1],newMsg); //username
			newMsg.push_back('\0');
			addBytesToVec(cmd[2],newMsg); //password
			newMsg.push_back('\0');
            set_msg(newMsg);
        }
        if (typeofMessage == "LOGIN") {
            char opcode[2];
            Task::shortToBytes(short(2),opcode);
            std::vector<char> newMsg = {opcode[0],opcode[1]};
			addBytesToVec(cmd[1],newMsg); //username
			newMsg.push_back('\0');
			addBytesToVec(cmd[2],newMsg); //password
			newMsg.push_back('\0');
			set_msg(newMsg);
        }
        if (typeofMessage == "LOGOUT") {
        	//TODO: should only exit when gets an ACK
            char opcode[2];
            Task::shortToBytes(short(3),opcode);
            std::vector<char> newMsg = {opcode[0],opcode[1]};
            set_msg(newMsg);
        }
        if (typeofMessage == "FOLLOW") {
            char opcode[2] ;
			char followunfollow;
			char numofusers[2];


			Task::shortToBytes(short(4),opcode);
			followunfollow = char(stoi(cmd[1]));
			shortToBytes(short(stoi(cmd[2])),numofusers);
			std::vector<char> newMsg = {opcode[0],opcode[1],followunfollow,numofusers[0],numofusers[1]};
			addBytesToVec(cmd[3],newMsg);//UserNameList
			set_msg(newMsg);
        }
        if (typeofMessage == "POST") {
			char opcode[2];
			Task::shortToBytes(short(5),opcode);
			std::vector<char> newMsg = {opcode[0],opcode[1]};
			addBytesToVec(cmd[1],newMsg); //content
			newMsg.push_back('\0');
			set_msg(newMsg);
        }
        if (typeofMessage == "PM") {
			char opcode[2];
			Task::shortToBytes(short(6),opcode);
			std::vector<char> newMsg = {opcode[0],opcode[1]};
			addBytesToVec(cmd[1],newMsg); //username
			newMsg.push_back('\0');
			addBytesToVec(cmd[2],newMsg); //content
			newMsg.push_back('\0');
			set_msg(newMsg);
        }
        if (typeofMessage == "USERLIST") {

			char opcode[2];
			Task::shortToBytes(short(7),opcode);
			std::vector<char> newMsg = {opcode[0],opcode[1]};
			set_msg(newMsg);
        }
        if (typeofMessage == "STAT") {

			char opcode[2];
			Task::shortToBytes(short(8),opcode);
			std::vector<char> newMsg = {opcode[0],opcode[1]};
			addBytesToVec(cmd[1],newMsg); //username
			newMsg.push_back('\0');
			set_msg(newMsg);
        }
        return 1;
    }
    catch (...){
    	std::cerr<<"HAD PROBLEMS READING FROM THE KEYBOARD\n";
        return 0;
    }
}
bool ReadFromstdinTask::sendMsg() {
	bool success = true;
	if (sizeOfMsg != 0)
		success = _connectionHandler.sendBytes(msg2server, sizeOfMsg);
	sizeOfMsg = 0;
	std::cout<<"TEST\n";
	return success;
}

void ReadFromstdinTask::set_msg(std::vector<char> command) {
    sizeOfMsg = (int)command.size();
	for (int i = 0; i < command.size(); ++i) {
		msg2server[i] = command[i];
		std::cout<<(int)msg2server[i]<<',';
	}
	std::cout<<"setMsg worked\n";
}

void ReadFromstdinTask::terminate() {
    std::cout<<"terminated\n";
}

ReadFromstdinTask::~ReadFromstdinTask() {
    terminate();
}


/**
 * ReadFromSocketTask
 */
ReadFromSocketTask::ReadFromSocketTask(int id, std::mutex &mutex,ConnectionHandler &connectionHandler) : Task(id,mutex,connectionHandler),opcode(65){}
bool ReadFromSocketTask::getMsg() {
    std::cout<<"trying to get message from socket...\n";
	char opcode_char[2];
    _connectionHandler.getBytes(opcode_char,2);
    opcode = bytesTOShort(opcode_char);
	std::cout<<"got a message from the server! opcode: "<<opcode<<'\n';
	return true;
}
bool ReadFromSocketTask::sendMsg() {
	std::cout << "now processing the message from socket\n";
	bool s = 1; //true-success false-failure
	try {
		//TODO: check how the message is perceived
		std::string msg_from_server;
		//std::cout << "now processing the message from socket\n";
		//check which opcode is the message that was received:
		switch ((opcode)) {
			case short(9): {
				std::cout<<"RECEIVED A NOTIF\n";
				char notifTyp;
				std::string postingUser;
				std::string content;
				std::string notifTypStr;
				s *= _connectionHandler.getBytes(&notifTyp, 1);
				s *= _connectionHandler.getFrameAscii(postingUser, '\0');
				s *= _connectionHandler.getFrameAscii(content, '\0');
				switch (notifTyp) {
					case 0:
						notifTypStr = "Public";
						break;
					case 1:
						notifTypStr = "PM";
						break;
					default:
						break;
				}
				_mutex.lock();
				std::cout << "NOTIFICATION " << notifTypStr << " " << postingUser << " " << content << '\n';
				_mutex.unlock();
				break;
			}
			case short(10): {
				std::cout<<"RECEIVED AN ACK\n";
				char msgOpcode[2];
				std::string optional;
				//TODO: handle optional properly
				s *= _connectionHandler.getBytes(msgOpcode, 2);
				//s *= _connectionHandler.getFrameAscii(optional, '\0');
				//_mutex.lock();
				std::cout << "ACK " << bytesTOShort(msgOpcode) << "" << '\n';
				//_mutex.unlock();
				break;
			}
			case short(11): {
				std::cout<<"RECEIVED AN ERROR\n";
				char errorOpcode[2];
				s *= _connectionHandler.getBytes(errorOpcode, 2);
				_mutex.lock();
				std::cout << "ERROR " << errorOpcode << '\n';
				_mutex.unlock();
				break;
			}
			default:
				std::cout << "could not figure out what the message is\n";
				break;
		}
	}
	catch (...){
		std::cout<<"error w/ send msg in taskReadFromSocket\n";
	}
    return s;
}

void ReadFromSocketTask::terminate() {
    Task::should_terminate = true;
    _connectionHandler.close();
}
ReadFromSocketTask::~ReadFromSocketTask() {
	terminate();
}
