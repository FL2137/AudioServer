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

#include <deque>
#include <thread>
#include <mutex>

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
	std::mutex mainMutex;

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

	void syncWrite(const std::string& message) {
		boost::asio::post(_ws.get_executor(), [self = shared_from_this(), this, msg = std::move(message)] {
			do_post_message(std::move(msg));
		});
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
		
		//_ws.async_write(b.data(), beast::bind_front_handler(&BeastSession::writeHandler, shared_from_this()));

		do_post_message(str);

		_buffer.consume(_buffer.size());

		doRead();
	}

	void do_post_message(std::string msg) {
		outbox.push_back(std::move(msg));

		if (outbox.size() == 1)
			doWriteLoop();
	}

	void doWriteLoop() {
		if (outbox.empty())
			return;

		_ws.async_write(boost::asio::buffer(outbox.front()), [self = shared_from_this(), this](beast::error_code error, size_t bytesTrans) {
			boost::ignore_unused(bytesTrans);
			if (error)
				return beastFail(error, "AsyncWrite");

			outbox.pop_front();

			doWriteLoop();
		});
	}

	std::deque<std::string> outbox = {};


public:

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

		std::cout << "Server running on " << acceptor.local_endpoint().address().to_string() << ":" << acceptor.local_endpoint().port() << std::endl;

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

		currentConnection = std::make_shared<BeastSession>(std::move(socket), parser);
		
		currentConnection->run();
		
		connections.push_back(currentConnection);

		doAccept();
	}


	void doNotify(std::string const& message) {
		for (auto& con : connections) {
			std::cout << con << " -- " << currentConnection << std::endl;
			if (con != currentConnection) {
				con->syncWrite(message);
			}
		}
	}

	void notify(std::string message) {
		boost::asio::post(acceptor.get_executor(), beast::bind_front_handler(&BeastWebSocket::doNotify, shared_from_this(), std::move(message)));
	}

private:

	std::function<void(std::string, std::string&)> parser = [&](std::string request, std::string& response) {
		
		std::cout << "Request: " << request << std::endl;
		
		if (request == "PING") {
			response = "PONG";
			return;
		}
		
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
				js["type"] = "RESPONSE_LOGIN";
				response = js.dump();
				std::cout << "Sending back: " << response << std::endl;

				json notification;
				notification["type"] = "NOTIFY_FRIENDS";
				notification["uid"] = audioServer->lastUid - 1;
				notify(notification.dump());
				//std::thread s(&AudioServer::notifyFriends, audioServer->lastUid - 1, audioServer->loggedUsers);
				//s.detach();
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
			json js;
			js["type"] = "RESPONSE_ROOMCHECK";
			js["ok"] = "OK";
			response = js.dump();
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

	std::shared_ptr<BeastSession> currentConnection;

};