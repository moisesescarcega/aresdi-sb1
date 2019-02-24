#include <SPI.h>
#include <SD.h>
#include <Wire.h>
#include <EEPROM.h>
#include <DHT11.h>
#include <LiquidCrystal_I2C.h>
#include "RTClib.h"
//#include "U8glib.h"
//U8GLIB_SH1106_128X64 u8g(U8G_I2C_OPT_NONE);
LiquidCrystal_I2C lcd(0x3F,20,4);
RTC_DS3231 rtc;
int pinHT = 2;
DHT11 dht11(pinHT);
const int rele0 = 9;
const int rele1 = 6;
const int sen1 = 14;
const int sen2 = 15;
const int sen3 = 16;
const int sen4 = 17;
const int sen5 = 5;
const int alarm = 4;
const int buretardop = 7;
const int bdretardop = 8;
int estadobup = 0;
int estadobdown = 0;
float temp, hum;
unsigned long tprevio = 0;
unsigned long tespera = 0;
unsigned long tdelay = 1000;
unsigned long tdelay2 = 900;
byte multip = 2;
bool everificado = 0;
bool muxValue1;
bool muxValue2;
bool muxValue3;
bool muxValue4;
bool muxValue5;
bool vaciadesde;
byte nivelH;
bool sbomba1;
bool sbomba2;
byte albomba;
int anio;
byte mes;
byte dia;
byte hora;
byte minuto;
byte segundo;
int vhora;
int vminuto;
int addr = 0;
byte retardop;
byte vretardo;
byte cdelay;
byte cdelayG;
int esled = LOW;
byte numavisos = 0;
const byte avisoslim = 5;
File miArchivo;
byte ccS1[] = {
  B01110,
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
  B01110,
  B00000
};
byte ccS0[] = {
  B01010,
  B10001,
  B00000,
  B10001,
  B00000,
  B10001,
  B01010,
  B00000
};
byte ccBL[] = {
  B00111,
  B01111,
  B11110,
  B11100,
  B11110,
  B01111,
  B00111,
  B00000
};
byte ccBR[] = {
  B11100,
  B11110,
  B01111,
  B00111,
  B01111,
  B11110,
  B11100,
  B00000
};
void inicial(void) {
  muxValue1 = digitalRead(sen1);
  muxValue2 = digitalRead(sen2);
  muxValue3 = digitalRead(sen3);
  muxValue4 = digitalRead(sen4);
  muxValue5 = digitalRead(sen5);
}
int regtime(int tipo) {
  dht11.read(hum, temp);
  miArchivo = SD.open("REG.CSV", FILE_WRITE);
  if (miArchivo) {
    miArchivo.print(anio, DEC);
    miArchivo.print('/');
    miArchivo.print(mes, DEC);
    miArchivo.print('/');
    miArchivo.print(dia, DEC);
    miArchivo.print(", ");
    miArchivo.print(hora, DEC);
    miArchivo.print(':');
    miArchivo.print(minuto, DEC);
    miArchivo.print(':');
    miArchivo.print(segundo, DEC);
    miArchivo.print(", ");
    miArchivo.print(temp, 0);
    miArchivo.print(", ");
    miArchivo.print(hum, 0);
    miArchivo.print(", ");
    miArchivo.println(tipo);
    miArchivo.close();
  }
}
void cambiabomba() {
  if (albomba == 3) {
    sbomba1 = true;
    sbomba2 = false;
    albomba = 1;
    regtime(1);
  } else if (albomba == 1) {
    sbomba1 = true;
    sbomba2 = false;
    regtime(1);
  } else if (albomba == 2) {
    sbomba1 = false;
    sbomba2 = true;
    regtime(2);
  }
}
void setup() {
  pinMode(sen1, INPUT);
  pinMode(sen2, INPUT);
  pinMode(sen3, INPUT);
  pinMode(sen4, INPUT);
  pinMode(sen5, INPUT);
  pinMode(alarm, OUTPUT);
  pinMode(buretardop, INPUT);
  pinMode(bdretardop, INPUT);
  lcd.init();
  lcd.backlight();
  lcd.createChar(0,ccS0);
  lcd.createChar(1,ccS1);
  lcd.createChar(2,ccBL);
  lcd.createChar(3,ccBR);
  everificado = 0;
  cdelay = 0;
  cdelayG = 0;
  nivelH = 6;
  //EEPROM.write(addr, 3);
  vretardo = EEPROM.read(addr);
  if (vretardo > 0) {
    retardop = vretardo;
  } else {
    retardop = 0;
  }
  sbomba1 = false;
  sbomba2 = false;
  albomba = 0;
  SD.begin();
  rtc.begin();
//  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
}
void loop() {
  tespera = millis();
//  tiempoenciendeb = millis();
  DateTime now = rtc.now();
  anio = now.year();
  mes = now.month();
  dia = now.day();
  hora = now.hour();
  minuto = now.minute();
  segundo = now.second();
  estadobup = digitalRead(buretardop);
  estadobdown = digitalRead(bdretardop);
  if (estadobup == HIGH) {
    if(retardop < 9) {
      retardop++;
      EEPROM.update(addr, retardop);      
    }
  }
  if (estadobdown == HIGH) {
    if (retardop >= 1) {
      retardop--;
      EEPROM.update(addr, retardop);
    }
  }
  if (tespera > tprevio + tdelay) {
    tprevio = tespera;
    inicial();
    if (muxValue1 == 1 && muxValue2 == 1) {
      sbomba1 = false;
      sbomba2 = false;
      if (numavisos < avisoslim) {
        if (esled == LOW) {
          esled = HIGH;
          numavisos++;
        } else {
          esled = LOW;
        }
        digitalWrite(alarm, esled);
      } else {
        digitalWrite(alarm, LOW);
      }
      if (vaciadesde == 0) {
        vhora = hora;
        vminuto = minuto;
        vaciadesde = 1;
        regtime(3);
      } else {
        vaciadesde = 1;
      }
    } else if (muxValue1 == 1 && muxValue2 == 0) {
      sbomba1 = false;
      sbomba2 = false;
      numavisos = 0;
    } else {
      if (muxValue3 == 0 && muxValue4 == 0 && muxValue5 == 0) {
        if (cdelay >= retardop && nivelH != 3) {
          sbomba1 = false;
          sbomba2 = false;
          regtime(5);
          nivelH = 3;
        } else {
          cdelay++;
        }
      } else {
        cdelay = 0;
        if (cdelayG >= multip) {
          if (muxValue1 == 0 && muxValue2 == 1) {
            vaciadesde = 0;
            numavisos = 0;
            if (muxValue3 == 0 && muxValue4 == 1 && muxValue5 == 1) {
              if (sbomba1 == true && sbomba2 == true) {
                albomba++;
                cambiabomba();
              }
              nivelH = 2;
            } else if (muxValue3 == 0 && muxValue4 == 0 && muxValue5 == 1) {
              if (sbomba1 == true && sbomba2 == true) {
                albomba++;
                cambiabomba();
              }
              nivelH = 1;
            } else {
              nivelH = 4;
            }
          } else if (muxValue1 == 0 && muxValue2 == 0) {
            vaciadesde = 0;
            numavisos = 0;
            if (muxValue3 == 0 && muxValue4 == 0 && muxValue5 == 1) {
              nivelH = 2;
            }
            if (muxValue3 == 0 && muxValue4 == 1 && muxValue5 == 1 && nivelH != 1) {
              albomba++;
              cambiabomba();
              nivelH = 1;
            }
            if (muxValue3 == 1 && muxValue4 == 1 && muxValue5 == 1 && nivelH != 0) {
              sbomba1 = true;
              sbomba2 = true;
              regtime(6);
              nivelH = 0;
            }
          }
          cdelayG = 0;
        } else {
          cdelayG++;
        }
      }
    }
  }
  if (sbomba1 == true && sbomba2 == true) {
    if (albomba == 2) {
      pinMode(rele1, OUTPUT);
      pinMode(rele0, OUTPUT);
      digitalWrite(rele1, LOW);
      if (tespera > tprevio + tdelay2){
        digitalWrite(rele0, LOW);
      }
      // delay(1000); //sustituir delay
      // pinMode(rele0, OUTPUT);
      // digitalWrite(rele0, LOW);
    } else {
      pinMode(rele0, OUTPUT);
      pinMode(rele1, OUTPUT);
      digitalWrite(rele0, LOW);
      if (tespera > tprevio + tdelay2){
        digitalWrite(rele1, LOW);
      }
      // delay(1000); //sustituir delay
      // pinMode(rele1, OUTPUT);
      // digitalWrite(rele1, LOW);
    }
  } else if (sbomba1 == true && sbomba2 == false) {
    pinMode(rele0, OUTPUT);
    digitalWrite(rele0, LOW);
    pinMode(rele1, OUTPUT);
    digitalWrite(rele1, HIGH);
  } else if (sbomba1 == false && sbomba2 == true) {
    pinMode(rele0, OUTPUT);
    digitalWrite(rele0, HIGH);
    pinMode(rele1, OUTPUT);
    digitalWrite(rele1, LOW);
  } else if (sbomba1 == false && sbomba2 == false) {
    pinMode(rele0, OUTPUT);
    digitalWrite(rele0, HIGH);
    pinMode(rele1, OUTPUT);
    digitalWrite(rele1, HIGH);
  }
  if (muxValue3 == 1 && muxValue4 == 0 && muxValue5 == 1) {
    lcd.clear();
    lcd.setCursor(3,0);
    lcd.print("Verificar");
    lcd.setCursor(0,1);
    lcd.print("electrodos en");
    lcd.setCursor(3,2);
    lcd.print("tanque !");
    everificado = 1;
  } else if (muxValue3 == 0 && muxValue4 == 1 && muxValue5 == 0) {
    lcd.clear();
    lcd.setCursor(3,0);
    lcd.print("Verificar");
    lcd.setCursor(0,1);
    lcd.print("electrodos en");
    lcd.setCursor(3,2);
    lcd.print("tanque !");
    everificado = 1;
  } else if (muxValue3 == 1 && muxValue4 == 1 && muxValue5 == 0) {
    lcd.clear();
    lcd.setCursor(3,0);
    lcd.print("Verificar");
    lcd.setCursor(0,1);
    lcd.print("electrodos en");
    lcd.setCursor(3,2);
    lcd.print("tanque !");
    everificado = 1;
  } else if (muxValue3 == 1 && muxValue4 == 0 && muxValue5 == 0) {
    lcd.clear();
    lcd.setCursor(3,0);
    lcd.print("Verificar");
    lcd.setCursor(0,1);
    lcd.print("electrodos en");
    lcd.setCursor(3,2);
    lcd.print("tanque !");
    everificado = 1;
  } else if (muxValue1 == 1 && muxValue2 == 0) {
    lcd.clear();
    lcd.setCursor(3,0);
    lcd.print("Verificar");
    lcd.setCursor(0,1);
    lcd.print("electrodos en");
    lcd.setCursor(3,2);
    lcd.print("cisterna !");
    everificado = 1;
  } else {
    //lcd.clear();
    if (everificado == 1) {
      lcd.clear();
      everificado = 0;
    }
    lcd.setCursor(0,0);
    lcd.print("CISTERNA");
    if (muxValue1 == 1) {
      lcd.setCursor(1,1);
      lcd.write(byte(0));
      lcd.setCursor(13,2);
      lcd.print("VACIA");
      lcd.setCursor(13,3);
      lcd.print(vhora);
      lcd.setCursor(15,3);
      lcd.print(":");
      lcd.setCursor(17,3);
      lcd.print(vminuto);
      everificado = 0;
      //u8g.drawStr( 79, 36, "Vacia");
      //u8g.drawStr( 79, 50, "desde");
      //u8g.setPrintPos(80, 64);
      //u8g.print(vhora);
      //u8g.setPrintPos(94, 64);
      //u8g.print(":");
      //u8g.setPrintPos(100, 64);
      //u8g.print(vminuto);
      
    } else {
      lcd.setCursor(1,1);
      lcd.write(byte(1));
      lcd.setCursor(13,3);
      lcd.print("RP:");
      if (muxValue5 == 1) {
        lcd.setCursor(5,3);
        lcd.write(byte(0));
        lcd.setCursor(16,3);
        lcd.print(retardop);
      } else {
        lcd.setCursor(5,3);
        lcd.write(byte(1));
        if (sbomba1 == false && sbomba2 == false) {
          lcd.setCursor(16,3);
          lcd.print(retardop);
        } else {
          lcd.setCursor(16,3);
          lcd.print(retardop - cdelay);
        }
      }
    }
    if (muxValue2 == 1) {
      lcd.setCursor(3,1);
      lcd.write(byte(0));
    } else {
      lcd.setCursor(3,1);
      lcd.write(byte(1));
    }
    lcd.setCursor(0,2);
    lcd.print("TAN1");
    if (muxValue3 == 1) {
      lcd.setCursor(1,3);
      lcd.write(byte(0));
    } else {
      lcd.setCursor(1,3);
      lcd.write(byte(1));
    }
    if (muxValue4 == 1) {
      lcd.setCursor(3,3);
      lcd.write(byte(0));
    } else {
      lcd.setCursor(3,3);
      lcd.write(byte(1));
    }
    lcd.setCursor(10,0);
    lcd.print("|");
    lcd.setCursor(10,1);
    lcd.print("|");
    lcd.setCursor(10,2);
    lcd.print("|");
    lcd.setCursor(10,3);
    lcd.print("|");
    if (sbomba1 == true) {
      lcd.setCursor(13,1);
      lcd.print("B1");
      lcd.setCursor(13,2);
      lcd.write(byte(2));
      lcd.setCursor(14,2);
      lcd.write(byte(3));
    }
    if (sbomba2 == true) {
      lcd.setCursor(16,1);
      lcd.print("B2");
      lcd.setCursor(16,2);
      lcd.write(byte(2));
      lcd.setCursor(17,2);
      lcd.write(byte(3));
    }
//    lcd.setCursor(13,3);
//    lcd.print("RP:");
//    if (muxValue5 == 1) {
//      lcd.setCursor(5,3);
//      lcd.write(byte(0));
//      lcd.setCursor(16,3);
//      lcd.print(retardop);
//    } else {
//      lcd.setCursor(5,3);
//      lcd.write(byte(1));
//      if (sbomba1 == false && sbomba2 == false) {
//        lcd.setCursor(16,3);
//        lcd.print(retardop);
//      } else {
//        lcd.setCursor(16,3);
//        lcd.print(retardop - cdelay);
//      }
//    }
  }
}
