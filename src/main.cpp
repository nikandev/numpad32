#include <Arduino.h>
#include <BleKeyboard.h>
#include <Button2.h>
#include <Keypad.h>

#define LED_PIN 2
#define MODE_BUTTON_PIN 33

#define ROW_1_PIN 14
#define ROW_2_PIN 32
#define ROW_3_PIN 5
#define ROW_4_PIN 21

#define COLUMN_1_PIN 27
#define COLUMN_2_PIN 19
#define COLUMN_3_PIN 25
#define COLUMN_4_PIN 26
#define COLUMN_5_PIN 4

const byte ROWS = 4;
const byte COLS = 5;

char numKeys[ROWS][COLS] =
    {
        {KEY_F21, KEY_NUM_7, KEY_NUM_4, KEY_NUM_1, KEY_NUM_0},
        {KEY_F22, KEY_NUM_SLASH, KEY_NUM_8, KEY_NUM_5, KEY_NUM_2},
        {KEY_NUM_9, KEY_NUM_6, KEY_NUM_3, KEY_NUM_PERIOD, KEY_NUM_ENTER},
        {KEY_NUM_ASTERISK, KEY_F23, KEY_F24, KEY_NUM_MINUS, KEY_NUM_PLUS}
    };

char lockedKeys[ROWS][COLS] =
    {
        {KEY_F21, KEY_HOME, KEY_LEFT_ARROW, KEY_END, KEY_INSERT},
        {KEY_F22, KEY_NUM_SLASH, KEY_UP_ARROW, KEY_NUM_5, KEY_DOWN_ARROW},
        {KEY_PAGE_UP, KEY_RIGHT_ARROW, KEY_PAGE_DOWN, KEY_DELETE, KEY_NUM_ENTER},
        {KEY_NUM_ASTERISK, KEY_F23, KEY_F24, KEY_NUM_MINUS, KEY_NUM_PLUS}
    };

byte rowPins[ROWS] = {ROW_1_PIN, ROW_2_PIN, ROW_3_PIN, ROW_4_PIN};
byte colPins[COLS] = {COLUMN_1_PIN, COLUMN_2_PIN, COLUMN_3_PIN, COLUMN_4_PIN, COLUMN_5_PIN};

Keypad kpdNum = Keypad(makeKeymap(numKeys), rowPins, colPins, ROWS, COLS);
Keypad kpdLock = Keypad(makeKeymap(lockedKeys), rowPins, colPins, ROWS, COLS);

Keypad* kpd;

BleKeyboard bleKeyboard;

Button2 button;

bool numlocked = false;

void modeChanged(Button2& btn)
{
  numlocked = !numlocked;
}

void setup()
{
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  bleKeyboard.begin();

  kpdNum.setDebounceTime(30);
  kpdNum.setHoldTime(500);
  kpdLock.setDebounceTime(30);
  kpdLock.setHoldTime(500);

  kpd = &kpdNum;

  button.begin(MODE_BUTTON_PIN, INPUT_PULLUP);
  button.setTapHandler(modeChanged);
}

void loop()
{
  if (!bleKeyboard.isConnected())
  {
    return;
  }

  button.loop();

  if (numlocked)
  {
    digitalWrite(LED_PIN, HIGH);
    kpd = &kpdLock;
  }
  else
  {
    digitalWrite(LED_PIN, LOW);
    kpd = &kpdNum;
  }

  if (kpd->getKeys())
  {
    for (int i = 0; i < LIST_MAX; i++)
    {
      Key k = kpd->key[i];

      if (k.stateChanged)
      {
        uint8_t bleKey = (uint8_t)k.kchar;

        switch (k.kstate)
        {
          case KeyState::PRESSED:
          {
            bleKeyboard.write(bleKey);
            break;
          }
          case KeyState::HOLD:
          {
            bleKeyboard.press(bleKey);
            break;
          }
          case KeyState::RELEASED:
          {
            bleKeyboard.release(bleKey);
            break;
          }
          case KeyState::IDLE:
          {
            break;
          }
        }
      }
    }
  }
}