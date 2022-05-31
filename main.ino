#include "Wire.h"
#include "LiquidCrystal_I2C.h"
#include <SPI.h>
#include <MFRC522.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "string.h"
#include <ArduinoJson.h>


LiquidCrystal_I2C lcd(0x27, 20, 4); 



constexpr uint8_t RST_PIN = 0;     
constexpr uint8_t SS_PIN = 2;     

constexpr uint8_t BUZZ = 15; 

MFRC522 rfid(SS_PIN, RST_PIN); // Instance of the class
MFRC522::MIFARE_Key key;



String tag;

const char* ssid = "rpi";
const char* password = "1192989Aa";
const char* mqtt_server = "10.42.0.1";
WiFiClient espClient;
PubSubClient client(espClient);






void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("WiFi connected - ESP IP address: ");
  Serial.println(WiFi.localIP());
}


void callback(String topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;

  
  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }

  DynamicJsonDocument doc(1024);
  deserializeJson(doc, messageTemp);
  const String name = doc["name"];
  const bool access = doc["access"];



  if(name == "unknown"){
    lcd.setCursor(0, 1);
    lcd.print("Unknown card");

    digitalWrite(BUZZ, HIGH);
    delay(1000);
    digitalWrite(BUZZ, LOW);
    delay(3000);
    digitalWrite(BUZZ, HIGH);
    delay(1000);
    digitalWrite(BUZZ, LOW);
    
    
  }else if(name != "unknown" && access == false){
    
    lcd.setCursor(0, 1);
    lcd.print("Access denied");
     digitalWrite(BUZZ, HIGH);
    delay(1000);
    digitalWrite(BUZZ, LOW);
    delay(1000);
    digitalWrite(BUZZ, HIGH);
    delay(1000);
    digitalWrite(BUZZ, LOW);
    delay(1000);
  }else if(access==true){

    //Open lock
   
   lcd.setCursor(0, 1);
    lcd.print("Welcome ");
    lcd.setCursor(9, 1);
    lcd.print(name);
     digitalWrite(BUZZ, HIGH);
    delay(3000);
    digitalWrite(BUZZ, LOW);

    //Close lock
   }
  
  
 
  delay(1000);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Scan your card");
  
  Serial.println();
  

}
 
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");


     if (client.connect("ESP8266Client")) {
      Serial.println("connected");  
      
      client.subscribe("rfreader");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
    
  }
}



void setup()
{

  pinMode(BUZZ, OUTPUT);
  
  
  lcd.init();

  lcd.backlight();
  lcd.setCursor(1, 0);
  lcd.print("Scan your card");
  
  


  Serial.begin(9600);
  SPI.begin(); 
  rfid.PCD_Init(); 

  setup_wifi();
  
  client.setServer(mqtt_server, 2000);
  client.setCallback(callback);

  client.subscribe("rfreader");
  
  

  
}



void loop(){



  if (!client.connected()) {
    reconnect();
  }
  if(!client.loop())
    client.connect("ESP8266Client");

  
  if ( ! rfid.PICC_IsNewCardPresent())
    return;
  if (rfid.PICC_ReadCardSerial()) {
    for (byte i = 0; i < 4; i++) {
      tag += rfid.uid.uidByte[i];
    }
   
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Tag:");
    lcd.setCursor(5, 0);
    lcd.print(tag);
    


    char s[7];
    sprintf(s, "%s", tag);
    
    
    client.publish("rfreader/1", s);
   
    
    
    tag = "";
    rfid.PICC_HaltA();
    rfid.PCD_StopCrypto1();
  }
  
}
