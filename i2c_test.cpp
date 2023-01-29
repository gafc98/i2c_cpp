#include <iostream>
#include <fcntl.h>	/* For O_RDWR */
#include <unistd.h> /* For open(), creat() */
#include <sys/ioctl.h>
extern "C"
{
	#include <linux/i2c-dev.h>
}

const float FULL_SCALES[] = {6.144, 4.096, 2.048, 1.024, 0.512, 0.256};

class ADS1115
{
public:
	ADS1115(__u16 i2c_bus = 0, __u16 board_address = 0x48)
	{
		char filename[20];
		snprintf(filename, 19, "/dev/i2c-%d", i2c_bus);
		_file = open(filename, O_RDWR);

		if (_file < 0)
			throw std::runtime_error("Error opening the i2c devide. Does the device exist? Run as Sudo?\n");

		if (ioctl(_file, I2C_SLAVE, board_address) < 0)
			throw std::runtime_error("Error calling ioctl.\n");

		set_config(0);
	}

	void set_config(__u8 analog_input, __u8 fs_mode = 2)
	{
		if (analog_input >= 4 || analog_input < 0)
			throw std::runtime_error("analog_input is incorrect.\n");

		if (fs_mode >= 5 || fs_mode < 0)
			throw std::runtime_error("fs_mode is incorrect.\n");

		_conversion_factor = FULL_SCALES[fs_mode] / 32768.0;

		_buffer[0] = 1;
		_buffer[1] = 0b00010000 * analog_input + 0b00000010 * fs_mode + 0b11000000;
		_buffer[2] = 0x83;
		if (write(_file, _buffer, 3) != 3)
			throw std::runtime_error("Something went wrong when trying to write to device.");

		_buffer[0] = 0;
		if (write(_file, _buffer, 1) != 1)
			throw std::runtime_error("Something went wrong when trying to write to device.");
	}

	float read_voltage()
	{
		if (read(_file, _buffer, 2) != 2)
			throw std::runtime_error("Something went wrong when trying to read from device.");
		return _conversion_factor * static_cast<__s16>(_buffer[0] << 8 | _buffer[1]);
	}

	~ADS1115()
	{
		close(_file);
	}

private:
	float _conversion_factor;
	int _file;
	__u8 _buffer[3];
};

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
}
