// Trello2Monthly_Client.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"

using namespace web;
using namespace utility;
using namespace http;
using namespace http::client;

int main()
{
	http_client client_(U("http://localhost:34568"));

	uri_builder builder;

	json::value send_data = json::value::object();
	send_data = json::value::string(L"PDF");

	auto response = client_.request(methods::POST, U(""), send_data).get();

	if (response.status_code() == status_codes::OK)
	{
		std::wcout << response.to_string() << "\n";

		auto new_response = client_.request(methods::GET, U("")).get();
		if (new_response.status_code() == status_codes::OK)
		{
			auto extracted = new_response.extract_json().get().as_string();
			auto convert = conversions::from_base64(extracted);
			std::ofstream fout("test.pdf", std::ios::out | std::ios::binary);
			fout.write(reinterpret_cast<const char*>(&convert[0]), convert.size());
			fout.close();
		}
	}
	else
	{
		std::wcout << response.to_string() << "\n";
		std::wcout << "Got nothing: " << response.status_code() << "\n";
	}

	std::getchar();
}
