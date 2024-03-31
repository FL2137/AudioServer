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
namespace _net = boost::asio;
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

	void syncWrite(std::string message) {
		_ws.write(_net::buffer(message));
	}


private:

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
		//std::cout << "Accepted connection\n";
		doRead();
	}

	void doRead() {
		_ws.async_read(_buffer, beast::bind_front_handler(&BeastSession::readHandler, shared_from_this()));
	}

	void readHandler(beast::error_code error, size_t bytesTransferred) {

		boost::ignore_unused(bytesTransferred);
		if (error == websocket::error::closed)
			return;

		if (error)
			return beastFail(error, "Read");

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
	BeastWebSocket(boost::asio::io_context& _ioc, tcp::endpoint endpoint, AudioServer *audioServer) : ioc(_ioc), acceptor(ioc) {

		this->audioServer = audioServer;

		beast::error_code error;
		//parser = _parser;
		acceptor.open(endpoint.protocol(), error);
		acceptor.set_option(boost::asio::socket_base::reuse_address(true), error);
		acceptor.bind(endpoint, error);
		acceptor.listen(boost::asio::socket_base::max_listen_connections, error);

		if (error.value() != 0) {
			std::cerr << error.message() << std::endl;
		}
	}

	void run() {
		doAccept();
	}

	void doAccept() {
		acceptor.async_accept(boost::asio::make_strand(ioc), beast::bind_front_handler(&BeastWebSocket::onAccept, shared_from_this()));
	}

	void onAccept(beast::error_code error, tcp::socket socket) {

		auto connection = std::make_shared<BeastSession>(std::move(socket), parser);
		
		connections.push_back(connection);

		connection->run();

		doAccept();
	}

	void notify(const std::string& message) {
		for (auto& con : connections)
			con->syncWrite(message);
	}

private:

	std::function<void(std::string, std::string&)> parser = [&](std::string request, std::string& response) {
		json jsRequest = json::parse(request.c_str());

		if (jsRequest["type"] == "CREATE_ROOM") {

			int uid = jsRequest["uid"].get<int>();

			int rid;
			if ((rid = audioServer->createRoom(uid))) {
				json js;
				js["ok"] = "OK";
				js["rid"] = rid;
				response = js.dump();
			}
			else {
				response = audioServer->lastError;
			}
			return;
		}
		else if (jsRequest["type"] == "JOIN_ROOM") {

			int uid = jsRequest["uid"].get<int>();
			int roomId = jsRequest["rid"].get<int>();

			if (audioServer->joinRoom(roomId, uid)) {
				json js;
				js["ok"] = "OK";
				response = js.dump();

			}
			else {
				json js;
				js["ok"] = audioServer->lastError;
				response = js.dump();
			}
			return;
		}
		else if (jsRequest["type"] == "SET_ENDPOINT") {
			std::cout << "SET_ENDPOINT\n";
			int uid = jsRequest["uid"].get<int>();
			if (audioServer->setUserEndpoint(uid, jsRequest["address"], jsRequest["port"])) {
				json js;
				js["ok"] = "OK";
				response = js.dump();
			}
			else {
				json js;
				js["ok"] = audioServer->lastError;
				response = js.dump();
			}

		}
		else if (jsRequest["type"] == "LOGIN") {
			if (audioServer->userConnected(jsRequest["data"].dump())) {
				json js;
				js["ok"] = "OK";
				js["uid"] = audioServer->lastUid - 1;
				response = js.dump();
				/*std::thread s(&AudioServer::notifyFriends, &audioServer, audioServer.lastUid - 1, audioServer.loggedUsers);
				s.detach();*/
			}
			else {
				json js;
				js["ok"] = audioServer->lastError;
				response = js.dump();
			}

			return;
		}
		else if (jsRequest["type"] == "SET_AVATAR") {
			int uid = jsRequest["uid"].get<int>();

			if (audioServer->setAvatar(uid, jsRequest["data"].dump())) {
				json js;
				js["ok"] = "OK";
				response = js.dump();
			}
			else {
				json js;
				js["ok"] = audioServer->lastError;
				response = js.dump();
			}
			return;
		}
		else if (jsRequest["type"] == "QUIT_ROOM") {
			int uid = jsRequest["uid"].get<int>();
			int roomId = jsRequest["rid"].get<int>();

			if (audioServer->quitRoom(roomId, uid)) {
				response = "OK";
			}
			else {
				response = audioServer->lastError;
			}
			return;
		}
		else if (jsRequest["type"] == "ROOM_CHECK") {
			int uid = jsRequest["uid"].get<int>();
			int rid = jsRequest["rid"].get<int>();
		}
		else if (jsRequest["type"] == "FRIENDS_CHECK") {
			int uid = jsRequest["uid"].get<int>();

			std::vector<std::string> friends = audioServer->friendListCheck(uid);
			json js;
			js["data"] = json::array();
			for (const std::string& _friend : friends) {
				js["data"].push_back(_friend);
			}
			response = js.dump();
		}
	};
	std::vector<std::shared_ptr<BeastSession>> connections;
	AudioServer* audioServer;
};