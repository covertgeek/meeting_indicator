#include <BLEDevice.h> //Header file for BLE 
#include <WiFi.h>      //Header file for Wifi connections

// Wifi settings
const char* ssid = "your_wifi_ssid";  // Replace with you wifi SSID
const char* password = "your_wifi_password"; // Replace with your wifi password
const int SERVER_PORT = 80;
WiFiServer server(SERVER_PORT);

// Variable to store the HTTP request
String header;

// Assign output variables to GPIO pins
const int output26 = 26;
const int output27 = 27;


// Current time
unsigned long currentTime = millis();
// Previous time
unsigned long previousTime = 0; 
// Define timeout time in milliseconds (example: 2000ms = 2s)
const long timeoutTime = 2000;


// End wifi server settings



// Bluetooth Settings
String Headset_Address = "aa:bb:cc:11:22:33"; // Replace with your headphone's Bluetooth MAC address 
static BLERemoteCharacteristic* pRemoteCharacteristic;
int LED_BUILTIN = 2;
unsigned long lastSeenTime = 0;
//unsigned long currentTime;
unsigned long bluetoothTimeout = 5 * 1000; // 10 seconds * 1000 (milliseconds since being seen)

boolean meeting = false;
boolean headphones = false;

BLEScan* pBLEScan; //Name the scanning device as pBLEScan
BLEScanResults foundDevices;

static BLEAddress *Server_BLE_Address;
String Scanned_BLE_Address;

// End Bluetooth settings


class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks 
{
    void onResult(BLEAdvertisedDevice advertisedDevice) {
      // Serial.printf("Scan Result: %s \n", advertisedDevice.toString().c_str());
      Server_BLE_Address = new BLEAddress(advertisedDevice.getAddress());
      Scanned_BLE_Address = Server_BLE_Address->toString().c_str();
      if (Scanned_BLE_Address == Headset_Address) {
        lastSeenTime = millis();
        Serial.print("Last seen at ");
        Serial.println(lastSeenTime);
        if (!headphones) {
          Serial.println("LED is on");
          digitalWrite (output27,HIGH);
          headphones = true;
        }
      }
    }
};

void setup() {
    Serial.begin(115200); //Start serial monitor 
    Serial.println("ESP32 BLE Server program"); //Intro message 


    pinMode(output26, OUTPUT);
    pinMode(output27, OUTPUT);
    // Set outputs to LOW
    digitalWrite(output26, LOW);
    digitalWrite(output27, LOW);
    
    // Connect to Wi-Fi network with SSID and password
    Serial.print("Connecting to ");
    Serial.println(ssid);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }
    // Print local IP address and start web server
    Serial.println("");
    Serial.println("WiFi connected.");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
    
    server.begin();

    BLEDevice::init("");
    pBLEScan = BLEDevice::getScan(); //create new scan
    pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks()); //Call the class that is defined above 
    pBLEScan->setActiveScan(true); //active scan uses more power, but get results faster
    Serial.println("Current timeout: " + bluetoothTimeout);
    pinMode (LED_BUILTIN,OUTPUT); //Declare the in-built LED pin as output 
}

void loop() {
  // Begin webserver
    WiFiClient client = server.available();   // Listen for incoming clients

  if (client) {                             // If a new client connects,
    currentTime = millis();
    previousTime = currentTime;
    Serial.println("New Client.");          // print a message out in the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected() && currentTime - previousTime <= timeoutTime) {  // loop while the client's connected
      currentTime = millis();
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        header += c;
        if (c == '\n') {                    // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type: application/json");
            client.println("Connection: close");
            client.println();

            // turns the GPIOs on and off
            if (header.indexOf("GET /meeting/on") >= 0) {
              Serial.println("Meeting on");
              digitalWrite(output26, HIGH);
              meeting = true;
            } else if (header.indexOf("GET /meeting/off") >= 0) {
              Serial.println("Meeting off");
              digitalWrite(output26, LOW);
              meeting = false;
            } 
            
            // Display the HTML web page
            client.println("{");
            client.print("  \"meeting\": ");
            if (meeting) {
              client.println(" true, ");
            } else {
              client.println(" false, ");
            }

            client.print("  \"headphones\": ");
            if (headphones) {
              client.println(" true ");
            } else {
              client.println(" false ");
            }

            client.println("}");

            // The HTTP response ends with another blank line
            client.println();
            // Break out of the while loop
            break;
          } else { // if you got a newline, then clear currentLine
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
      }
    }
    // Clear the header variable
    header = "";
    // Close the connection
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  }


  // End Webserver

  // Begin Bluetooth
  
  
  foundDevices = pBLEScan->start(1); // Scan for 3 seconds
  Serial.print(currentTime);
  Serial.print(" - ");
  Serial.println(lastSeenTime);
  while (foundDevices.getCount() >= 1)
  {
    // Get current time
    currentTime = millis();
    
    if (headphones && lastSeenTime > 0 && (currentTime - lastSeenTime >= bluetoothTimeout)) {
      Serial.println("Device has been missing long enough. Turning off LED.");
      digitalWrite (output27,LOW);
      headphones = false;
      lastSeenTime = 0;
      //ESP.restart();
      break;
    } 
    else {
      Serial.println("Found a different BLe device in range");
      break;
    }
  }

  // End bluetooth
  Serial.println("Beginning next loop"); 
}
