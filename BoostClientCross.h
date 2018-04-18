/*
Source code for the KUKA robot controller communication client.
Author: Eirik B. Njaastad.
NTNU 2015

Communicates with the KUKAVARPROXY server made by
Massimiliano Fago - massimiliano.fago@gmail.com
*/

#ifndef BOOSTCLIENTCROSS
#define BOOSTCLIENTCROSS

#include <iostream>
#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>

boost::asio::io_service iosClientCross;
boost::asio::ip::tcp::socket socketClientCross(iosClientCross);
boost::system::error_code socketError;

class BOOSTCLIENTCROSS{
public:
	// Function for opening a socket connection and initiate the server connection:
	void connectSocket(std::string ipAddress, std::string portNumber){
		socketClientCross.connect(
			boost::asio::ip::tcp::endpoint(boost::asio::ip::address::from_string(ipAddress),
			boost::lexical_cast<unsigned>(portNumber)));
	}
	// For writing a variable to the robot controller, the message to send must contain
	// a variable name (varName) and a value to write (varValue).
	std::vector<unsigned char> formatWriteMsg(std::vector<unsigned char> varName, std::vector<unsigned char> varValue){
		std::vector<unsigned char> header, block;
		int varNameLength, varValueLength, blockSize;
		int messageId;
		BYTE hbyte, lbyte, hbytemsg, lbytemsg;

		varNameLength = varName.size();
		varValueLength = varValue.size();
		messageId = 05;

		hbyte = (BYTE)((varNameLength >> 8) & 0xff00);
		lbyte = (BYTE)(varNameLength & 0x00ff);

		block.push_back((unsigned char)1);
		block.push_back((unsigned char)hbyte);
		block.push_back((unsigned char)lbyte);

		for (int i = 0; i != varNameLength; ++i) {
			block.push_back(varName[i]);
		}

		hbyte = (BYTE)((varValueLength >> 8) & 0xff00);
		lbyte = (BYTE)(varValueLength & 0x00ff);

		block.push_back((unsigned char)hbyte);
		block.push_back((unsigned char)lbyte);

		for (int i = 0; i != varValueLength; ++i) {
			block.push_back(varValue[i]);
		}

		blockSize = block.size();
		hbyte = (BYTE)((blockSize >> 8) & 0xff00);
		lbyte = (BYTE)(blockSize & 0x00ff);

		hbytemsg = (BYTE)((messageId >> 8) & 0xff00);
		lbytemsg = (BYTE)(messageId & 0x00ff);

		header.push_back((unsigned char)hbytemsg);
		header.push_back((unsigned char)lbytemsg);
		header.push_back((unsigned char)hbyte);
		header.push_back((unsigned char)lbyte);

		block.insert(block.begin(), header.begin(), header.end());
		return block;
	}
	// For reading a variable from the robot controller, the message to send must contain
	// the desired variable name (varName).
	std::vector<unsigned char> formatReadMsg(std::vector<unsigned char> varName){
		std::vector<unsigned char> header, block;
		int varNameLength, blockSize;
		int messageId;
		BYTE hbyte, lbyte, hbytemsg, lbytemsg;

		varNameLength = varName.size();
		messageId = 05;

		hbyte = (BYTE)((varNameLength >> 8) & 0xff00);
		lbyte = (BYTE)(varNameLength & 0x00ff);

		block.push_back((unsigned char)0);
		block.push_back((unsigned char)hbyte);
		block.push_back((unsigned char)lbyte);

		for (int i = 0; i != varNameLength; ++i) {
			block.push_back(varName[i]);
		}

		blockSize = block.size();

		hbyte = (BYTE)((blockSize >> 8) & 0xff00);
		lbyte = (BYTE)(blockSize & 0x00ff);

		hbytemsg = (BYTE)((messageId >> 8) & 0xff00);
		lbytemsg = (BYTE)(messageId & 0x00ff);

		header.push_back((unsigned char)hbytemsg);
		header.push_back((unsigned char)lbytemsg);
		header.push_back((unsigned char)hbyte);
		header.push_back((unsigned char)lbyte);

		block.insert(block.begin(), header.begin(), header.end());
		return block;
	}
	// Send the formatted message and recieve server response: 
	std::vector<unsigned char> sendMsg(std::vector<unsigned char> message){
		// Send message:
		const size_t bytes = boost::asio::write(socketClientCross, boost::asio::buffer(message));
		
		// Read answer:
		boost::array<unsigned char, 7> recheader;
		size_t sendLen = socketClientCross.read_some(boost::asio::buffer(recheader), socketError); // Header
		int messageLength = recheader[3] - 6;
		std::vector<unsigned char> recblock(messageLength);
		size_t recLen = socketClientCross.read_some(boost::asio::buffer(recblock), socketError); // Message

		// Error handling:
		if (socketError == boost::asio::error::eof)
			std::cout << "Connection closed cleanly by peer" << std::endl;
		else if (socketError)
			throw boost::system::system_error(socketError); // Some other error.	

		// Print results (alternative):
		// std::cout << "received:: " << std::endl;
		// for (int i = 0; i != recLen; ++i){
		//	std::cout << recblock[i];
		}
	}
	// Function for terminating the socket and thus disconnect from server:
	void disconnectSocket(){
		socketClientCross.shutdown(boost::asio::ip::tcp::socket::shutdown_both, socketError);
		socketClientCross.close();

		// Error handling:
		if (socketError)
			throw boost::system::system_error(socketError);
	}
}; 

#endif
