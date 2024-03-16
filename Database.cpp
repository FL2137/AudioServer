#include "Database.hpp"

void Database::setupConnection() {
	//setting connection options
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