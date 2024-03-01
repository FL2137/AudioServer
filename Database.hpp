#pragma once
#include <pgfe/pgfe.hpp>
#include <fstream>
#include <string>
#include <iostream>
#include <libpq-fe.h>
using namespace dmitigr;

class Database {

public:

	Database() {
		pgfe::Connection_options options;
		pgfe::Connection con;

	}


	//void connection() {
	//	//setting connection options
	//	pgfe::Connection_options options;

	//	std::ifstream file("dbCreds.txt");

	//	std::string creds;
	//	options.set(pgfe::Communication_mode::net);

	//	std::getline(file, creds);
	//	options.set_hostname(creds);

	//	std::getline(file, creds);
	//	options.set_database(creds);

	//	std::getline(file, creds);
	//	options.set_username(creds);

	//	std::getline(file, creds);
	//	options.set_password(creds);



	//	//making the connection
	//	pgfe::Connection connection(options);

	//	connection.connect();
	//}


};