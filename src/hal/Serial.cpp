#include "hal/Serial.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <math.h>
#include <float.h>
#include <errno.h>

#include <iostream>

#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <sys/types.h>


/**************** Termios(3) Man Page ****************/
/* struct termios {
 * 		tcflag_t c_iflag;	// Input modes
 *		tcflag_t c_oflag;	// Output modes
 *		tcflag_t c_lflag;	// Local modes
 *		tcflag_t c_cflag;	// Control modes
 *		cc_t c_cc[NCCS];
 * };
 *
 *
 *
 *
 *
 *
 *
 *
 *
 */


static inline speed_t toSpeed_t(unsigned long baud)
{
	if (baud == 50)
		return B50;
	if (baud == 75)
		return B75;
	if (baud == 110)
		return B110;
	if (baud == 134)
		return B134;
	if (baud == 150)
		return B150;
	if (baud == 200)
		return B200;
	if (baud == 300)
		return B300;
	if (baud == 600)
		return B600;
	if (baud == 1200)
		return B1200;
	if (baud == 1800)
		return B1800;
	if (baud == 2400)
		return B2400;
	if (baud == 4800)
		return B4800;
	if (baud == 9600)
		return B9600;
	if (baud == 19200)
		return B19200;
	if (baud == 38400)
		return B38400;
	if (baud == 57600)
		return B57600;
	if (baud == 115200)
		return B115200;
	if (baud == 230400)
		return B230400;

	return B0;
}

static inline unsigned long toBaudLong(speed_t speed)
{
	if (speed == B50)
		return 50;
	if (speed == B75)
		return 75;
	if (speed == B110)
		return 110;
	if (speed == B134)
		return 134;
	if (speed == B150)
		return 150;
	if (speed == B200)
		return 200;
	if (speed == B300)
		return 300;
	if (speed == B600)
		return 600;
	if (speed == B1200)
		return 1200;
	if (speed == B1800)
		return 1800;
	if (speed == B2400)
		return 2400;
	if (speed == B4800)
		return 4800;
	if (speed == B9600)
		return 9600;
	if (speed == B19200)
		return 19200;
	if (speed == B38400)
		return 38400;
	if (speed == B57600)
		return 57600;
	if (speed == B115200)
		return 115200;
	if (speed == B230400)
		return 230400;

	return 0;
}



HardwareSerial::HardwareSerial(const char *pathname): fd_(0)
{
	fd_ = ::open(pathname, O_RDWR | O_NOCTTY);
	if (fd_ < 0) {
		// exception throw
	}


}

HardwareSerial::~HardwareSerial(void)
{
	if (fd_ > 0)
		close(fd_);
}

void HardwareSerial::begin(unsigned long baud, uint8_t cfg)
{
	if (fd_ <= 0)
		return;
	if (fcntl(fd_, F_SETFL, O_NONBLOCK))
		return;

	struct termios attr;
	speed_t speed = (speed_t)toSpeed_t(baud);
	memset(&attr, 0, sizeof(attr));
	
	if (tcgetattr(fd_, &attr))
		return;
	attr.c_iflag = IGNBRK | IGNPAR;
	attr.c_oflag = 0;
	attr.c_cflag = CS8 | CREAD | CLOCAL; // Character Bits, Stop Bits and Parity Bits
	attr.c_lflag = 0;	// Non-Canonical Mode

	attr.c_cc[VTIME] = 0;
	attr.c_cc[VMIN] = 0;

	if (cfsetispeed(&attr, speed))
		return;
	if (cfsetospeed(&attr, speed))
		return;
	if (tcsetattr(fd_, TCSANOW, &attr))
		return;

	struct termios readAttr;
	if (tcgetattr(fd_, &readAttr))
		return;
	if (memcmp(&attr, &readAttr, sizeof(attr)))
		return;

	// std::cout << "Successfull" << std::endl;
}

void HardwareSerial::end(void)
{
	if (fd_ > 0)
		close(fd_);
	fd_ = 0;
}

void HardwareSerial::flush(void)
{
	while (tcdrain(fd_));
}

size_t HardwareSerial::write(uint8_t c)
{
	return ::write(fd_, &c, sizeof(c));
}

int HardwareSerial::available(void)
{
	int in_bytes;
	ioctl(fd_, FIONREAD, &in_bytes);
	return in_bytes;
}

int HardwareSerial::peek(void)
{

}

int HardwareSerial::read(void)
{
	uint8_t byte;
	if (::read(fd_, &byte, sizeof(byte)) <= 0)
		return -1;
	return byte;
}

int HardwareSerial::println(const char *cstr)
{
	int written = 0;
	int len = strlen(cstr);
	for (int i = 0; i < len; i++)
		written += write(cstr[i]);

	written += write('\r');
	// written += write('\n');

	return written;
}

std::string HardwareSerial::readLine(void)
{
	std::string str;
	while (1) {
		if (available() <= 0)
			continue;
		char c = read();
		str.push_back(c);
		if (c == '\n')
			break;
	}
	return str;
}

#ifdef __MAIN__
int main(int argc, char *argv[])
{
	std::cout << "Program Started" << std::endl;
	if (argc != 2) {
		std::cout << "Two arguments required" << std::endl;
		exit(EXIT_FAILURE);
	}

	HardwareSerial serial(argv[1]);
	serial.begin(115200);
	serial.println("status");
	sleep(2);
	std::cout << "Reply: " << serial.available() << " " <<	 serial.readLine() << std::endl;
	return 0;
}
#endif