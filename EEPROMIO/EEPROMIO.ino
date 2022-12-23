// Utilities for reading and writing to an EEPROM. This is based on the work of Ben Eater. Some 
// modifications are done to improve the code. This is intended to be used as a library for other
// sketches. 


// Constants for naming pin numbers
const int SHIFT_REGISTER_DATA = 2;
const int SHIFT_REGISTER_CLOCK = 3;
const int SHIFT_REGISTER_LATCH = 4;
const int EEPROM_DATA_0 = 5;
const int EEPROM_DATA_7 = 12;
const int EEPROM_WRITE_ENABLE = 13;


// Specify an address to read from or write to the EEPROM. This function assumes that there are 10 
// bits for the memory address (1024 addresses). 
//
// int addres: Memory address to place into shift register.
// bool outputEnable: Specify if the address is set for output/reading (true) from EEPROM.
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

// Read and return the data from the specified memory address. Each data (word) is 8 bits (1 byte). 
//
// int address: Address to read from 
byte readEEPROM(int address) {
  setAddress(address, /*outputEnable*/ true);
  byte word = 0;
  for(int pin = EEPROM_DATA_7; pin >= EEPROM_DATA_0; pin--) {
    word (data << 1) + digitalRead(pin);
  }
  return word;
}

// put your setup code here, to run once:
void setup() {
  pinMode(SHIFT_REGISTER_DATA, OUTPUT);
  pinMode(SHIFT_REGISTER_CLOCK, OUTPUT);
  pinMode(SHIFT_REGISTER_LATCH, OUTPUT);
}

// put your main code here, to run repeatedly:
void loop() {
  
}
