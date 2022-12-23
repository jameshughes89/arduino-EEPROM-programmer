const int SHIFT_REGISTER_DATA = 2;
const int SHIFT_REGISTER_CLOCK = 3;
const int SHIFT_REGISTER_LATCH = 4;

// Specify an address to read to or write from the EEPROM. 
void setAddress(int address) {
  
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
