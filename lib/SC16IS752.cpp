/* 
SC16IS752 driver for Arduino
*/

//#define SC16IS750_DEBUG_PRINT
#include <SC16IS752.h>
#include <SPI.h>
#include <Wire.h>


#ifdef __AVR__
 #define WIRE Wire
#elif ESP8266 // ESP8266
 #define WIRE Wire
#else // Arduino Due
 #define WIRE Wire1
#endif


SC16IS752::SC16IS752(uint8_t prtcl, uint8_t addr_sspin)
{
	protocol = prtcl;
	if ( protocol == SC16IS750_PROTOCOL_I2C ) {
		device_address_sspin = (addr_sspin>>1);
	} else {
		device_address_sspin = addr_sspin;
	}
	peek_flag = 0;
	//timeout = 1000;
}


void SC16IS752::begin(uint32_t baud_A, uint32_t baud_B)
{
	if ( protocol == SC16IS750_PROTOCOL_I2C) {
		WIRE.begin(25, 26);
		WIRE.setClock(200000);
	} else {
		::pinMode(device_address_sspin, OUTPUT);
		::digitalWrite(device_address_sspin, HIGH);
		SPI.setDataMode(SPI_MODE0);
		SPI.setClockDivider(SPI_CLOCK_DIV4);
		SPI.setBitOrder(MSBFIRST);
		SPI.begin();
		//SPI.setClockDivider(32);
	};
	ResetDevice();
	FIFOEnable(SC16IS752_CHANNEL_A, 1);
	FIFOEnable(SC16IS752_CHANNEL_B, 1);
	SetBaudrate(SC16IS752_CHANNEL_A, baud_A);
	SetBaudrate(SC16IS752_CHANNEL_B, baud_B);
	SetLine(SC16IS752_CHANNEL_A, 8, 0, 1);
	SetLine(SC16IS752_CHANNEL_B, 8, 0, 1);
}

int SC16IS752::available(uint8_t channel)
{
	return FIFOAvailableData(channel);
}

uint8_t SC16IS752::linestate(uint8_t channel)
{
	return ReadRegister(channel, SC16IS750_REG_LSR);
}

int SC16IS752::read(uint8_t channel)
{
	if ( peek_flag == 0) {
		return ReadByte(channel);
	} else {
		peek_flag = 0;
		return peek_buf;
	}
}

int SC16IS752::read(uint8_t channel, size_t received)
{
	if ( peek_flag == 0) {
		return ReadByte(channel, received);
	} else {
		peek_flag = 0;
		return peek_buf;
	}
}


size_t SC16IS752::write(uint8_t channel, uint8_t val)
{
	WriteByte(channel, val);
}

size_t SC16IS752::write(uint8_t channel, const uint8_t *val, size_t received)
{
	WriteByte(channel, val, received);
}

void SC16IS752::write(uint8_t channel, String val)
{
	char* stringVal = &val[0];
    	char buf[64];
   	sprintf(buf, stringVal);
    	for (int i = 0; i < strlen(buf); i++) {
      		WriteByte(channel, buf[i]);
    	}
}

void SC16IS752::pinMode(uint8_t pin, uint8_t i_o)
{
	GPIOSetPinMode(pin, i_o);
}

void SC16IS752::digitalWrite(uint8_t pin, uint8_t value)
{
	GPIOSetPinState(pin, value);
}

uint8_t SC16IS752::digitalRead(uint8_t pin)
{
	return GPIOGetPinState(pin);
}


uint8_t SC16IS752::ReadRegister(uint8_t channel, uint8_t reg_addr)
{
	uint8_t result;
	if ( protocol == SC16IS750_PROTOCOL_I2C ) {			// register read operation via I2C
		WIRE.beginTransmission(device_address_sspin);
		WIRE.write((reg_addr<<3 | channel<<1));
		WIRE.endTransmission(0);
		WIRE.requestFrom(device_address_sspin,(uint8_t)1);
		result = WIRE.read();
	} else if (protocol == SC16IS750_PROTOCOL_SPI) {	//register read operation via SPI
		::digitalWrite(device_address_sspin, LOW);
		delayMicroseconds(10);
		SPI.transfer(0x80|((reg_addr<<3 | channel<<1)));
		result = SPI.transfer(0xff);
		delayMicroseconds(10);
		::digitalWrite(device_address_sspin, HIGH);
	}

#ifdef  SC16IS750_DEBUG_PRINT
	Serial.print("ReadRegister channel=");
	Serial.print(channel,HEX);
	Serial.print(" reg_addr=");
	Serial.print((reg_addr<<3 | channel<<1),HEX);
	Serial.print(" result=");
	Serial.println(result,HEX);
#endif
	return result;

}

// WRITEREG
void SC16IS752::WriteRegister(uint8_t channel, uint8_t reg_addr, const uint8_t *val, size_t received)
{
#ifdef  SC16IS750_DEBUG_PRINT
	Serial.print("WriteRegister channel=");
	Serial.print(channel,HEX);
	Serial.print(" reg_addr=");
	Serial.print((reg_addr<<3 | channel<<1),HEX);
	Serial.print(" val=");
	Serial.println(val,HEX);
#endif

	Serial.print("writeee");
	if ( protocol == SC16IS750_PROTOCOL_I2C ) {

		int currentPackages = ReadRegister(channel, SC16IS750_REG_RXLVL);
		bool send = true;

		/* Serial.println(ReadRegister(channel, SC16IS750_REG_RXLVL), DEC);
		while(currentPackages != 0) {
		  	Serial.println("not 0");

			delay(1000);
		} */

      		uint8_t buff2[64] = { 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70 };

		WIRE.flush();
		WIRE.beginTransmission(device_address_sspin);
		WIRE.write((reg_addr<<3 | channel<<1));
		WIRE.write(buff2, 64);

		Serial.println(ReadRegister(channel, SC16IS750_REG_RXLVL), DEC);

		WIRE.endTransmission(1);
	} 
	return ;
}

// READREG
uint8_t SC16IS752::ReadRegister(uint8_t channel, uint8_t reg_addr, size_t received)
{

	int currentPackages = ReadRegister(channel, SC16IS750_REG_RXLVL);
	Serial.println(ReadRegister(channel, SC16IS750_REG_RXLVL), DEC);
		


	uint8_t result;
	WIRE.flush();
	WIRE.beginTransmission(device_address_sspin);
	WIRE.write((reg_addr<<3 | channel<<1));
	WIRE.endTransmission(0);
	WIRE.requestFrom(device_address_sspin, (uint8_t) 64);
	
	uint8_t buff[64];
	
	delay(1);
	result = WIRE.readBytes(buff, 64);
	for(int i = 0; i < 64; i++) {
		Serial.print(buff[i]);
		Serial.print(" ");
	}


	return result;

}

void SC16IS752::WriteRegister(uint8_t channel, uint8_t reg_addr, uint8_t val)
{
#ifdef  SC16IS750_DEBUG_PRINT
	Serial.print("WriteRegister channel=");
	Serial.print(channel,HEX);
	Serial.print(" reg_addr=");
	Serial.print((reg_addr<<3 | channel<<1),HEX);
	Serial.print(" val=");
	Serial.println(val,HEX);
#endif

	if ( protocol == SC16IS750_PROTOCOL_I2C ) {			// register write operation via I2C
		WIRE.beginTransmission(device_address_sspin);
		WIRE.write((reg_addr<<3 | channel<<1));
		WIRE.write(val);
		WIRE.endTransmission(1);
	} else {											//register write operation via SPI
		::digitalWrite(device_address_sspin, LOW);
		delayMicroseconds(10);
		SPI.transfer((reg_addr<<3 | channel<<1));
		SPI.transfer(val);
		delayMicroseconds(10);
		::digitalWrite(device_address_sspin, HIGH);
	}
	return ;
}

int16_t SC16IS752::SetBaudrate(uint8_t channel, uint32_t baudrate) //return error of baudrate parts per thousand
{
	uint16_t divisor;
	uint8_t prescaler;
	uint32_t actual_baudrate;
	int16_t error;
	uint8_t temp_lcr;
	if ( (ReadRegister(channel, SC16IS750_REG_MCR)&0x80) == 0) { //if prescaler==1
		prescaler = 1;
	} else {
		prescaler = 4;
	}

	divisor = (SC16IS750_CRYSTCAL_FREQ/prescaler)/(baudrate*16);

	temp_lcr = ReadRegister(channel, SC16IS750_REG_LCR);
	temp_lcr |= 0x80;
	WriteRegister(channel, SC16IS750_REG_LCR, temp_lcr);
	//write to DLL
	WriteRegister(channel, SC16IS750_REG_DLL, (uint8_t)divisor);
	//write to DLH
	WriteRegister(channel, SC16IS750_REG_DLH, (uint8_t)(divisor>>8));
	temp_lcr &= 0x7F;
	WriteRegister(channel, SC16IS750_REG_LCR, temp_lcr);


	actual_baudrate = (SC16IS750_CRYSTCAL_FREQ/prescaler)/(16*divisor);
	error = ((float)actual_baudrate-baudrate)*1000/baudrate;
#ifdef  SC16IS750_DEBUG_PRINT
	Serial.print("Desired baudrate: ");
	Serial.println(baudrate,DEC);
	Serial.print("Calculated divisor: ");
	Serial.println(divisor,DEC);
	Serial.print("Actual baudrate: ");
	Serial.println(actual_baudrate,DEC);
	Serial.print("Baudrate error: ");
	Serial.println(error,DEC);
#endif

	return error;

}

void SC16IS752::SetLine(uint8_t channel, uint8_t data_length, uint8_t parity_select, uint8_t stop_length )
{
	uint8_t temp_lcr;
	temp_lcr = ReadRegister(channel, SC16IS750_REG_LCR);
	temp_lcr &= 0xC0; //Clear the lower six bit of LCR (LCR[0] to LCR[5]
#ifdef  SC16IS750_DEBUG_PRINT
	Serial.print("LCR Register:0x");
	Serial.println(temp_lcr,DEC);
#endif
	switch (data_length) {			//data length settings
		case 5:
			break;
		case 6:
			temp_lcr |= 0x01;
			break;
		case 7:
			temp_lcr |= 0x02;
			break;
		case 8:
			temp_lcr |= 0x03;
			break;
		default:
			temp_lcr |= 0x03;
			break;
	}

	if ( stop_length == 2 ) {
		temp_lcr |= 0x04;
	}

	//parity selection length settings
	switch (parity_select) {
		case 0:						//no parity
			 break;
		case 1:						//odd parity
			temp_lcr |= 0x08;
			break;
		case 2:						//even parity
			temp_lcr |= 0x18;
			break;
		case 3:						//force '1' parity
			temp_lcr |= 0x03;
			break;
		case 4:						//force '0' parity
			break;
		default:
			break;
	}

	WriteRegister(channel, SC16IS750_REG_LCR,temp_lcr);
}

void SC16IS752::GPIOSetPinMode(uint8_t pin_number, uint8_t i_o)
{
	uint8_t temp_iodir;

	temp_iodir = ReadRegister(SC16IS752_CHANNEL_BOTH, SC16IS750_REG_IODIR);
	if ( i_o == OUTPUT ) {
	  temp_iodir |= (0x01 << pin_number);
	} else {
	  temp_iodir &= (uint8_t)~(0x01 << pin_number);
	}

	WriteRegister(SC16IS752_CHANNEL_BOTH, SC16IS750_REG_IODIR, temp_iodir);
	return;
}

void SC16IS752::GPIOSetPinState(uint8_t pin_number, uint8_t pin_state)
{
	uint8_t temp_iostate;

	temp_iostate = ReadRegister(SC16IS752_CHANNEL_BOTH, SC16IS750_REG_IOSTATE);
	if ( pin_state == 1 ) {
	  temp_iostate |= (0x01 << pin_number);
	} else {
	  temp_iostate &= (uint8_t)~(0x01 << pin_number);
	}

	WriteRegister(SC16IS752_CHANNEL_BOTH, SC16IS750_REG_IOSTATE, temp_iostate);
	return;
}


uint8_t SC16IS752::GPIOGetPinState(uint8_t pin_number)
{
  uint8_t temp_iostate;

  temp_iostate = ReadRegister(SC16IS752_CHANNEL_BOTH, SC16IS750_REG_IOSTATE);

  if ((temp_iostate & (0x01 << pin_number)) == 0) {
    return 0;
  }
  return 1;
}

uint8_t SC16IS752::GPIOGetPortState(void)
{
	return ReadRegister(SC16IS752_CHANNEL_BOTH, SC16IS750_REG_IOSTATE);
}

void SC16IS752::GPIOSetPortMode(uint8_t port_io)
{
	WriteRegister(SC16IS752_CHANNEL_BOTH, SC16IS750_REG_IODIR, port_io);
	return;
}

void SC16IS752::GPIOSetPortState(uint8_t port_state)
{
	WriteRegister(SC16IS752_CHANNEL_BOTH, SC16IS750_REG_IOSTATE, port_state);
	return;
}

void SC16IS752::SetPinInterrupt(uint8_t io_int_ena)
{
	WriteRegister(SC16IS752_CHANNEL_BOTH, SC16IS750_REG_IOINTENA, io_int_ena);
	return;
}

void SC16IS752::ResetDevice()
{
	uint8_t reg;

	reg = ReadRegister(SC16IS752_CHANNEL_BOTH, SC16IS750_REG_IOCONTROL);
	reg |= 0x08;
	WriteRegister(SC16IS752_CHANNEL_BOTH, SC16IS750_REG_IOCONTROL, reg);

	return;
}

void SC16IS752::ModemPin(uint8_t gpio) //gpio == 0, gpio[7:4] are modem pins, gpio == 1 gpio[7:4] are gpios
{
	uint8_t temp_iocontrol;

	temp_iocontrol = ReadRegister(SC16IS752_CHANNEL_BOTH, SC16IS750_REG_IOCONTROL);
	if ( gpio == 0 ) {
		temp_iocontrol |= 0x02;
	} else {
		temp_iocontrol &= 0xFD;
	}
	WriteRegister(SC16IS752_CHANNEL_BOTH, SC16IS750_REG_IOCONTROL, temp_iocontrol);

	return;
}

void SC16IS752::GPIOLatch(uint8_t latch)
{
	uint8_t temp_iocontrol;

	temp_iocontrol = ReadRegister(SC16IS752_CHANNEL_BOTH, SC16IS750_REG_IOCONTROL);
	if ( latch == 0 ) {
		temp_iocontrol &= 0xFE;
	} else {
		temp_iocontrol |= 0x01;
	}
	WriteRegister(SC16IS752_CHANNEL_BOTH, SC16IS750_REG_IOCONTROL, temp_iocontrol);

	return;
}

void SC16IS752::InterruptControl(uint8_t channel, uint8_t int_ena)
{
	WriteRegister(channel, SC16IS750_REG_IER, int_ena);
}

uint8_t SC16IS752::InterruptPendingTest(uint8_t channel)
{
	return (ReadRegister(channel,SC16IS750_REG_IIR) & 0x01);
}

int SC16IS752::InterruptEventTest(uint8_t channel)
{
	uint8_t irq_src;

	irq_src = ReadRegister(channel, SC16IS750_REG_IIR);
	//irq_src = (irq_src >> 1);
	//irq_src &= 0x3F;
	irq_src &= 0x3E;

	switch (irq_src) {
		case 0x06:			//Receiver Line Status Error
			return SC16IS750_RECEIVE_LINE_STATUS_ERROR;
		case 0x0c:			//Receiver time-out interrupt
			return SC16IS750_RECEIVE_TIMEOUT_INTERRUPT;
		case 0x04:			//RHR interrupt
			return SC16IS750_RHR_INTERRUPT;
		case 0x02:			//THR interrupt
			return SC16IS750_THR_INTERRUPT;
		case 0x00:			//modem interrupt;
			return SC16IS750_MODEM_INTERRUPT;
		case 0x30:			//input pin change of state
			return SC16IS750_INPUT_PIN_CHANGE_STATE;
		case 0x10:			//XOFF
			return SC16IS750_RECEIVE_XOFF;
		case 0x20:			//CTS,RTS
			return SC16IS750_CTS_RTS_CHANGE;
		default:
			return -1;
	}
	return -1;
}

void SC16IS752::FIFOEnable(uint8_t channel, uint8_t fifo_enable)
{
	uint8_t temp_fcr;

	temp_fcr = ReadRegister(channel, SC16IS750_REG_FCR);

	if (fifo_enable == 0){
		temp_fcr &= 0xFE;
	} else {
		temp_fcr |= 0x01;
	}
	WriteRegister(channel, SC16IS750_REG_FCR, temp_fcr);

	return;
}

void SC16IS752::FIFOReset(uint8_t channel, uint8_t rx_fifo)
{
	uint8_t temp_fcr;

	temp_fcr = ReadRegister(channel, SC16IS750_REG_FCR);

	if (rx_fifo == 0){
		temp_fcr |= 0x04;
	} else {
		temp_fcr |= 0x02;
	}
	WriteRegister(channel, SC16IS750_REG_FCR,temp_fcr);

	return;

}

void SC16IS752::FIFOSetTriggerLevel(uint8_t channel, uint8_t rx_fifo, uint8_t length)
{
	uint8_t temp_reg;

	temp_reg = ReadRegister(channel, SC16IS750_REG_MCR);
	temp_reg |= 0x04;
	WriteRegister(channel, SC16IS750_REG_MCR,temp_reg); //SET MCR[2] to '1' to use TLR register or trigger level control in FCR register

	temp_reg = ReadRegister(channel, SC16IS750_REG_EFR);
	WriteRegister(channel, SC16IS750_REG_EFR, temp_reg|0x10); //set ERF[4] to '1' to use the  enhanced features
	if (rx_fifo == 0) {
		WriteRegister(channel, SC16IS750_REG_TLR, length<<4); //Tx FIFO trigger level setting
	} else {
		WriteRegister(channel, SC16IS750_REG_TLR, length);	//Rx FIFO Trigger level setting
	}
	WriteRegister(channel, SC16IS750_REG_EFR, temp_reg); //restore EFR register

	return;
}

uint8_t SC16IS752::FIFOAvailableData(uint8_t channel)
{
#ifdef  SC16IS750_DEBUG_PRINT
	Serial.print("=====Available data:");
	Serial.println(ReadRegister(channel, SC16IS750_REG_RXLVL), DEC);
#endif
	return ReadRegister(channel, SC16IS750_REG_RXLVL);
//	return ReadRegister(channel, SC16IS750_REG_LSR) & 0x01;
}

uint8_t SC16IS752::FIFOAvailableSpace(uint8_t channel)
{
   return ReadRegister(channel, SC16IS750_REG_TXLVL);
}

void SC16IS752::WriteByte(uint8_t channel, uint8_t val)
{
	uint8_t tmp_lsr;

/*
	while ( FIFOAvailableSpace(channel) == 0 ){
#ifdef  SC16IS750_DEBUG_PRINT
		Serial.println("No available space");
#endif

	};

#ifdef  SC16IS750_DEBUG_PRINT
	Serial.println("++++++++++++Data sent");
#endif
	WriteRegister(SC16IS750_REG_THR,val);
*/

	do {
		tmp_lsr = ReadRegister(channel, SC16IS750_REG_LSR);
	} while ((tmp_lsr&0x20) ==0);

	WriteRegister(channel, SC16IS750_REG_THR,val);
}

void SC16IS752::WriteByte(uint8_t channel, const uint8_t *val, size_t received) {
	uint8_t tmp_lsr;

/*
	while ( FIFOAvailableSpace(channel) == 0 ){
#ifdef  SC16IS750_DEBUG_PRINT
		Serial.println("No available space");
#endif

	};

#ifdef  SC16IS750_DEBUG_PRINT
	Serial.println("++++++++++++Data sent");
#endif
	WriteRegister(SC16IS750_REG_THR,val);
*/
	do {
		tmp_lsr = ReadRegister(channel, SC16IS750_REG_LSR);
	} while ((tmp_lsr&0x20) ==0);

	WriteRegister(channel, SC16IS750_REG_THR, val, received);
}


int SC16IS752::ReadByte(uint8_t channel)
{
	volatile uint8_t val;
	if (FIFOAvailableData(channel) == 0) {
#ifdef  SC16IS750_DEBUG_PRINT
		Serial.println("No data available");
#endif
		return -1;

	} else {
#ifdef  SC16IS750_DEBUG_PRINT
		Serial.println("***********Data available***********");
#endif
		val = ReadRegister(channel, SC16IS750_REG_RHR);
		return val;
	}
}

int SC16IS752::ReadByte(uint8_t channel, size_t received)
{
	volatile uint8_t val;
	if (FIFOAvailableData(channel) == 0) {
#ifdef  SC16IS750_DEBUG_PRINT
		Serial.println("No data available");
#endif
		return -1;

	} else {
#ifdef  SC16IS750_DEBUG_PRINT
		Serial.println("***********Data available***********");
#endif
		val = ReadRegister(channel, SC16IS750_REG_RHR, received);
		return val;
	}
}

void SC16IS752::EnableRs485(uint8_t channel, uint8_t invert)
{
	uint8_t temp_efcr;
	temp_efcr = ReadRegister(channel, SC16IS750_REG_EFCR);
	if ( invert == NO_INVERT_RTS_SIGNAL) {
		temp_efcr |= 0x20;
	} else {
		temp_efcr |= 0x30;
	}
	WriteRegister(channel, SC16IS750_REG_EFCR,temp_efcr);

	return;
}


void SC16IS752::EnableTransmit(uint8_t channel, uint8_t tx_enable)
{
	uint8_t temp_efcr;
	temp_efcr = ReadRegister(channel, SC16IS750_REG_EFCR);
	if ( tx_enable == 0) {
		temp_efcr |= 0x04;
	} else {
		temp_efcr &= 0xFB;
	}
	WriteRegister(channel, SC16IS750_REG_EFCR,temp_efcr);

	return;
}

uint8_t SC16IS752::ping()
{
	WriteRegister(SC16IS752_CHANNEL_A, SC16IS750_REG_SPR,0x55);
	if (ReadRegister(SC16IS752_CHANNEL_A, SC16IS750_REG_SPR) !=0x55) {
		return 0;
	}

	WriteRegister(SC16IS752_CHANNEL_A, SC16IS750_REG_SPR,0xAA);
	if (ReadRegister(SC16IS752_CHANNEL_A, SC16IS750_REG_SPR) !=0xAA) {
		return 0;
	}

	WriteRegister(SC16IS752_CHANNEL_B, SC16IS750_REG_SPR,0x55);
	if (ReadRegister(SC16IS752_CHANNEL_B, SC16IS750_REG_SPR) !=0x55) {
		return 0;
	}

	WriteRegister(SC16IS752_CHANNEL_B, SC16IS750_REG_SPR,0xAA);
	if (ReadRegister(SC16IS752_CHANNEL_B, SC16IS750_REG_SPR) !=0xAA) {
		return 0;
	}

	return 1;

}


void SC16IS752::setTimeout(uint32_t time_out)
{
	timeout = time_out;
}

size_t SC16IS752::readBytes(const uint8_t *val, size_t received)
{
	size_t count = 0;
	int16_t tmp;

	int remaining = read(SC16IS752_CHANNEL_A, received);
	/* 
	for(int i = 0; i < 10; i++) {
		Serial.println(val[i]);
	} */

	
	Serial.println("###########");
	Serial.print("Received");
	Serial.println(received);
	Serial.print("Remaining");
	Serial.println(remaining);
	Serial.println("###########"); 

  	return remaining;
}


int16_t SC16IS752::readwithtimeout(uint8_t * channel)
{
	int16_t tmp;
	uint32_t time_stamp;
	time_stamp = millis();
	do {
		*channel = SC16IS752_CHANNEL_A;
		tmp = read(SC16IS752_CHANNEL_A);
		if (tmp >= 0) return tmp;
		*channel = SC16IS752_CHANNEL_B;
		tmp = read(SC16IS752_CHANNEL_B);
		if (tmp >= 0) return tmp;
	} while(millis() - time_stamp < timeout);
	return -1;	 // -1 indicates timeout
}

void SC16IS752::flush(uint8_t channel)
{
	uint8_t tmp_lsr;

	do {
		tmp_lsr = ReadRegister(channel, SC16IS750_REG_LSR);
	} while ((tmp_lsr&0x20) ==0);


}

int SC16IS752::peek(uint8_t channel)
{
	if ( peek_flag == 0 ) {
		peek_buf =ReadByte(channel);
		if (  peek_buf >= 0 ) {
			peek_flag = 1;
		}
	}

	return peek_buf;

}