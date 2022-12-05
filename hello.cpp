#include <iostream>

#include <cstdio>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <array>

using namespace std;

std::string exec(const char* cmd)
{
    std::array<char, 128> buffer;
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
    if (!pipe) {
        throw std::runtime_error("popen() failed!");
    }
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    return result;
}

int main()
{
	cout << "Hello World!\n";
	const char cmd[] = "sudo i2cdetect -y 0";
	string result = exec(cmd);
	cout << result;
	// or: system("sudo i2cdetect -y 0");
	return 0;
}

// sudo i2ctransfer -y 0 w3@0x48 1 0x84 0x83
// sudo i2ctransfer -y 0 w3@0x48 1 0x84 0x83
// sudo i2ctransfer -y 0 w3@0x48 1 0x84 0x83

