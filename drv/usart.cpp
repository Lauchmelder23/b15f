#include "usart.h"

void USART::openDevice(std::string device)
{
	file_desc = open(device.c_str(), O_RDWR | O_NOCTTY | O_NDELAY);
	if(file_desc <= 0)
		throw USARTException("Fehler beim Öffnen des Gerätes");
	
	struct termios options;
	int code = tcgetattr(file_desc, &options);
	if(code)
		throw USARTException("Fehler beim Lesen der Geräteparameter");
	
	options.c_cflag = CS8 | CLOCAL | CREAD; 
	options.c_iflag = IGNPAR;
	options.c_oflag = 0;
	options.c_lflag = 0;
	options.c_cc[VTIME] = timeout;	
    code = cfsetspeed(&options, baudrate);
	if(code)
		throw USARTException("Fehler beim Setzen der Baudrate");
    
	code = tcsetattr(file_desc, TCSANOW, &options);
	if(code)
		throw USARTException("Fehler beim Setzen der Geräteparameter");
	
	flushOutputBuffer();
	flushInputBuffer();
}
	
void USART::closeDevice()
{
	int code = close(file_desc);
	if(code)
		throw USARTException("Fehler beim Schließen des Gerätes");
}
	
void USART::flushInputBuffer()
{
	int code = tcflush(file_desc, TCIFLUSH);
	if(code)
		throw USARTException("Fehler beim Leeren des Eingangspuffers");
}
	
void USART::flushOutputBuffer()
{
	int code = tcflush(file_desc, TCOFLUSH);
	if(code)
		throw USARTException("Fehler beim Leeren des Ausgangspuffers");
}

void USART::writeByte(uint8_t b)
{
	int sent = write(file_desc, &b, 1);
	if(sent != 1)
	{
		std::cout << "WARNUNG: Fehler beim Senden (" << sent << "): writeByte(), wiederhole..." << std::endl;
		usleep(100000);
		sent = write(file_desc, &b, 1);
		if(sent != 1)
			throw USARTException("Fehler beim Senden: writeByte()");
	}
		
}
	
void USART::writeInt(uint16_t d)
{
	int sent = write(file_desc, reinterpret_cast<char*>(&d), 2);
	if(sent != 2)
		throw USARTException("Fehler beim Senden: writeInt()");
}



int USART::read_timeout(uint8_t* buffer, uint16_t offset, uint8_t len, uint32_t timeout)
{
	uint32_t elapsed = 0;
	int n_read = -1;
	auto start = std::chrono::steady_clock::now();
	auto end = start;
	while(elapsed < timeout)
	{
		n_read = read(file_desc, buffer + offset, len);
		if (n_read == len)
			return n_read;
				
		end = std::chrono::steady_clock::now();
		elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
	}
	
	return n_read;
}

int USART::write_timeout(uint8_t* buffer, uint16_t offset, uint8_t len, uint32_t timeout)
{
	uint32_t elapsed = 0;
	int n_sent = -1;
	auto start = std::chrono::steady_clock::now();
	auto end = start;
	while(elapsed < timeout)
	{
		n_sent = write(file_desc, buffer + offset, len);
		if (n_sent == len)
			return n_sent;
				
		end = std::chrono::steady_clock::now();
		elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
	}
	
	return n_sent;
}

void USART::writeBlock(uint8_t* buffer, uint16_t offset, uint8_t len)
{
	uint8_t crc;
	uint8_t aw;
	const uint16_t us_per_bit = (1000000 / baudrate) * 16;
	const uint16_t n_total = len + 3;
	
	do
	{	
		// calc crc
		crc = 0;
		for(uint8_t i = 0; i < len; i++)
		{			
			crc ^= buffer[i];
			for (uint8_t k = 0; k < 8; k++)
			{
				if (crc & 1)
					crc ^= CRC7_POLY;
				crc >>= 1;
			}			
		}
		
		// construct block
		block_buffer[0] = len;
		std::memcpy(&block_buffer[1], buffer + offset, len);
		block_buffer[len + 1] = crc;
		block_buffer[len + 2] = BLOCK_END;
			
		// send block
		int n_sent = write_timeout(&block_buffer[0], 0, len + 3, us_per_bit * n_total);
		if(n_sent != n_total)
			throw std::runtime_error("fatal (send): " + std::to_string(n_sent));
		flushOutputBuffer();
		
		usleep(1000);
			
		// check response
		int n_read = read_timeout(&aw, 0, 1, us_per_bit);
		for(uint8_t i = 0; i < 10 && n_read != 1; i++)
		{
			flushOutputBuffer();
			flushInputBuffer();
			std::cout << "WARNING: read error (" << n_read << "), retry #" << (int) i << std::endl;
			usleep(1000000);
			n_read = read_timeout(&aw, 0, 1, us_per_bit * 2);
		}
		
		if(n_read == 0)
		{
			std::cout << "timeout info" << std::endl;
			for(uint8_t i = 0; i < MAX_BLOCK_SIZE; i++)
			{
				writeByte(0x80); // Stoppzeichen für Block
								
				n_read = read_timeout(&aw, 0, 1, us_per_bit * 2);
				if(n_read == 1)
					break;
			}
		}
		else if(n_read != 1)
			throw std::runtime_error("fatal: " + std::to_string(n_read));
	
		flushInputBuffer();
	}
	while(aw != 0xFF);
	
	//std::cout << "OK" << std::endl;
}
	
uint8_t USART::readByte(void)
{
	char b;
	auto start = std::chrono::steady_clock::now();
	auto end = start;
	uint16_t elapsed = 0;
	while(elapsed < timeout * 100)
	{
		int code = read(file_desc, &b, 1);
		if (code > 0)
			return static_cast<uint8_t>(b);
				
		end = std::chrono::steady_clock::now();
		elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
	}
	
	throw TimeoutException("Verbindung unterbrochen.", timeout);
}

uint16_t USART::readInt(void)
{
	return readByte() | readByte() << 8;
}

bool USART::readBlock(uint8_t* buffer, uint16_t offset)
{
	uint8_t len = readByte();
	uint8_t crc = 0;
	buffer += offset;
	
	uint32_t block_timeout = timeout / 10;
	
	// wait for block
	int n_ready;
	uint16_t elapsed = 0;
	auto start = std::chrono::steady_clock::now();
	auto end = start;
	while(elapsed < block_timeout)
	{
		int code = ioctl(file_desc, FIONREAD, &n_ready);
		if(code != 0)
		{
			std::cout <<  "n_ready code: " << code << std::endl;
			return false;
		}
		
		if(n_ready >= len + 1)
			break;
			
		end = std::chrono::steady_clock::now();
		elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
	}
	if(elapsed >= timeout)
	{
		std::cout  << "block timeout: " << std::endl;
		return false;
	}
	
	while(len--)
	{
		*buffer = readByte();
		
		crc ^= *buffer++;
		for (uint8_t i = 0; i < 8; i++)
		{
			if (crc & 1)
				crc ^= CRC7_POLY;
			crc >>= 1;
		}
	}
	
	crc ^= readByte();
	for (uint8_t i = 0; i < 8; i++)
	{
		if (crc & 1)
			crc ^= CRC7_POLY;
		crc >>= 1;
	}
	
	if(TEST == 1)
		crc = 1;
	if(TEST > 100)
		TEST = 0;
	
	if (crc == 0)
	{
		writeByte(0xFF);
		return true;
	}
	else
	{
		writeByte(0xFE);
		return false;
	}
}

uint32_t USART::getBaudrate()
{
	return baudrate;
}

uint8_t USART::getTimeout()
{
	return timeout;
}
	
void USART::setBaudrate(uint32_t baudrate)
{
	this->baudrate = baudrate;
}

void USART::setTimeout(uint8_t timeout)
{
	this->timeout = timeout;
}
