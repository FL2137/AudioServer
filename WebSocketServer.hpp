#pragma once

#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>

#include <algorithm>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <vector>

#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio/dispatch.hpp>
#include <boost/asio/strand.hpp>

typedef websocketpp::server<websocketpp::config::asio> server;

namespace beast = boost::beast;
namespace http = beast::http;
namespace websocket = beast::websocket;
namespace net = boost::asio;
using tcp = boost::asio::ip::tcp;

void beastFail(beast::error_code error, const char* what) {
	std::cerr << what << ": " << error.message() << std::endl;
}

class BeastSession : public std::enable_shared_from_this<BeastSession> {
	websocket::stream<beast::tcp_stream> _ws;
	beast::flat_buffer _buffer;

public:
	BeastSession(tcp::socket&& socket, std::function<void(std::string, std::string&)> parser) : _ws(std::move(socket)) {
		this->parser = parser;
	}

	void run() {
		boost::asio::dispatch(	
			_ws.get_executor(),
			beast::bind_front_handler(
				&BeastSession::onRun,
				shared_from_this()
		));
	}

	void onRun() {
		_ws.set_option(websocket::stream_base::timeout::suggested(beast::role_type::server));

		_ws.set_option(websocket::stream_base::decorator([](websocket::response_type& res) {
			res.set(http::field::server,
			std::string(BOOST_BEAST_VERSION_STRING) + " websocket-async-server");
		}));

		_ws.async_accept(
			beast::bind_front_handler(&BeastSession::onAccept, shared_from_this())
		);
	}

	void onAccept(beast::error_code error) {
		if (error) {
			return beastFail(error, "accept");
		}
		std::cout << "Accepted connection\n";
		doRead();
	}

	void doRead() {
		_ws.async_read(_buffer, beast::bind_front_handler(&BeastSession::readHandler, shared_from_this()));
	}

	void readHandler(beast::error_code error, size_t bytesTransferred) {
		std::cout << "::readHandler() \n";

		boost::ignore_unused(bytesTransferred);
		if (error == websocket::error::closed)
			return;

		if (error)
			return beastFail(error, "Read");

		//*message parsing*
		//std::string request((char*)_buffer.data().data());
		//std::cout << request << std::endl; 
		//std::string response = "response from server";
		//parser(request, response);
		_ws.text(_ws.got_text());

		std::string str = beast::buffers_to_string(_buffer.data());
		
		parser(str, str);

		beast::flat_buffer b;
		beast::ostream(b) << str;
		

		_ws.async_write(b.data(), beast::bind_front_handler(&BeastSession::writeHandler, shared_from_this()));
	}

	void writeHandler(beast::error_code error, size_t bytesTransferred) {
		boost::ignore_unused(bytesTransferred);

		if (error)
			return beastFail(error, "Write");

		_buffer.consume(_buffer.size());

		doRead();
	}

private:
	std::function<void(std::string, std::string&)> parser;

};

class BeastWebSocket : public std::enable_shared_from_this<BeastWebSocket> {

	boost::asio::io_context& ioc;
	tcp::acceptor acceptor;

public:
	BeastWebSocket(boost::asio::io_context& _ioc, tcp::endpoint endpoint, std::function<void(std::string, std::string&)> _parser) : ioc(_ioc), acceptor(ioc) {
		beast::error_code error;
		parser = _parser;
		acceptor.open(endpoint.protocol(), error);
		acceptor.set_option(boost::asio::socket_base::reuse_address(true), error);
		acceptor.bind(endpoint, error);
		acceptor.listen(boost::asio::socket_base::max_listen_connections, error);
	}

	void run() {
		doAccept();
	}

	void doAccept() {
		acceptor.async_accept(boost::asio::make_strand(ioc), beast::bind_front_handler(&BeastWebSocket::onAccept, shared_from_this()));
	}
	void onAccept(beast::error_code error, tcp::socket socket) {

		std::make_shared<BeastSession>(std::move(socket), parser)->run();

		doAccept();
	}

private:

	std::function<void(std::string, std::string&)> parser;

};