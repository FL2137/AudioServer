#pragma once

#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>

#include <functional>

typedef websocketpp::server<websocketpp::config::asio> server;


class WebSocketServer {
public:
	WebSocketServer() {
		endpoint.set_error_channels(websocketpp::log::elevel::all);
		endpoint.set_access_channels(websocketpp::log::alevel::all ^ websocketpp::log::alevel::frame_payload);

		endpoint.init_asio();
	}

	void run() {
		endpoint.listen(3005);
		endpoint.start_accept();
		endpoint.run();
	}

private:
	server endpoint;
};