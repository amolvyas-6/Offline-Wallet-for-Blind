// #include "Arduino.h"
// #include "EEPROM_FNS.h"
// #include "NFC_FNS.h"
// #include "WiFi.h"
// #include "ThingSpeak.h"
// #include "GPS_FNS.h"

// #define THINGSPEAK_CHANNEL_ID 2975843
// #define THINGSPEAK_API_KEY "2VD6T29L6WC7361C"

// #define WIFI_NETWORK "Ssid"
// #define WIFI_PASSWORD "amolvyas"
// #define BUTTON_PIN 4
// #define MOTOR_PIN 5

// WiFiServer server(80);
// EEPROMData userData;
// WiFiClient thingSpeakClient;

// void connectToWifi();
// void sendHTMLPage(WiFiClient &client);
// bool checkInput(int correctPin);
// void wifiClientTask(void *pvParameters)
// {
//   while (true)
//   {
//     WiFiClient client = server.available();

//     if (client)
//     {
//       Serial.println("Client Connected");

//       unsigned long timeout = millis();
//       while (!client.available() && millis() - timeout < 2000)
//       {
//         vTaskDelay(pdMS_TO_TICKS(10)); // short wait to yield
//       }

//       if (client.available())
//       {
//         String req = client.readStringUntil('\r');
//         Serial.println(req);
//         client.read(); // consume '\n'
//         sendHTMLPage(client);
//       }

//       client.stop();
//       Serial.println("Client Disconnected");
//     }

//     vTaskDelay(pdMS_TO_TICKS(50)); // small delay to avoid CPU hogging
//   }
// }

// void setup()
// {
//   Serial.begin(115200);

//   while (!Serial)
//   {
//     ;
//   }

//   NFC_init();
//   loadHeadTailFromEEPROM(true);
//   connectToWifi();
//   gpsInit();
//   ThingSpeak.begin(thingSpeakClient); // Start ThingSpeak client
//   server.begin();

//   pinMode(BUTTON_PIN, INPUT);
//   pinMode(MOTOR_PIN, OUTPUT);

//   readEncryptedEEPROM(0, &userData, sizeof(EEPROMData));

//   Serial.println("Decrypted Data:");
//   Serial.print("Name: ");
//   Serial.println(userData.name);
//   Serial.print("PIN: ");
//   Serial.println(userData.pin);
//   Serial.print("Balance: ");
//   Serial.println(userData.balance);
//   Serial.print("Wallet ID: ");
//   Serial.println(userData.walletID);

//   xTaskCreatePinnedToCore(wifiClientTask, "WiFi Client Task", 4096, NULL, 1, NULL, 1);
// }

// void loop()
// {
//   TransactionData t{};
//   bool success = false;
//   int gpsAttempt = 0;
//   // snprintf(t.transactionAmount, sizeof(t.transactionAmount), "%ld", random(1, 1000));
//   // snprintf(t.creditCardNumber, sizeof(t.creditCardNumber), "%ld", random(10000000, 100000000)); // 8-digit card number
//   // strcpy(t.currency, "INR");
//   // addTransaction(&t);

//   Serial.println("Waiting for button press to enter PIN...");

//   unsigned long waitTimeout = millis() + 10000;
//   bool buttonPressed = false;

//   while (!buttonPressed && millis() < waitTimeout)
//   {
//     if (digitalRead(BUTTON_PIN) == HIGH)
//     {
//       buttonPressed = true;
//     }
//     delay(50);
//     yield();
//   }

//   if (!buttonPressed)
//   {
//     Serial.println("No button pressed. Restart loop.");
//     return;
//   }

//   delay(300);

//   if (checkInput(userData.pin))
//   {
//     Serial.println("Correct PIN entered!");
//     digitalWrite(MOTOR_PIN, HIGH);
//     delay(500);
//     digitalWrite(MOTOR_PIN, LOW);

//     readNFC();
//     delay(100);
//     t = getLatestTransaction();
//     if (atoi(t.transactionAmount) >= 2000 || atoi(t.transactionAmount) <= 0)
//     {
//       Serial.print("Invalid Transaction Amount: ");
//       Serial.print(t.transactionAmount);
//       return;
//     }
//     else if (userData.balance <= atoi(t.transactionAmount))
//     {
//       Serial.println("Balance is too low");
//       return;
//     }

//     userData.balance -= atoi(t.transactionAmount);
//     writeNFC(random(10000, 99999));
//     ThingSpeak.writeField(THINGSPEAK_CHANNEL_ID, 1, t.transactionAmount, THINGSPEAK_API_KEY); // ThingSpeak Write Fields

//     Serial.println("Getting GPS Data...");
//     while (!getGPSData(&t) and gpsAttempt < 5)
//     {
//       delay(5000);
//       Serial.println("Failed! Trying Again");
//       gpsAttempt++;
//     }
//     if (gpsAttempt == 5)
//     {
//       Serial.println("Max Attempts for GPS Reached. Defaulting to default value of -1 for Latitude and Longitude.");
//       t.latitude = -1;
//       t.longitude = -1;
//     }
//     else
//     {
//       Serial.println("Received GPS Data");
//     }

//     addTransaction(&t);
//     success = true;
//     writeEncryptedEEPROM(0, &userData, sizeof(EEPROMData));
//   }
//   else
//   {
//     Serial.println("Wrong PIN!");
//   }
//   if (!success)
//   {
//     for (int i = 0; i < 3; i++)
//     {
//       digitalWrite(MOTOR_PIN, HIGH);
//       delay(100);
//       digitalWrite(MOTOR_PIN, LOW);
//       delay(100);
//     }
//   }
//   delay(1000);
// }

// void connectToWifi()
// {
//   Serial.print("Connecting to WIFI");
//   WiFi.mode(WIFI_STA);
//   WiFi.begin(WIFI_NETWORK, WIFI_PASSWORD);

//   while (WiFi.status() != WL_CONNECTED)
//   {
//     Serial.print(".");
//     delay(1000);
//   }

//   Serial.println("Connected to Wifi");
//   Serial.println(WiFi.localIP());
// }

// void sendHTMLPage(WiFiClient &client)
// {
//   String html = R"rawliteral(
//   <!DOCTYPE html>
//   <html>
//   <head>
//     <title>ESP32 Web Server</title>
//     <style>
//       body {
//         font-family: Arial, sans-serif;
//         background-color: #f0f2f5;
//         margin: 0;
//         padding: 0;
//         display: flex;
//         justify-content: center;
//         align-items: center;
//         flex-direction: column;
//       }

//       h1 {
//         background-color: #4CAF50;
//         color: white;
//         padding: 20px;
//         width: 100%;
//         text-align: center;
//         margin: 0;
//       }

//       .container {
//         background: white;
//         padding: 30px;
//         margin-top: 20px;
//         border-radius: 10px;
//         box-shadow: 0 4px 8px rgba(0,0,0,0.2);
//         max-width: 800px;
//         width: 90%;
//       }

//       h2 {
//         color: #333;
//         font-weight: normal;
//         margin-bottom: 10px;
//       }

//       .transaction-log {
//         margin-top: 20px;
//         background: #f9f9f9;
//         padding: 15px;
//         border-radius: 5px;
//         font-family: monospace;
//         overflow-x: auto;
//       }

//       .transaction-table {
//         width: 100%;
//         border-collapse: collapse;
//         margin-top: 10px;
//         max-height: 70vh;
//       }

//       .transaction-table th,
//       .transaction-table td {
//         border: 1px solid #ddd;
//         padding: 12px;
//         text-align: center;
//       }

//       .transaction-table th {
//         background-color: #4CAF50;
//         color: white;
//       }

//       .transaction-table tr:nth-child(even) {
//         background-color: #f2f2f2;
//       }

//       .transaction-table tr:hover {
//         background-color: #e1f5e1;
//       }

//       @media (max-width: 600px) {
//         .container {
//           padding: 20px;
//         }

//         .transaction-table th,
//         .transaction-table td {
//           padding: 8px;
//           font-size: 14px;
//         }
//       }
//     </style>

//   </head>
//   <body>
//     <h1>Offline Wallet</h1>
//     <div class="container">
//   )rawliteral";

//   client.println("HTTP/1.1 200 OK");
//   client.println("Content-Type: text/html");
//   client.println("Connection: close");
//   client.println();

//   html += "<h2>Name of User: " + String(userData.name) + "</h2>\n";
//   html += "<h2>Wallet ID: " + String(userData.walletID) + "</h2>\n";
//   html += "<h2>2FA PIN: " + String(userData.pin) + "</h2>\n";
//   html += "<h2>Remaining Balance: " + String(userData.balance) + " Rs.</h2>\n";

//   html += "<div class=\"transaction-log\">\n";
//   html += getAllTransactions();
//   html += "</div>\n";

//   html += R"rawliteral(
//     </div>
//   </body>
//   </html>
//   )rawliteral";

//   client.println(html);

//   client.stop();
// }

// bool checkInput(int correctPIN)
// {
//   int pin = 0;
//   int digitCount = 0;
//   unsigned long buttonPressTime = 0;
//   int digit = 0;
//   bool buttonWasPressed = false;
//   Serial.println("Enter PIN: Press and hold to increment, release to confirm");
//   unsigned long pinEntryTimeout = millis() + 30000;

//   while (digitCount < 2 && millis() < pinEntryTimeout)
//   {
//     if (digitalRead(BUTTON_PIN) == HIGH)
//     {
//       if (!buttonWasPressed)
//       {
//         buttonWasPressed = true;
//         buttonPressTime = millis();
//         digit = 0;
//         Serial.println("Button pressed...");
//       }

//       if (millis() - buttonPressTime >= 1000)
//       {
//         buttonPressTime = millis();
//         digitalWrite(MOTOR_PIN, HIGH);
//         delay(300);
//         digitalWrite(MOTOR_PIN, LOW);
//         digit = (digit + 1) % 10;
//         Serial.print("Current Digit: ");
//         Serial.println(digit);
//       }
//     }
//     else if (buttonWasPressed)
//     {
//       buttonWasPressed = false;
//       Serial.print("Digit confirmed: ");
//       Serial.println(digit);

//       pin = pin * 10 + digit;
//       digitCount++;

//       delay(500);
//     }
//     delay(50);
//     yield();
//   }

//   if (digitCount < 2)
//   {
//     Serial.println("Timeout entering PIN");
//     return false;
//   }

//   Serial.print("Entered PIN: ");
//   Serial.println(pin);

//   return (pin == correctPIN);
// }


/*
  Wifi secure connection example for ESP32
  Running on TLS 1.2 using mbedTLS
  Supporting the following ciphersuites:
  "TLS_ECDHE_ECDSA_WITH_AES_256_GCM_SHA384","TLS_ECDHE_RSA_WITH_AES_256_GCM_SHA384","TLS_DHE_RSA_WITH_AES_256_GCM_SHA384","TLS_ECDHE_ECDSA_WITH_AES_256_CCM","TLS_DHE_RSA_WITH_AES_256_CCM","TLS_ECDHE_ECDSA_WITH_AES_256_CBC_SHA384","TLS_ECDHE_RSA_WITH_AES_256_CBC_SHA384","TLS_DHE_RSA_WITH_AES_256_CBC_SHA256","TLS_ECDHE_ECDSA_WITH_AES_256_CBC_SHA","TLS_ECDHE_RSA_WITH_AES_256_CBC_SHA","TLS_DHE_RSA_WITH_AES_256_CBC_SHA","TLS_ECDHE_ECDSA_WITH_AES_256_CCM_8","TLS_DHE_RSA_WITH_AES_256_CCM_8","TLS_ECDHE_ECDSA_WITH_CAMELLIA_256_GCM_SHA384","TLS_ECDHE_RSA_WITH_CAMELLIA_256_GCM_SHA384","TLS_DHE_RSA_WITH_CAMELLIA_256_GCM_SHA384","TLS_ECDHE_ECDSA_WITH_CAMELLIA_256_CBC_SHA384","TLS_ECDHE_RSA_WITH_CAMELLIA_256_CBC_SHA384","TLS_DHE_RSA_WITH_CAMELLIA_256_CBC_SHA256","TLS_DHE_RSA_WITH_CAMELLIA_256_CBC_SHA","TLS_ECDHE_ECDSA_WITH_AES_128_GCM_SHA256","TLS_ECDHE_RSA_WITH_AES_128_GCM_SHA256","TLS_DHE_RSA_WITH_AES_128_GCM_SHA256","TLS_ECDHE_ECDSA_WITH_AES_128_CCM","TLS_DHE_RSA_WITH_AES_128_CCM","TLS_ECDHE_ECDSA_WITH_AES_128_CBC_SHA256","TLS_ECDHE_RSA_WITH_AES_128_CBC_SHA256","TLS_DHE_RSA_WITH_AES_128_CBC_SHA256","TLS_ECDHE_ECDSA_WITH_AES_128_CBC_SHA","TLS_ECDHE_RSA_WITH_AES_128_CBC_SHA","TLS_DHE_RSA_WITH_AES_128_CBC_SHA","TLS_ECDHE_ECDSA_WITH_AES_128_CCM_8","TLS_DHE_RSA_WITH_AES_128_CCM_8","TLS_ECDHE_ECDSA_WITH_CAMELLIA_128_GCM_SHA256","TLS_ECDHE_RSA_WITH_CAMELLIA_128_GCM_SHA256","TLS_DHE_RSA_WITH_CAMELLIA_128_GCM_SHA256","TLS_ECDHE_ECDSA_WITH_CAMELLIA_128_CBC_SHA256","TLS_ECDHE_RSA_WITH_CAMELLIA_128_CBC_SHA256","TLS_DHE_RSA_WITH_CAMELLIA_128_CBC_SHA256","TLS_DHE_RSA_WITH_CAMELLIA_128_CBC_SHA","TLS_ECDHE_ECDSA_WITH_3DES_EDE_CBC_SHA","TLS_ECDHE_RSA_WITH_3DES_EDE_CBC_SHA","TLS_DHE_RSA_WITH_3DES_EDE_CBC_SHA","TLS_DHE_PSK_WITH_AES_256_GCM_SHA384","TLS_DHE_PSK_WITH_AES_256_CCM","TLS_ECDHE_PSK_WITH_AES_256_CBC_SHA384","TLS_DHE_PSK_WITH_AES_256_CBC_SHA384","TLS_ECDHE_PSK_WITH_AES_256_CBC_SHA","TLS_DHE_PSK_WITH_AES_256_CBC_SHA","TLS_DHE_PSK_WITH_CAMELLIA_256_GCM_SHA384","TLS_ECDHE_PSK_WITH_CAMELLIA_256_CBC_SHA384","TLS_DHE_PSK_WITH_CAMELLIA_256_CBC_SHA384","TLS_PSK_DHE_WITH_AES_256_CCM_8","TLS_DHE_PSK_WITH_AES_128_GCM_SHA256","TLS_DHE_PSK_WITH_AES_128_CCM","TLS_ECDHE_PSK_WITH_AES_128_CBC_SHA256","TLS_DHE_PSK_WITH_AES_128_CBC_SHA256","TLS_ECDHE_PSK_WITH_AES_128_CBC_SHA","TLS_DHE_PSK_WITH_AES_128_CBC_SHA","TLS_DHE_PSK_WITH_CAMELLIA_128_GCM_SHA256","TLS_DHE_PSK_WITH_CAMELLIA_128_CBC_SHA256","TLS_ECDHE_PSK_WITH_CAMELLIA_128_CBC_SHA256","TLS_PSK_DHE_WITH_AES_128_CCM_8","TLS_ECDHE_PSK_WITH_3DES_EDE_CBC_SHA","TLS_DHE_PSK_WITH_3DES_EDE_CBC_SHA","TLS_RSA_WITH_AES_256_GCM_SHA384","TLS_RSA_WITH_AES_256_CCM","TLS_RSA_WITH_AES_256_CBC_SHA256","TLS_RSA_WITH_AES_256_CBC_SHA","TLS_ECDH_RSA_WITH_AES_256_GCM_SHA384","TLS_ECDH_RSA_WITH_AES_256_CBC_SHA384","TLS_ECDH_RSA_WITH_AES_256_CBC_SHA","TLS_ECDH_ECDSA_WITH_AES_256_GCM_SHA384","TLS_ECDH_ECDSA_WITH_AES_256_CBC_SHA384","TLS_ECDH_ECDSA_WITH_AES_256_CBC_SHA","TLS_RSA_WITH_AES_256_CCM_8","TLS_RSA_WITH_CAMELLIA_256_GCM_SHA384","TLS_RSA_WITH_CAMELLIA_256_CBC_SHA256","TLS_RSA_WITH_CAMELLIA_256_CBC_SHA","TLS_ECDH_RSA_WITH_CAMELLIA_256_GCM_SHA384","TLS_ECDH_RSA_WITH_CAMELLIA_256_CBC_SHA384","TLS_ECDH_ECDSA_WITH_CAMELLIA_256_GCM_SHA384","TLS_ECDH_ECDSA_WITH_CAMELLIA_256_CBC_SHA384","TLS_RSA_WITH_AES_128_GCM_SHA256","TLS_RSA_WITH_AES_128_CCM","TLS_RSA_WITH_AES_128_CBC_SHA256","TLS_RSA_WITH_AES_128_CBC_SHA","TLS_ECDH_RSA_WITH_AES_128_GCM_SHA256","TLS_ECDH_RSA_WITH_AES_128_CBC_SHA256","TLS_ECDH_RSA_WITH_AES_128_CBC_SHA","TLS_ECDH_ECDSA_WITH_AES_128_GCM_SHA256","TLS_ECDH_ECDSA_WITH_AES_128_CBC_SHA256","TLS_ECDH_ECDSA_WITH_AES_128_CBC_SHA","TLS_RSA_WITH_AES_128_CCM_8","TLS_RSA_WITH_CAMELLIA_128_GCM_SHA256","TLS_RSA_WITH_CAMELLIA_128_CBC_SHA256","TLS_RSA_WITH_CAMELLIA_128_CBC_SHA","TLS_ECDH_RSA_WITH_CAMELLIA_128_GCM_SHA256","TLS_ECDH_RSA_WITH_CAMELLIA_128_CBC_SHA256","TLS_ECDH_ECDSA_WITH_CAMELLIA_128_GCM_SHA256","TLS_ECDH_ECDSA_WITH_CAMELLIA_128_CBC_SHA256","TLS_RSA_WITH_3DES_EDE_CBC_SHA","TLS_ECDH_RSA_WITH_3DES_EDE_CBC_SHA","TLS_ECDH_ECDSA_WITH_3DES_EDE_CBC_SHA","TLS_RSA_PSK_WITH_AES_256_GCM_SHA384","TLS_RSA_PSK_WITH_AES_256_CBC_SHA384","TLS_RSA_PSK_WITH_AES_256_CBC_SHA","TLS_RSA_PSK_WITH_CAMELLIA_256_GCM_SHA384","TLS_RSA_PSK_WITH_CAMELLIA_256_CBC_SHA384","TLS_RSA_PSK_WITH_AES_128_GCM_SHA256","TLS_RSA_PSK_WITH_AES_128_CBC_SHA256","TLS_RSA_PSK_WITH_AES_128_CBC_SHA","TLS_RSA_PSK_WITH_CAMELLIA_128_GCM_SHA256","TLS_RSA_PSK_WITH_CAMELLIA_128_CBC_SHA256","TLS_RSA_PSK_WITH_3DES_EDE_CBC_SHA","TLS_PSK_WITH_AES_256_GCM_SHA384","TLS_PSK_WITH_AES_256_CCM","TLS_PSK_WITH_AES_256_CBC_SHA384","TLS_PSK_WITH_AES_256_CBC_SHA","TLS_PSK_WITH_CAMELLIA_256_GCM_SHA384","TLS_PSK_WITH_CAMELLIA_256_CBC_SHA384","TLS_PSK_WITH_AES_256_CCM_8","TLS_PSK_WITH_AES_128_GCM_SHA256","TLS_PSK_WITH_AES_128_CCM","TLS_PSK_WITH_AES_128_CBC_SHA256","TLS_PSK_WITH_AES_128_CBC_SHA","TLS_PSK_WITH_CAMELLIA_128_GCM_SHA256","TLS_PSK_WITH_CAMELLIA_128_CBC_SHA256","TLS_PSK_WITH_AES_128_CCM_8","TLS_PSK_WITH_3DES_EDE_CBC_SHA","TLS_EMPTY_RENEGOTIATION_INFO_SCSV"]
  2017 - Evandro Copercini - Apache 2.0 License.
*/

#include <NetworkClientSecure.h>
#include <WiFi.h>

const char *ssid = "your-ssid";          // your network SSID (name of wifi network)
const char *password = "your-password";  // your network password

const char *server = "www.howsmyssl.com";  // Server URL

// www.howsmyssl.com root certificate authority, to verify the server
// change it to your server root CA
// SHA1 fingerprint is broken now!

const char *test_root_ca = R"literal(
-----BEGIN CERTIFICATE-----
MIIFBTCCAu2gAwIBAgIQS6hSk/eaL6JzBkuoBI110DANBgkqhkiG9w0BAQsFADBP
MQswCQYDVQQGEwJVUzEpMCcGA1UEChMgSW50ZXJuZXQgU2VjdXJpdHkgUmVzZWFy
Y2ggR3JvdXAxFTATBgNVBAMTDElTUkcgUm9vdCBYMTAeFw0yNDAzMTMwMDAwMDBa
Fw0yNzAzMTIyMzU5NTlaMDMxCzAJBgNVBAYTAlVTMRYwFAYDVQQKEw1MZXQncyBF
bmNyeXB0MQwwCgYDVQQDEwNSMTAwggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEK
AoIBAQDPV+XmxFQS7bRH/sknWHZGUCiMHT6I3wWd1bUYKb3dtVq/+vbOo76vACFL
YlpaPAEvxVgD9on/jhFD68G14BQHlo9vH9fnuoE5CXVlt8KvGFs3Jijno/QHK20a
/6tYvJWuQP/py1fEtVt/eA0YYbwX51TGu0mRzW4Y0YCF7qZlNrx06rxQTOr8IfM4
FpOUurDTazgGzRYSespSdcitdrLCnF2YRVxvYXvGLe48E1KGAdlX5jgc3421H5KR
mudKHMxFqHJV8LDmowfs/acbZp4/SItxhHFYyTr6717yW0QrPHTnj7JHwQdqzZq3
DZb3EoEmUVQK7GH29/Xi8orIlQ2NAgMBAAGjgfgwgfUwDgYDVR0PAQH/BAQDAgGG
MB0GA1UdJQQWMBQGCCsGAQUFBwMCBggrBgEFBQcDATASBgNVHRMBAf8ECDAGAQH/
AgEAMB0GA1UdDgQWBBS7vMNHpeS8qcbDpHIMEI2iNeHI6DAfBgNVHSMEGDAWgBR5
tFnme7bl5AFzgAiIyBpY9umbbjAyBggrBgEFBQcBAQQmMCQwIgYIKwYBBQUHMAKG
Fmh0dHA6Ly94MS5pLmxlbmNyLm9yZy8wEwYDVR0gBAwwCjAIBgZngQwBAgEwJwYD
VR0fBCAwHjAcoBqgGIYWaHR0cDovL3gxLmMubGVuY3Iub3JnLzANBgkqhkiG9w0B
AQsFAAOCAgEAkrHnQTfreZ2B5s3iJeE6IOmQRJWjgVzPw139vaBw1bGWKCIL0vIo
zwzn1OZDjCQiHcFCktEJr59L9MhwTyAWsVrdAfYf+B9haxQnsHKNY67u4s5Lzzfd
u6PUzeetUK29v+PsPmI2cJkxp+iN3epi4hKu9ZzUPSwMqtCceb7qPVxEbpYxY1p9
1n5PJKBLBX9eb9LU6l8zSxPWV7bK3lG4XaMJgnT9x3ies7msFtpKK5bDtotij/l0
GaKeA97pb5uwD9KgWvaFXMIEt8jVTjLEvwRdvCn294GPDF08U8lAkIv7tghluaQh
1QnlE4SEN4LOECj8dsIGJXpGUk3aU3KkJz9icKy+aUgA+2cP21uh6NcDIS3XyfaZ
QjmDQ993ChII8SXWupQZVBiIpcWO4RqZk3lr7Bz5MUCwzDIA359e57SSq5CCkY0N
4B6Vulk7LktfwrdGNVI5BsC9qqxSwSKgRJeZ9wygIaehbHFHFhcBaMDKpiZlBHyz
rsnnlFXCb5s8HKn5LsUgGvB24L7sGNZP2CX7dhHov+YhD+jozLW2p9W4959Bz2Ei
RmqDtmiXLnzqTpXbI+suyCsohKRg6Un0RC47+cpiVwHiXZAW+cn8eiNIjqbVgXLx
KPpdzvvtTnOPlC7SQZSYmdunr3Bf9b77AiC/ZidstK36dRILKz7OA54=
-----END CERTIFICATE-----
)literal";
// You can use x.509 client certificates if you want
//const char* test_client_key = "";   //to verify the client
//const char* test_client_cert = "";  //to verify the client

NetworkClientSecure client;

void setup() {
  //Initialize serial and wait for port to open:
  Serial.begin(115200);
  delay(100);

  Serial.print("Attempting to connect to SSID: ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  // attempt to connect to Wifi network:
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    // wait 1 second for re-trying
    delay(1000);
  }

  Serial.print("Connected to ");
  Serial.println(ssid);

  client.setCACert(test_root_ca);
  //client.setCertificate(test_client_cert); // for client verification
  //client.setPrivateKey(test_client_key);	// for client verification

  Serial.println("\nStarting connection to server...");
  if (!client.connect(server, 443)) {
    Serial.println("Connection failed!");
  } else {
    Serial.println("Connected to server!");
    // Make a HTTP request:
    client.println("GET https://www.howsmyssl.com/a/check HTTP/1.0");
    client.println("Host: www.howsmyssl.com");
    client.println("Connection: close");
    client.println();

    while (client.connected()) {
      String line = client.readStringUntil('\n');
      if (line == "\r") {
        Serial.println("headers received");
        break;
      }
    }
    // if there are incoming bytes available
    // from the server, read them and print them:
    while (client.available()) {
      char c = client.read();
      Serial.write(c);
    }

    client.stop();
  }
}

void loop() {
  // do nothing
}