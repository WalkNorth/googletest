#include <iostream>
#include "spdlog/spdlog.h"
#include "spdlog/sinks/basic_file_sink.h"
// Extended demo: basic usage + asynchronous multi-sink example
#include <thread>
#include <vector>
#include <chrono>
#include "spdlog/async.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/sinks/rotating_file_sink.h"

void basic_demo()
{
    spdlog::set_pattern("[%Y-%m-%d %H:%M:%S] [%^%l%$] %v");
    // create a basic file logger (will create spdlog_demo.log)
    auto file_logger = spdlog::basic_logger_mt("file_logger", "spdlog_demo.log");

    spdlog::info("spdlog demo: basic console message");
    spdlog::warn("spdlog demo: a warning with number {}", 42);
    file_logger->error("spdlog demo: an error written to file");

    spdlog::info("basic demo finished, see spdlog_demo.log for file output");
}

void async_multi_sink_demo()
{
    // init thread pool: queue size 8192, 1 backing thread
    spdlog::init_thread_pool(8192, 1);

    // Create sinks: colored stdout and rotating file
    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    console_sink->set_level(spdlog::level::info);

    auto rotating_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>("spdlog_demo_rotating.log", 1024 * 1024, 3);
    rotating_sink->set_level(spdlog::level::trace);

    std::vector<spdlog::sink_ptr> sinks{console_sink, rotating_sink};

    // create async logger using the global thread pool
    auto async_logger_ptr = std::make_shared<spdlog::async_logger>("async_logger", sinks.begin(), sinks.end(), spdlog::thread_pool(), spdlog::async_overflow_policy::block);
    async_logger_ptr->set_level(spdlog::level::debug);
    spdlog::register_logger(async_logger_ptr);

    // spawn multiple threads to generate concurrent log messages
    const int threads = 4;
    const int messages_per_thread = 2000;
    std::vector<std::thread> ths;
    for (int t = 0; t < threads; ++t) {
        ths.emplace_back([t, messages_per_thread]() {
            for (int i = 0; i < messages_per_thread; ++i) {
                spdlog::get("async_logger")->info("thread {} message {}", t, i);
            }
        });
    }

    for (auto &th : ths) th.join();

    // flush and shutdown spdlog so all messages are written
    spdlog::shutdown();
    std::cout << "async multi-sink demo finished, see spdlog_demo_rotating.log for file output" << std::endl;
}

int main()
{
    try {
        basic_demo();
        // small pause between demos
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        async_multi_sink_demo();
    } catch (const spdlog::spdlog_ex &ex) {
        std::cerr << "spdlog initialization failed: " << ex.what() << std::endl;
        return 1;
    }

    return 0;
}
