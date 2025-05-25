
// Include-uri necesare
#include "IR_remote.h"
#include "Keymap.h"


#ifndef __AVR_ATmega32U4__
#include <avr/interrupt.h>


volatile irparams_t irparams;


bool MATCH(uint8_t measured_ticks, uint8_t desired_us) {
  return (measured_ticks >= desired_us - (desired_us >> 2) - 1 && measured_ticks <= desired_us + (desired_us >> 2) + 1);
}


ISR(TIMER_INTR_NAME) {
  uint8_t irdata = (uint8_t)digitalRead(irparams.recvpin);
  irparams.timer++;

  if (irparams.rawlen >= RAWBUF) {
    irparams.rcvstate = STATE_STOP;
  }

  switch (irparams.rcvstate) {
    case STATE_IDLE:
      if (irdata == MARK) {
        irparams.rawlen = 0;
        irparams.timer = 0;
        irparams.rcvstate = STATE_MARK;
      }
      break;

    case STATE_MARK:
      if (irdata == SPACE) {
        irparams.rawbuf[irparams.rawlen++] = irparams.timer;
        irparams.timer = 0;
        irparams.rcvstate = STATE_SPACE;
      }
      break;

    case STATE_SPACE:
      if (irdata == MARK) {
        irparams.rawbuf[irparams.rawlen++] = irparams.timer;
        irparams.timer = 0;
        irparams.rcvstate = STATE_MARK;
      } else if (irparams.timer > GAP_TICKS) {
        irparams.rcvstate = STATE_STOP;
        irparams.lastTime = millis();
      }
      break;

    case STATE_STOP:
      if (millis() - irparams.lastTime > 120) {
        irparams.rawlen = 0;
        irparams.timer = 0;
        irparams.rcvstate = STATE_IDLE;
      } else if (irdata == MARK) {
        irparams.timer = 0;
      }
      break;
  }
}


IRremote::IRremote(int pin) {
  pinMode(pin, INPUT);
  irparams.recvpin = pin;
  begin();
}


void IRremote::begin() {
  cli(); 
  TIMER_CONFIG_NORMAL();
  TIMER_ENABLE_INTR;
  sei(); 

  irparams.rawlen = 0;
  irparams.rcvstate = STATE_IDLE;
}

void IRremote::end() {
  EIMSK &= ~(1 << INT0);
}


ErrorStatus IRremote::decode() {
  rawbuf = irparams.rawbuf;
  rawlen = irparams.rawlen;

  if (irparams.rcvstate != STATE_STOP) {
    return ERROR;
  }

  if (decodeNEC()) {
    begin();
    return SUCCESS;
  }

  begin();
  return ERROR;
}


ErrorStatus IRremote::decodeNEC() {
  static unsigned long repeat_value = 0xFFFFFFFF;
  uint32_t data = 0;
  int offset = 0;

  if (!MATCH(rawbuf[offset], NEC_HDR_MARK / 50)) {
    return ERROR;
  }
  offset++;

  if (rawlen == 3 && MATCH(rawbuf[offset], NEC_RPT_SPACE / 50) && MATCH(rawbuf[offset + 1], NEC_BIT_MARK / 50)) {
    rawbuf[offset] = 0;
    rawbuf[offset + 1] = 0;
    value = repeat_value;
    decode_type = NEC;
    return SUCCESS;
  }

  if (rawlen < (2 * NEC_BITS + 3)) {
    return ERROR;
  }

  if (!MATCH(rawbuf[offset], NEC_HDR_SPACE / 50)) {
    return ERROR;
  }
  rawbuf[offset] = 0;
  offset++;

  for (int i = 0; i < NEC_BITS; i++) {
    if (!MATCH(rawbuf[offset], NEC_BIT_MARK / 50)) {
      return ERROR;
    }
    rawbuf[offset] = 0;
    offset++;

    if (MATCH(rawbuf[offset], NEC_ONE_SPACE / 50)) {
      data = (data >> 1) | 0x80000000;
    } else if (MATCH(rawbuf[offset], NEC_ZERO_SPACE / 50)) {
      data >>= 1;
    } else {
      return ERROR;
    }
    rawbuf[offset] = 0;
    offset++;
  }

  value = data;
  repeat_value = data;
  decode_type = NEC;
  return SUCCESS;
}


void IRremote::mark(uint16_t us) {
  TIMER_ENABLE_PWM;
  delayMicroseconds(us);
}

void IRremote::space(uint16_t us) {
  TIMER_DISABLE_PWM;
  delayMicroseconds(us);
}

void IRremote::enableIROut(uint8_t khz) {
  TIMER_DISABLE_INTR;
  TIMER_CONFIG_KHZ(khz);
}


void IRremote::enableIRIn() {
  cli();
  TIMER_CONFIG_NORMAL();
  TIMER_ENABLE_INTR;
  sei();

  irparams.rcvstate = STATE_IDLE;
  irparams.rawlen = 0;
  pinMode(irparams.recvpin, INPUT);
}


void IRremote::sendRaw(unsigned int buf[], int len, uint8_t hz) {
  enableIROut(hz);
  for (int i = 0; i < len; i++) {
    if (i % 2 == 0) {
      mark(buf[i]);
    } else {
      space(buf[i]);
    }
  }
  space(0);
}


String IRremote::getString() {
  if (decode()) {
    irRead = (value >> 16) & 0xFF;
    if (irRead == 0x0A || irRead == 0x0D) {
      irIndex = 0;
      irReady = true;
    } else {
      irBuffer += irRead;
      irIndex++;
    }
    irDelayTime = millis();
  } else {
    if (irRead > 0 && millis() - irDelayTime > 100) {
      irPressed = false;
      irRead = 0;
      irDelayTime = millis();
      Pre_Str = "";
    }
  }

  if (irReady) {
    irReady = false;
    Pre_Str = irBuffer;
    irBuffer = "";
    return Pre_Str;
  }

  return Pre_Str;
}


unsigned char IRremote::getCode() {
  irIndex = 0;
  loop();
  return irRead;
}


String IRremote::getKeyMap(byte keycode, byte ir_type) {
  ST_KEY_MAP *irkeymap = (ir_type == IR_TYPE_EM) ? em_ir_keymap : normal_ir_keymap;

  for (byte i = 0; i < KEY_MAX; i++) {
    if (irkeymap[i].keycode == keycode)
      return irkeymap[i].keyname;
  }

  return "";
}


byte IRremote::getIrKey(byte keycode, byte ir_type) {
  ST_KEY_MAP *irkeymap = (ir_type == IR_TYPE_EM) ? em_ir_keymap : normal_ir_keymap;

  for (byte i = 0; i < KEY_MAX; i++) {
    if (irkeymap[i].keycode == keycode)
      return i;
  }

  return 0xFF;
}


void IRremote::sendString(String s) {
  enableIROut(38);
  s.concat('\n');

  for (int i = 0; i < s.length(); i++) {
    unsigned long l = 0xFFFFFFFF;
    uint8_t data = s.charAt(i);
    l &= ~(uint32_t)data;
    l |= (uint32_t)data << 24;
    sendNEC(l, 32);
    delay(20);
  }

  enableIRIn();
}

void IRremote::sendString(float v) {
  dtostrf(v, 5, 8, floatString);
  sendString(floatString);
}

void IRremote::sendNEC(unsigned long data, int nbits) {
  enableIROut(38);
  mark(NEC_HDR_MARK);
  space(NEC_HDR_SPACE);

  for (int i = 0; i < nbits; i++) {
    mark(NEC_BIT_MARK);
    if (data & 1) {
      space(NEC_ONE_SPACE);
    } else {
      space(NEC_ZERO_SPACE);
    }
    data >>= 1;
  }

  mark(NEC_BIT_MARK);
  space(0);
}

void IRremote::loop() {
  if (decode()) {
    irRead = (value >> 16) & 0xFF;
    irPressed = true;
    if (irRead == 0x0A || irRead == 0x0D) {
      irIndex = 0;
      irReady = true;
    } else {
      irBuffer += irRead;
      irIndex++;
      if (irIndex > 64) {
        irIndex = 0;
        irBuffer = "";
      }
    }
    irDelayTime = millis();
  } else {
    if (irRead > 0 && millis() - irDelayTime > 100) {
      irPressed = false;
      irRead = 0;
      irDelayTime = millis();
    }
  }
}

boolean IRremote::keyPressed(unsigned char r) {
  irIndex = 0;
  loop();
  return irRead == r;
}

#endif // !defined(__AVR_ATmega32U4__)
