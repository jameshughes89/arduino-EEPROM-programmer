// Utilities for reading and writing to an EEPROM. This is based on the work of Ben Eater. Some 
// modifications are done to improve the code. This ~~is~~ was intended to be used as a library 
// for other sketches, but arduino is weird, and it takes more effort than I am willing to put in
// to figure out how to do it that way. Thus, this sketch will just contain everything and the 
// contents of the setup and loop will change depending on what is needed at the time.


// Constants for naming pin numbers
const int SHIFT_REGISTER_DATA = 2;
const int SHIFT_REGISTER_CLOCK = 3;
const int SHIFT_REGISTER_LATCH = 4;
const int EEPROM_DATA_0 = 5;
const int EEPROM_DATA_7 = 12;
const int EEPROM_WRITE_ENABLE = 13;


// Constants for the 7-segment display. The display is laid out as follows:
//
//              a
//              -
//            f|g|b
//              - 
//            e| |c
//              -  
//              d
//
// labelling goes clockwise, starting at twelve o'clock, with the centre being the last value. 
// There is a decimal point, but that will not be used. Based on the physical layout of the EEPROM
// programmer, the binary representation is 0abcdefg. The most significant bit of the byte is 
// always 0 since the decimal is not used. 
const int ZERO      = 0b01111110;
const int ONE       = 0b00110000;
const int TWO       = 0b01101101;
const int THREE     = 0b01111001;
const int FOUR      = 0b00110011;
const int FIVE      = 0b01011011;
const int SIX       = 0b01011111;
const int SEVEN     = 0b01110000;
const int EIGHT     = 0b01111111;
const int NINE      = 0b01111011;
const int TEN       = 0b01110111; // A
const int ELEVEN    = 0b00011111; // b
const int TWELVE    = 0b01001110; // C
const int THIRTEEN  = 0b00111101; // d
const int FOURTEEN  = 0b01001111; // E
const int FIFTEEN   = 0b01000111; // F
const int NEGATIVE  = 0b00000001; // -
const int DIGITS[16] = {ZERO, ONE, TWO, THREE, FOUR, FIVE, SIX, SEVEN, EIGHT, NINE, TEN, ELEVEN, TWELVE, THIRTEEN, FOURTEEN, FIFTEEN};



// Specify an address to read from or write to the EEPROM. This function assumes that there are 10 
// bits for the memory address (1024 addresses). 
//
// int addres:          Memory address to place into shift register.
// bool outputEnable:   Specify if the address is set for output/reading (true) from EEPROM.
void setAddress(int address, bool outputEnable) {
  // There are 16 bits on shift register, but only 10 are needed for specifying the memory address,
  // thus, there are 6 bits free. The most significant bit of those 6 are used for toggling ~OE.
  byte highTwoBitsAndOE = address >> 8;   // Shift removes all bits but the 2 most significant
  byte lowEightBits = address;            // Byte truncates beyond the least significant 8 bits
  
  // Active low, so if false, set highest bit to 1
  if (!outputEnable){
    highTwoBitsAndOE = highTwoBitsAndOE | 0b10000000;
  } 

  // Put the 10 bit memory address into the shift register
  shiftOut(SHIFT_REGISTER_DATA, SHIFT_REGISTER_CLOCK, MSBFIRST, highTwoBitsAndOE);
  shiftOut(SHIFT_REGISTER_DATA, SHIFT_REGISTER_CLOCK, MSBFIRST, lowEightBits);

  // Pulse latch
  digitalWrite(SHIFT_REGISTER_LATCH, LOW);
  digitalWrite(SHIFT_REGISTER_LATCH, HIGH);
  digitalWrite(SHIFT_REGISTER_LATCH, LOW);
}

// Write a single 8 bit (1 byte) data (word) to the EEPROM at the specified address. 
//
// int address:   Memory address to write to. 
// int word:      Data (word) to write to the specified EEPROM memory address. 
void writeEepromAddress(int address, byte word) {
// Set pins to write to EEPROM  
  for (int pin = EEPROM_DATA_0; pin <= EEPROM_DATA_7; pin++) {
    pinMode(pin, OUTPUT);
  }

  // Set address and write data EEPROM
  setAddress(address, /*outputEnable*/ false);
  for (int pin = EEPROM_DATA_0; pin <= EEPROM_DATA_7; pin++) {
    digitalWrite(pin, word & 1);    // Mastk the single bit we want (least significant)
    word = word >> 1;               // Shift the next bit to be output to be the least significant bit
  }

  // Pulse write enable to latch data into EEPROM
  digitalWrite(EEPROM_WRITE_ENABLE, HIGH);
  digitalWrite(EEPROM_WRITE_ENABLE, LOW);
  delayMicroseconds(10);
  digitalWrite(EEPROM_WRITE_ENABLE, HIGH);
  delay(5);
}

// Read and return the data from the specified memory address. Each data (word) is 8 bits (1 byte). 
//
// int address:   Address to read from.
// return:        Data (word) read from the specified EEPROM memory address.
byte readEepromAddress(int address) {
  // Set pins to read from EEPROM  
  for (int pin = EEPROM_DATA_0; pin <= EEPROM_DATA_7; pin++) {
    pinMode(pin, INPUT);
  }
  
  // Set address and read data from EEPROM
  setAddress(address, /*outputEnable*/ true);
  byte word = 0;
  for (int pin = EEPROM_DATA_7; pin >= EEPROM_DATA_0; pin--) {
    word = (word << 1) + digitalRead(pin);
  }
  return word;
}


// Read the contents of the EEPROM within the specified address range. The contents of the EEPROM 
// are written to the serial output. This assumes that pins TX & RX are not used since the serial
// interface requires them to be open. This assumes the addresses are a multiple of 16. If the 
// specified address is not a multiple of 16 it is rounded down to the nearest multiple of 16.
//
// int addressFrom:   Address to start reading from. If it is not a multiple of 16 it is rounded
//                    down to the nearest multiple of 16. 
// int addressTo:     Address to read up to. If it is not a multiple of 16 it is rounded down to
//                    the nearest multiple of 16. 
void readEepromRangeSerial(int addressFrom, int addressTo) {
  Serial.begin(57600);
  // Loop from/to the rounded down version of the specified addresses
  for (int base = addressFrom - (addressFrom % 16); base < addressTo - (addressTo % 16); base += 16) {
    byte words[16];
    for (int offset = 0; offset < 16; offset++) {
      words[offset] = readEepromAddress(base + offset);
    }
    char outputBuffer[80];
    sprintf(outputBuffer, 
      "%03x:\t%02x %02x %02x %02x  %02x %02x %02x %02x    %02x %02x %02x %02x  %02x %02x %02x %02x",
      base, 
      words[0], words[1], words[2], words[3], words[4], words[5], words[6], words[7], 
      words[8], words[9], words[10], words[11], words[12], words[13], words[14], words[15]);
      Serial.println(outputBuffer);
  }
  Serial.end();
}


// Write the seven segment display patterns to the EEPROM. The EEPROM is serving as a map. The 
// key is a combination of the binary value to be displayed and the digit of the value to be 
// displayed. The value is the seven segment pattern for the specific digit. 
//
// The key is the memory address of the value. The EEPROM this is designed for has eleven address 
// lines. The 11th (A in the below example) is not used here. The 2 most significant used bits 
// represent which digit of the number to display and the 8 least significant bits is the binary 
// representation of the number to be displayed. 
//
//                  A|9 8|7 6 5 4 3 2 1 0
//                  X|dig|    number
//
// The seven segment display is configured such that only one digit is diaplayed at a time, but at
// a high enough frequency, it appears as if all digits are displayed. This means if address lines
// 0 -- 7 correspond to a given number, for example, 123 (0b01111011), the address lines 8 & 9 are 
// set to correspond to specific digits (1, 2, 3, or 4) at a high frequency. 
//
// Example: 123 (0b01111011)
//
//                  0|00|01111011 -> THREE      0b01111001
//                  0|01|01111011 -> TWO        0b00110000
//                  0|10|01111011 -> ONE        0b00110000
//                  0|11|01111011 -> nothing    0b00000000
//
//
// This function also writes the two's complement version of the numbers (- 0.5*maxNumber) -- 
// 0.5*maxNumber (exclusively). The most significant bit in the address line (A in the above 
// example) is high for the two's complement numbers. Thus, the most signficant bit acts as a 
// toggle for the mode --- off for unsigned integers, on for signed two's complement numbers. 
// Note that the values for 2s complement are not quite in sequential order; 0 -- 0.5*maxNumber 
// (exclusive) will be first followd by - 0.5*maxNumber -- -1. In other words, it loops to the 
// negative values after all the positive numbers in the two's complement representation. The
// below example with the binary representation of a negative two's complement number demonstrates
// this, but for another example, for the 8 least significant bits on the address line, consider 
// that 0b01111111 is the last positive integer in 8 bits. This means addresses 0b00000000 -- 
// 0b01111111 are the positives, but once 0b10000000 is hit, which is the subsequent number after 
// 0b011111111, it jumps to -128, which is followed by -127 (0b10000001), and so on. 
//
// Example: -123 (0b10000101)
//
//                  1|00|10000101 -> THREE      0b01111001
//                  1|01|10000101 -> TWO        0b00110000
//                  1|10|10000101 -> ONE        0b00110000
//                  1|11|10000101 -> NEGATIVE   0b00000001
//
//
// int maxNumber: The max number (not inclusive) to write to the EEPROM. For an 8 bit number, this
//                is typically set to 256 (0 -- 255). 
void writeEepromSevenSegmentDigits(int maxNumber) {
  int digitIndex;
  // Write ones digit 
  for (int value = 0; value < maxNumber; value++){
    digitIndex = (value / 1) % 10;
    writeEepromAddress(maxNumber * 0 + value, DIGITS[digitIndex]);
  }
  // Write tens digit 
  for (int value = 0; value < maxNumber; value++){
    digitIndex = (value / 10) % 10;
    writeEepromAddress(maxNumber * 1 + value, DIGITS[digitIndex]);
  }
  // Write hundreds digit 
  for (int value = 0; value < maxNumber; value++){
    digitIndex = (value / 100) % 10;
    writeEepromAddress(maxNumber * 2 + value, DIGITS[digitIndex]);
  }
  // Write sign (nothing since positive)
  for (int value = 0; value < maxNumber; value++){
    writeEepromAddress(maxNumber * 3 + value, 0b00000000);
  }
}

// put your setup code here, to run once:
void setup() {
  pinMode(SHIFT_REGISTER_DATA, OUTPUT);
  pinMode(SHIFT_REGISTER_CLOCK, OUTPUT);
  pinMode(SHIFT_REGISTER_LATCH, OUTPUT);
  digitalWrite(EEPROM_WRITE_ENABLE, HIGH);
  pinMode(EEPROM_WRITE_ENABLE, OUTPUT);
}

// put your main code here, to run repeatedly:
void loop() {
  
}
