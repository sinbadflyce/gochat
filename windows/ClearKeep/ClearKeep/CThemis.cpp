
#define _CRT_SECURE_NO_WARNINGS

#pragma once

#include <cstdlib>
#include <deque>
#include <iostream>
#include <thread>
#include <boost/asio.hpp>
#include <boost/asio.hpp>
#include <boost/thread/thread.hpp>
#include "CThemis.h"
#include "message.h"

using boost::asio::ip::tcp;
using namespace std;

const uint8_t client_private_key[] = { 0x52, 0x45, 0x43, 0x32, 0x00, 0x00, 0x00, 0x2d, 0x00, 0xb2, 0x7f, 0x81, 0x00, 0x60, 0x9d, 0xe7, 0x7a, 0x39, 0x93, 0x68, 0xfc, 0x25, 0xd1, 0x79, 0x88, 0x6d, 0xfb, 0xf6, 0x19, 0x35, 0x53, 0x74, 0x10, 0xfc, 0x5b, 0x44, 0xe1, 0xf6, 0xf4, 0x4e, 0x59, 0x8d, 0x94, 0x99, 0x4f };
const uint8_t client_public_key[] = { 0x55, 0x45, 0x43, 0x32, 0x00, 0x00, 0x00, 0x2d, 0x10, 0xf4, 0x68, 0x8c, 0x02, 0x1c, 0xd0, 0x3b, 0x20, 0x84, 0xf2, 0x7a, 0x38, 0xbc, 0xf6, 0x39, 0x74, 0xbf, 0xc3, 0x13, 0xae, 0xb1, 0x00, 0x26, 0x78, 0x07, 0xe1, 0x7f, 0x63, 0xce, 0xe0, 0xb8, 0xac, 0x02, 0x10, 0x40, 0x10 };

const uint8_t server_private_key[] = { 0x52, 0x45, 0x43, 0x32, 0x00, 0x00, 0x00, 0x2d, 0xd0, 0xfd, 0x93, 0xc6, 0x00, 0xae, 0x83, 0xb3, 0xef, 0xef, 0x06, 0x2c, 0x9d, 0x76, 0x63, 0xf2, 0x50, 0xd8, 0xac, 0x32, 0x6e, 0x73, 0x96, 0x60, 0x53, 0x77, 0x51, 0xe4, 0x34, 0x26, 0x7c, 0xf2, 0x9f, 0xb6, 0x96, 0xeb, 0xd8 };
const uint8_t server_public_key[] = { 0x55, 0x45, 0x43, 0x32, 0x00, 0x00, 0x00, 0x2d, 0xa5, 0xb3, 0x9b, 0x9d, 0x03, 0xcd, 0x34, 0xc5, 0xc1, 0x95, 0x6a, 0xb2, 0x50, 0x43, 0xf1, 0x4f, 0xe5, 0x88, 0x3a, 0x0f, 0xb1, 0x11, 0x8c, 0x35, 0x81, 0x82, 0xe6, 0x9e, 0x5c, 0x5a, 0x3e, 0x14, 0x06, 0xc5, 0xb3, 0x7d, 0xdd };


typedef std::deque< ChatMessage >  chatMessageQueue_t;


class ChatClient {
public:
	ChatClient(boost::asio::io_service& io_service,
		tcp::resolver::iterator endpoint_iterator)
		: io_service_(io_service),
		socket_(io_service)
	{
		boost::asio::async_connect(socket_, endpoint_iterator,
			boost::bind(&ChatClient::handle_connect, this,
				boost::asio::placeholders::error));
	}

	void write(const ChatMessage& msg)
	{
		io_service_.post(boost::bind(&ChatClient::do_write, this, msg));
	}

	void close()
	{
		io_service_.post(boost::bind(&ChatClient::do_close, this));
	}

private:

	void handle_connect(const boost::system::error_code& error)
	{
		if (!error)
		{
			boost::asio::async_read(socket_,
				boost::asio::buffer(read_msg_.data(), ChatMessage::header_length),
				boost::bind(&ChatClient::handle_read_header, this,
					boost::asio::placeholders::error));
		}
	}

	void handle_read_header(const boost::system::error_code& error)
	{
		if (!error && read_msg_.decode_header())
		{
			boost::asio::async_read(socket_,
				boost::asio::buffer(read_msg_.body(), read_msg_.body_length()),
				boost::bind(&ChatClient::handle_read_body, this,
					boost::asio::placeholders::error));
		}
		else
		{
			do_close();
		}
	}

	void handle_read_body(const boost::system::error_code& error)
	{
		if (!error)
		{
			std::cout.write(read_msg_.body(), read_msg_.body_length());
			std::cout << "\n";
			boost::asio::async_read(socket_,
				boost::asio::buffer(read_msg_.data(), ChatMessage::header_length),
				boost::bind(&ChatClient::handle_read_header, this,
					boost::asio::placeholders::error));
		}
		else
		{
			do_close();
		}
	}

	void do_write(ChatMessage msg)
	{
		bool write_in_progress = !write_msgs_.empty();
		write_msgs_.push_back(msg);
		if (!write_in_progress)
		{
			boost::asio::async_write(socket_,
				boost::asio::buffer(write_msgs_.front().data(),
					write_msgs_.front().length()),
				boost::bind(&ChatClient::handle_write, this,
					boost::asio::placeholders::error));
		}
	}

	void handle_write(const boost::system::error_code& error)
	{
		if (!error)
		{
			write_msgs_.pop_front();
			if (!write_msgs_.empty())
			{
				boost::asio::async_write(socket_,
					boost::asio::buffer(write_msgs_.front().data(),
						write_msgs_.front().length()),
					boost::bind(&ChatClient::handle_write, this,
						boost::asio::placeholders::error));
			}
		}
		else
		{
			do_close();
		}
	}

	void do_close()
	{
		socket_.close();
	}

private:
	boost::asio::io_service& io_service_;
	tcp::socket socket_;
	ChatMessage read_msg_;
	chatMessageQueue_t write_msgs_;
};


CThemis::CThemis()
{
	nType = 0; // default using EC as algorithm encrypt
	strHost = "";
	strPort = "";
	
}

CThemis::~CThemis()
{

}

int CThemis::doConnnection()
{
	return 0;
}

void CThemis::doLogin(string strUsername, string password)
{
	// Create pair key
	createPairKey();

	// processing with raw login message [9/20/2017 Canhnh]
	pLogin.set_username(strUsername.c_str());
	pLogin.set_type(1);
	pLogin.set_platform("window");
	pLogin.set_authentoken(nullptr);
	pLogin.set_devicetoken(nullptr);

	// encrypt raw login message 
 	//themispp::secure_message_t sm(std::vector<uint8_t>(prKey, prKey + sizeof(prKey)), std::vector<uint8_t>(server_public_key, server_public_key + sizeof(server_public_key)));
	themispp::secure_message_t smsg(prKey, plKey);

 	boost::asio::io_service io_service;
 	tcp::resolver resolver(io_service);
 	tcp::resolver::query query(tcp::v4(), strHost, strPort);
 	tcp::resolver::iterator iterator = resolver.resolve(query);
 
 	tcp::socket s(io_service);
 	s.connect(*iterator);


	// send or connect to server


}

void CThemis::createPairKey()
{
	if (!nType)
	{
		themispp::secure_key_pair_generator_t<themispp::EC> g;
		prKey = g.get_priv();
		plKey = g.get_pub();
	}
	else
	{
		themispp::secure_key_pair_generator_t<themispp::RSA> g;
		prKey = g.get_priv();
		plKey = g.get_pub();
	}
}
