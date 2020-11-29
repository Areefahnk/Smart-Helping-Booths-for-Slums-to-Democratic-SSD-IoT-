#include <SoftwareSerial.h>

#include <ESP8266WiFi.h>
#include <Adafruit_Fingerprint.h>
#define digitalpin D4 
 
String apiKey = "*****************"; //apiKey generated while creating ThingSpeak Channel   

const char *ssid =  "**********";      //replace with your wifi ssid and wpa2 key
const char *pass =  "******";
const char* server = "api.thingspeak.com";
int c=0;
int state,laststate=0;
const int sensorOut = A0;
const int led = 16;

WiFiClient client;

//#include Adafruit_Fingerprint.h

// On LeonardoMicro or others with hardware serial, use those! #0 is green wire, #1 is white
 //uncomment this line
 //#define mySerial Serial1

 //For UNO and others without hardware serial, we must use software serial...
// pin #2 is IN from sensor (GREEN wire)
 //pin #3 is OUT from arduino  (WHITE wire)
 //comment these two lines if using hardware serial
SoftwareSerial mySerial(D2, D3);

Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

void setup()  
{
 Serial.begin(115200);
  while (!Serial);   //For YunLeoMicroZero...
  delay(100);
  Serial.println("\n\nAdafruit finger detect test");

   //set the data rate for the sensor serial port
  finger.begin(57600);
  delay(5);
  if (finger.verifyPassword()) {
    Serial.println("Found fingerprint sensor!");
  } else {
    Serial.println("Did not find fingerprint sensor");
    while (1) { delay(1); }
  }

  finger.getTemplateCount();
  Serial.print("Sensor contains" ); Serial.print(finger.templateCount); Serial.println( "templates");
  Serial.println("Waiting for valid finger...");

    
       pinMode(D4, OUTPUT); 
       Serial.println("Connecting to ");
       Serial.println(ssid);
 
       WiFi.begin(ssid, pass);
 
      while (WiFi.status() != WL_CONNECTED) 
     {
            delay(500);
            Serial.print(".");
     }
      Serial.println(" ");
      Serial.println("WiFi connected");
 
}
void loop()                     // run over and over again
{
  int x;
  x=getFingerprintIDez();
  delay(50);            //don't ned to run this at full speed.
       if (x==0) 
                 {
                     Serial.println("Failed to read from sensor!");
                      
                 }
          else if (x>0 && x<127) 
                         if (client.connect(server,80))  
                      {  
                            
                             String postStr = apiKey;
                             postStr +="&field1=";
                             postStr += String(x);
                             postStr += "\r\n\r\n";
 
                             client.print("POST /update HTTP/1.1\n");
                             client.print("Host: api.thingspeak.com\n");
                             client.print("Connection: close\n");
                             client.print("X-THINGSPEAKAPIKEY: "+apiKey+"\n");
                             client.print("Content-Type: application/x-www-form-urlencoded\n");
                             client.print("Content-Length: ");
                             client.print(postStr.length());
                             client.print("\n\n");
                             client.print(postStr);
 
                             Serial.print("dis= ");
                             Serial.print(x);
                             Serial.println(". Send to Thingspeak.");

                        }
          client.stop();
 
          Serial.println("Waiting...");
  
  // thingspeak needs minimum 15 sec delay between updates, i've set it to 30 seconds
  delay(5000);
}

uint8_t getFingerprintID() {
  uint8_t p = finger.getImage();
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image taken");
      break;
    case FINGERPRINT_NOFINGER:
      Serial.println("No finger detected");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("Imaging error");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }

  // OK success!

  p = finger.image2Tz();
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }
  
  // OK converted!
  p = finger.fingerFastSearch();
  if (p == FINGERPRINT_OK) {
    Serial.println("Found a print match!");
    digitalWrite(digitalpin,HIGH);
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    digitalWrite(digitalpin,LOW);
    return p;
  } else if (p == FINGERPRINT_NOTFOUND) {
    Serial.println("Did not find a match");
    digitalWrite(digitalpin,LOW);
    return p;
  } else {
    Serial.println("Unknown error");
     digitalWrite(digitalpin,LOW);
    return p;
  }   
  
  // found a match!
  Serial.print("Found ID #"); Serial.print(finger.fingerID); 
  if(finger.fingerID>0 && finger.fingerID<127)
  {
    digitalWrite(D4,HIGH);
  }
  Serial.print(" with confidence of "); Serial.println(finger.confidence); 

  return finger.fingerID;
}

// returns -1 if failed, otherwise returns ID #
int getFingerprintIDez() {
  uint8_t p = finger.getImage();
  if (p != FINGERPRINT_OK)  return -1;

  p = finger.image2Tz();
  if (p != FINGERPRINT_OK)  return -1;

  p = finger.fingerFastSearch();
  if (p != FINGERPRINT_OK)  return -1;
  
  // found a match!
  Serial.print("Found ID #"); Serial.print(finger.fingerID); 
  Serial.print(" with confidence of "); Serial.println(finger.confidence);
  return finger.fingerID; 
 
  // Upload to thingspeak

}
