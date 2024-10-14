#include <unordered_map>
#include <map>
#include <string>
#include <thread>
#include <chrono>
#include <mutex>
#include <vector>
#include <functional>
#include <atomic>

#include "../../include/Structures/AccountInfo/MainAccountInfo.h"
#include "../../include/Structures/Item/MainItem.h"
#include "../../include/Structures/Item/MainEquippedItem.h"
#include "../../include/Persistence/MainDatabaseManager.h"
#include "../../include/Persistence/MainScheduler.h"
#include "../../include/Database/ThreadPool.h" // Ensure this is the correct path

namespace Main
{
    namespace Database
    {
        class MainScheduler
        {
        public:
            MainScheduler(std::size_t wakeupFrequency, Main::Persistence::PersistentDatabase& database)
                : m_wakeupFrequency{ wakeupFrequency }
                , m_database{ database }
                , m_stopRequested{ false }
                , m_incrementalDifferentiationKey{ 0 }
                , m_threadPool{ std::thread::hardware_concurrency() } // Initialize ThreadPool with available hardware threads
            {
                m_schedulerThread = std::thread(&MainScheduler::schedulerLoop, this);
            }

            ~MainScheduler()
            {
                if (m_schedulerThread.joinable())
                {
                    m_stopRequested = true;
                    m_schedulerThread.join();
                }
            }

        private:
            void schedulerLoop()
            {
                while (!m_stopRequested)
                {
                    std::this_thread::sleep_for(std::chrono::seconds(m_wakeupFrequency));
                    persist();
                }
            }

            void persist()
            {
                std::lock_guard<std::mutex> lock(m_mutex);
                std::vector<std::function<void()>> allCallbacks;

                // Collect callbacks incrementally
                for (const auto& [accountId, callbacks] : m_databaseCallbacksIncremental)
                {
                    for (const auto& [updateType, callback] : callbacks)
                    {
                        allCallbacks.push_back(callback);
                    }
                }

                // Collect standard callbacks
                for (const auto& [accountId, callbacks] : m_databaseCallbacks)
                {
                    for (const auto& [updateType, callback] : callbacks)
                    {
                        allCallbacks.push_back(callback);
                    }
                }

                // Execute all callbacks using the thread pool
                for (const auto& callback : allCallbacks)
                {
                    m_threadPool.enqueue(callback); // Use the thread pool
                }

                m_databaseCallbacks.clear();
                m_databaseCallbacksIncremental.clear();
                m_incrementalDifferentiationKey = 0;
            }

            void persistFor(std::uint32_t accountId)
            {
                std::lock_guard<std::mutex> lock(m_mutex);

                // Check if there are callbacks for this accountId
                if (m_databaseCallbacksIncremental.count(accountId))
                {
                    for (const auto& [updateType, callback] : m_databaseCallbacksIncremental[accountId])
                    {
                        m_threadPool.enqueue(callback); // Use the thread pool
                    }
                    m_databaseCallbacksIncremental.erase(accountId);
                }

                if (m_databaseCallbacks.count(accountId))
                {
                    for (const auto& [updateType, callback] : m_databaseCallbacks[accountId])
                    {
                        m_threadPool.enqueue(callback); // Use the thread pool
                    }
                    m_databaseCallbacks.erase(accountId);
                }

                m_incrementalDifferentiationKey = 0;
            }

            std::size_t m_wakeupFrequency;
            Main::Persistence::PersistentDatabase& m_database;
            std::thread m_schedulerThread;
            std::mutex m_mutex;
            std::atomic<bool> m_stopRequested; // Use atomic for thread safety
            std::uint32_t m_incrementalDifferentiationKey;

            std::unordered_map<std::uint32_t, std::map<int, std::function<void()>>> m_databaseCallbacksIncremental;
            std::unordered_map<std::uint32_t, std::map<int, std::function<void()>>> m_databaseCallbacks;

            ThreadPool m_threadPool; // Add the thread pool member
        };
    }
}