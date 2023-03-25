#include <iostream>
#include <fcntl.h>	/* For O_RDWR */
#include <unistd.h> /* For open(), creat() */
#include <sys/ioctl.h>
extern "C"
{
	#include <linux/i2c-dev.h>
}


class I2C_BUS
{
public:
	I2C_BUS(__u16 i2c_bus = 0)
	{
		char filename[20];
		snprintf(filename, 19, "/dev/i2c-%d", i2c_bus);
		file = open(filename, O_RDWR);

		if (file < 0)
			throw std::runtime_error("Error opening the i2c device. Does the device exist? Run as Sudo?\n");
	}
	
	void set_device_address(__u16 new_device_address)
	{
		if (_device_address == new_device_address && _first_address_was_set)
			return; // first device has been set and new device is the same as the last one, no need to change devices.
		
		_device_address = new_device_address;
		
		if (ioctl(file, I2C_SLAVE, _device_address) < 0)
			throw std::runtime_error("Error setting board address.\n");
		
		if (!_first_address_was_set)
			_first_address_was_set = true;
	}

	void write_to_device(__u8* buffer, __u8 num_bytes)
	{
		if (write(file, buffer, num_bytes) != num_bytes)
			throw std::runtime_error("Something went wrong when trying to write to device.");
	}

	void read_from_device(__u8* buffer, __u8 num_bytes)
	{
		if (read(file, buffer, num_bytes) != num_bytes)
			throw std::runtime_error("Something went wrong when trying to read from device.");
	}
	
	~I2C_BUS()
	{
		close(file);
	}
	
	int file;
	
private:
	__u16 _device_address;
	bool _first_address_was_set = false;
};


const float FULL_SCALES[] = {6.144, 4.096, 2.048, 1.024, 0.512, 0.256};

class ADS1115
{
public:
	ADS1115(I2C_BUS* i2c_bus, __u16 device_address = 0x48)
	{
		_i2c_bus = i2c_bus;
		_device_address = device_address;
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
		
		// writing to device
		_i2c_bus->set_device_address(_device_address);
		_i2c_bus->write_to_device(_buffer, 3);
		_buffer[0] = 0;
		_i2c_bus->write_to_device(_buffer, 1);
	}

	float read_voltage()
	{
		_i2c_bus->set_device_address(_device_address);
		_i2c_bus->read_from_device(_buffer, 2);
		return _conversion_factor * static_cast<__s16>(_buffer[0] << 8 | _buffer[1]);
	}

private:
	float _conversion_factor;
	__u16 _device_address;
	I2C_BUS* _i2c_bus;
	__u8 _buffer[3];
};


int main()
{
	I2C_BUS i2c_bus = I2C_BUS(0);
	ADS1115 adc = ADS1115(&i2c_bus, 0x48);
	adc.set_config(1);
	while (true)
	{
		usleep(100000);
		float T = -66.875 + 218.75 * adc.read_voltage() / 3.3;
		std::cout << "Temp = " << T << " ºC" << std::endl;
	}
	return 0;
}