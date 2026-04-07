#include <fstream>
#include <cmath>
#include <iostream>
#include <sstream>
#include <string>

bool check_file(const std::string& filename, double (*func)(double)) {
    std::ifstream in(filename);
    std::string line;
    bool all_ok = true;
    while (std::getline(in, line)) {
        std::istringstream iss(line);
        double arg, res_file;
        std::string skip;
        size_t id;
        iss >> arg  >> res_file  >> id;
        double res_calc = func(arg);
        if (std::abs(res_calc - res_file) > 1e-5) {
            std::cout << "Mismatch: arg=" << arg << " file=" << res_file << " calc=" << res_calc << "\n";
            all_ok = false;
        }
    }
    return all_ok;
}

bool check_file2(const std::string& filename, double (*func)(double, double)) {
    std::ifstream in(filename);
    std::string line;
    bool all_ok = true;
    while (std::getline(in, line)) {
        std::istringstream iss(line);
        double arg1, arg2, res_file;
        std::string skip;
        size_t id;
        iss >> arg1 >> arg2 >> res_file >> skip >> id;
        double res_calc = func(arg1, arg2);
        if (std::abs(res_calc - res_file) > 1e-5) {
            std::cout << "Mismatch: arg1=" << arg1 << " arg2=" << arg2 << " file=" << res_file << " calc=" << res_calc << "\n";
            all_ok = false;
        }
    }
    return all_ok;
}

int main() {
    bool ok1 = check_file("sin.txt", std::sin);
    bool ok2 = check_file("sqrt.txt", std::sqrt);
    bool ok3 = check_file2("pow.txt", std::pow);
    
    if (ok1 && ok2 && ok3) std::cout << "All results correct!\n";
    return 0;
}