/* Sweep
 by BARRAGAN <http://barraganstudio.com>
 This example code is in the public domain.

 modified 8 Nov 2013
 by Scott Fitzgerald

 modified for the ESP32 on March 2017
 by John Bennett

 see http://www.arduino.cc/en/Tutorial/Sweep for a description of the original code

 * Different servos require different pulse widths to vary servo angle, but the range is 
 * an approximately 500-2500 microsecond pulse every 20ms (50Hz). In general, hobbyist servos
 * sweep 180 degrees, so the lowest number in the published range for a particular servo
 * represents an angle of 0 degrees, the middle of the range represents 90 degrees, and the top
 * of the range represents 180 degrees. So for example, if the range is 1000us to 2000us,
 * 1000us would equal an angle of 0, 1500us would equal 90 degrees, and 2000us would equal 1800
 * degrees.
 * 
 * Circuit: (using an ESP32 Thing from Sparkfun)
 * Servo motors have three wires: power, ground, and signal. The power wire is typically red,
 * the ground wire is typically black or brown, and the signal wire is typically yellow,
 * orange or white. Since the ESP32 can supply limited current at only 3.3V, and servos draw
 * considerable power, we will connect servo power to the VBat pin of the ESP32 (located
 * near the USB connector). THIS IS ONLY APPROPRIATE FOR SMALL SERVOS. 
 * 
 * We could also connect servo power to a separate external
 * power source (as long as we connect all of the grounds (ESP32, servo, and external power).
 * In this example, we just connect ESP32 ground to servo ground. The servo signal pins
 * connect to any available GPIO pins on the ESP32 (in this example, we use pin 18.
 * 
 * In this example, we assume a Tower Pro MG995 large servo connected to an external power source.
 * The published min and max for this servo is 1000 and 2000, respectively, so the defaults are fine.
 * These values actually drive the servos a little past 0 and 180, so
 * if you are particular, adjust the min and max values to match your needs.
 */

#include <ESP32Servo.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>

Servo servoP;  // create servo object to control a servo
// 16 servo objects can be created on the ESP32

// int pos = 0;    // variable to store the servo position
// Recommended PWM GPIO pins on the ESP32 include 2,4,12-19,21-23,25-27,32-33 
// Possible PWM GPIO pins on the ESP32-S2: 0(used by on-board button),1-17,18(used by on-board LED),19-21,26,33-42
// Possible PWM GPIO pins on the ESP32-S3: 0(used by on-board button),1-21,35-45,47,48(used by on-board LED)
// Possible PWM GPIO pins on the ESP32-C3: 0(used by on-board button),1-7,8(used by on-board LED),9-10,18-21

int servoPinP = 26;
int adcP = 35;
int button = 5;
int adcR = 32;

int correctionMin = 0;
int correctionMax = 0;
int angleMax = 180;
int angleMin = 0;
int mode = 0;
int PMax = 60;
int PMin = 15;
int RMax = 40;
int RMin = 12;

int pValue;
portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;
int num = 0;
int step = 0;
int angleValue;
int rValue;
int T;
int tH, tT;
int dH, dT;
int I = 1, E = 2;
Adafruit_PCD8544 display = Adafruit_PCD8544(18, 23, 4, 15, 2);
int contrastValue = 60; // Default Contrast Value


void IRAM_ATTR buttonPush() {
  portENTER_CRITICAL_ISR(&mux);//Phần quan trọng cần được bảo vệ khỏi mọi truy cập đồng thời để thay đổi nó
  delayMicroseconds(50000);
  Serial.println("Button Pushed!!!");
  
  if (num < 3)
  {
	num++;
  } else {
	num = 0;
  }
  
  portEXIT_CRITICAL_ISR(&mux);//Cho phép tiếp tục chạy các task khác
}


void setup() {
	pinMode(button, INPUT_PULLUP);
  // cài đặt ngắt vào chân button, kiểu ngắt là falling (xung xuống), hàm gọi khi có sự kiện ngắt là button push
  attachInterrupt(digitalPinToInterrupt(button), buttonPush, FALLING);
	analogSetWidth(12);
	// Allow allocation of all timers
	// ESP32PWM::allocateTimer(0);
	// ESP32PWM::allocateTimer(1);
	// ESP32PWM::allocateTimer(2);
	// ESP32PWM::allocateTimer(3);
	// myservo.setPeriodHertz(50);    // standard 50 hz servo
	servoP.attach(servoPinP); // attaches the servo on pin 18 to the servo object
	Serial.begin(9600);
	/* Initialize the Display*/
  display.begin();

//   /* Change the contrast using the following API*/
  display.setContrast(contrastValue);

//   /* Clear the buffer */
  display.clearDisplay();
  display.display();
//   delay(1000);

  
}

void loop() {

	// for (pos = 0; pos <= 180; pos += 1) { // goes from 0 degrees to 180 degrees
	// 	// in steps of 1 degree
	// 	myservo.write(pos);    // tell servo to go to position in variable 'pos'
	// 	delay(5);             // waits 15ms for the servo to reach the position
	// }
	// delay(200);
	// for (pos = 180; pos >= 0; pos -= 1) { // goes from 180 degrees to 0 degrees
	// 	myservo.write(pos);    // tell servo to go to position in variable 'pos'
	// 	delay(5);             // waits 15ms for the servo to reach the position
	// }
	// delay(200);
	
		
		// read values of bien tro
		//dieu chinh ap suat
		pValue = analogRead(adcP);
		pValue = pValue / (4096 / (PMax - PMin));
		if (pValue==0)
		{
			pValue=1;
		}
		
		step = (angleMax - angleMin) / (PMax - PMin);
		angleValue = pValue * step;
		pValue += PMin;
		//dieu chinh chu ky
		rValue = analogRead(adcR);
		rValue /= (4096 / (RMax - RMin));
		rValue += RMin;

		T = 60000 / rValue;

	if(num != 0) {
		if(num == 1) {
			I = 1;
			E = 2;
		} else if (num == 2)
		{
			I = 1;
			E = 3;
		} else if (num == 3)
		{
			/* code */
			I = 1;
			E = 4;
		}
		//tinh toan thoi gian hit va tho theo chu ky tho
		
		tH = T / (I + E) * I;
		tT = T / (I + E) * E;
		dH = tH / (angleValue/step);
		dT = tT / (angleValue / step);
		//dieu khien servo
		for(int i = angleMin; i <= angleValue; i += step) {
			servoP.write(i);
			delay(dH);
		}
		for(int i = angleValue; i >= angleMin; i -= step) {
			servoP.write(i);
			delay(dT);
		}

		// display.clearDisplay();
		// display.setTextColor(WHITE, BLACK);
		// display.setCursor(0,1);
		// display.setTextSize(1);
		// display.println("|ESP32|");
		// display.setTextSize(1);
		// display.setTextColor(BLACK);
		// display.setCursor(10,15);
		// display.print("Mode: ");
		// display.println(num);
		// display.setCursor(10,22);
		// display.print("R: ");
		// display.println(rValue);
		// display.setCursor(10,29);
		// display.print("P: ");
		// display.println(pValue);
		// display.setCursor(10,36);
		// display.print("I:E ");
		// display.print(I);
		// display.print(":");
		// display.println(E);
		// display.display();
		

		// Serial.println(step);
		Serial.println(num);
		Serial.println(rValue);
		Serial.println(pValue);
		Serial.println(T);
		Serial.print(I);
		Serial.print(":");
		Serial.println(E);
		// delay(1000);
	}
	else {
		
		Serial.println(num);
		Serial.println(rValue);
		Serial.println(pValue);
		Serial.println(T);
		Serial.print(I);
		Serial.print(":");
		Serial.println(E);
		// delay(2000);
	}
	
	display.clearDisplay();
	display.setTextColor(WHITE, BLACK);
	display.setCursor(0,1);
	display.setTextSize(2);
	display.println("|Nhom|");
	display.setTextSize(1);
	display.setTextColor(BLACK);
	display.setCursor(10,20);
	display.print("Mode: ");
	if(num == 0) {
		display.println("off");
	} else {
		display.print(I);
		display.print(":");
		display.println(E);
	}
	
	display.setCursor(10,30);
	display.print("R: ");
	display.println(rValue);
	display.setCursor(50,30);
	display.print("P: ");
	display.println(pValue);
	display.setCursor(10,40);
	display.print("T: ");
	display.println(T);
	display.display();
	delay(1000);
}

