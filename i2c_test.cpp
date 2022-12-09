#include <iostream>

#include <cstdio>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <array>

using namespace std;

const float FULL_SCALES[] = {6.144, 4.096, 2.048, 1.024, 0.512, 0.256};

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

inline int16_t str_to_int16(std::string& bytes)
{
    return static_cast<int16_t>(static_cast<uint8_t>(stoi(bytes.substr(0, 4), 0, 16)) << 8 | 
        static_cast<uint8_t>(stoi(bytes.substr(5), 0, 16)));
}

class ADS1115
{
public:
	ADS1115(uint16_t i2c_bus, uint16_t board_address)
	{
		_i2c_bus = to_string(i2c_bus);
		_board_address = to_string(board_address);
		set_config(0);
	}
	
	void set_config(uint8_t analog_input, uint8_t fs_mode = 2)
	{
		if (analog_input >= 4 || analog_input < 0)
		{
			cout << "analog_input is incorrect. You tried to assign AI: " << analog_input << "\n";
			return;
		}
		if (fs_mode >= 5 || fs_mode < 0)
		{
			cout << "fs_mode is incorrect. You tried to assign FS mode: " << fs_mode << "\n";
			return;
		}
		
		uint8_t config = 0b00010000 * analog_input + 0b00000010 * fs_mode + 0b11000000;
		
		_conversion_factor = FULL_SCALES[fs_mode] / 32768.0;
		
		// sudo i2ctransfer -y 0 w3@0x48 1 config 0x83
		_cmd = "sudo i2ctransfer -y " + _i2c_bus + " w3@" + _board_address + " 1 " + to_string(config) + " 0x83";
		execute_cmd(_cmd);
		
		// sudo i2cset -y 0 0x48 0
		_cmd = "sudo i2cset -y " + _i2c_bus + " " + _board_address + " 0";
		execute_cmd(_cmd);
	}
	
	float read_voltage()
	{
		// sudo i2ctransfer -y 0 w1@0x48 0 r2 -> reads two bytes from board address at offset zero
		static string read_cmd = "sudo i2ctransfer -y " + _i2c_bus + " w1@" + _board_address + " 0 r2";
		string _bytes = execute_cmd(read_cmd);
		return str_to_int16(_bytes) * _conversion_factor;
	}
	
private:
	float _conversion_factor;
	string _cmd, _bytes, _board_address, _i2c_bus;
};


#include <unistd.h>

int main()
{
	ADS1115 adc = ADS1115(0, 0x48);
	adc.set_config(1);
	while (true)
	{
		usleep(100000);
		float T = -66.875 + 218.75 * adc.read_voltage() / 3.3;
		std::cout << "Temp = " << T << " ºC" << std::endl;
	}
	
	return 0;
} // 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

