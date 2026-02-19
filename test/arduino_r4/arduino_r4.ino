ource on Github
/*
  CANWrite

  Write and send CAN Bus messages

  See the full documentation here:
  https://docs.arduino.cc/tutorials/uno-r4-wifi/can
*/

/**************************************************************************************
 * INCLUDE
 **************************************************************************************/

#include <Arduino_CAN.h>
#include <FspTimer.h>

/**************************************************************************************
 * CONSTANTS
 **************************************************************************************/

static uint32_t const CAN_ID = 0x20;

FspTimer gFspTimer;

#define LEDPIN 13

/**************************************************************************************
 * TIMER Callback
 **************************************************************************************/

void callbackfunc(timer_callback_args_t *arg)
{
  static bool led=false;

  if (led) {
    digitalWrite(LEDPIN, HIGH);
  }
  else {
    digitalWrite(LEDPIN, LOW);
  }

  led = !led;
}

/**************************************************************************************
 * SETUP/LOOP
 **************************************************************************************/

void setup()
{
  Serial.begin(115200);
  while (!Serial) { }

  if (!CAN.begin(CanBitRate::BR_250k))
  {
    Serial.println("CAN.begin(...) failed.");
    for (;;) {}
  }

  pinMode(LEDPIN, OUTPUT);
 
  uint8_t type = AGT_TIMER;
  
  int8_t ch = FspTimer::get_available_timer(type);
  if (ch < 0) {
    return;
  }

  gFspTimer.begin(TIMER_MODE_PERIODIC, type, ch, 1.0f, 50.0f, callbackfunc, nullptr);
  gFspTimer.setup_overflow_irq();
  gFspTimer.open();
  gFspTimer.start();
}

static uint32_t msg_cnt = 0;

void loop()
{
  /* Assemble a CAN message with the format of
   * 0xCA 0xFE 0x00 0x00 [4 byte message counter]
   */
  uint8_t const msg_data[] = {0xCA,0xFE,0,0,0,0,0,0};
  memcpy((void *)(msg_data + 4), &msg_cnt, sizeof(msg_cnt));
  CanMsg const msg(CanStandardId(CAN_ID), sizeof(msg_data), msg_data);

  /* Transmit the CAN message, capture and display an
   * error core in case of failure.
   */
  if (int const rc = CAN.write(msg); rc < 0)
  {
    Serial.print  ("CAN.write(...) failed with error code ");
    Serial.println(rc);
    for (;;) { }
  }

  /* Increase the message counter. */
  msg_cnt++;

  /* Only send one message per second. */
  delay(1000);
}