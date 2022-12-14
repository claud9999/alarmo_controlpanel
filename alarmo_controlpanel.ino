#include "Inkplate.h"
#include "alarmo_controlpanel.hpp"
#include "alarmo_cpsettings.hpp" /* put your wifi ssid, mqtt, etc. here */

#include "WiFi.h"
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"

Inkplate display(INKPLATE_1BIT);

void Rect::Paint(int border_sz) {
  for (int i = 0; i < border_sz; i++) {
    display.drawRect(this->x + i, this->y + i, this->width - i * 2, this->height - i * 2, 1);
  }
  display.fillRect(this->x + border_sz, this->y + border_sz, this->width - border_sz * 2, this->height - border_sz * 2, 0);
}

bool Rect::Inside(int x, int y) { return (x >= this->x && x < this->x + this->width && y >= this->y && y < this->y + this->height); }

void Button::Paint(void) {
  this->rect.Paint(2);

  display.setCursor(this->rect.x + 10, this->rect.y + 5);
  display.setTextSize(this->font_sz);
  display.print(this->text);
}

void Button::Press(void) {
  display.fillRect(this->rect.x, this->rect.y, this->rect.width, this->rect.height, 1);
  display.partialUpdate();
}

Button arm_button, arm_home_button, cancel_button, disarm_button;

WiFiClient client;

Adafruit_MQTT_Client mqtt(&client, MQTT_HOST, MQTT_PORT, MQTT_USERNAME, MQTT_KEY);
Adafruit_MQTT_Subscribe alarm_state = Adafruit_MQTT_Subscribe(&mqtt, "alarmo/state");
Adafruit_MQTT_Publish alarmo_command = Adafruit_MQTT_Publish(&mqtt, "alarmo/command");
Adafruit_MQTT_Publish mqtt_state_volt = Adafruit_MQTT_Publish(&mqtt, "inkplate/state/volt");
Adafruit_MQTT_Publish mqtt_state_temp = Adafruit_MQTT_Publish(&mqtt, "inkplate/state/temp");

uint32_t last_update = 0, last_mqtt_status = 0;
bool show = false, repaint = false, sleeping = false, frontlight = false, set_frontlight = false;
uint8_t *img = NULL;
int img_sz = E_INK_WIDTH * E_INK_HEIGHT * 4;

AlarmState arm_state = Alarm_Unknown, prev_arm_state = Alarm_Unknown;

void print(char *message) {
  display.print(message);
  Serial.print(message);
}

void println(char *message) {
  display.println(message);
  Serial.println(message);
}

void (*resetFn)(void) = 0;

void setup() {
  arm_button.text = "ARM"; arm_button.font_sz = 8; arm_button.rect.x = 0; arm_button.rect.y = 0; arm_button.rect.width = 500; arm_button.rect.height = 500;
  arm_home_button.text = "HOME"; arm_home_button.font_sz = 8; arm_home_button.rect.x = 520; arm_home_button.rect.y = 0; arm_home_button.rect.width = 500; arm_home_button.rect.height = 500;
  cancel_button.text = "CANCEL"; cancel_button.font_sz = 8; cancel_button.rect.x = 0; cancel_button.rect.y = 0; cancel_button.rect.width = 500; cancel_button.rect.height = 500;
  disarm_button.text = "DISARM"; disarm_button.font_sz = 8; disarm_button.rect.x = 0; disarm_button.rect.y = 0; disarm_button.rect.width = 500; disarm_button.rect.height = 500;

  Serial.begin(115200);
  display.begin();
  display.clearDisplay();
  display.setCursor(0, 100);
  display.setTextSize(3);
  println("starting..."); display.display();

  if(!display.tsInit(true)) { // init touchscreen
    println("unable to init touchscreen,");
    println("power cycle to try again.");
    display.display();
    while(1) delay(500000);
//    delay(5000); // one second
//    resetFn();
  }

  print("Connecting to WiFi"); display.partialUpdate();
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    print("."); display.partialUpdate();
  }
  println("connected!"); display.partialUpdate();

  println("subscribing to MQTT topic"); display.partialUpdate();
  mqtt.subscribe(&alarm_state);

  println("loading image"); display.partialUpdate();
  img = display.downloadFile("http://www.hotcat.org/media/IMG_8018.jpeg", &img_sz);

  repaint = false; show = true; frontlight = false;
  last_update = display.rtcGetEpoch();

  display.frontlight(true);
  println("setup done"); display.partialUpdate();
}

void loop() {
  uint32_t now = display.rtcGetEpoch();
  if (!mqtt.connected()) mqtt.connect();
  else {
    if (last_mqtt_status + 300 < now) {
      mqtt_state_volt.publish(display.readBattery());
      mqtt_state_temp.publish(display.readTemperature());
      last_mqtt_status = now;
    }
    Adafruit_MQTT_Subscribe *subscription;
    while(subscription = mqtt.readSubscription(10)) {
      Serial.println("mqtt");
      if (subscription == &alarm_state) {
        if (strcmp((const char *)alarm_state.lastread, "disarmed") == 0) {
          arm_state = Alarm_Disarmed;
        } else if (strcmp((const char *)alarm_state.lastread, "arming") == 0) {
          arm_state = Alarm_Arming;
        } else if (strcmp((const char *)alarm_state.lastread, "armed_away") == 0) {
          arm_state = Alarm_Armed;
        } else if (strcmp((const char *)alarm_state.lastread, "armed_home") == 0) {
          arm_state = Alarm_Home;
        }
  
        if(arm_state != prev_arm_state) {
          prev_arm_state = arm_state;
          last_update = now;
          repaint = true; show = true; set_frontlight = true;
        }
      }
    }

    uint16_t touchX[2], touchY[2];
    uint8_t touchCount = 0;
    
    if(display.tsAvailable()) {
      if(display.tsGetData(touchX, touchY)) {
        Serial.println("touch");
        last_update = now;
        sleeping = false;
        set_frontlight = true;
        if (!show) {
          show = true;
          repaint = true;
        } else {
          switch(arm_state) {
            case Alarm_Armed:
            case Alarm_Home:
              if (disarm_button.rect.Inside(touchX[0], touchY[0])) {
                disarm_button.Press();
                Serial.println("...disarm");
                alarmo_command.publish("DISARM");
              }
              break;
            case Alarm_Disarmed:
              if (arm_button.rect.Inside(touchX[0], touchY[0])) {
                arm_button.Press();
                Serial.println("...arm");
                alarmo_command.publish("ARM_AWAY");
              }
              if (arm_home_button.rect.Inside(touchX[0], touchY[0])) {
                arm_home_button.Press();
                Serial.println("...arm home");
                alarmo_command.publish("ARM_HOME");
              }
              break;
            case Alarm_Arming:
              if (cancel_button.rect.Inside(touchX[0], touchY[0])) {
                cancel_button.Press();
                Serial.println("...cancel");
                alarmo_command.publish("DISARM");
              }
              break;
          } /* end switch */
        } /* end if show */
      } /* end if tsGetData */
    } /* end if tsAvaiable() */
    if(now > last_update + IDLE_DELAY && !sleeping) {
      Serial.println("idle...");
      show = false;
      repaint = true;
      sleeping = true;
      set_frontlight = false;
    } /* end if last_update */
    if(now > last_update + DEEPSLEEP_DELAY) {
      Serial.println("deep sleep...");
      display.setFrontlight(0);
      display.frontlight(false);
      
      // Setup mcp interrupts
      display.setIntOutput(1, false, false, HIGH);
      display.setIntPin(PAD1, RISING);
      display.setIntPin(PAD2, RISING);
      display.setIntPin(PAD3, RISING);

      esp_sleep_enable_ext0_wakeup(GPIO_NUM_36, LOW);
      mqtt.disconnect();
      WiFi.disconnect();
      esp_deep_sleep_start();
    } /* end if deep sleep */
  } /* end if mqtt connected */

  if(set_frontlight && !frontlight) {
    display.setFrontlight(255);
    frontlight = true;
  } else if(!set_frontlight && frontlight) {
    display.setFrontlight(0);
    frontlight = false;
  }

  if(repaint) {
    Serial.println("drawing image");
    display.drawJpegFromBuffer(img, img_sz, 0, 0, true, false);
    Serial.println("image drawn");
    if(show) {
      Serial.println("drawing buttons");
      switch(arm_state) {
        case Alarm_Arming:
          cancel_button.Paint();
          break;
        case Alarm_Armed:
        case Alarm_Home:
          disarm_button.Paint();
          break;
        case Alarm_Disarmed:
          arm_button.Paint();
          arm_home_button.Paint();
          break;
      }
    }
    display.display();
    repaint = false;
  }
} /* end loop */
