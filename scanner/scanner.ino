/*
  RFID Authorization Daemon
  
  This sketch continuously scans for RFID cards. When one is found
  it queries an external server to confirm authorization. Access is
  either granted or denied based on the server's response.
  
  This system makes use of the Arduino Ethernet Shield and the SM130
  RFID Reader/Writer module.
  
  SM130 rx/tx pins are 7 & 8 respectively.
  
*/

#define HALT 147
#define SEEK 130
#define DATA_LENGTH 2
#define FOUND 6

#include <SoftwareSerial.h>
#include <SPI.h>
#include <Ethernet.h>

//Configure Ethernet MAC and destination addresses
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress server(xxx,xxx,xxx,xxx);

EthernetClient client;

//Create serial connection to RFID module on pins 7 and 8
SoftwareSerial rfid(7, 8);

//Buzzer and LED pins used to provide feedback to person at the scanner
int greenLED = 4;
int redLED = 5;
int buzzer = 6;

void setup()
{
  //Initialize buzzer and LEDs
  pinMode(greenLED, OUTPUT);
  pinMode(redLED, OUTPUT);
  pinMode(buzzer, OUTPUT);
  
  //Red light signifies access denied
  digitalWrite(redLED, HIGH);

  // start the Ethernet connection:
  if (Ethernet.begin(mac) == 0) {
    //If ethernet fails to start, flash red LED forever
    while (true) {
      digitalWrite(redLED, HIGH);
      delay(250);
      digitalWrite(redLED, LOW);
      delay(250);
    }
  }
  // give the Ethernet shield a second to initialize:
  delay(1000);
  rfid.begin(19200);
  delay(10);

  // Initialize RFID module
  send_command(HALT);
}

void loop()
{
  int* tag;
  int flag;
  boolean authorized;

  send_command(SEEK);
  delay(10);

  tag = parse();
  authorized = get_auth(tag);
  
  if (authorized){
    //Flash green light and unlock door, then return to red
    digitalWrite(greenLED, HIGH);
    delay(250);
    digitalWrite(greenLED, LOW);
    delay(250);
    digitalWrite(greenLED, HIGH);
    delay(1000);
    digitalWrite(greenLED, LOW);
    digitalWrite(redLED, HIGH);
  }
  else if (tag[DATA_LENGTH] == FOUND && !authorized){
    //If tag is scanned and not granted access, flash red light    
    digitalWrite(redLED, LOW);
    delay(250);
    digitalWrite(redLED, HIGH);
    delay(250);
    digitalWrite(redLED, LOW);
    delay(250);
    digitalWrite(redLED, HIGH);
    delay(1000);
  }
}

//For sending commands to RFID module
//Specific commands and return values can be found in SM130 documentation
void send_command(int cmd)
{
  rfid.write(byte(255));
  rfid.write(byte(0));
  rfid.write(byte(1));
  rfid.write(byte(cmd));
  rfid.write(byte(cmd+1));
  delay(10);
}

//Get tag ID when one is found (ie. if first byte is 255)
int* parse()
{
  int tag[11] = { 0 };
  while(rfid.available()){
    if(rfid.read() == 255){
      for(int i=1;i<11;i++){
        tag[i]= rfid.read();
      }
    }
    rfid.flush();
  }
  return tag;
}

//Query server with tag and check response for authorization
boolean get_auth(int tag[])
{
  boolean authorized = false;
  //SM130 response contains a byte containing the length of the response data
  //Use this byte to determine if tag is found or not
  if (tag[DATA_LENGTH] == FOUND) {
    query(tag);
    if (response() == 200) {
      authorized = true;
    }
  }
  return authorized;
}

//Query creates a HTTP GET request using the inputted tag ID
void query(int tag[])
{
  char hex[] = { tag[8], tag[7], tag[6], tag[5] };
  String request = String("GET /id/") + String(tag[8], HEX) + String(tag[7], HEX) + String(tag[6], HEX) + String(tag[5], HEX) + String(" HTTP/1.0");
  //Stop any open server connections
  client.stop();
  //Attempt new connection
  if (client.connect(server, 80)) {
    // Make a HTTP request:
    client.println(request);
    client.println();
  } 
  else {
    // if you didn't get a connection to the server:
    Serial.println("connection failed");
  }
}

//Response parses the server's response, looking for the HTTP response code
//Possible values are currently 200 (OK) and 404 (Not Found)
int response()
{
  int pos = 0;
  char buffer[32];
  long int num = 0;
  boolean found = false;
  // check if there are incoming bytes available from the server
  while (client.available() && !found) {
    //Get each character until a space is found and add into buffer
    char c = client.read();
    if (c != ' ') {
      buffer[pos++] = c;
    }
    else {
      //When space found, end the string and attempt conversion to integer
      buffer[pos++] = '\o';
      num = strtol(buffer, NULL, 10);
      //If is integer, response code is found so flush the remaining characters and return the response code 
      if (num) {
        while (client.available()) {
          c = client.read();
        }
        return num;
      }
      //Reset buffer
      pos = 0;
    }
  }
}
