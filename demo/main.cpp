// Copyright 2022 Shevelyova Darya photodoshfy@gmail.com

// cd cmake-build-debug
// ./demo

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/connect.hpp>
#include <boost/config.hpp>
#include <nlohmann/json.hpp>
#include <cstdlib>
#include <iostream>
#include <string>
using nlohmann::json;

namespace beast = boost::beast;     // from <boost/beast.hpp>
namespace http = beast::http;       // from <boost/beast/http.hpp>
namespace net = boost::asio;        // from <boost/asio.hpp>
using tcp = net::ip::tcp;           // from <boost/asio/ip/tcp.hpp>

int main() {
  //HTTP_Client client;
  //client.create_req();
  //client.start();
  try {
    std::string require;
    std::cout << "Input id" << std::endl;
    std::string str = "";
    std::cin >> str;
    json r;
    r["input"] = str;
    std::stringstream ss;
    ss << r;
    require = ss.str();
    //  require = "\"input\":" + str;
    //  std::cout << require << std::endl;

    auto const host = "127.0.0.1";
    auto const port = "80";
    auto const target = "/v1/api/suggest";
    int version = 11;

    std::cout << host << " " << port << " " << target
              << " " <<  require << std::endl;

    // io_context требуется для всех операций ввода/вывода
    net::io_context ioc;

    // Эти объекты выполняют наш ввод-вывод
    tcp::resolver resolver(ioc);
    beast::tcp_stream stream(ioc);

    // Ищем доменное имя
    auto const results = resolver.resolve(host, port);

    // Устанавливаем соединение по IP-адресу, полученному из поиска
    stream.connect(results);

    // Настройка сообщения запроса HTTP GET
    http::request<http::string_body> req{http::verb::post, target, version};
    req.body() = require;
    req.prepare_payload();
    req.set(http::field::host, host);
    req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
    // Отправляем HTTP-запрос на удаленный хост
    http::write(stream, req);

    // Этот буфер используется для чтения и должен быть сохранен
    beast::flat_buffer buffer;

    // Объявить контейнер для хранения ответа
    http::response<http::dynamic_body> res;

    // Объявить контейнер для хранения ответа
    http::read(stream, buffer, res);

    // Записываем сообщение на стандартный вывод
    std::cout << res << std::endl;

    // Закрыть сокет
    beast::error_code ec;
    stream.socket().shutdown(tcp::socket::shutdown_both, ec);

    // Иногда случается not_connected
    // так что не беспокойтесь об этом.
    if(ec && ec != beast::errc::not_connected)
      throw beast::system_error{ec};

    // Если мы доходим до этого места, то соединение корректно закрывается
  }
  catch(std::exception const& e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}