#define PIN_CLK   2
#define PIN_DAT   3
#define PIN_LATCH 4

void pulse(uint8_t pin) {
  digitalWrite(pin, HIGH);
  delayMicroseconds(5);
  digitalWrite(pin, LOW);
  delayMicroseconds(5);
}

void shiftOutByte(uint8_t val) {
  for (int i = 7; i >= 0; i--) {
    digitalWrite(PIN_DAT, (val >> i) & 1);
    pulse(PIN_CLK);
  }
  pulse(PIN_LATCH);
}

// Try sending a command using guessed control/data bit mapping
void lcdSendGuess(uint8_t data, uint8_t rs_bit, uint8_t en_bit) {
  uint8_t command = data;

  // Add RS and E flags
  if (rs_bit < 8) command |= (1 << rs_bit);
  if (en_bit < 8) command |= (1 << en_bit);
  shiftOutByte(command);

  // Pulse E off
  command &= ~(1 << en_bit);
  shiftOutByte(command);
}

void lcdInitGuess(uint8_t rs_bit, uint8_t en_bit) {
  delay(50); // Wait for LCD power-up

  // HD44780 8-bit initialization sequence
  lcdSendGuess(0x38, rs_bit, en_bit);  // Function set: 8-bit, 2 lines
  delay(5);
  lcdSendGuess(0x0C, rs_bit, en_bit);  // Display on, cursor off
  delay(5);
  lcdSendGuess(0x06, rs_bit, en_bit);  // Entry mode: increment
  delay(5);
  lcdSendGuess(0x01, rs_bit, en_bit);  // Clear display
  delay(5);
}

// Print message to LCD
void lcdPrintGuess(const char* msg, uint8_t rs_bit, uint8_t en_bit) {
  while (*msg) {
    lcdSendGuess(*msg++, rs_bit, en_bit);
    delay(2);
  }
}

void setup() {
  pinMode(PIN_CLK, OUTPUT);
  pinMode(PIN_DAT, OUTPUT);
  pinMode(PIN_LATCH, OUTPUT);

  digitalWrite(PIN_CLK, LOW);
  digitalWrite(PIN_DAT, LOW);
  digitalWrite(PIN_LATCH, LOW);
}

void loop() {
  // Brute-force RS and E pin mapping combinations
  for (uint8_t rs = 0; rs < 8; rs++) {
    for (uint8_t en = 0; en < 8; en++) {
      if (rs == en) continue;

      lcdInitGuess(rs, en);
      lcdPrintGuess("Testing RS", rs, en);
      lcdSendGuess('0' + rs, rs, en);
      lcdSendGuess(' ', rs, en);
      lcdSendGuess('E', rs, en);
      lcdSendGuess('=', rs, en);
      lcdSendGuess('0' + en, rs, en);

      delay(3000);  // Observe
    }
  }
}
