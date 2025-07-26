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

// Applies bit-order mapping and bit offset
uint8_t mapDataByte(uint8_t data, const uint8_t* bit_order, uint8_t data_offset) {
  uint8_t result = 0;
  for (uint8_t i = 0; i < 8; i++) {
    if (data & (1 << i)) {
      result |= (1 << (bit_order[i] + data_offset));
    }
  }
  return result;
}

// Send byte to LCD using mapping
void lcdSendMapped(uint8_t val, bool isData, const uint8_t* bit_order, uint8_t data_offset, uint8_t rs_bit, uint8_t en_bit) {
  uint8_t out = mapDataByte(val, bit_order, data_offset);

  if (isData) out |= (1 << rs_bit);
  out |= (1 << en_bit);
  shiftOutByte(out);

  // Falling edge on E
  out &= ~(1 << en_bit);
  shiftOutByte(out);
}

void lcdInitMapped(const uint8_t* bit_order, uint8_t data_offset, uint8_t rs_bit, uint8_t en_bit) {
  delay(50);
  lcdSendMapped(0x38, false, bit_order, data_offset, rs_bit, en_bit);  // Function set
  delay(5);
  lcdSendMapped(0x0C, false, bit_order, data_offset, rs_bit, en_bit);  // Display on
  delay(5);
  lcdSendMapped(0x06, false, bit_order, data_offset, rs_bit, en_bit);  // Entry mode
  delay(5);
  lcdSendMapped(0x01, false, bit_order, data_offset, rs_bit, en_bit);  // Clear
  delay(5);
}

void lcdPrintTest(const char* msg, const uint8_t* bit_order, uint8_t data_offset, uint8_t rs_bit, uint8_t en_bit) {
  while (*msg) {
    lcdSendMapped(*msg++, true, bit_order, data_offset, rs_bit, en_bit);
    delay(2);
  }
}

// Possible bit permutations (identity and reversed for now)
const uint8_t identity_order[8] = {0,1,2,3,4,5,6,7};
const uint8_t reverse_order[8]  = {7,6,5,4,3,2,1,0};

const uint8_t* bit_orders[] = {
  identity_order,
  reverse_order,
};

void setup() {
  pinMode(PIN_CLK, OUTPUT);
  pinMode(PIN_DAT, OUTPUT);
  pinMode(PIN_LATCH, OUTPUT);
  digitalWrite(PIN_CLK, LOW);
  digitalWrite(PIN_DAT, LOW);
  digitalWrite(PIN_LATCH, LOW);
}

void loop() {
  for (uint8_t rs = 0; rs < 8; rs++) {
    for (uint8_t en = 0; en < 8; en++) {
      if (rs == en) continue;

      for (uint8_t offset = 0; offset <= 4; offset++) {  // bit offset (max 4 to keep within 8-bit)
        for (uint8_t o = 0; o < sizeof(bit_orders)/sizeof(bit_orders[0]); o++) {
          const uint8_t* order = bit_orders[o];

          lcdInitMapped(order, offset, rs, en);

          char buf[32];
          sprintf(buf, "RS%d E%d O%d", rs, en, offset);
          lcdPrintTest(buf, order, offset, rs, en);
          delay(3000);
        }
      }
    }
  }
}
