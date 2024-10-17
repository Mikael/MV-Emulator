

#include <iostream>
#include <chrono>
#include <format>
#include <asio/execution_context.hpp>
#include "../include/MainServer.h"
#include "../include/Structures/AccountInfo/MainAccountInfo.h"
#include "../include/ConstantDatabase/CdbSingleton.h"

#include <ConstantDatabase/Structures/CdbWeaponsInfo.h>
#include <ConstantDatabase/Structures/CdbItemInfo.h>
#include <ConstantDatabase/Structures/CdbUpgradeInfo.h>
#include <ConstantDatabase/Structures/CdbCapsuleInfo.h>
#include <ConstantDatabase/Structures/CdbCapsulePackageInfo.h>
#include <ConstantDatabase/Structures/SetItemInfo.h>
#include <Utils/IPC_Structs.h>
#include <ConstantDatabase/Structures/CdbRewardInfo.h>
#include <asio.hpp>
#include <thread>
#include <atomic>
#include <windows.h>

void printInitialInformation()
{
	auto const time = std::chrono::current_zone()->to_local(std::chrono::system_clock::now());
	auto const time_s = std::format("{:%Y-%m-%d %X}", time);
	std::cout << "[Info] Main server initialized on " << time_s << "\n\n";
}

void initializeCdbFiles()
{
	const std::string cdbItemInfoPath = "../ExternalLibraries/cgd_original/ENG";
	const std::string cdbItemInfoName = "iteminfo.cdb";
	const std::string cdbSetItemInfoName = "setiteminfo.cdb";
	const std::string cdbWeaponItemInfoName = "itemweaponsinfo.cdb";
	const std::string cdbCapsuleInfoName = "gachaponinfo.cdb";
	const std::string cdbCapsulePackageInfoName = "gachaponpackageinfo.cdb";
	const std::string cdbUpgradeInfoName = "upgradeinfo.cdb";
	const std::string cdbRewardInfo = "rewardinfo.cdb";
	const std::string cdbGradeInfo = "gradeinfo.cdb";

	using cdbItems = Common::ConstantDatabase::CdbSingleton<Common::ConstantDatabase::CdbItemInfo>;
	using setItems = Common::ConstantDatabase::CdbSingleton<Common::ConstantDatabase::SetItemInfo>;
	using cdbWeapons = Common::ConstantDatabase::CdbSingleton<Common::ConstantDatabase::CdbWeaponInfo>;
	using upgradeInfos = Common::ConstantDatabase::CdbSingleton<Common::ConstantDatabase::CdbUpgradeInfo>;
	using capsulePackageInfos = Common::ConstantDatabase::CdbSingleton<Common::ConstantDatabase::CdbCapsulePackageInfo>;
	using capsuleInfos = Common::ConstantDatabase::CdbSingleton<Common::ConstantDatabase::CdbCapsuleInfo>;
	using rewardInfo = Common::ConstantDatabase::CdbSingleton<Common::ConstantDatabase::CdbRewardInfo>;
	using levelInfo = Common::ConstantDatabase::CdbSingleton<Common::ConstantDatabase::CdbGradeInfo>;

	std::cout << "[Info] Initializing constant database maps...\n";
	cdbItems::initialize(cdbItemInfoPath, cdbItemInfoName);
	cdbItems::initializeItemTypes(cdbItemInfoPath, cdbWeaponItemInfoName, cdbItemInfoName);
	setItems::initialize(cdbItemInfoPath, cdbSetItemInfoName);
	cdbWeapons::initialize(cdbItemInfoPath, cdbWeaponItemInfoName);
	upgradeInfos::initialize(cdbItemInfoPath, cdbUpgradeInfoName);
	capsuleInfos::initialize(cdbItemInfoPath, cdbCapsuleInfoName);
	capsulePackageInfos::initialize(cdbItemInfoPath, cdbCapsulePackageInfoName);
	rewardInfo::initialize(cdbItemInfoPath, cdbRewardInfo);
	levelInfo::initialize(cdbItemInfoPath, cdbGradeInfo);
	std::cout << "[Info] Constant database successfully initialized.\n";
}

std::atomic<int> uptime_seconds{ 0 };

static void updateConsoleTitle() {
	while (true) {
		int hours = uptime_seconds / 3600;
		int minutes = (uptime_seconds % 3600) / 60;
		int seconds = uptime_seconds % 60;
		std::wstring title = L"Surge Main Server | Uptime: " +
			std::to_wstring(hours) + L" hours, " +
			std::to_wstring(minutes) + L" minutes, " +
			std::to_wstring(seconds) + L" seconds";
		SetConsoleTitleW(title.c_str());
		std::this_thread::sleep_for(std::chrono::seconds(1));
	}
}

int main() {
	SetConsoleTitleW(L"Surge Main Server");

	asio::io_context io_context;
	Main::MainServer srv(io_context, 13005, 1);

	printInitialInformation();
	initializeCdbFiles();
	Utils::IPCManager::cleanupSharedMemory();

	// Start the uptime counter in a separate thread
	std::thread uptime_thread([&]() {
		while (true) {
			std::this_thread::sleep_for(std::chrono::seconds(1));
			uptime_seconds++;
		}
		});

	std::thread title_thread(updateConsoleTitle);

	srv.asyncAccept();
	io_context.run();

	uptime_thread.join();
	title_thread.join();
}
