//
// Created by gal on 12/26/18.
//
#include "../include/Task.h"
#include "../include/ConnectionHandler.h"
#include "boost/algorithm/string.hpp"
//Papa Task
bool Task::sent_LOGOUT = false;
bool Task::recv_ACK4LOGOUT = false;
Task::Task(int id, std::mutex &mutex,std::condition_variable &cv,ConnectionHandler &connectionHandler) : _id(id), _mutex(mutex),_cv(cv), _connectionHandler(connectionHandler){
    Task::sent_LOGOUT = false;
	recv_ACK4LOGOUT = false;
}
void Task::run() {
    while(!Task::sent_LOGOUT && !Task::recv_ACK4LOGOUT) {
		getMsg();
		sendMsg();
	}
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
	for (size_t i = 0; i < str.size(); i++) {
		charVec.push_back(str[i]);
	}
	return (int)str.size();
}

/**
 * ReadFromstdinTask
 */

ReadFromstdinTask::ReadFromstdinTask(int id, std::mutex &mutex,std::condition_variable &cv, ConnectionHandler &connectionHandler) : Task(id,mutex,cv,connectionHandler),sizeOfMsg(0){
}

bool ReadFromstdinTask::getMsg() {
	try {
    const short bufsize = 1024;
    char buf[bufsize];
    //read from stdin
    std::cin.getline(buf, bufsize);
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
            char opcode[2];
            Task::shortToBytes(short(3),opcode);
            std::vector<char> newMsg = {opcode[0],opcode[1]};
			set_msg(newMsg);
			sent_LOGOUT = true;
        }
        if (typeofMessage == "FOLLOW") {
            char opcode[2] ;
			char followunfollow;
			char numofusers[2];
			Task::shortToBytes(short(4),opcode);
			followunfollow = char(stoi(cmd[1]));
			shortToBytes(short(stoi(cmd[2])),numofusers);
			std::vector<char> newMsg = {opcode[0],opcode[1],followunfollow,numofusers[0],numofusers[1]};
			for (int i = 3; i < int(cmd.size()); i++) {
				addBytesToVec(cmd[i], newMsg);//UserNameList
				newMsg.push_back('\0');
			}
			set_msg(newMsg);
        }
        if (typeofMessage == "POST") {
			char opcode[2];
			Task::shortToBytes(short(5),opcode);
			std::vector<char> newMsg = {opcode[0],opcode[1]};
			addBytesToVec(line.substr(line.find(' ') + 1), newMsg); //content
			newMsg.push_back('\0');
			set_msg(newMsg);
        }
        if (typeofMessage == "PM") {
			char opcode[2];
			Task::shortToBytes(short(6),opcode);
			std::vector<char> newMsg = {opcode[0],opcode[1]};
			addBytesToVec(line = line.substr(line.find(' ') + 1), newMsg); //username
			newMsg.push_back('\0');
			addBytesToVec(line.substr(line.find(' ') + 1), newMsg);//content
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
        return 0;
    }
}
bool ReadFromstdinTask::sendMsg() {
	bool success = true;
	if (sizeOfMsg != 0)
		success = _connectionHandler.sendBytes(msg2server, sizeOfMsg);
	sizeOfMsg = 0;
	if (sent_LOGOUT) {
		std::unique_lock<std::mutex> lk(_mutex);
		_cv.wait(lk);
		sent_LOGOUT = recv_ACK4LOGOUT;
	}
	return success;
}

void ReadFromstdinTask::set_msg(std::vector<char> command) {
    sizeOfMsg = (int)command.size();
	for (int i = 0; i < sizeOfMsg; ++i) {
		msg2server[i] = command[i];
	}
}



/**
 * ReadFromSocketTask
 */
ReadFromSocketTask::ReadFromSocketTask(int id, std::mutex &mutex,std::condition_variable &cv, ConnectionHandler &connectionHandler) : Task(id,mutex,cv,connectionHandler),opcode(0){}
bool ReadFromSocketTask::getMsg() {
	char opcode_char[2];
    _connectionHandler.getBytes(opcode_char,2);
    opcode = bytesTOShort(opcode_char);
	return true;
}
bool ReadFromSocketTask::sendMsg() {
	bool s = 1; //true-success false-failure
	std::string msg_from_server;
		//check which opcode is the message that was sved:
	switch ((opcode)) {
		case short(9): {
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
			char ack_opcode_arr[2];
			std::string optional="";
			//TODO: handle optional properly
			s *= _connectionHandler.getBytes(ack_opcode_arr, 2);
				
			short ack_opcode = bytesTOShort(ack_opcode_arr);
			switch (ack_opcode) {
			case 3: //LOGOUT
				recv_ACK4LOGOUT = true;
				_cv.notify_all();
				break;
			case 7 : //USERLIST
			case 4 : //FOLLOW
			{
				char numOfUsers_arr[2];
				short numOfUsers;
				s *= _connectionHandler.getBytes(numOfUsers_arr, 2);
				numOfUsers = bytesTOShort(numOfUsers_arr);
				optional += numOfUsers;
				for (int i=0;i<(int)numOfUsers;i++)
					s *= _connectionHandler.getFrameAscii(optional, '\0');
			}
				break;
			case 8: //STAT
			{
				char numPosts_arr[2];
				char numFollowers_arr[2];
				char numFollowing_arr[2];
				s *= _connectionHandler.getBytes(numPosts_arr, 2);
				s *= _connectionHandler.getBytes(numFollowers_arr, 2);
				s *= _connectionHandler.getBytes(numFollowing_arr, 2);
				optional += bytesTOShort(numPosts_arr) + bytesTOShort(numFollowers_arr) + bytesTOShort(numFollowing_arr);
			}
				break;
			}
			std::cout << "ACK " << ack_opcode << optional << '\n';
			break;
		}
		case short(11): {
			char errorOpcode_arr[2];
			s *= _connectionHandler.getBytes(errorOpcode_arr, 2);
			short errorOpcode = bytesTOShort(errorOpcode_arr);
			if (errorOpcode == short(3)) {
				recv_ACK4LOGOUT = false;
				_cv.notify_all();
			}
			std::cout << "ERROR " << errorOpcode << '\n';
			break;
		}
	}
    return s;
}
