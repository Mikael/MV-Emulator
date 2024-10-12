#include <iostream>
#include <chrono>
#include <format>
#include <asio/execution_context.hpp>
#include "../include/AuthServer.h"

void printInitialInformation()
{
	auto const time = std::chrono::current_zone()->to_local(std::chrono::system_clock::now());
	auto const time_s = std::format("{:%Y-%m-%d %X}", time);
	std::cout << "Auth server initialized on " << time_s << "\n\n";
}

int main() {
    SetConsoleTitleW(L"Microvolts Auth Server");

    asio::io_context io_context;

    std::cout << "Server initialization...\n";

    std::shared_ptr<Auth::AuthServer> srv = std::make_shared<Auth::AuthServer>(io_context, 13000);

    printInitialInformation();

    srv->asyncAccept();

    const std::uint32_t num_threads = std::thread::hardware_concurrency();
    std::vector<std::thread> threads;

    threads.reserve(num_threads);
    for (std::uint32_t i = 0; i < num_threads; ++i) {
        threads.emplace_back([&io_context]() {
            try {
                io_context.run();
            }
            catch (const std::exception& e) {
                std::cerr << "Error in io_context.run(): " << e.what() << std::endl;
            }
            });
    }

    for (auto& thread : threads) {
        thread.join();
    }

    std::cout << "Server shutting down...\n";
    return 0;
}