/*

TITRE          : Communication I2C ESP32 BMP280 RTC OLEC LCD
AUTEUR         : Franck Nkeubou Awougang
DATE           : 19/03/2022
DESCRIPTION    : Programme utilisant I2C comme protocol de communication entre les composantes.
VERSION        : 0.0.1

*/


#include <Arduino.h>

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "DFRobot_RGBLCD1602.h"
#include <Adafruit_BMP280.h>
#include "RTClib.h"

RTC_DS3231 rtc;

char Days[7][12] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};


#define SEALEVELPRESSURE_HPA (1013.25)


#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

/***************************************************************************
	This is a library for the BMP280 humidity, temperature & pressure sensor

	Designed specifically to work with the Adafruit BMP280 Breakout
	----> http://www.adafruit.com/products/2651

	These sensors use I2C or SPI to communicate, 2 or 4 pins are required
	to interface.

	Adafruit invests time and resources providing this open source code,
	please support Adafruit andopen-source hardware by purchasing products
	from Adafruit!

	Written by Limor Fried & Kevin Townsend for Adafruit Industries.
	BSD license, all text above must be included in any redistribution
 ***************************************************************************/


#define BMP_SCK  (13)
#define BMP_MISO (12)
#define BMP_MOSI (11)
#define BMP_CS   (10)

Adafruit_BMP280 bmp; // I2C
//Adafruit_BMP280 bmp(BMP_CS); // hardware SPI
//Adafruit_BMP280 bmp(BMP_CS, BMP_MOSI, BMP_MISO,  BMP_SCK);

// Déclaration de la classe LCD pour notre afficheur.
DFRobot_RGBLCD1602 lcd(/*lcdCols*/16,/*lcdRows*/2);  //16 characters and 2 lines of show 


void setup() {
	Serial.begin(9600);
	Serial.println(F("BMP280 Forced Mode Test."));

	//if (!bmp.begin(BMP280_ADDRESS_ALT, BMP280_CHIPID)) {
	if (!bmp.begin(0x76)) {
		Serial.println(F("Could not find a valid BMP280 sensor, check wiring or "
											"try a different address!"));
		while (1) delay(10);
	}

	/* Default settings from datasheet. */
	bmp.setSampling(Adafruit_BMP280::MODE_FORCED,     /* Operating Mode. */
									Adafruit_BMP280::SAMPLING_X2,     /* Temp. oversampling */
									Adafruit_BMP280::SAMPLING_X16,    /* Pressure oversampling */
									Adafruit_BMP280::FILTER_X16,      /* Filtering. */
									Adafruit_BMP280::STANDBY_MS_500); /* Standby time. */

	if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
		Serial.println(F("SSD1306 allocation failed"));
		for(;;);
	}

	// Ajout du code pour initialiser le RTC.
	#ifndef ESP8266
		while (!Serial); // wait for serial port to connect. Needed for native USB
	#endif

	if (! rtc.begin()) {
		Serial.println("Couldn't find RTC");
		Serial.flush();
		while (1) delay(10);
	}
	if (rtc.lostPower()) {
		Serial.println("RTC lost power, let's set the time!");
		// When time needs to be set on a new device, or after a power loss, the
		// following line sets the RTC to the date & time this sketch was compiled
		rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
		// This line sets the RTC with an explicit date & time, for example to set
		// January 21, 2014 at 3am you would call:
		// rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
  	}

	  /**
     *  @brief initialize the LCD and master IIC
     */ 
    lcd.init();
    // Print a first message to the LCD.
		lcd.clear();
		lcd.setRGB(200, 200, 200);
		lcd.setCursor(0,0);
		lcd.print("Getting Date...");
		lcd.setCursor(0,1);
		lcd.print("Getting Time...");

	delay(3000);
	// Initialisation de l'écran OLED.
	display.clearDisplay();

	display.setTextSize(2);
	display.setTextColor(INVERSE);

	display.setCursor(0, 0);
	// Display static text
	display.println("Initialisation....");
	display.display(); 
	
}

void loop() {
	// must call this to wake sensor up and get new measurement data
	// it blocks until measurement is complete
	DateTime TempsActuel = rtc.now();
	if (bmp.takeForcedMeasurement()) {
		lcd.clear();
		lcd.setRGB(200, 200, 200);
		lcd.setCursor(0,0);
		lcd.print(TempsActuel.year(), DEC);
		lcd.print('/');
		lcd.print(TempsActuel.month(), DEC);
		lcd.print('/');
		lcd.print(TempsActuel.day(), DEC);
		lcd.setCursor(0,1);
		lcd.print(" (");
		lcd.print(Days[TempsActuel.dayOfTheWeek()]);
		lcd.print(") ");
		lcd.print(TempsActuel.hour(), DEC);
		lcd.print(':');
		lcd.print(TempsActuel.minute(), DEC);
		lcd.print(':');
		lcd.print(TempsActuel.second(), DEC);
		// can now print out the new measurements
		display.clearDisplay();
		display.setCursor(0, 0);
		display.setTextSize(2);
		// Display static text
		display.print("T= ");
		display.print(bmp.readTemperature());
		display.print(" C\n");
		display.print("P= ");
		display.print(bmp.readPressure());
		display.print("Pa\n");
		display.print("Al= ");
		display.print(bmp.readAltitude(1013.25));
		display.print("m\n");

		display.display(); 

		delay(2000);
	} else {
		display.clearDisplay();
		display.setTextSize(2);
		display.print("Forced measurement failed !");
		display.display(); 
	}
}