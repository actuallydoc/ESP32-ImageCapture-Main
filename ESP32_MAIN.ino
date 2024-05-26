#include "Config.h";
#include <SC16IS752.h>
#include <SPI.h>
#include <esp_task_wdt.h>
#include <JPEGDecoder.h>
#include <WiFi.h>
#include <WebSocketsServer.h>
// dodaj knižnice za pomoč za ekran, dotik in uart
#include "TouchScreenFunctions.h";
#include "Screens.h";
#include "SerialHelper.h";

// Podatki za WIFI povezavo. na ESP32CAM se morajo ujemati drugače se kamera ne poveže
const char *ssid = "ESP32_AP";
const char *password = "12345678";

// Ustvariš instanco websocket strežnika na portu 81
WebSocketsServer webSocket = WebSocketsServer(81);

// zunanje spremenljivke za ponastavljanje gumbov in sledenje časa
unsigned long previousMillisSubType = 0;
unsigned long currentMillis = 0;
// Parametri gumbov
bool ZAJEMI = false;
bool IZBRISI = false;

void setup()
{
  // Začetek glavne UART komunikacije
  Serial.begin(115200);
  SPI.begin();

  // pin v izhod na katerem je SC16IS752
  pinMode(EXPANDER_PIN, OUTPUT);
  // Vklopi pina na katerem je SC16IS752
  digitalWrite(EXPANDER_PIN, HIGH);
  // Vklop uart komunkacije z hitrostjo (baudrate) 115200 na kanalu A in kanalu B
  I2CExpander.begin(115200, 115200);

  // Nastavitev ekrana
  NastaviEkran();

  // Vklop WIFI postaje
  WiFi.softAP(ssid, password);
  IPAddress ipNaslov = WiFi.softAPIP();
  Serial.print("Ip naslov postaje: ");
  Serial.println(ipNaslov);

  // Vklop websocket strežnika
  webSocket.begin();
  // Ko pride nov event preusmeri podatke v webSocketEvent metodo
  webSocket.onEvent(webSocketEvent);
}

void loop()
{
  // Zaženi websocket zanko
  webSocket.loop();
  // Ko pritisnemo gumb Zajemi sliko pošljemo PAKET preko UART na ESP32CAM katera potem pošlje prek binarne podatke prek websocket strežnika
  if (ZAJEMI)
  {
    posljiPaket();
    // Ko pritisnemo gumb izbirši sliko izbrišemo iz ekrana
  }
  else if (IZBRISI)
  {
    izbrisiGumb();
  }

  currentMillis = millis();

  /*
  // reset timer for watch dog
  if (WATCHDOG_RESTART) {
    esp_task_wdt_reset();
  }*/

  // ponastavi gumbe v izklop
  if (currentMillis - previousMillisSubType >= INTERVAL_BTN_SUBTYPE_UNSELECT && previousMillisSubType > 0 && BTN_SUBTYPE > 0 && BTN_SUBTYPE_PREVIOUS > 0)
  {
    previousMillisSubType = currentMillis;
    renderButton(4, false);
    renderButton(6, false);

    delay(1);
  }

  // Poglej če uporabnik pritisne na ekran (touch)
  uint16_t x, y;
  tft.getTouchRaw(&x, &y);
  if (x > 0 && y > 0)
  {
    TouchScreenPositionX = x / 10;
    TouchScreenPositionY = y / 10;
    previousMillisSubType = currentMillis;
    ButtonPressed();
    delay(50);
  }
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length)
{
  switch (type)
  {
  // Ko se na websocket strežnik poveže nov odjemalec se zažene ta veja
  case WStype_CONNECTED:
    Serial.printf("[%u] Websocket povezava vzpostavljena!\n", num);
    break;
  // Ko odjemalec prekine povezavo iz websocket strežnika se zažene ta veja
  case WStype_DISCONNECTED:
    Serial.printf("[%u] Websocket povezava prekinjena!\n", num);
    break;
  // Ko websocket strežnik prejme binarne podatke se zažene ta veja
  case WStype_BIN:
    JpegDec.decodeArray(payload, length); // Podaš podatke slike ,ki jih prejmeš prek websocket v decoder kateri potem pretvori binarne podatke v JPEG sliko
    // Če je slika bila ustrezno dekodirana potem jo nariši na ekran
    if (JpegDec.width && JpegDec.height)
    {
      drawArrayJpeg(payload, length, 0, 0);
      delay(1000);
    }
    break;
  default:
    break;
  }
}
