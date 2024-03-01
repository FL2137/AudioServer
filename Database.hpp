#pragma once
#include <pgfe/pgfe.hpp>
#include <fstream>
#include <string>
#include <iostream>
#include <memory>

#include <libpq-fe.h>

using namespace dmitigr;

class Database {

public:

	Database() {
		setupConnection();
	}

	void setupConnection() {
		//setting connection options
		pgfe::Connection_options options;

		std::ifstream file("dbCreds.txt");

		std::string creds;
		options.set(pgfe::Communication_mode::net);

		std::getline(file, creds);
		std::cout << creds << std::endl;
		options.set_hostname(creds);

		std::getline(file, creds);
		std::cout << creds << std::endl;
		options.set_database(creds);

		std::getline(file, creds);
		std::cout << creds << std::endl;
		options.set_username(creds);

		std::getline(file, creds);
		std::cout << creds << std::endl;
		options.set_password(creds);

		//making the connection

		connection = new pgfe::Connection(options);

		try {
			connection->connect();
		}
		catch (const std::exception& e) {
			std::cout << "Database::exception: " << e.what() << std::endl;
		}
		if (connection->is_connected()) {
			std::cout << "Connected!";
		}
		else {
			std::cout << "Could'nt connect :(";
		}
	}

private:

	pgfe::Connection* connection;

};