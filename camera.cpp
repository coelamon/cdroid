/* code taken from ComputerNerd/ov7670-no-ram-arduino-uno
 * and changed a little
 *
 * OV7670 NO FIFO
 * Arduino Mega 2560
 * 
 */

#include "camera.h"

// global variables
bool isCapturingImage = false;
uint8_t frameBuffer[40*120]; // 4800
Stream *debugStream = 0;

static inline void serialWrB(uint8_t dat) {
	while(!( UCSR0A & (1<<UDRE0))); // wait for byte to transmit
	UDR0=dat;
	while(!( UCSR0A & (1<<UDRE0))); // wait for byte to transmit
}

static void StringPgm(const char * str) {
	if (!pgm_read_byte_near(str))
		return;
	do{
		serialWrB(pgm_read_byte_near(str));
	}while(pgm_read_byte_near(++str));
}

void cameraDebug(char *msg) {
  if (debugStream == 0) {
    return;
  }
  debugStream->println(msg);
}

void cameraCaptureImage() {

    uint16_t columnNum = 160;
    uint16_t rowNum = 120;

    uint8_t *dest = frameBuffer;

    cameraDebug("[debug] capture begin");
    
    if (isCapturingImage)
    {
      return;
    }
    isCapturingImage = true;
    
    netShotBegin();
    
    cli();

    // wait for vsync
    while(!(PINA&_BV(4))); // wait for high
    while((PINA&_BV(4)));  // wait for low

    int i, j;
    j = rowNum;
    while(j--) {
        i = columnNum/4;
        while(i--) {
            while((PINA&_BV(6)));  //wait for low
            *dest = PINC >> 4;
            while(!(PINA&_BV(6))); //wait for high
            while((PINA&_BV(6)));  //wait for low
            // skip
            while(!(PINA&_BV(6))); //wait for high
            while((PINA&_BV(6)));  //wait for low
            // skip
            while(!(PINA&_BV(6))); //wait for high
            while((PINA&_BV(6)));  //wait for low
            // skip
            while(!(PINA&_BV(6))); //wait for high

            while((PINA&_BV(6)));  //wait for low
            *dest |= PINC & 0xF0;
            while(!(PINA&_BV(6))); //wait for high
            while((PINA&_BV(6)));  //wait for low
            // skip
            dest++;
            while(!(PINA&_BV(6))); //wait for high
            while((PINA&_BV(6)));  //wait for low
            // skip
            while(!(PINA&_BV(6))); //wait for high
            while((PINA&_BV(6)));  //wait for low
            // skip
            while(!(PINA&_BV(6))); //wait for high
        }
    }
    
    sei();
    
    uint8_t *src = frameBuffer;
    for (int i = 0; i < 4800; i+=80)
    {
        netShotPiece(0, i / 40, 160, 80, (char*)src);
        src += 80;
    }
    
    _delay_ms(10);
    
    netShotEnd();
    
    isCapturingImage = false;
    
    cameraDebug("[debug] capture end");
}

void cameraInit(Stream *dbgStream) {
  
  debugStream = dbgStream;

	// Setup the 8mhz PWM clock
	// OV7670 XCLK => (through level shift!) => pin 46 = PL3(OC5A), output
	DDRL |= _BV(3);
	TCCR5A = _BV(COM5A0) | _BV(WGM51) | _BV(WGM50);
	TCCR5B = _BV(WGM53) | _BV(WGM52) | _BV(CS50);
	OCR5A = 0; // (F_CPU)/(2*(X+1))
	// OV7670 VSYNC => pin 26 = PA4, input
	DDRA &= ~ _BV(4);
	// OV7670 PCLK => pin 28 = PA6, input
	DDRA &= ~ _BV(6);
	// OV7670 D7-D0 => pin 30-37 = PC7-PC0, input
	DDRC = 0;
    
	_delay_ms(3000);
    
	// set up twi for 100khz
	TWSR&=~3; // disable prescaler for TWI
	TWBR=72;  // set to 100khz

	camInit();

	setResolution(QQVGA);
	setColorSpace(YUV422);
	wrReg(REG_CLKRC,2);
}
