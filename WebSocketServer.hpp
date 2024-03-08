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


class WebSocketServer {
public:
	WebSocketServer() {
		endpoint.set_error_channels(websocketpp::log::elevel::all);
		endpoint.set_access_channels(websocketpp::log::alevel::all ^ websocketpp::log::alevel::frame_payload);
		endpoint.init_asio();



		endpoint.set_message_handler(
			std::bind(
				&WebSocketServer::readHandler,
				this,
				std::placeholders::_1,
				std::placeholders::_2
			)
		);

	}

	void readHandler(websocketpp::connection_hdl hdl, server::message_ptr message) {
		std::cout << message->get_payload();
	}

	void run() {

		boost::system::error_code error;
		endpoint.listen("192.168.1.109", "3005", error);
		endpoint.start_accept();
		if (endpoint.is_listening() && endpoint.is_server()) {

			boost::system::error_code er;
			auto ep = endpoint.get_local_endpoint(er);
			std::cout << "Server listening on " << ep.address().to_string() + ":" << ep.port() << std::endl;
		}
		endpoint.run();
	}

private:
	server endpoint;
};

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

		doRead();
	}

	void doRead() {
		_ws.async_read(, beast::bind_front_handler(&BeastSession::readHandler, shared_from_this()));
	}

	void readHandler(beast::error_code error, size_t bytesTransferred) {
		boost::ignore_unused(bytesTransferred);
		if (error == websocket::error::closed)
			return;

		if (error)
			return beastFail(error, "Read");

		//*message parsing*
		std::string request((char*)_buffer.data().data());
		std::cout << request << std::endl; 
		std::string response = "response from server";
		//parser(request, response);

		_ws.async_write(boost::asio::buffer(response), beast::bind_front_handler(&BeastSession::writeHandler, shared_from_this()));
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