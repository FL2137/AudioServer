#pragma once
#include <fstream>
#include <string>
#include <iostream>
#include <memory>

#include <libpq-fe.h>
#include <pgfe/pgfe.hpp>

using namespace dmitigr;

class Database {

public:

	Database() {
		setupConnection();
	}

	~Database() {
		connection->disconnect();

		if (connection->is_connected() == false) {
			//ok
		}
	}

	void setupConnection() {
	
		pgfe::Connection_options options;

		std::ifstream file("dbCreds.txt");

		if (file.is_open()) {
			std::string creds;
			options.set(pgfe::Communication_mode::net);

			std::getline(file, creds);
			options.set_hostname(creds);

			std::getline(file, creds);
			options.set_database(creds);

			std::getline(file, creds);
			options.set_username(creds);

			std::getline(file, creds);
			options.set_password(creds);
		}
		else {
			throw std::exception("Could'nt open database credentials file");
		}

		//making the connection

		connection = new pgfe::Connection(options);

		try {
			connection->connect();
		}
		catch (const pgfe::Exception& e) {
			std::cout << "Database::exception: " << e.what() << std::endl;
		}
		if (connection->is_connected()) {
			std::cout << "Connected!";
		}
		else {
			std::cout << "Could'nt connect :(";
		}
	}

	void executeSQL() {

		try {
			connection->execute([](auto&& row) {
			
				auto nick = pgfe::to<std::string>(row["nickname"]);
				std::cout << nick << std::endl;

			}, "SELECT * FROM \"Credentials\".\"Creds\"");

			connection->wait_response(std::chrono::milliseconds(3000));
		}
		catch (pgfe::Server_exception& e) {
			std::cout << e.what() << std::endl;
		}
	}
		
private:

	pgfe::Connection* connection;

};