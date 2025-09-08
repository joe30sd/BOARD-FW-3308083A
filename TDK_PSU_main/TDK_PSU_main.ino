//  V1.01        added ramp up 3 Min after relays Close   YB
//  ramp Up : Turn on Relay , 4 min delay, increase current to target ,  park, 2 min delay, close Relay , waith 3 min, Decrease current to zero
//
//
//
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_ADS1X15.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <Update.h>
HardwareSerial SerialPort(2);  // use UART2

const char* ssid = "mpsone";
const char* password = "mpsone1234";
 
#define RXD2 16
#define TXD2 17

#define RXD3 32
#define TXD3 33

#define AX_relay_pin 25
#define T1_relay_pin 26
#define T2_relay_pin 27
#define RDA_relay_pin 12
#define MAIN_relay_pin 14

const char* firmwareUrl = "https://raw.githubusercontent.com/byronin/rnn-bins/main/firmware2.bin";


unsigned long previousMillis = 0, pvm1 = 0, pvm2 = 0, pvm3 = 0, diag_pvm = 0;
const unsigned long interval = 60000;  // 1 dakika = 60,000 milisaniye
int minutesPassed = 0, old_value = 0, minutes2 = 0, dm4timer = 0;


float live_magnet_current = 0, live_magnet_voltage = 0, magnet_current = 0, magnet_voltage = 0;


float u_target_current, u_inc_current, u_voltage = 6.00, u_check = 0.8;
uint8_t up_mode = 0, up_done = 0;

float d_start_current, d_dec_current, d_voltage = 6.00, d_check = 0.9;
uint8_t d_mode = 0, d_done = 0;

uint8_t att_relay = 0, rda_relay = 0, main_relay = 0, rda_relay_dm = 0, display_mode = 0, get_out_number = 100;
String s_cmd;


String echo_killer1, st_echo_killer1, TDK_ID;

int index2, index3, ask_mode = 0, dm4_flag = 0, G_MODE = 0;

float v_ts = 0;
float Vin = 3.3;
float R1 = 2000.0;
int adcMaxValue = 17800;

int16_t adc0_1, adc1_1, adc2_1, adc3_1;
int16_t adc0_2, adc1_2, adc2_2, adc3_2;
boolean adc_ready = 0;
float att_current = 0, rda_current = 0, main_current = 0, att_voltage = 0, rda_voltage = 0, main_voltage = 0, R2 = 0;

int diag_x = 0, diag_y = 0, IDN_MODE = 0;
Adafruit_ADS1115 ads1;
Adafruit_ADS1115 ads2;
void setup() {
  // put your setup code here, to run once:

  Serial.begin(115200);
  delay(100);
  Serial1.begin(115200, SERIAL_8N1, RXD2, TXD2);  // Serial Out for TDK
  Serial2.begin(115200, SERIAL_8N1, RXD3, TXD3);  // Serial Out for Display

  if (!ads1.begin(0x48)) {
    Serial.println("Right ADS1115 not found!");  // Right ADS1115 for Res, Mv-, Mv+, ATT Current
    while (1)
      ;
  }
  if (!ads2.begin(0x49)) {
    Serial.println("Left ADS1115 not found!");  // Left ADS1115 for ATT Voltage, Main Voltage, Main Current, RDA Voltage
    while (1)
      ;
  }
  // Serial2.println(" Hello ");
  pinMode(AX_relay_pin, OUTPUT);
  pinMode(T1_relay_pin, OUTPUT);
  pinMode(T2_relay_pin, OUTPUT);
  pinMode(RDA_relay_pin, OUTPUT);
  pinMode(MAIN_relay_pin, OUTPUT);
  pinMode(35, INPUT);

  digitalWrite(AX_relay_pin, LOW);
  digitalWrite(T1_relay_pin, LOW);
  digitalWrite(T2_relay_pin, LOW);
  digitalWrite(RDA_relay_pin, LOW);
  digitalWrite(MAIN_relay_pin, LOW);


  // Set TDK Out ON
  Set_Out(0);
}

void loop() {
  if (display_mode == 0) {
    minutesPassed = 0;
    old_value = 0;
    minutes2 = 0;
    d_mode = 0;
    d_done = 0;
    up_mode = 0;
    up_done = 0;
    live_magnet_current = 0;
    live_magnet_voltage = 0;
    magnet_current = 0;
    magnet_voltage = 0;
    d_start_current = 0;
    d_dec_current = 0;
    d_voltage = 6.00;
    d_check = 0.9;
    u_target_current = 0;
    u_inc_current = 0;
    u_voltage = 6.00;
    u_check = 0.8;
    att_current = 0;
    rda_current = 0;
    main_current = 0;
    att_voltage = 0;
    rda_voltage = 0;
    main_voltage = 0;
    att_relay = 0;
    rda_relay = 0;
    rda_relay_dm = 0;
    main_relay = 0;
    adc_ready = 0;
    Set_Out(0);
    dm4_flag = 0;
    G_MODE = 0 ;
  Serial2.print("AV");
  Serial2.println(att_voltage);
  Serial2.print("CV");
  Serial2.println(main_voltage);
  Serial2.print("RV");
  Serial2.println(rda_voltage);
  Serial2.print("RA");
  delay(50);
  }

  att_current = 0;
  rda_current = 0;
  main_current = 0;

  unsigned long currentMillis = millis();  // Timer for main time.
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    minutesPassed++;  // timer variable for operation
    minutes2++;
  }

  if (adc_ready == 1) {  //  Control by operation D1, D2 & D3

    Get_adc_values();  // Getting all ADS values

    float Vout = (adc0_1 * Vin) / adcMaxValue;  // Res calculation
    R2 = (Vout * R1) / (Vin - Vout);

    if (adc1_1 <= 0 && adc2_1 > 0) {  // Mv- calculation
      adc1_1 = adc1_1 * -1;

      v_ts = adc1_1 + adc2_1;
      v_ts = (v_ts / 920);
    } else if (adc1_1 > 0 && adc2_1 <= 0) {  // Mv+ calculation
      adc2_1 = adc2_1 * -1;

      v_ts = adc1_1 + adc2_1;
      v_ts = (v_ts / 920);
    }


    att_voltage = adc0_2;  // ATT relay input voltage calculation
    att_voltage = (att_voltage / 920) + 0.2;

    main_voltage = adc1_2;  // Main relay input voltage calculation
    main_voltage = (main_voltage / 920) + 0.2;

    rda_voltage = adc3_2;  //  RDA relay input voltage calculation
    rda_voltage = (rda_voltage / 920) + 0.2;


    if (att_relay > 0) {
      att_current = adc3_1;  // ATT relay input voltage calculation
      att_current = (att_current / 920) + 0.2;
      att_current = att_current - 13.40;
    } else {
      att_current = 0;
    }

    if (main_relay > 0) {
      main_current = adc2_2;  // Main relay input voltage calculation
      main_current = (main_current / 920) + 0.2;
      main_current = main_current - 12.30;
    } else {
      main_current = 0;
    }




    if (rda_relay > 0) {
      rda_current = (analogRead(35) / 4095.0) * 3.3;  //  RDA relay input voltage calculation
      rda_current = rda_current * (11.0 / 10.0);
      rda_current = (rda_current - 2.5) / 0.185;
      rda_current = rda_current + 2.0;
    } else {
      rda_current = 0;
    }



    //Serial.print("rda_current");
    // Serial.println(rda_current);
    //Serial.print("att_current");
    //Serial.println(att_current);
    //Serial.print("main_current");
    // Serial.println(main_current);
  }



  if (currentMillis - pvm2 >= 250 && display_mode == 2) {
    pvm2 = currentMillis;
    Get_power_values();  // Power request for TDK
    adc_ready = 1;       // assent for reading & calculation for ads side
    Send_values();       // Sending all data to display
  }
  if (currentMillis - pvm2 >= 250 && display_mode == 3) {
    pvm2 = currentMillis;
    Get_power_values();  // Power request for TDK
    adc_ready = 1;       // assent for reading & calculation for ads side
    Send_values();       // Sending all data to display
  }
  if (currentMillis - pvm1 >= 250 && display_mode == 1) {
    pvm1 = currentMillis;
    Set_Relay(att_relay, main_relay, rda_relay);
    Get_power_values();  // Power request for TDK
    adc_ready = 1;
    Send_values();  // Sending all data to display
  }
  if (currentMillis - pvm1 >= 250 && display_mode == 5 && diag_x > 20) {

    pvm1 = currentMillis;
    Get_power_values();  // Power request for diagnostic
    if (millis() - diag_pvm < 7000 && diag_x == 26 && live_magnet_current > 740) {
      diag_x = 27;
    } else if (millis() - diag_pvm > 7000 && diag_x == 26 && live_magnet_current < 740) {
      diag_x = 28;
    }
    Serial.print(live_magnet_current);
    if (millis() - diag_pvm > 7000) {
      if (live_magnet_current < 1 && live_magnet_voltage > 5.8 && diag_x == 21) {
        Serial2.print("YD");
        Serial2.println(24);
        diag_x = 0;
        diag_pvm = 0;
        Set_CV(0.0, 0.0);
        Set_Out(0);
      }
    }
  }

  if (old_value == 0) {
    Set_CV(magnet_current, magnet_voltage);
    old_value = 1;
  }


  /* // put your main code here, to run repeatedly:
    magnet_current = magnet_current + 0.5;
    magnet_voltage = magnet_voltage + 0.3;

    if (magnet_current < 2.00) {
      att_relay = 1;
      main_relay = 1 ;
      rda_relay = 0 ;

     } else if (magnet_current < 4.00) {
      att_relay = 2;
      main_relay = 0;
      rda_relay = 1;
     } else if (magnet_current < 6.00) {
      att_relay = 3;
      main_relay = 1;
      rda_relay = 1;
     } else if (magnet_current < 6.00) {
      att_relay = 0;
      main_relay = 0;
      rda_relay = 0;
     }

    //Set_Relay(att_relay, main_relay, rda_relay);
    Set_CV(magnet_current, magnet_voltage);
    delay(1000);
    Get_power_values();
    //Get_relay_values();
    Send_values();
    if (magnet_current > 10.00) {
     magnet_current = 2.00;

    }
    if (magnet_voltage > 10.00) {
     magnet_voltage = 2.60;
    }*/




  if (Serial1.available() > 0) {  // Reading data from TDK

    if (IDN_MODE == 1) {
      TDK_ID = Serial1.readString();
      Serial.print("1:");
      Serial.println(TDK_ID);
      if (TDK_ID.startsWith("TDK")) {
        IDN_MODE = 0;
        Serial2.print("ID");
        Serial2.println(TDK_ID);
        delay(100);
        Serial2.print("YD");
        Serial2.println(19);
        diag_x = 0;

      } else if (TDK_ID.startsWith("*IDN") || TDK_ID.startsWith("IDN")) {
        Serial.print("2:");
        Serial.println(TDK_ID);
        if (TDK_ID.indexOf("TDK") > 0) {
          IDN_MODE = 0;
          Serial2.print("ID");
          Serial2.println(TDK_ID);
          delay(100);
          Serial2.print("YD");
          Serial2.println(19);
          diag_x = 0;
        }
      } else {
        IDN_MODE = 0;
        Serial2.print("YD");
        Serial2.println(17);
        diag_x = 0;
      }
    } else {
      echo_killer1 = Serial1.readString();
    }
    Serial.println(echo_killer1);
    index2 = fastIndexOf(echo_killer1, "MEAS:CURR?");  // own string indexof method, echo_killer temp variable for TDK
    index3 = fastIndexOf(echo_killer1, "MEAS:VOLT?");
    if (ask_mode == 0 && index2 <= 0) {
      live_magnet_current = echo_killer1.toFloat();

    } else if (ask_mode == 1 && index3 <= 0) {
      live_magnet_voltage = echo_killer1.toFloat();
    }

    if (index2 >= 0 && fastIndexOf(echo_killer1, ".") >= 0) {
      echo_killer1 = echo_killer1.substring(index2 + 12, index2 + 19);
      live_magnet_current = echo_killer1.toFloat();  // reading current from tdk
    } else if (index3 >= 0 && fastIndexOf(echo_killer1, ".") >= 0) {
      echo_killer1 = echo_killer1.substring(index3 + 12, index3 + 18);
      live_magnet_voltage = echo_killer1.toFloat();  // redaing voltage from tdk
    }
    if (index3 == -1 && index2 == -1) {
      st_echo_killer1 = Serial1.readString();  // I forgot what exactly this but no problem if you have any problem about reading data from tdk check here, debug it.
    }
  }


  if (Serial2.available() > 0) {  // Reading data from display
    s_cmd = Serial2.readStringUntil('\n');

    if (s_cmd.startsWith("D")) {
      display_mode = s_cmd.substring(1).toInt();  // display mode 0 = nothing, 1 = manual mode, 2 = ramp up, 3 = ramp down
      //Serial2.println(display_mode);
      if (display_mode == 1) {
        Set_Out(1);
      }
    }
    if (display_mode == 5) {
      if (s_cmd.startsWith("X")) {
        Serial2.println("");
        diag_x = s_cmd.substring(1).toInt();
        Set_Out(0);
      }
    }
   if (s_cmd.startsWith("G") ) {
       G_MODE = s_cmd.substring(1).toInt();
       if(G_MODE == 1){
        Set_Relay(0, 0, 0);
        Set_CV(0.0, 0.0);
       }
      }
    if (display_mode == 1) {
      if (s_cmd.startsWith("V")) {
        magnet_voltage = s_cmd.substring(1).toFloat();
        old_value = 0;
      } else if (s_cmd.startsWith("C")) {
        magnet_current = s_cmd.substring(1).toFloat();
        old_value = 0;
      } else if (s_cmd.startsWith("A")) {
        att_relay = s_cmd.substring(1).toInt();
      } else if (s_cmd.startsWith("M")) {
        main_relay = s_cmd.substring(1).toInt();
      } else if (s_cmd.startsWith("R")) {
        rda_relay = s_cmd.substring(1).toInt();
      }
    }

    if (display_mode == 2) {
      if (s_cmd.startsWith("T")) {
        u_target_current = s_cmd.substring(1).toFloat();
        // Serial2.println(u_target_current);
      } else if (s_cmd.startsWith("I")) {
        u_inc_current = s_cmd.substring(1).toFloat();
        //Serial2.println(u_inc_current);
      } else if (s_cmd.startsWith("U")) {
        up_mode = s_cmd.substring(1).toInt();
        //Serial2.println(up_mode);
      } else if (s_cmd.startsWith("A")) {
        att_relay = s_cmd.substring(1).toInt();
        //Serial2.println(att_relay );
      } else if (s_cmd.startsWith("M")) {
        main_relay = s_cmd.substring(1).toInt();
        //Serial2.println(main_relay);
      } else if (s_cmd.startsWith("R")) {
        rda_relay = s_cmd.substring(1).toInt();
        //Serial2.println(rda_relay);
      } else if (s_cmd.startsWith("V") && up_mode == 6) {
        magnet_voltage = s_cmd.substring(1).toFloat();
      } else if (s_cmd.startsWith("C") && up_mode == 6) {
        magnet_current = s_cmd.substring(1).toFloat();
      }else if (s_cmd.startsWith("G") ) {
       G_MODE = s_cmd.substring(1).toInt();
       if(G_MODE == 1){
        Set_Relay(0, 0, 0);
        Set_CV(0.0, 0.0);
       }
      }
    }


    if (display_mode == 3) {
      if (s_cmd.startsWith("T")) {
        d_start_current = s_cmd.substring(1).toFloat();
        // Serial2.println(u_target_current);
      } else if (s_cmd.startsWith("I")) {
        d_dec_current = s_cmd.substring(1).toFloat();
        //Serial2.println(u_inc_current);
      } else if (s_cmd.startsWith("U")) {
        d_mode = s_cmd.substring(1).toInt();
        //Serial2.println(up_mode);
      } else if (s_cmd.startsWith("A")) {
        att_relay = s_cmd.substring(1).toInt();
        //Serial2.println(att_relay );
      } else if (s_cmd.startsWith("M")) {
        main_relay = s_cmd.substring(1).toInt();
        main_relay = 0;
        //Serial2.println(main_relay);
      } else if (s_cmd.startsWith("R")) {
        rda_relay_dm = s_cmd.substring(1).toInt();
        rda_relay = 0;
        //Serial2.println(rda_relay);
      }
    }
  }
  if (display_mode == 5) {
    adc_ready = 1;
    if (diag_pvm > 0) {
      if (millis() - diag_pvm > 20000) {
        if (diag_x == 16) {
          IDN_MODE = 0;
          Serial2.print("YD");
          Serial2.println(16);
          diag_x = 0;
          diag_pvm = 0;
          Set_CV(0.0, 0.0);
          Set_Out(0);
        } else if (millis() - diag_pvm > 20000) {
          Serial2.print("YD");
          Serial2.println(21);
          diag_x = 0;
          diag_pvm = 0;
          Set_CV(0.0, 0.0);
          Set_Out(0);
        }
      }
    }

    if (diag_x == 40) {
      Serial2.print("YD");
      Serial2.println(40);
      diag_x = 0;
    } else if (diag_x == 1) {
      diag_x = 0;
      delay(1000);
      Set_Relay(1, 0, 0);
      delay(1000);
      Set_Relay(2, 0, 0);
      delay(1000);
      Set_Relay(3, 0, 0);
      delay(1000);
      Set_Relay(0, 0, 0);
      if (att_voltage > 5) {
        Serial2.print("YD");
        Serial2.println(4);
      } else {
        Serial2.print("YD");
        Serial2.println(3);
      }



    } else if (diag_x == 5) {
      diag_x = 0;
      delay(1000);
      Set_Relay(0, 1, 0);
      delay(1000);
      Set_Relay(0, 0, 0);
      if (main_voltage > 5) {
        Serial2.print("YD");
        Serial2.println(9);
      } else {
        Serial2.print("YD");
        Serial2.println(7);
      }
    } else if (diag_x == 10) {
      diag_x = 0;
      delay(1000);
      Set_Relay(0, 0, 1);
      delay(1000);
      Set_Relay(0, 0, 0);
      if (rda_voltage > 5) {
        Serial2.print("YD");
        Serial2.println(14);
      } else {
        Serial2.print("YD");
        Serial2.println(12);
      }
    } else if (diag_x == 15) {
      Serial1.println("*IDN?");
      IDN_MODE = 1;
      diag_pvm = millis();
      diag_x = 16;
    } else if (diag_x == 20) {
      delay(1000);
      Set_CV(7.50, 6.0);
      Set_Out(1);
      diag_x = 21;
      diag_pvm = millis();
    } else if (diag_x == 25) {
      Set_Out(1);
      delay(1000);
      Set_CV(30.0, 6.0);
      delay(1000);
      Set_CV(80.0, 6.0);
      delay(1000);
      Set_CV(150.0, 6.0);
      delay(1000);
      Set_CV(220.0, 6.0);
      delay(1000);
      Set_CV(290.0, 6.0);
      delay(1000);
      Set_CV(360.0, 6.0);
      delay(1000);
      Set_CV(420.0, 6.0);
      delay(1000);
      Set_CV(480.0, 6.0);
      delay(1000);
      Set_CV(540.0, 6.0);
      delay(1000);
      Set_CV(630.0, 6.0);
      delay(1000);
      Set_CV(700.0, 6.0);
      delay(1000);
      Set_CV(750.0, 6.0);
      delay(200);
      diag_x = 26;
      diag_pvm = millis();
    } else if (diag_x == 27) {
      Set_CV(700.0, 6.0);
      delay(1000);
      Set_CV(630.0, 6.0);
      delay(1000);
      Set_CV(560.0, 6.0);
      delay(1000);
      Set_CV(490.0, 6.0);
      delay(1000);
      Set_CV(420.0, 6.0);
      delay(1000);
      Set_CV(350.0, 6.0);
      delay(1000);
      Set_CV(280.0, 6.0);
      delay(1000);
      Set_CV(210.0, 6.0);
      delay(1000);
      Set_CV(140.0, 6.0);
      delay(1000);
      Set_CV(70.0, 6.0);
      delay(1000);
      Set_CV(0.0, 0.0);
      delay(1000);
      diag_x = 0;
      diag_pvm = 0;
      Set_Out(0);
      Serial2.print("YD");
      Serial2.println(29);
    } else if (diag_x == 28) {
      Set_CV(700.0, 6.0);
      delay(1000);
      Set_CV(630.0, 6.0);
      delay(1000);
      Set_CV(560.0, 6.0);
      delay(1000);
      Set_CV(490.0, 6.0);
      delay(1000);
      Set_CV(420.0, 6.0);
      delay(1000);
      Set_CV(350.0, 6.0);
      delay(1000);
      Set_CV(280.0, 6.0);
      delay(1000);
      Set_CV(210.0, 6.0);
      delay(1000);
      Set_CV(140.0, 6.0);
      delay(1000);
      Set_CV(70.0, 6.0);
      delay(1000);
      Set_CV(0.0, 0.0);
      delay(1000);
      diag_x = 0;
      diag_pvm = 0;
      Set_Out(0);
      Serial2.print("YD");
      Serial2.println(27);
    } else if(diag_x == 50){
     performOTAUpdate();
    }
  }


  if (display_mode == 2) {

    if (up_mode == 1) {
      minutes2 = 0;
      Set_CV(0.0, 0.0);
      up_done = 1;
      Set_Out(1);
    } else if (up_mode == 2) {
      Set_Relay(att_relay, main_relay, rda_relay);
      previousMillis = currentMillis;
      minutesPassed = 0;
      up_done = 2;
    } else if (up_mode == 3) {



      if (minutesPassed > 3) {                 // change this 3 to 4 YB  this is where current starts incressing . after 4 mins.   
        magnet_current = 2.00;
        magnet_voltage = u_voltage;
        up_done = 3;
        Set_CV(magnet_current, magnet_voltage);
        live_magnet_current = 2.00;
      }
    } else if (up_mode == 4) {

      //Serial2.println(magnet_current - live_magnet_current);
      //Serial2.println(u_inc_current * u_check);

      if ((magnet_current - live_magnet_current) <= u_inc_current * u_check) {
        magnet_current = magnet_current + u_inc_current;
        Set_CV(magnet_current, magnet_voltage);
      }
      if (u_target_current - (u_inc_current * u_check) <= magnet_current) {
        magnet_current = u_target_current;
        Set_CV(magnet_current, magnet_voltage);
        up_done = 4;
      }

    } else if (up_mode == 5) {

    } else if (up_mode == 6) {
      Set_CV(magnet_current, magnet_voltage);
    } else if (up_mode == 7) {
      up_done = 7;
      previousMillis = currentMillis;
      minutesPassed = 0;
    } else if (up_mode == 8 && minutesPassed > 1) {    //change 1 to 3  . this is before relays are open no need to wait 3 min
      att_relay = 0;
      main_relay = 0;
      rda_relay = 0;
      Set_Relay(0, 0, 0);
      if(minutesPassed > 3){    //change 3 to 4  this is where axial and main still open . no need to wait 4 minutes
        up_done = 8;
        delay(60000);  // added YB to test 3 Min   // REACHED 3 MIN GOAL
      }
      
    } else if (up_mode == 9) {
      magnet_current = magnet_current - (u_target_current / 50);
      Set_CV(magnet_current, magnet_voltage);
      if (magnet_current <= 3.00) {
        magnet_current = 0;
        magnet_voltage = 0;
        Set_CV(magnet_current, magnet_voltage);
        Serial2.print("LV");
        Serial2.println(0.0);
        Serial2.print("LC");
        Serial2.println(0.0);


        up_done = 9;
      }
    }
  }

  if (display_mode == 3) {

    if (d_mode == 1) {
      rda_relay = rda_relay_dm;
      Set_Relay(0, 0, rda_relay);
      rda_relay_dm = 0;
      magnet_current = d_start_current;
      magnet_voltage = d_voltage;
      Set_CV(magnet_current, magnet_voltage);
      Set_CV(magnet_current, magnet_voltage);
      Set_Out(1);
      previousMillis = currentMillis;
      minutesPassed = 0;
      minutes2 = 0;
      d_mode = 2;

    } else if (d_mode == 2 ) {
       if (minutesPassed > 0) {
        d_mode = 3;
        main_relay = 1;
        Set_Relay(att_relay, main_relay, rda_relay);
        minutesPassed = 0;
        Set_CV(magnet_current, magnet_voltage);
        Set_Out(1);

        }
        


    } else if (d_mode == 3) {

      if (minutesPassed > 2) {   // change 2 to 3 YB  not here either
        d_mode = 4;
        
        Serial2.println("DM4");
      }


    } else if (d_mode == 4) {
      Serial.print("LC= ");
      Serial.println(live_magnet_current);
      Serial.print("MC= ");
      Serial.println(magnet_current);
      Serial.print("DC= ");
      Serial.println(d_dec_current);
      Serial.print("CC= ");
      Serial.println(d_dec_current * d_check);


      if ((live_magnet_current - magnet_current) <= d_dec_current * d_check) {
        magnet_current = magnet_current - d_dec_current;
        Set_CV(magnet_current, magnet_voltage);
      }
      if (live_magnet_current <= 0 || magnet_current <= 0) {
        d_mode = 5;
       dm4timer = minutesPassed ;
       Set_Relay(0, 0, 0);
      }
    } else if (d_mode == 5) {
      if (minutesPassed >  dm4timer + 2) {
        d_mode = 0;
        Set_Relay(0, 0, 0);
        Set_CV(0, 0);
        delay(1000);
        Serial2.println("DM5");
      }
    } else if (d_mode == 8) {
        delay(60000);    // added YB  to increase delay to 3 min.addming delay ddint help with 3 min goal either     . REACHED 3 MIN GOAL
    }
  }
}




void Set_Out(boolean pw_out) {  // TDK Out ON-OFF command
  if (pw_out == 1) {
    Serial1.println("INST:NSEL 6");
    delay(100);
    Serial1.println("OUTPut ON");
  } else {
    Serial1.println("INST:NSEL 6");
    delay(100);
    Serial1.println("OUTPut OFF");
  }
}

void Set_CV(float s_current, float s_voltage) {
  Serial1.print("SOUR:CURR ");
  Serial1.println(s_current);
  Serial1.print("SOUR:VOLT ");
  Serial1.println(s_voltage);
}

void Set_Relay(uint8_t att_value, uint8_t main_value, uint8_t rda_value) {

  if (att_value == 0) {
    digitalWrite(AX_relay_pin, LOW);
    digitalWrite(T1_relay_pin, LOW);
    digitalWrite(T2_relay_pin, LOW);
  } else if (att_value == 1) {
    digitalWrite(AX_relay_pin, HIGH);
    digitalWrite(T1_relay_pin, LOW);
    digitalWrite(T2_relay_pin, LOW);
  } else if (att_value == 2) {
    digitalWrite(AX_relay_pin, LOW);
    digitalWrite(T1_relay_pin, HIGH);
    digitalWrite(T2_relay_pin, LOW);
  } else if (att_value == 3) {
    digitalWrite(AX_relay_pin, LOW);
    digitalWrite(T1_relay_pin, LOW);
    digitalWrite(T2_relay_pin, HIGH);
  }

  if (main_value == 1) {
    digitalWrite(MAIN_relay_pin, HIGH);
  } else {
    digitalWrite(MAIN_relay_pin, LOW);
  }

  if (rda_value == 1) {
    digitalWrite(RDA_relay_pin, HIGH);
  } else {
    digitalWrite(RDA_relay_pin, LOW);
  }
}

int fastIndexOf(const String &str, const String &toFind) {
  int strLen = str.length();
  int findLen = toFind.length();

  if (findLen == 0) return -1;  // Aranacak dize boşsa -1 döndür

  for (int i = 0; i <= strLen - findLen; i++) {
    int j = 0;
    while (j < findLen && str[i + j] == toFind[j]) {
      j++;
    }
    if (j == findLen) return i;  // Alt dize bulundu
  }
  return -1;  // Alt dize bulunamadı
}

void Get_power_values() {  // Sending request to TDK for active current & voltage


  if (ask_mode == 0) {
    Serial1.println("MEAS:VOLT?");
    ask_mode = 1;
  } else {
    ask_mode = 0;
    Serial1.println("MEAS:CURR?");
  }
  /* int get_out_timer = 0; // Timer başlat
    String echo_killer, st_echo_killer;
    boolean get_data_mode = 1 ;
    int index1;
    while (get_data_mode == 1) {
     if (Serial1.available() > 0) {
       echo_killer = Serial1.readString();
       index1 = fastIndexOf(echo_killer, "MEAS:VOLT?");
       if (index1 >= 0 && fastIndexOf(echo_killer, ".") >= 0) {
         echo_killer = echo_killer.substring(index1 + 12, index1 + 18);
         live_magnet_voltage = echo_killer.toFloat();
         get_data_mode = 0;
       } else {
         st_echo_killer = Serial1.parseFloat();
       }
     }

     delay(1); // Küçük bir gecikme ekle, seri bağlantı hızlı olabilir
     get_out_timer++; // Timer'ı artır

     if (get_out_timer >= get_out_number) {
       get_data_mode = 0; // get_current_mode'u 0 yap
     }
    }

    Serial1.println("MEAS:CURR?");  // Voltaj ölçüm komutunu gönder
    get_out_timer = 0; // Timer başlat
    get_data_mode = 1;
    while (get_data_mode == 1) {
     if (Serial1.available() > 0) {
       echo_killer = Serial1.readString();
       index1 = fastIndexOf(echo_killer, "MEAS:CURR?");
       if (index1 >= 0 && fastIndexOf(echo_killer, ".") >= 0) {
         echo_killer = echo_killer.substring(index1 + 12, index1 + 19);
         live_magnet_current = echo_killer.toFloat();
         get_data_mode = 0;
       } else {
         st_echo_killer = Serial1.parseFloat();
       }
     }

     delay(1); // Küçük bir gecikme ekle, seri bağlantı hızlı olabilir
     get_out_timer++; // Timer'ı artır

     if (get_out_timer >= get_out_number) {
       get_data_mode = 0; // get_current_mode'u 0 yap
     }
    }
  */
}


/*void Get_power_values() {
  Serial1.println("MEAS:VOLT?");  // Voltaj ölçüm komutunu gönder
  int get_out_timer = 0; // Timer başlat
  String echo_killer, st_echo_killer;
  boolean get_data_mode = 1 ;
  int index1;
  while (get_data_mode == 1  ) {
    if (Serial1.available() > 0) {
      echo_killer = Serial1.readString();
      index1 = echo_killer.indexOf("MEAS:VOLT?");
      if (index1 >= 0 && echo_killer.indexOf(".") >= 0 ) {
        echo_killer = echo_killer.substring(index1 + 12, index1 + 18);
        live_magnet_voltage = echo_killer.toFloat();
        get_data_mode = 0;
      } else {
        st_echo_killer = Serial1.parseFloat();
      }

    }

    delay(1); // Küçük bir gecikme ekle, seri bağlantı hızlı olabilir
    get_out_timer++; // Timer'ı artır

    if (get_out_timer >= 500) {
      get_data_mode = 0; // get_current_mode'u 0 yap

    }
  }


  Serial1.println("MEAS:CURR?");  // Voltaj ölçüm komutunu gönder
  get_out_timer = 0; // Timer başlat
  get_data_mode = 1 ;
  while (get_data_mode == 1  ) {
    if (Serial1.available() > 0) {
      echo_killer = Serial1.readString();
      index1 = echo_killer.indexOf("MEAS:CURR?");
      if (index1 >= 0 && echo_killer.indexOf(".") >= 0  ) {
        echo_killer = echo_killer.substring(index1 + 12, index1 + 19);
        live_magnet_current = echo_killer.toFloat();
        get_data_mode = 0;
      } else {
        st_echo_killer = Serial1.parseFloat();
      }

    }

    delay(1); // Küçük bir gecikme ekle, seri bağlantı hızlı olabilir
    get_out_timer++; // Timer'ı artır

    if (get_out_timer >= 500) {
      get_data_mode = 0; // get_current_mode'u 0 yap

    }
  }
  }
*/
void Get_adc_values() {
  adc0_1 = ads1.readADC_SingleEnded(0);
  adc1_1 = ads1.readADC_SingleEnded(1);
  adc2_1 = ads1.readADC_SingleEnded(2);
  adc3_1 = ads1.readADC_SingleEnded(3);

  adc0_2 = ads2.readADC_SingleEnded(0);
  adc1_2 = ads2.readADC_SingleEnded(1);
  adc2_2 = ads2.readADC_SingleEnded(2);
  adc3_2 = ads2.readADC_SingleEnded(3);
  v_ts = 0;
}
void Send_values() {
  Serial2.print("MV");
  Serial2.println(magnet_voltage);
  Serial2.print("MC");
  Serial2.println(magnet_current);
  Serial2.print("LV");
  Serial2.println(live_magnet_voltage);
  Serial2.print("LC");
  Serial2.println(live_magnet_current);
  Serial2.print("AV");
  Serial2.println(att_voltage);
  Serial2.print("CV");
  Serial2.println(main_voltage);
  Serial2.print("RV");
  Serial2.println(rda_voltage);
  Serial2.print("RA");
  Serial2.println(att_relay);
  Serial2.print("RM");
  Serial2.println(main_relay);
  Serial2.print("RR");
  Serial2.println(rda_relay);
  Serial2.print("TP");
  Serial2.println(minutes2);
  Serial2.print("UP");
  Serial2.println(up_mode);
  Serial2.print("UD");
  Serial2.println(up_done);
  Serial2.print("OR");
  Serial2.println(R2);
  Serial2.print("OV");
  Serial2.println(v_ts);
}

void performOTAUpdate() {
  // Step 1: Connect to WiFi
    Serial2.print("YD");
    Serial2.println(51);
  Serial.println("Connecting to WiFi...");
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi");

  // Step 2: Download and apply firmware
  HTTPClient http;
  
  // Initialize HTTP client with the firmware URL
  http.begin(firmwareUrl);
  
  // Send GET request
  int httpCode = http.GET();
  
  if (httpCode == HTTP_CODE_OK) { // HTTP 200
    int contentLength = http.getSize();
    
    if (contentLength > 0) {
      // Prepare OTA update with the firmware size
      bool canBegin = Update.begin(contentLength);
      
      if (canBegin) {
        Serial.println("Starting OTA update...");
        WiFiClient* stream = http.getStreamPtr();
        
        // Write the firmware stream to the Update library
        size_t written = Update.writeStream(*stream);
        
        if (written == contentLength) {
          Serial.println("Written : " + String(written) + " bytes successfully");
        } else {
          Serial.println("Written only : " + String(written) + "/" + String(contentLength) + " bytes");
          http.end();
          return;
        }
        
        // Finalize the update
        if (Update.end()) {
          Serial.println("OTA update completed!");
              Serial2.print("YD");
            Serial2.println(52);
          if (Update.isFinished()) {
            Serial.println("Update successful. Rebooting...");
            ESP.restart(); // Restart ESP32 to boot into new firmware
          } else {
            Serial.println("Update not finished. Something went wrong!");
          }
        } else {
          Serial.println("Update error: #" + String(Update.getError()));
          Serial2.print("YD");
          Serial2.println(53);
        }
      } else {
        Serial.println("Not enough space to begin OTA update");
        Serial2.print("YD");
          Serial2.println(53);
      }
    } else {
      Serial.println("Firmware size is invalid (<= 0)");
    }
  } else {
    Serial.println("HTTP error: " + String(httpCode));
    Serial2.print("YD");
     Serial2.println(53);
  }
  
  // Clean up
  http.end();
}
