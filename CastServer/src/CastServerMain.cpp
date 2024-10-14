#include <iostream>
#include <chrono>
#include <string>
#include <format>
#include <thread>
#include <vector>
#include <asio.hpp>
#include "../include/CastServer.h"
#include "../include/ConstantDatabase/CdbSingleton.h"
#include "../include/ConstantDatabase/Structures/CdbMapInfo.h"
#include "../include/Utils/HostSuicideUtils.h"

void printInitialInformation()
{
    auto const time = std::chrono::current_zone()->to_local(std::chrono::system_clock::now());
    auto const time_s = std::format("{:%Y-%m-%d %X}", time);
    std::cout << "[Info] Cast server initialized on " << time_s << "\n\n";

    std::cout << "[Info] Initializing constant database maps...\n";
    const std::string cdbItemInfoPath = "../ExternalLibraries/cgd_original/ENG";
    const std::string cdbMapInfoName = "mapinfo.cdb";
    using mapInfo = Common::ConstantDatabase::CdbSingleton<Common::ConstantDatabase::CdbMapInfo>;
    mapInfo::initialize(cdbItemInfoPath, cdbMapInfoName);

    std::cout << "[Info] Constant database successfully initialized.\n";
}

int main() {
    SetConsoleTitleW(L"Microvolts Cast Server");

    asio::io_context io_context;

    std::shared_ptr<Cast::CastServer> srv = std::make_shared<Cast::CastServer>(io_context, 13006, 4);

    printInitialInformation();
    srv->asyncAccept();

    std::vector<std::thread> threads;

    const std::size_t thread_count = std::thread::hardware_concurrency();

    for (std::size_t i = 0; i < thread_count; ++i) {
        threads.emplace_back([&io_context]() {
            asio::executor_work_guard<asio::io_context::executor_type> workGuard(io_context.get_executor());
            io_context.run();
            });
    }

    for (auto& thread : threads) {
        thread.join();
    }

    return 0;
}
