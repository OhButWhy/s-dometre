#include <unordered_map>
#include <cstddef>
#include <queue>
#include <functional>
#include <mutex>
#include <cmath>
#include <thread>
#include <chrono>
#include <fstream>
#include <iostream>
#include <random>
#include <vector>
#include <condition_variable>


template <typename T>
class Server {
    public:
        void start() {
            {
                std::lock_guard<std::mutex> lock(mut);
                running = true;
            }
            for (size_t i = 0; i < 4; i++) {
                workers.emplace_back(&Server::worker, this);
            }
        }

        void stop() {
            {
                std::lock_guard<std::mutex> lock(mut);
                running = false;
            }
            cv_tasks.notify_all();
            cv_results.notify_all();
            for (auto& worker : workers) {
                worker.join();
            }
        }

        size_t add_task(std::function<T()> task) {
            std::lock_guard<std::mutex> lock(mut);
            size_t id = next_id++;
            tasks.push({id, std::move(task)});
            cv_tasks.notify_one();
            return id;
        }

        T request_result(size_t id_res) {
            std::unique_lock<std::mutex> lock(mut);
            cv_results.wait(lock, [&]() {
                return results.find(id_res) != results.end();
            });

            T res = results[id_res];
            results.erase(id_res);
            return res;
        }
    private:
        std::unordered_map<size_t, T> results;
        std::queue<std::pair<size_t, std::function<T()>>> tasks;
        std::mutex mut;
        std::condition_variable cv_tasks;
        std::condition_variable cv_results;
        size_t next_id = 0;
        bool running = false;
        // std::thread servak;
        std::vector<std::thread> workers;

        void worker() {
            while (true) {
                std::pair<size_t, std::function<T()>> task_pair;

                {
                    std::unique_lock<std::mutex> lock(mut);
                    cv_tasks.wait(lock, [&]() {
                        return !running || !tasks.empty();
                    });

                    if (!running && tasks.empty()) {
                        return;
                    }

                    task_pair = std::move(tasks.front());
                    tasks.pop();
                }

                T res = task_pair.second();
                {
                    std::lock_guard<std::mutex> lock(mut);
                    results[task_pair.first] = res;
                }
                cv_results.notify_all();
            }
        }
};


void client_sin(Server<double>& server, size_t N) {
    std::ofstream out("sin.txt");
    out.precision(6);
    out << std::fixed;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<double> dis(0.0, 1.0);

    for (size_t i = 0; i < N; i++) {
        double arg = dis(gen);
        size_t id = server.add_task([arg]() {
            return std::sin(arg);
        });
        double res = server.request_result(id);
        out << arg << " " << res << " " << id << "\n";
    }
}


void client_sqrt(Server<double>& server, size_t N) {
    std::ofstream out("sqrt.txt");
    out.precision(6);
    out << std::fixed;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<double> dis(1.0, 3.0);
    
    for (size_t i = 0; i < N; i++) {
        double arg = dis(gen);
        size_t id = server.add_task([arg]() {
            return std::sqrt(arg);
        });
        double res = server.request_result(id);
        out << arg << " " << res << " " << id << "\n";
    }
}


void client_pow(Server<double>& server, size_t N) {
    std::ofstream out("pow.txt");
    out.precision(6);
    out << std::fixed;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<double> dis(1.0, 3.0);
    
    for (size_t i = 0; i < N; i++) {
        double arg1 = dis(gen);
        double arg2 = dis(gen);
        size_t id = server.add_task([arg1, arg2]() {
            return std::pow(arg1, arg2);
        });
        double res = server.request_result(id);
        out << arg1 << " " << arg2 << " " << res << " " << id << "\n";
    }
}


int main() {
    Server<double> server;
    server.start();
    auto start = std::chrono::steady_clock::now();
    std::thread client1(client_sin, std::ref(server), 10000);
    std::thread client2(client_sqrt, std::ref(server), 10000);
    std::thread client3(client_pow, std::ref(server), 10000);

    client1.join();
    client2.join();
    client3.join();
    auto end = std::chrono::steady_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    server.stop();

    std::cout << elapsed.count() << std::endl;
    return 0;
}