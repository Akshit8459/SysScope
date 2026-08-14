#include <iostream>
#include <fstream>
#include <vector>
#include <chrono>
#include <thread>
#include <cstdlib>

int main(int argc, char* argv[]) {
    int file_size_mb = 64;
    if (argc > 1) file_size_mb = std::atoi(argv[1]);

    std::cout << "[io_stressor] Writing " << file_size_mb << " MB synthetic file I/O...\n";

    std::string temp_filename = "sysscope_io_test_tmp.bin";
    std::ofstream outfile(temp_filename, std::ios::binary);

    std::vector<char> buffer(1024 * 1024, 'X'); // 1 MB buffer

    auto start = std::chrono::steady_clock::now();
    for (int i = 0; i < file_size_mb; ++i) {
        outfile.write(buffer.data(), buffer.size());
        outfile.flush();
    }
    outfile.close();
    auto end = std::chrono::steady_clock::now();

    double elapsed = std::chrono::duration<double>(end - start).count();
    std::cout << "[io_stressor] Written " << file_size_mb << " MB in " << elapsed << " seconds.\n";

    std::remove(temp_filename.c_str());
    return 0;
}
