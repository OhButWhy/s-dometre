#include <iostream>
#include <vector>
#include <ctime>
#include <thread>


std::clock_t single(int size, int p) {
    std::vector<std::vector<double>> A(size, std::vector<double>(size, 1.0));
    std::vector<double> x(size, 1.0);
    std::vector<double> y(size, 0.0);
    std::clock_t start = std::clock();
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            y[i] += A[i][j] * x[j];
        }
    }
    std::clock_t end = std::clock();
    return (end - start) / CLOCKS_PER_SEC;
}

std::clock_t multi(int size, int p) {
    int chunk = size/p;
    std::vector<std::vector<double>> A(size, std::vector<double>(size));
    std::vector<double> x(size);
    std::vector<double> y(size);

    auto init = [&A, &x, &y](int start, int end) {
        for (int i = start; i < end; i++) {
            for (int j = 0; j < A[i].size(); j++) {
                A[i][j] = 1.0;
            }
            x[i] = 1.0;
            y[i] = 0.0;
        }
    };

    auto worker = [&A, &x, &y](int start, int end) {
        for (int i = start; i < end; i++) {
            for (int j = 0; j < A[i].size(); j++) {
                y[i] += A[i][j] * x[j];
            }
        }
    };

    std::vector<std::thread> threads;
    for (int i = 0; i < p; i++) {
        int start = i * chunk;
        int end = (i == p - 1) ? size : start + chunk;
        threads.emplace_back(init, start, end);
    }
    for (auto& t : threads) t.join();
    threads.clear();

    std::clock_t start = std::clock();
    for (int i = 0; i < p; i++) {
        int start = i * chunk;
        int end = (i == p - 1) ? size : start + chunk;
        threads.emplace_back(worker, start, end);
    }
    for (auto& t : threads) t.join();
    std::clock_t end = std::clock();
    return (end - start) / CLOCKS_PER_SEC;
}

int main(int argc, char** argv){
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <size> <p>" << std::endl;
        return 1;
    }
    int size = std::stoi(argv[1]);
    int p = std::stoi(argv[2]);
    std::clock_t time0 = single(size, p);
    std::cout << "Time: " << time0 << " seconds" << std::endl;
    std::clock_t time1 = multi(size, p);
    std::cout << "Time: " << time1 << " seconds" << std::endl;
    std::cout << "Speedup: " << double(time0) / double(time1) << std::endl;
    return 0;
}