#include <Arduino.h>
#include "Line6Fbv.h"
#include <Control_Surface.h>
USBMIDI_Interface usbmidi;

Line6Fbv fbv;               // створюємо об’єкт бібліотеки
bool buttonState[20] = {0}; // поточний стан кожної кнопки

int16_t increment_preset = 0;
int8_t increment_snapshot = 0;

#define TUNER_HOLD 3000
uint32_t timing_tuner = 0;
uint8_t tap_flag = 0;

void clean_all_led()
{
  for (int i = 0; i < 21; i++)
  {
    fbv.setLedOnOff(i, 0);
    buttonState[i] = 0;
  }
  fbv.updateUI();
}

void display_info(uint8_t mode, uint8_t value = 0)
{
  switch (mode)
  {
  case 0: // Тюнер виключений
    fbv.setLedOnOff(11, 0);
    fbv.setDisplayTitle("PLAY");
    break;
  case 1: // Тюнер включений
    fbv.setLedOnOff(11, 1);
    fbv.setDisplayTitle("TUNER");
    break;
  case 2:
    fbv.setDisplayTitle("Effect:");
    break;
  }
  fbv.updateUI();
}

void timing_tick()
{
  if (tap_flag == 1 && (millis() - timing_tuner) > TUNER_HOLD) // Вмикаємо тюнер
  {
    Serial.println("Тюнер увімкнено");
    tap_flag = 2;
    display_info(1);
    usbmidi.sendControlChange({68, Channel_1}, 127);
    timing_tuner = millis();
  }
}

// ==== CALLBACK-и ====
void onKeyPressed(byte key)
{
  Serial.print(F("Натиснуто: "));
  Serial.println(key);
  buttonState[key] = !buttonState[key]; // зберігаємо стан кнопки

  switch (key)
  {
  case 0: // WAH
    usbmidi.sendControlChange({58, Channel_1}, buttonState[0] ? 127 : 0);
    fbv.setLedOnOff(20, buttonState[0] ? 1 : 0);
    fbv.updateUI();
    break;
  case 1: // COMP
    usbmidi.sendControlChange({49, Channel_1}, buttonState[1] ? 127 : 0);
    fbv.setLedOnOff(1, buttonState[1] ? 1 : 0);
    fbv.updateUI();
    break;
  case 2: // STOMP
    usbmidi.sendControlChange({50, Channel_1}, buttonState[2] ? 127 : 0);
    fbv.setLedOnOff(2, buttonState[2] ? 1 : 0);
    fbv.updateUI();
    break;
  case 7: // REVERB
    usbmidi.sendControlChange({53, Channel_1}, buttonState[7] ? 127 : 0);
    fbv.setLedOnOff(7, buttonState[7] ? 1 : 0);
    fbv.updateUI();
    break;
  case 9: // MOD
    usbmidi.sendControlChange({51, Channel_1}, buttonState[9] ? 127 : 0);
    fbv.setLedOnOff(9, buttonState[9] ? 1 : 0);
    fbv.updateUI();
    break;
  case 10: // DELAY
    usbmidi.sendControlChange({52, Channel_1}, buttonState[10] ? 127 : 0);
    fbv.setLedOnOff(10, buttonState[10] ? 1 : 0);
    fbv.updateUI();
    break;

  case 11: // TAP
    if (tap_flag == 0)
    {
      tap_flag = 1;
      timing_tuner = millis();
    }
    else if (tap_flag == 2)
    {
      Serial.println("Вимкнув Тюнер");
      tap_flag = 0;
      display_info(0);
      usbmidi.sendControlChange({68, Channel_1}, 0);
    }
    break;

  case 12: // BANK UP
    increment_snapshot++;
    if (increment_snapshot >= 7)
      increment_snapshot = 7;
    usbmidi.sendControlChange({69, Channel_1}, increment_snapshot);
    //usbmidi.sendProgramChange(Channel_1, 20);
    clean_all_led();
    break;
  case 13: // BANK DOWN
    increment_snapshot--;
    if (increment_snapshot <= 0)
      increment_snapshot = 0;
     usbmidi.sendControlChange({69, Channel_1}, increment_snapshot);
    //usbmidi.sendProgramChange(Channel_1, 18);
    clean_all_led();
    break;

  case 14: // CHANNEL A
    usbmidi.sendControlChange({54, Channel_1}, buttonState[14] ? 127 : 0);
    fbv.setLedOnOff(14, buttonState[14] ? 1 : 0);
    fbv.updateUI();
    break;
  case 15: // CHANNEL B
    usbmidi.sendControlChange({55, Channel_1}, buttonState[15] ? 127 : 0);
    fbv.setLedOnOff(15, buttonState[15] ? 1 : 0);
    fbv.updateUI();
    break;
  case 16: // CHANNEL C
    usbmidi.sendControlChange({56, Channel_1}, buttonState[16] ? 127 : 0);
    fbv.setLedOnOff(16, buttonState[16] ? 1 : 0);
    fbv.updateUI();
    break;
  case 17: // CHANNEL D
    usbmidi.sendControlChange({57, Channel_1}, buttonState[17] ? 127 : 0);
    fbv.setLedOnOff(17, buttonState[17] ? 1 : 0);
    fbv.updateUI();
    break;

  default:
    Serial.print(F("Натиснуто щось інше: "));
    Serial.println(key);
    break;
  }
}

void onKeyReleased(byte key, byte state)
{
  Serial.print(F("Відпущено: "));
  Serial.print(key);
  Serial.print(F("  Стан="));
  Serial.println(state);

  switch (key)
  {
  case 11: // TAP
    if (tap_flag == 1 && (millis() - timing_tuner) < TUNER_HOLD)
    {
      Serial.print("Відпустив рано Тюнер");
      tap_flag = 0;
    }
    break;
  }
  // usbmidi.sendControlChange({key, Channel_1}, state);
}

void onCtrlChanged(byte ctrl, byte value)
{
  // обмежуємо нижній поріг
  if (value < 23)
    value = 20;

  // мапування педалі (20–127 → 0–127)
  byte cc_value = map(value, 20, 127, 0, 127);

  if (ctrl == LINE6FBV_CC_PDL1)
  {
    usbmidi.sendControlChange({60, Channel_1}, cc_value);
  }
}

void onHeartbeat()
{
  Serial.println(F("[Heartbeat] Зв’язок з педаллю активний"));
  fbv.setLedOnOff(19, 1);
  fbv.updateUI();
}

void onDisconnected()
{
  Serial.println(F("[FBV] Зв’язок втрачено"));
  fbv.setLedOnOff(19, 0);
  fbv.updateUI();
}

// ==== SETUP ====
void setup()
{
  delay(1500);
  Serial.begin(115200); // USB для монітора
  Serial1.begin(31250); // швидкість FBV (MIDI)
  Control_Surface.begin();
  fbv.begin(&Serial1); // передаємо в бібліотеку Serial1

  fbv.setHandleKeyPressed(&onKeyPressed);
  fbv.setHandleKeyReleased(&onKeyReleased);
  fbv.setHandleCtrlChanged(&onCtrlChanged);
  fbv.setHandleHeartbeat(&onHeartbeat);
  fbv.setHandleDisconnected(&onDisconnected);

  delay(200);
  fbv.setDisplayTitle("Okay let's GO");
  fbv.updateUI();
  clean_all_led();
}

// ==== LOOP ====
void loop()
{
  Control_Surface.loop(); // якщо ти використовуєш бібліотеку Control Surface
  fbv.read();             // обробляємо події від педалборду
  timing_tick();
  /*
    // якщо в серійному моніторі введено байт — засвітити відповідний LED
    if (Serial.available() > 0)
    {
      int led = Serial.parseInt(); // читає число, введене в моніторі
      if (led > 0 && led <= 0x60)
      { // перевірка, щоб не було сміття
        Serial.print("Увімкнено LED із кодом: 0x");
        Serial.println(led, HEX);

        fbv.setLedOnOff(led, 0);
        fbv.updateUI();
      }
      else
      {
        Serial.println("Невірний код LED!");
      }

      // очистити буфер після натискання Enter
      while (Serial.available())
        Serial.read();
    }*/
}
