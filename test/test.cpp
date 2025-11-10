#include <Control_Surface.h>
#include "Line6Fbv.h"

// USB MIDI interface (Control Surface)
USBMIDI_Interface midi; // необязательно для вызова Control_Surface methods, но нормально держать

// Используем безопасный тип Channel (не uint8_t)
constexpr Channel MIDI_CHANNEL = Channel_1; // Channel_1 .. Channel_16

// FBV объект
Line6Fbv fbv;

// Колбэки
void onFbvKeyPressed(byte key) {
  // создаём адрес безопасно через конструктор/brace-init непосредствено при инициализации:
  MIDIAddress program = MIDIAddress{MIDI_PC::Harpsichord, MIDI_CHANNEL};
  // отправляем Program Change через Control_Surface API:
  Control_Surface.sendProgramChange(program);
}

void onFbvKeyReleased(byte key, byte val) {
  (void)key; (void)val;
}

void onFbvCtrlChanged(byte pedal, byte value) {
  // используем существующий CC имя: Channel_Volume или Damper_Pedal и Channel тип
  MIDIAddress cc = MIDIAddress{MIDI_CC::Channel_Volume, MIDI_CHANNEL};
  // sendControlChange принимает MIDIAddress и uint8_t value
  Control_Surface.sendControlChange(cc, value);
}

void setup() {
  Control_Surface.begin();

  // Serial1 используется Line6Fbv.begin(&Serial1) — как у вас
  Serial1.begin(31250);
  fbv.begin(&Serial1);

  // Если вы не внесли патч в Line6Fbv (setDirectionPin), НЕ вызывайте этот метод.
  // fbv.setDirectionPin(RS485_DIR_PIN); // <- закомментировать, пока не патчнули библиотеку

  fbv.setHandleKeyPressed(&onFbvKeyPressed);
  fbv.setHandleKeyReleased(&onFbvKeyReleased);
  fbv.setHandleCtrlChanged(&onFbvCtrlChanged);
}

void loop() {
  fbv.read();
  Control_Surface.loop(); // обновляет интерфейсы и отправку MIDI
}
