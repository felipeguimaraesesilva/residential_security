
#include <SoftwareSerial.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

String ARDUINO_ID = "ARDUINO_001";

bool ACTIVATE = false;
String lastCommand = "OK";
const int LED_ACTIVATE = 8;
const int LED_INACTIVATE = 9;

//GSM controll
SoftwareSerial sim(10, 11);
int _timeout;
String _buffer;
String number = "################";

//PresenceSensor
const int pinoPIR = 3;
const int pinoLED = 7;

//LCD
LiquidCrystal_I2C lcd(0x27, 20, 4);

void setup()
{
    initializeLCD();
    printLcd("Waiting for GSM Module...");
    delay(15000);
    Serial.begin(9600);
    _buffer.reserve(50);
    Serial.println("Sistem Started...");
    sim.begin(9600);
    delay(1000);
    sim.println("AT+CMGF=1");
    delay(1000);
    sim.println("AT+CNMI=1,2,0,0,0");

    pinMode(pinoLED, OUTPUT);
    pinMode(pinoPIR, INPUT);
    digitalWrite(LED_ACTIVATE, LOW);
    digitalWrite(LED_INACTIVATE, HIGH);
}

void loop()
{
    printLcd("checkNewCommand...");
    checkNewCommand();

    if (ACTIVATE)
    {
        printLcd("ACTIVATE...");
        checkSensorStatus();
    }

    delay(2000);
}

void checkNewCommand()
{
    String currentCommand = readSMSCommands();

    if (currentCommand.length() > 1)
    {
        lastCommand = currentCommand;
    }

    if (lastCommand.indexOf("1234") > 1)
    {
        digitalWrite(LED_ACTIVATE, HIGH);
        digitalWrite(LED_INACTIVATE, LOW);
        ACTIVATE = true;
        sendSMSMessage("ACTIVE");
    }

    if (lastCommand.indexOf("4321") > 1)
    {
        digitalWrite(LED_ACTIVATE, LOW);
        digitalWrite(LED_INACTIVATE, HIGH);
        ACTIVATE = false;
        sendSMSMessage("INACTIVE");
    }
}

void checkSensorStatus()
{
    if (digitalRead(pinoPIR) == HIGH)
    {
        digitalWrite(pinoLED, HIGH);
        sendSMSMessage("{\"arduino\":\"" + ARDUINO_ID + "\",\"sensorName\":\"presenceSensor\",\"status\":\"active\"}");
        delay(2000);
        digitalWrite(pinoLED, LOW);
        delay(10000);
    }
    else
    {
        digitalWrite(pinoLED, LOW);
        printLcd("PresenceNotDetected\n");
    }
}

void sendSMSMessage(String msg)
{
    printLcd("Sending Message");
    delay(1000);
    printLcd("Set SMS Number");
    sim.println("AT+CMGS=\"" + number + "\"\r");
    delay(1000);

    sim.println(msg);
    delay(100);
    sim.println((char)26);
    delay(1000);
}

String readSMSCommands()
{

    printLcd("Reading SMS messages...");
    if (sim.available())
    {
        String smsContent = sim.readString();
        Serial.println("Command received: " + smsContent);
        delay(5000);
        return smsContent;
    }
    else
    {
        return "";
    }
}

void initializeLCD()
{
    lcd.begin();
    lcd.backlight();
}

void printLcd(String message)
{
    lcd.clear();
    lcd.setCursor(1, 0);
    lcd.print(ACTIVATE ? "Active" : "Inactive");
    lcd.setCursor(1, 1);
    lcd.print(message);
    
}
