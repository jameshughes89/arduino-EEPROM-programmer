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
    setAddress(address, /*outputEnable*/ false);
    for (int pin = EEPROM_DATA_0; pin <= EEPROM_DATA_7; pin++) {
      // Move bit to write to least significant spot and keep only the least significant bit
      int writeBit = (word >> pin) & 1;   
      digitalWrite(pin, writeBit);
    }
    digitalWrite(EEPROM_WRITE_ENABLE, HIGH);
    digitalWrite(EEPROM_WRITE_ENABLE, LOW);
    delayMicroseconds(1);
    digitalWrite(EEPROM_WRITE_ENABLE, HIGH);
}

// Read and return the data from the specified memory address. Each data (word) is 8 bits (1 byte). 
//
// int address:   Address to read from.
// return:        Data (word) read from the specified EEPROM memory address.
byte readEepromAddress(int address) {
  setAddress(address, /*outputEnable*/ true);
  byte word = 0;
  for (int pin = EEPROM_DATA_7; pin >= EEPROM_DATA_0; pin--) {
    word = (word << 1) + digitalRead(pin);
  }
  return word;
}


// Read the contents of the EEPROM up to the specified address. The contents of the EEPROM are 
// written to the serial output. This assumes that pins TX & RX are not used since the serial
// interface requires them to be open. This assumes the addresses are a multiple of 16. If the 
// specified address is not a multiple of 16 it is rounded down to the nearest multiple of 16.
//
// int addressFrom:   Address to start reading from. If it is not a multiple of 16 it is rounded
//                    down to the nearest multiple of 16. 
// int addressTo:     Address to read up to. If it is not a multiple of 16 it is rounded down to
//                    the nearest multiple of 16. 
void readEepromSerial(int addressFrom, int addressTo) {
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

// put your setup code here, to run once:
void setup() {
  pinMode(SHIFT_REGISTER_DATA, OUTPUT);
  pinMode(SHIFT_REGISTER_CLOCK, OUTPUT);
  pinMode(SHIFT_REGISTER_LATCH, OUTPUT);

  readEepromSerial(0, 256); 
}

// put your main code here, to run repeatedly:
void loop() {
  
}
