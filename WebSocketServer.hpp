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