// Copyright 2022 Shevelyova Darya photodoshfy@gmail.com

#include <http_client.hpp>

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <cstdlib>
#include <iostream>
#include <string>

namespace beast = boost::beast;     // from <boost/beast.hpp>
namespace http = beast::http;       // from <boost/beast/http.hpp>
namespace net = boost::asio;        // from <boost/asio.hpp>
using tcp = net::ip::tcp;           // from <boost/asio/ip/tcp.hpp>

// Выполняет HTTP GET и печатает ответ
int main(int argc, char** argv) {
  try {
    // Проверяем аргументы командной строки.
    if(argc != 5) {
      std::cerr << "Usage: http-client-sync <host> <port> <target> "
                   "<request>\n"
                << "Example:\n"
                   "localhost 8080 /v1/api/suggest "
                << R"({"input":"<user_input>"})"
                   "\n" << argv[1] << " " << argv[2]
                << " " << argv[3] << " " << argv[4];
      return EXIT_FAILURE;
    }
    auto const host = argv[1];
    auto const port = argv[2];
    auto const target = argv[3];
    auto const request = argv[4];
    int version = 11;
    std::cout << host << " " << port << " " << target
              << " " <<  request << std::endl;
    // io_context требуется для всех операций ввода/вывода
    net::io_context ioc;

    // Эти объекты выполняют наш ввод-вывод
    tcp::resolver resolver(ioc);
    beast::tcp_stream stream(ioc);

    // Ищем доменное имя
    auto const results = resolver.resolve(host, port);

    // Устанавливаем соединение по IP-адресу, полученному из поиска
    stream.connect(results);

    http::string_body::value_type body = request;

    // Настройка сообщения запроса HTTP GET
    http::request<http::string_body> req{http::verb::post, target, version};
    req.set(http::field::host, host);
    req.body() = body;
    req.prepare_payload();
    req.set(http::field::content_type, "text/html");
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