#include <unordered_map>
#include <cstddef>
#include <queue>
#include <functional>
#include <mutex>
#include <cmath>
#include <thread>
#include <chrono>
#include <fstream>
#include <random>
#include <vector>


template <typename T>
class Server {
    public:
        void start() {
            running = true;
            // servak = std::thread(&Server::run, this);
            for (size_t i = 0; i < 4; i++) {
                workers.emplace_back(&Server::worker, this);
            }
        }

        void stop() {
            running = false;
            // servak.join();
            for (auto& worker : workers) {
                worker.join();
            }
        }

        size_t add_task(std::function<T()> task) {
            mut.lock();

            size_t id = next_id++;
            tasks.push({id, std::move(task)});
            mut.unlock();
            return id;
        }

        T request_result(size_t id_res) {
           while (true) {
                mut.lock();
                auto it = results.find(id_res);
                if (it != results.end()) {
                    T res = it->second;
                    results.erase(it);
                    mut.unlock();
                    return res;
                }
                mut.unlock();
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
        }
    private:
        std::unordered_map<size_t, T> results;
        std::queue<std::pair<size_t, std::function<T()>>> tasks;
        std::mutex mut;
        size_t next_id = 0;
        bool running = false;
        // std::thread servak;
        std::vector<std::thread> workers;

        void worker() {
            while (running) {
                std::pair<size_t, std::function<T()>> task_pair;
                bool has_task = false;
                mut.lock();
                if (!tasks.empty()) {
                    task_pair = tasks.front();
                    tasks.pop();
                    has_task = true;
                }
                mut.unlock();

                if (has_task) {
                    T res = task_pair.second();
                    mut.lock();
                    results[task_pair.first] = res;
                    mut.unlock();
                } else {
                    std::this_thread::sleep_for(std::chrono::milliseconds(10));
                }
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

    std::thread client1(client_sin, std::ref(server), 100);
    std::thread client2(client_sqrt, std::ref(server), 100);
    std::thread client3(client_pow, std::ref(server), 100);

    client1.join();
    client2.join();
    client3.join();

    server.stop();
    return 0;
}