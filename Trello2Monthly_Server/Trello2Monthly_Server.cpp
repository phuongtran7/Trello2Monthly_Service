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
	// Get the number of querry
	auto querry = uri::split_query(message.request_uri().query());

	if (!querry.empty())
	{
		if (!querry.empty())
		{
			if (uri::decode(querry.at(U("task"))) == U("PDF"))
			{
				// The client is requesting that it's ready for PDF
				const auto filename_pdf = conversions::to_string_t(uri::decode(querry.at(U("name")))) + U(".pdf");
				std::ifstream in_file(filename_pdf, std::ifstream::binary);
				const std::vector<unsigned char> data((std::istreambuf_iterator<char>(in_file)), std::istreambuf_iterator<char>());
				const auto code = conversions::to_base64(data);

				json::value result = json::value::object();
				result = json::value::string(code);
				// Send OK reply
				message.reply(status_codes::OK, result);
			}
		}
		else
		{
			// Send OK reply
			message.reply(status_codes::BadRequest);
		}

		if (!querry.empty())
		{
			if (uri::decode(querry.at(U("task"))) == U("DOCX"))
			{
				// The client is requesting that it's ready for docx
				const auto filename_docx = conversions::to_string_t(uri::decode(querry.at(U("name")))) + U(".docx");
				std::ifstream in_file(filename_docx, std::ifstream::binary);
				const std::vector<unsigned char> data((std::istreambuf_iterator<char>(in_file)), std::istreambuf_iterator<char>());
				const auto code = conversions::to_base64(data);

				json::value result = json::value::object();
				result = json::value::string(code);
				// Send OK reply
				message.reply(status_codes::OK, result);
			}
		}
		else
		{
			// Send OK reply
			message.reply(status_codes::BadRequest);
		}
	}
}

void server::handle_post(http_request message)
{
	// TODO: implement receive text file from client then convert to approriate output.
}

// Handle receive tex file from client
void server::handle_put(http_request message)
{
	// Get the number of querry
	auto querry = uri::split_query(message.request_uri().query());

	if (!querry.empty())
	{
		if (uri::decode(querry.at(U("task"))) == U("TEX"))
		{
			// The client is requesting that it will send the tex file over
			const auto extracted = message.extract_json().get().as_string();
			auto convert = conversions::from_base64(extracted);
			// The file name comes from the second querry
			//const auto temp_file_name = conversions::to_utf8string(uri::decode(querry.at(U("name")))) + ".tex";
			auto temp_file_name = conversions::to_utf8string(uri::decode(querry.at(U("name"))));
			std::ofstream fout(temp_file_name + ".tex", std::ios::out | std::ios::binary);
			fout.write(reinterpret_cast<const char*>(&convert[0]), convert.size());
			fout.close();

			// Convert to PDF
			std::system((fmt::format(R"(pdflatex "{}")", temp_file_name + ".tex")).c_str());

			// Convert to word if pandoc is installed
			std::system((fmt::format(R"(pandoc -s "{}" -o "{}")", temp_file_name + ".tex", temp_file_name + ".docx")).c_str());

			// Clean up
			std::remove(fmt::format("{}", temp_file_name + ".aux").c_str());
			std::remove(fmt::format("{}", temp_file_name + ".log").c_str());
			std::remove(fmt::format("{}", temp_file_name + ".out").c_str());
			std::remove(fmt::format("{}", temp_file_name + ".synctex.gz").c_str());

			// Send OK reply
			message.reply(status_codes::OK);
		}
	}
	else
	{
		// Send OK reply
		message.reply(status_codes::BadRequest);
	}
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