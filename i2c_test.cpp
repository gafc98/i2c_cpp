#include <iostream>

#include <cstdio>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <array>

using namespace std;

string execute_cmd(const string& cmd)
{
	std::array<char, 128> buffer;
	std::string result;
	std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd.c_str(), "r"), pclose);
    
	if (!pipe)
		throw std::runtime_error("popen() failed!");

	while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr)
		result += buffer.data();

	return result;
}

int main()
{
	cout << "Hello World!\n";
	//const char cmd[] = "sudo i2cdetect -y 0";
	string cmd = "sudo i2ctransfer -y 0 w3@0x48 1 0x84 0x83";
	string result = execute_cmd(cmd);
	cout << result;
	
	cmd = "sudo i2cset -y 0 0x48 0";
	result = execute_cmd(cmd);
	cout << result;
	
	cmd = "sudo i2ctransfer -y 0 w1@0x48 0 r2";
	result = execute_cmd(cmd);
	cout << result;
	
	
	
	// or: system("sudo i2cdetect -y 0");
	return 0;
}

// sudo i2ctransfer -y 0 w3@0x48 1 0x84 0x83
// sudo i2cset -y 0 0x48 0
// sudo i2ctransfer -y 0 w1@0x48 0 r2

