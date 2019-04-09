// Trello2Monthly_Server.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"

using namespace web;
using namespace utility;
using namespace http;
using namespace web::http::experimental::listener;

class server
{
public:
	server() = default;

	explicit server(const string_t& url) : m_listener_(url)
	{
		m_listener_.support(methods::GET, std::bind(&server::handle_get, this, std::placeholders::_1));
		m_listener_.support(methods::PUT, std::bind(&server::handle_put, this, std::placeholders::_1));
		m_listener_.support(methods::POST, std::bind(&server::handle_post, this, std::placeholders::_1));
		m_listener_.support(methods::DEL, std::bind(&server::handle_delete, this, std::placeholders::_1));
	}

	pplx::task<void> open()
	{
		return m_listener_.open();
	}
	pplx::task<void> close()
	{
		return m_listener_.close();
	}

private:
	void handle_get(http_request message);
	void handle_put(http_request message);
	void handle_post(http_request message);
	void handle_delete(http_request message);

	http_listener m_listener_;
};

std::unique_ptr<server> httpserver;

void server::handle_get(http_request message)
{
	const auto message_body = message.extract_string().get();

	if (message_body.empty())
	{
		std::ifstream in_file("Monthly Status Report.pdf", std::ifstream::binary);
		std::vector<unsigned char> data((std::istreambuf_iterator<char>(in_file)), std::istreambuf_iterator<char>());
		const auto code = conversions::to_base64(data);

		json::value result = json::value::object();
		//result = json::value::string(L"GET SUCCEEDED");
		result = json::value::string(code);

		message.reply(status_codes::OK, result);
	}
}

void server::handle_post(http_request message)
{
	const auto message_body = message.extract_string().get();

	if (message_body == L"\"PDF\"")
	{
		std::wcout << "POST Body Success: " << message_body << "\n";
		json::value result = json::value::object();
		result = json::value::string(L"Success");

		message.reply(status_codes::OK, result);
	}
	else
	{
		std::wcout << "POST Body Failed: " << message_body << "\n";
		json::value result = json::value::object();
		result = json::value::string(L"Failed");

		message.reply(status_codes::OK, result);
	}
}

void server::handle_put(http_request message)
{
	// TODO: implement receive text file from client then convert to approriate output.
}

void server::handle_delete(http_request message)
{
	// TODO: implement delete recieved text file and converted output when client received the files
	// TODO: this has to make sure that none of the client files remains after finishing
}

void on_initialize(const string_t& address)
{
	uri_builder uri(address);

	auto addr = uri.to_uri().to_string();
	httpserver = std::make_unique<server>(addr);
	httpserver->open().wait();

	std::wcout << string_t(U("Listening for requests at: ")) << addr << std::endl;
}

void on_shutdown()
{
	httpserver->close().wait();
}
int main()
{
	const string_t address = U("http://localhost:34568");
	on_initialize(address);
	std::cout << "Press ENTER to exit." << std::endl;

	std::string line;
	std::getline(std::cin, line);

	on_shutdown();
	return 0;
}