// ***************************************************************************
// *                   PROTOSS AUTOMATIC WATER SAMPLER                       *
// * Copyright (C)2020 PT PROTOSS TEKNO CIPTA, TANGERANG. All right reserved *
// *                    web: http://www.protoss.id/                          *
// ***************************************************************************
//
// This program was made to run on serial modules with a screen 
// resolution of 320x240 pixels.
// 
// This program requires the UTFT, URTouch, Servo, and NewPing Ultrasonic library.
//
// IMPORTANT: Read the comments in the setup()function and global variables.

// ***      PRE-PROCESSOR DIRECTIVES     ***   
// Include all the necessary libraries
#include <UTFT.h>
#include <URTouch.h>
#include <Servo.h>
#include <NewPing.h>

// Define HC04 Ultrasonic Pins and MAX DISTANCE
#define TRIGGER 12
#define ECHO 11
#define MAX_DISTANCE 400      //Maximum Distance of the Ultrasonic Sensor
#define SENSOR_START 40UL     //The Distance (cm) where the sensor measurement will start, this number will be decremented each time by the amount of distanceSet

// ***    GLOBAL VARIABLES AND SETUP     ***

//Initiate the Ultrasonic Module
NewPing sonar(TRIGGER, ECHO, MAX_DISTANCE);

//Initiate the servo
Servo servo;

//Initiate LCD pins
UTFT    myGLCD(ILI9341_16,38,39,40,41);
URTouch  myTouch( 6, 5, 4, 3, 2);

// Declare which fonts type
extern uint8_t BigFont[];
extern uint8_t SmallFont[];
extern uint8_t SevenSegNumFont[];

//Initiate timer variables
unsigned long previousMillis = 0;  
unsigned long currentMillis ;

//initiate pumping interval
unsigned long interval = 1000;                        //Default value set to 1000, this need to be calibrated depending on your application
unsigned long multiplier = (1000*8);

//Declare global variables
int x, y;                                   // Touch coordinates
unsigned long timeSet = 5;                  // timer based parameter (menit)
int bottleCounter = 0;                      // initial Servo angle
int volume = 10;                            // initial volume to fill
char currentPage, currentMethod;            // navigation parameters
int start = 0;                              // for toggling start/stop button

//Make an array and sum
unsigned long distanceMean[1000];
unsigned long sum;
unsigned long EMA_S;
unsigned long EMA_a = 0.3;

//Declare Distance read from the ultrasonic sensor and distance set by the user
unsigned long distanceRead;                 // for reading the Ultrasonic sensor value
unsigned long distanceSave;
unsigned long distanceSet;                  // distance set by the user
unsigned long distanceInc;                  // for incrementing the distanceSet

void setup()
{
// Initialize LCD touchscreen
  myGLCD.InitLCD();
  myGLCD.clrScr();
  myTouch.InitTouch();
  myTouch.setPrecision(PREC_MEDIUM);

// initialize LCD touchscreen Font and Font Color
  myGLCD.setFont(BigFont);
  myGLCD.setBackColor(0, 0, 255);

//initialize the distanceInc and distanceSet variable
  distanceSet = 5;
  distanceInc = distanceSet;

// Call the draw HomeScreen function and set the current page to zero
  drawHomeScreen();
  currentPage = '0';

// Set the sampling method option to 0 (by reading ultrasonic sensor value)
  currentMethod = '0';

//Set the l298n motor driver pins
  pinMode(8, OUTPUT);
  pinMode(9, OUTPUT);
  pinMode(10, OUTPUT);

//Set the servo pin at pin 13 and set the value to zero
  servo.attach(13);
  servo.write(9);
  delay(1000);

  distanceSave = sonar.ping_cm();
}

void loop() {
      
switch (currentPage){
  case '0':
      touchHomeMenu();
      break;
  case '1':
      touchMulaiSampling();
      break;
  case '2':
      touchAturMetode();
      break;
  case '3':
      touchAturVolume();
      break;
}

  if(start == 1 && currentMethod == '1'){                               //START SAMPLING WITH TIMER METHOD
      previousMillis = millis();
      
     while(bottleCounter < 6){
        currentMillis = millis();                                       //activate timer
        
        if(currentMillis - previousMillis >= ((timeSet*1000UL*60UL)+((unsigned long)(interval * volume)))){
          digitalWrite(10,HIGH);                                        // Set the DC motor driver to HIGH
          digitalWrite(9,LOW);
          digitalWrite(8,255);
          currentMillis = millis();
          previousMillis = currentMillis;
          
          while(currentMillis - previousMillis < (unsigned long) (multiplier * volume)){
              currentMillis = millis();
              //myGLCD.print("pumpin", 105, 195);
                }
        digitalWrite(8,0);                                              //Turn off the pump
        bottleCounter += 1;
        servoAngleSet();
        myGLCD.clrScr();
        drawMulaiSampling();
        }
      if (myTouch.dataAvailable()) {
      myTouch.read();
      x=myTouch.getX();                                                 // X coordinate where the screen has been pressed
      y=myTouch.getY();                                                 // Y coordinates where the screen has been pressed

      //Stop and Start Button
      if ((x>=205) && (x<=305) && (y>=185) && (y<=225)) 
      {
        drawFrame(205, 185, 305, 225);
        myGLCD.clrScr();
        start ^= 1;
        drawMulaiSampling();
        break;
      }
      }
    }

  }
  else if(start == 1 && currentMethod == '0'){                          //START SAMPLING WITH TIMER METHOD

    while(bottleCounter < 6){
      //averageRead();                                                  //average the sensor reading data up to 50 data
      //delay(50);
      distanceRead = sonar.ping_cm();
     for(int i = 0; i<100; i++){
        delay(50);  
        //distanceRead = sonar.ping_cm();
      if(distanceRead < (distanceSave - 2)){
          distanceRead = sonar.ping_cm();
      while(distanceRead < 1){
        delay(50); 
      distanceRead = sonar.ping_cm();
      }
        }
        }
  distanceSave = distanceRead;
      
      if(distanceRead <= (((SENSOR_START+distanceSet) - distanceInc))){
        distanceInc = distanceInc + distanceSet;

        digitalWrite(10,HIGH);                                        // Set the DC motor driver to HIGH
        digitalWrite(9,LOW);
        digitalWrite(8,255);
        currentMillis = millis();
        previousMillis = currentMillis;

        while(currentMillis - previousMillis < (unsigned long)(multiplier * volume)){
          currentMillis = millis();
          //myGLCD.print("pumpin", 105, 195);
        }
      digitalWrite(8,0);                                              //Turn off the pump
      bottleCounter += 1;
      servoAngleSet();
      myGLCD.clrScr();
      drawMulaiSampling();
      }
      
      if (myTouch.dataAvailable()) {
      myTouch.read();
      x=myTouch.getX();                               // X coordinate where the screen has been pressed
      y=myTouch.getY();                               // Y coordinates where the screen has been pressed

      //Stop and Start Button
      if ((x>=205) && (x<=305) && (y>=185) && (y<=225)) 
      {
        drawFrame(205, 185, 305, 225);
        myGLCD.clrScr();
        start ^= 1;
        drawMulaiSampling();
        break;
      }

      }


    }                                               //while loop
    
  }                                                 //else if


}                                                   //void loop()


void averageRead(){
  sum = 0;
  for(int i = 0; i < 1000; i++){
  distanceMean[i] = sonar.ping_cm();
  while(distanceMean[i] < 1){
    distanceMean[i] = sonar.ping_cm();
  }
  sum = sum + distanceMean[i];
  }
  distanceRead = (sum/1000);   
}

// ***      Touch Screen Mechanics Functions      ***

void touchHomeMenu(){                               // Touch mechanics for the Home Menu

    if (myTouch.dataAvailable()) {
      myTouch.read();                               // Read the touch data
      x=myTouch.getX();                             // X coordinate where the screen has been pressed
      y=myTouch.getY();                             // Y coordinates where the screen has been pressed
      
      // If We Chose the Mulai Sampling menu 
      if ((x>=35) && (x<=285) && (y>=90) && (y<=130)) {
        drawFrame(35, 90, 285, 130);                 // Custom Function - Highlighs the buttons when it's pressed
        currentPage = '1';                           // Current page number for Sampling Menu
        myGLCD.clrScr();                             // Clears the screen
        drawMulaiSampling();                         // Call the sampling() function
      }
      
      // If We Chose the Atur Metode menu
      if ((x>=35) && (x<=285) && (y>=140) && (y<=180)) { 
        drawFrame(35, 140, 285, 180);                 //Button Highlight Function
        currentPage = '2';                            //Current page number for Parameter Menu
        myGLCD.clrScr();                              // Clears the screen
        drawAturMetode();                             // Call the parameter() function
      }  
      // If We Chose the Atur Volume menu
      if ((x>=35) && (x<=285) && (y>=190) && (y<=230)) {
        drawFrame(35, 190, 285, 230);                 //Button Highlight Function
        currentPage = '3';                            //Current page number for Parameter Menu
        myGLCD.clrScr();                              // Clears the screen
        drawAturVolume();                             // Call the kalibrasi() function
      }
    }
}           // end of touchHomeMenu() function

void touchMulaiSampling(){
  // Touch mechanics for Sampling menu (Mulai Sampling)
  
    if (myTouch.dataAvailable()) {
      myTouch.read();
      x=myTouch.getX();                               // X coordinate where the screen has been pressed
      y=myTouch.getY();                               // Y coordinates where the screen has been pressed

      //Stop and Start Button
      if ((x>=205) && (x<=305) && (y>=185) && (y<=225)) 
      {
        drawFrame(205, 185, 305, 225);
        myGLCD.clrScr();
        start ^= 1;
        drawMulaiSampling();
      }
      //Back to Home Menu (Menu Button)
      if ((x>=5) && (x<=105) && (y>=185) && (y<=225)) 
      {
        drawFrame(5, 185, 105, 225);
        currentPage = '0';
        myGLCD.clrScr();
        drawHomeScreen();
      }
    }
}

void touchAturMetode(){
                
    if (myTouch.dataAvailable()) {        // Touch mechanics for Atur Metode menu
      myTouch.read();
      x=myTouch.getX();                   // X coordinate where the screen has been pressed
      y=myTouch.getY();                   // Y coordinates where the screen has been pressed
      drawIncDecMetode();                 //Call the drawIncDecMetode() function
      
      
      if ((x>=5) && (x<=105) && (y>=185) && (y<=225))       //Back to Home Menu (Menu Button)
      {
        drawFrame(5, 185, 105, 225);
        currentPage = '0';
        myGLCD.clrScr();
        drawHomeScreen();
      }

     
      if ((x>=205) && (x<=305) && (y>=185) && (y<=225))                          //OKE Button 
      {
        drawFrame(205, 185, 305, 225);
        currentPage = '1';
        myGLCD.clrScr();
        drawMulaiSampling();
      }
      
      
      if ((x>=35) && (x<=135) && (y>=35) && (y<=75))                        //Sensor Option Button
      {
        drawFrame(35, 35, 135, 75);
        currentMethod = '0';
        drawIncDecMetode();
      }

      
      if ((x>=175) && (x<=275) && (y>=35) && (y<=75))                       //Time Option Button
      {
        drawFrame(175, 35, 275, 75);
        currentMethod = '1';
        drawIncDecMetode();
      }


   // Setting Increment - Decrement Navigation Buttons
   

       if ((x>=95) && (x<=135) && (y>=120) && (y<=165))       // If User push the LEFT arrow
      {
        drawFrame(95, 120, 135, 165);
        if (currentMethod == '0')
        {
          distanceSet=distanceSet-1;
          if (distanceSet <= 1)
          {
            distanceSet = 1;
          }
        }
        else if (currentMethod == '1')
        {
          timeSet=timeSet-1;
          if (timeSet <= 1)
          {
            timeSet = 1;
          }
        }
        drawIncDecMetode();
      }
     //RIGHT ARROW
      if ((x>=205) && (x<=245) && (y>=120) && (y<=165))       // If User push the RIGHT arrow
      {
        drawFrame(205, 120, 245, 165);
        if (currentMethod == '0')
        {
          distanceSet=distanceSet+1;
          if (distanceSet >= 20)
          {
            distanceSet = 20;
          }
        }
        else if (currentMethod == '1')
        {
        timeSet=timeSet+1;
          if (timeSet >= 30)
          {
            timeSet = 30;
          }
        }
        drawIncDecMetode();
      }
    }
      //distance_set = z_max;            please remove this later
}

void touchAturVolume(){                       // Touch mechanics for Atur Volume menu
  
      drawIncDecVolume();                     //This function will draw the increment - decrement navigation user interface
    if (myTouch.dataAvailable()) {
   
      myTouch.read();
      x=myTouch.getX();                       // X coordinate where the screen has been pressed
      y=myTouch.getY();                       // Y coordinates where the screen has been pressed

      
      
      if ((x>=5) && (x<=105) && (y>=185) && (y<=225))  // Back to Home Menu (Menu Button)
      {
        drawFrame(5, 185, 105, 225);
        currentPage = '0';
        myGLCD.clrScr();
        drawHomeScreen();
      }
      
      if ((x>=205) && (x<=305) && (y>=185) && (y<=225))     // OKE Button 
      {
        drawFrame(205, 185, 305, 225);
        currentPage = '1';
        myGLCD.clrScr();
        drawMulaiSampling();
      }
      

   // Setting Increment - Decrement Navigation Buttons
   

    if ((x>=55) && (x<=95) && (y>=70) && (y<=115))          // If User push the LEFT arrow
      {
        drawFrame(55, 70, 95, 115);
        volume = volume -10;
        if (volume <= 10)
          {
            volume = 10;
          }
      }
      

      if ((x>=215) && (x<=255) && (y>=70) && (y<=115))      // if User push the RIGHT arrow
      {
        drawFrame(215, 70, 255, 115);
          volume = volume + 10;
          if (volume >= 100)
          {
            volume = 100;
          }
        }
     
      }
}

//*** UPDATING VARIABLES VALUE TO THE "MULAI SAMPLING" SCREEN ***

void servoAngleSet(){
  switch (bottleCounter){
  case 1:
      servo.write(22);
      delay(1000);
      break;
  case 2:
      servo.write(34);
      delay(1000);
      break;
  case 3:
      servo.write(48);
      delay(1000);
      break;
  case 4:
      servo.write(62);
      delay(1000);
      break;
  case 5:
      servo.write(77);
      delay(1000);
      break;
  default:                               //Original Position First Bottle
      servo.write(9);
      delay(1000);
      break;
}
}                                       // end of servoAngleSet() function

//*** UPDATING VARIABLES VALUE TO THE "MULAI SAMPLING" SCREEN ***
void drawUpdateValue(){

  myGLCD.print("ml", 20, 95);                    // print ml for the volume
  myGLCD.setBackColor(0, 0, 0);

  if (currentMethod == '0')                       // Check wether the user has chosen the "sensor" method or the "timer" method
  {
  myGLCD.print("Sensor", 100, 55);                // print "cm" for sensor method
  myGLCD.print("cm", 125, 95);     
  }
  else
  {
  myGLCD.print("Waktu ", 115, 55);                // print "menit" for timer method
  myGLCD.print("menit", 125, 95);   
  }

  // Setting up seven segment font
  myGLCD.setColor(0, 255, 0);
  myGLCD.setFont(SevenSegNumFont);
 

if (currentMethod == '0')                                     // Check wether the user has chosen the "sensor" method or the "timer" method
  {
    myGLCD.printNumI(distanceSet, 110, 115, 2, '0');          // print the ultrasonic sensor value for sensor method
  }
  else
  {
    myGLCD.printNumI(timeSet, 110, 115, 2, '0');             // print the timer value for timer method
  }
  
  myGLCD.printNumI(420, 5, 115, 2, '0');
  myGLCD.printNumI(bottleCounter, 220, 115, 2, '0');

}

//****      DRAW LEFT-RIGHT NAVIGATION MENU FUNCTION      ***

//Atur Metode Increment-Decrement Buttons
void drawIncDecMetode()                                               // Equivalent to drawIncDecVolume() for "ATUR VOLUME" 
{
  myGLCD.setColor(0, 0, 255);
  myGLCD.fillRoundRect (95, 120, 135, 165);
  myGLCD.fillRoundRect (205, 120, 245, 165);
  myGLCD.setColor(0, 255, 0);
  myGLCD.drawRoundRect (95, 120, 135, 165);
  myGLCD.drawRoundRect (205, 120, 245, 165);
  myGLCD.setFont(BigFont);
  myGLCD.setBackColor(0, 0, 255);
  myGLCD.print("<", 110, 135);
  myGLCD.print(">", 210, 135);

  if (currentMethod == '0')
  {
  myGLCD.setBackColor(0, 0, 0);
  myGLCD.print("  cm ", 125, 165);
  myGLCD.setBackColor(0, 0, 0);
  myGLCD.setColor(0, 255, 0);
  myGLCD.setFont(SevenSegNumFont);
  myGLCD.printNumI(distanceSet, 140, 115, 2, '0');
  }
  
  else if (currentMethod == '1')
  {
  myGLCD.setBackColor(0, 0, 0);
  myGLCD.print("menit", 125, 165);
  myGLCD.setBackColor(0, 0, 0);
  myGLCD.setColor(0, 255, 0);
  myGLCD.setFont(SevenSegNumFont);
  myGLCD.printNumI(timeSet, 140, 115, 2, '0');
  
  }
}

//Atur Volume Increment-Decrement Buttons
void drawIncDecVolume()                                           // Equivalent to drawIncDecMetode() for "ATUR METODE" 
{
  myGLCD.setColor(0, 0, 255);
  myGLCD.fillRoundRect (55, 70, 95, 115);
  myGLCD.fillRoundRect (215, 70, 255, 115);
  myGLCD.setColor(0, 255, 0);
  myGLCD.drawRoundRect (55, 70, 95, 115);
  myGLCD.drawRoundRect (215, 70, 255, 115);
  myGLCD.setFont(BigFont);
  myGLCD.setBackColor(0, 0, 255);
  myGLCD.print("<", 60, 85);
  myGLCD.print(">", 220, 85);

  myGLCD.setBackColor(0, 0, 0);
  myGLCD.print("  ml ", 95, 115);
  myGLCD.setBackColor(0, 0, 0);
  myGLCD.setColor(0, 255, 0);
  myGLCD.setFont(SevenSegNumFont);
  myGLCD.printNumI(volume, 100, 65, 3, '0');

}
//****      DRAW HOME SCREEN MENU FUNCTION      ***
void drawHomeScreen() {
  // Clear Previous Screen
  myGLCD.clrScr();
  // Title - PROTOSS WATER SAMPLER
  myGLCD.setBackColor(0,0,0); // Sets the background color of the area where the text will be printed to black
  myGLCD.setColor(255, 255, 255); // Sets color to white
  myGLCD.setFont(BigFont); // Sets font to big
  myGLCD.print("PROTOSS WATER SAMPLE", CENTER, 10); // Prints the string on the screen
  myGLCD.setColor(0, 0, 255); // Sets color to blue
  myGLCD.drawLine(0,32,319,32); // Draws the BLUE line
  myGLCD.setColor(255, 255, 255); // Sets color to white
  myGLCD.setFont(SmallFont); // Sets the font to small
  myGLCD.print("Your trusted partner for water sampling", CENTER, 41); // Prints the string
  myGLCD.setFont(BigFont);
  myGLCD.print("Pilih Menu", CENTER, 64);
  
  // Button - MULAI SAMPLING
  myGLCD.setColor(16, 107, 163); // Sets green color
  myGLCD.fillRoundRect (35, 90, 285, 130); // Draws filled rounded rectangle
  myGLCD.setColor(0, 0, 255); // Sets color to blue
  myGLCD.drawRoundRect (35, 90, 285, 130); // Draws rounded rectangle without a fill, so the overall appearance of the button looks like it has a frame
  myGLCD.setColor(255, 255, 255);
  myGLCD.setFont(BigFont); // Sets the font to big
  myGLCD.setBackColor(16, 107, 163); // Sets the background color of the area where the text will be printed to green, same as the button 
  myGLCD.print("MULAI SAMPLING", CENTER, 102); // Prints the string
  
  // Button - ATUR METODE
  myGLCD.setColor(16, 107, 163);
  myGLCD.fillRoundRect (35, 140, 285, 180);
  myGLCD.setColor(0, 0, 255);
  myGLCD.drawRoundRect (35, 140, 285, 180);
  myGLCD.setColor(255, 255, 255);
  myGLCD.setFont(BigFont);
  myGLCD.setBackColor(16, 107, 163);
  myGLCD.print("ATUR METODE", CENTER, 152);
  
  // Button - ATUR VOLUME
  myGLCD.setColor(16, 107, 163);
  myGLCD.fillRoundRect (35, 190, 285, 230);
  myGLCD.setColor(0, 0, 255);
  myGLCD.drawRoundRect (35, 190, 285, 230);
  myGLCD.setColor(255, 255, 255);
  myGLCD.setFont(BigFont);
  myGLCD.setBackColor(16, 107, 163);
  myGLCD.print("ATUR VOLUME", CENTER, 202);
}

//****      DRAW MULAI SAMPLING USER INTERFACE      ***

void drawMulaiSampling(){
  // Clear Previous Screen
  myGLCD.clrScr();
  
  //Write Title
  myGLCD.setBackColor(0,0,0);                             // Sets the background color of the area where the text will be printed to black
  myGLCD.setColor(255, 255, 255);                         // Sets color to white
  myGLCD.setFont(BigFont);                                // Sets font to big
  myGLCD.print("PROTOSS WATER SAMPLE", CENTER, 10);       // Prints the string on the screen
  myGLCD.setColor(0, 0, 255);                             // Sets color to blue
  myGLCD.drawLine(0,32,319,32);                           // Draws the blue line
  
  //Write Column Identifiers
  myGLCD.setColor(255, 255, 255);
  myGLCD.print("Volum", 5, 35);
  myGLCD.print("Air", 15, 55);
  myGLCD.print("Botol", 215, 35);
  myGLCD.print("Terisi", 205, 55);
  myGLCD.print("Metode", 100, 35);

  //Draw the Menu Button
  myGLCD.setColor(16, 167, 103);
  myGLCD.fillRoundRect (5, 185, 105, 225);
  myGLCD.setColor(0, 0, 255);
  myGLCD.drawRoundRect(5, 185, 105, 225);
  myGLCD.setBackColor(16, 167, 103);
  myGLCD.setColor(255, 255, 255);
  myGLCD.print("Menu", 15, 195);
  myGLCD.setBackColor(0, 0, 0);

  //Draw the Start,Stop, or Pumping
  if(start == 1 && currentMethod == '0'){
      
  myGLCD.setColor(16, 167, 103);
  myGLCD.fillRoundRect (205, 185, 305, 225);
  myGLCD.setColor(0, 0, 255);
  myGLCD.drawRoundRect (205, 185, 305, 225);
  myGLCD.setBackColor(16, 167, 103);
  myGLCD.setColor(255, 255, 255);
  myGLCD.print("Stop", 215, 195);
  myGLCD.setBackColor(0, 0, 0);
  myGLCD.print("run...", 105, 195);
  }
  else if (start == 1 && currentMethod == '1')                                              // Used to Check wether the User has pushed the "Mulai" Button or not
  {
  
  myGLCD.setColor(16, 167, 103);
  myGLCD.fillRoundRect (205, 185, 305, 225);
  myGLCD.setColor(0, 0, 255);
  myGLCD.drawRoundRect (205, 185, 305, 225);
  myGLCD.setBackColor(16, 167, 103);
  myGLCD.setColor(255, 255, 255);
  myGLCD.print("Stop", 215, 195);
  myGLCD.setBackColor(0, 0, 0);
  myGLCD.print("run...", 105, 195);
}
  else
  {
  myGLCD.setColor(16, 167, 103);
  myGLCD.fillRoundRect(205, 185, 305, 225);
  myGLCD.setColor(0, 0, 255);
  myGLCD.drawRoundRect(205, 185, 305, 225);
  myGLCD.setBackColor(16, 167, 103);
  myGLCD.setColor(255, 255, 255);
  myGLCD.print("Mulai", 215, 195);
  myGLCD.setBackColor(0, 0, 0);
  myGLCD.print("stoped", 105, 195);
  digitalWrite(10,HIGH);
  digitalWrite(9,LOW);
  digitalWrite(8,0);
  delay(1000);
  }

  drawUpdateValue();
}

//****      DRAW ATUR METODE USER INTERFACE      ***

void drawAturMetode()
{
  // Clear Previous Screen
  myGLCD.clrScr();
  
  myGLCD.setBackColor(0, 0, 0);
  myGLCD.setColor(255, 255, 255);
  myGLCD.print("Pilih Metode Sampling", 0, 0);
  myGLCD.setColor(0, 0, 255);
  myGLCD.drawLine(0,15,319,15);

  myGLCD.setColor(16, 167, 103);
  myGLCD.fillRoundRect (5, 185, 105, 225);
  myGLCD.setColor(0, 0, 255);
  myGLCD.drawRoundRect (5, 185, 105, 225);
  myGLCD.setBackColor(16, 167, 103);
  myGLCD.setColor(255, 255, 255);
  myGLCD.print("Menu", 15, 195);
  myGLCD.setBackColor(0, 0, 0);

  myGLCD.setColor(16, 167, 103);
  myGLCD.fillRoundRect (205, 185, 305, 225);
  myGLCD.setColor(0, 0, 255);
  myGLCD.drawRoundRect (205, 185, 305, 225);
  myGLCD.setBackColor(16, 167, 103);
  myGLCD.setColor(255, 255, 255);
  myGLCD.print("Oke", 215, 195);
  myGLCD.setBackColor(0, 0, 0);

  myGLCD.setColor(16, 167, 103);
  myGLCD.fillRoundRect (35, 35, 135, 75);
  myGLCD.setColor(0, 0, 255);
  myGLCD.drawRoundRect (35, 35, 135, 75);
  myGLCD.setBackColor(16, 167, 103);
  myGLCD.setColor(255, 255, 255);
  myGLCD.print("Sensor", 38, 45);
  myGLCD.setBackColor(0, 0, 0);

  myGLCD.setColor(16, 167, 103);
  myGLCD.fillRoundRect (175, 35, 275, 75);
  myGLCD.setColor(0, 0, 255);
  myGLCD.drawRoundRect (175, 35, 275, 75);
  myGLCD.setBackColor(16, 167, 103);
  myGLCD.setColor(255, 255, 255);
  myGLCD.print("Waktu", 185, 45);
  myGLCD.setBackColor(0, 0, 0);
}

//****      DRAW ATUR VOLUME USER INTERFACE      ***

void drawAturVolume(){

  // Clear Previous Screen
  myGLCD.clrScr();
  
  myGLCD.setBackColor(0, 0, 0);
  myGLCD.setColor(255, 255, 255);
  myGLCD.print("Atur Volume", CENTER, 0);
  myGLCD.setColor(0, 0, 255);
  myGLCD.drawLine(0,15,319,15);

  myGLCD.setColor(16, 167, 103);
  myGLCD.fillRoundRect (5, 185, 105, 225);
  myGLCD.setColor(0, 0, 255);
  myGLCD.drawRoundRect (5, 185, 105, 225);
  myGLCD.setBackColor(16, 167, 103);
  myGLCD.setColor(255, 255, 255);
  myGLCD.print("Menu", 15, 195);
  myGLCD.setBackColor(0, 0, 0);

  myGLCD.setColor(16, 167, 103);
  myGLCD.fillRoundRect (205, 185, 305, 225);
  myGLCD.setColor(0, 0, 255);
  myGLCD.drawRoundRect (205, 185, 305, 225);
  myGLCD.setBackColor(16, 167, 103);
  myGLCD.setColor(255, 255, 255);
  myGLCD.print("Oke", 215, 195);
  myGLCD.setBackColor(0, 0, 0);

}

// ***      DRAW FRAME FUNCTION     ***

void drawFrame(int x1, int y1, int x2, int y2)            // This function used to draw a frame on a button's border
{
  myGLCD.setColor(255, 255, 255);
  myGLCD.drawRoundRect (x1, y1, x2, y2);       
  while (myTouch.dataAvailable())
  myTouch.read();
  myGLCD.setColor(0, 0, 255);
  myGLCD.drawRoundRect (x1, y1, x2, y2);
}
