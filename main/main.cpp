#include "Arduino.h"
#include "EEPROM_FNS.h"
#include "NFC_FNS.h"
#include "WiFi.h"
#include "ThingSpeak.h"
#include "GPS_FNS.h"

#define THINGSPEAK_CHANNEL_ID 2975843
#define THINGSPEAK_API_KEY "2VD6T29L6WC7361C"

#define WIFI_NETWORK "Ssid"
#define WIFI_PASSWORD "amolvyas"
#define BUTTON_PIN 4
#define MOTOR_PIN 5

WiFiServer server(80);
EEPROMData userData;
WiFiClient thingSpeakClient;

void connectToWifi();
void sendHTMLPage(WiFiClient &client);
bool checkInput(int correctPin);
void wifiClientTask(void *pvParameters)
{
  while (true)
  {
    WiFiClient client = server.available();

    if (client)
    {
      Serial.println("Client Connected");

      unsigned long timeout = millis();
      while (!client.available() && millis() - timeout < 2000)
      {
        vTaskDelay(pdMS_TO_TICKS(10)); // short wait to yield
      }

      if (client.available())
      {
        String req = client.readStringUntil('\r');
        Serial.println(req);
        client.read(); // consume '\n'
        sendHTMLPage(client);
      }

      client.stop();
      Serial.println("Client Disconnected");
    }

    vTaskDelay(pdMS_TO_TICKS(50)); // small delay to avoid CPU hogging
  }
}

void setup()
{
  Serial.begin(115200);

  while (!Serial)
  {
    ;
  }

  NFC_init();
  loadHeadTailFromEEPROM(true);
  connectToWifi();
  gpsInit();
  ThingSpeak.begin(thingSpeakClient); // Start ThingSpeak client
  server.begin();

  pinMode(BUTTON_PIN, INPUT);
  pinMode(MOTOR_PIN, OUTPUT);

  readEncryptedEEPROM(0, &userData, sizeof(EEPROMData));

  Serial.println("Decrypted Data:");
  Serial.print("Name: ");
  Serial.println(userData.name);
  Serial.print("PIN: ");
  Serial.println(userData.pin);
  Serial.print("Balance: ");
  Serial.println(userData.balance);
  Serial.print("Wallet ID: ");
  Serial.println(userData.walletID);

  xTaskCreatePinnedToCore(wifiClientTask, "WiFi Client Task", 4096, NULL, 1, NULL, 1);
}

void loop()
{
  TransactionData t{};
  bool success = false;
  int gpsAttempt = 0;
  // snprintf(t.transactionAmount, sizeof(t.transactionAmount), "%ld", random(1, 1000));
  // snprintf(t.creditCardNumber, sizeof(t.creditCardNumber), "%ld", random(10000000, 100000000)); // 8-digit card number
  // strcpy(t.currency, "INR");
  // addTransaction(&t);

  Serial.println("Waiting for button press to enter PIN...");

  unsigned long waitTimeout = millis() + 10000;
  bool buttonPressed = false;

  while (!buttonPressed && millis() < waitTimeout)
  {
    if (digitalRead(BUTTON_PIN) == HIGH)
    {
      buttonPressed = true;
    }
    delay(50);
    yield();
  }

  if (!buttonPressed)
  {
    Serial.println("No button pressed. Restart loop.");
    return;
  }

  delay(300);

  if (checkInput(userData.pin))
  {
    Serial.println("Correct PIN entered!");
    digitalWrite(MOTOR_PIN, HIGH);
    delay(500);
    digitalWrite(MOTOR_PIN, LOW);

    readNFC();
    delay(100);
    t = getLatestTransaction();
    if (atoi(t.transactionAmount) >= 2000 || atoi(t.transactionAmount) <= 0)
    {
      Serial.print("Invalid Transaction Amount: ");
      Serial.print(t.transactionAmount);
      return;
    }
    else if (userData.balance <= atoi(t.transactionAmount))
    {
      Serial.println("Balance is too low");
      return;
    }

    userData.balance -= atoi(t.transactionAmount);
    writeNFC(random(10000, 99999));
    ThingSpeak.writeField(THINGSPEAK_CHANNEL_ID, 1, t.transactionAmount, THINGSPEAK_API_KEY); // ThingSpeak Write Fields

    Serial.println("Getting GPS Data...");
    while (!getGPSData(&t) and gpsAttempt < 5)
    {
      delay(5000);
      Serial.println("Failed! Trying Again");
      gpsAttempt++;
    }
    if (gpsAttempt == 5)
    {
      Serial.println("Max Attempts for GPS Reached. Defaulting to default value of -1 for Latitude and Longitude.");
      t.latitude = -1;
      t.longitude = -1;
    }
    else
    {
      Serial.println("Received GPS Data");
    }

    addTransaction(&t);
    success = true;
    writeEncryptedEEPROM(0, &userData, sizeof(EEPROMData));
  }
  else
  {
    Serial.println("Wrong PIN!");
  }
  if (!success)
  {
    for (int i = 0; i < 3; i++)
    {
      digitalWrite(MOTOR_PIN, HIGH);
      delay(100);
      digitalWrite(MOTOR_PIN, LOW);
      delay(100);
    }
  }
  delay(1000);
}

void connectToWifi()
{
  Serial.print("Connecting to WIFI");
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_NETWORK, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(1000);
  }

  Serial.println("Connected to Wifi");
  Serial.println(WiFi.localIP());
}

void sendHTMLPage(WiFiClient &client)
{
  String html = R"rawliteral(
  <!DOCTYPE html>
  <html>
  <head>
    <title>ESP32 Web Server</title>
    <style>
      body {
        font-family: Arial, sans-serif;
        background-color: #f0f2f5;
        margin: 0;
        padding: 0;
        display: flex;
        justify-content: center;
        align-items: center;
        flex-direction: column;
      }

      h1 {
        background-color: #4CAF50;
        color: white;
        padding: 20px;
        width: 100%;
        text-align: center;
        margin: 0;
      }

      .container {
        background: white;
        padding: 30px;
        margin-top: 20px;
        border-radius: 10px;
        box-shadow: 0 4px 8px rgba(0,0,0,0.2);
        max-width: 800px;
        width: 90%;
      }

      h2 {
        color: #333;
        font-weight: normal;
        margin-bottom: 10px;
      }

      .transaction-log {
        margin-top: 20px;
        background: #f9f9f9;
        padding: 15px;
        border-radius: 5px;
        font-family: monospace;
        overflow-x: auto;
      }

      .transaction-table {
        width: 100%;
        border-collapse: collapse;
        margin-top: 10px;
        max-height: 70vh;
      }

      .transaction-table th,
      .transaction-table td {
        border: 1px solid #ddd;
        padding: 12px;
        text-align: center;
      }

      .transaction-table th {
        background-color: #4CAF50;
        color: white;
      }

      .transaction-table tr:nth-child(even) {
        background-color: #f2f2f2;
      }

      .transaction-table tr:hover {
        background-color: #e1f5e1;
      }

      @media (max-width: 600px) {
        .container {
          padding: 20px;
        }

        .transaction-table th,
        .transaction-table td {
          padding: 8px;
          font-size: 14px;
        }
      }
    </style>

  </head>
  <body>
    <h1>Offline Wallet</h1>
    <div class="container">
  )rawliteral";

  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html");
  client.println("Connection: close");
  client.println();

  html += "<h2>Name of User: " + String(userData.name) + "</h2>\n";
  html += "<h2>Wallet ID: " + String(userData.walletID) + "</h2>\n";
  html += "<h2>2FA PIN: " + String(userData.pin) + "</h2>\n";
  html += "<h2>Remaining Balance: " + String(userData.balance) + " Rs.</h2>\n";

  html += "<div class=\"transaction-log\">\n";
  html += getAllTransactions();
  html += "</div>\n";

  html += R"rawliteral(
    </div>
  </body>
  </html>
  )rawliteral";

  client.println(html);

  client.stop();
}

bool checkInput(int correctPIN)
{
  int pin = 0;
  int digitCount = 0;
  unsigned long buttonPressTime = 0;
  int digit = 0;
  bool buttonWasPressed = false;
  Serial.println("Enter PIN: Press and hold to increment, release to confirm");
  unsigned long pinEntryTimeout = millis() + 30000;

  while (digitCount < 2 && millis() < pinEntryTimeout)
  {
    if (digitalRead(BUTTON_PIN) == HIGH)
    {
      if (!buttonWasPressed)
      {
        buttonWasPressed = true;
        buttonPressTime = millis();
        digit = 0;
        Serial.println("Button pressed...");
      }

      if (millis() - buttonPressTime >= 1000)
      {
        buttonPressTime = millis();
        digitalWrite(MOTOR_PIN, HIGH);
        delay(300);
        digitalWrite(MOTOR_PIN, LOW);
        digit = (digit + 1) % 10;
        Serial.print("Current Digit: ");
        Serial.println(digit);
      }
    }
    else if (buttonWasPressed)
    {
      buttonWasPressed = false;
      Serial.print("Digit confirmed: ");
      Serial.println(digit);

      pin = pin * 10 + digit;
      digitCount++;

      delay(500);
    }
    delay(50);
    yield();
  }

  if (digitCount < 2)
  {
    Serial.println("Timeout entering PIN");
    return false;
  }

  Serial.print("Entered PIN: ");
  Serial.println(pin);

  return (pin == correctPIN);
}
