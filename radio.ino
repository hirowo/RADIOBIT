#include <Wire.h>
#include <stdlib.h>

#define P_ON 3
#define FM 1
#define AM 0
#define RADIO_ADDR 0x10

//DSPラジオレジスタ
#define CONFIG0 0x00
#define CONFIG1 0x01
#define CONFIG2 0x02
#define CONFIG3 0x03
#define CONFIG4 0x04
#define CONFIG5 0x05
#define CONFIG6 0x06
#define CONFIG7 0x07
#define CONFIG8 0x08
#define CONFIG9 0x09

#define BUFSIZE 20

//DSPラジオレジスタ設定
//0x00
#define POWER_ON  0x80
#define FM_MODE   0x40
#define TUNE      0x20
#define SEEK      0x10
#define MUTE      0x08
//0x01
// バンド設定
#define LM      0x00
#define MW_L      0x08
#define MW_M     0x10
#define MW_H     0x18

#define FM1     0x00
#define FM2     0x01
#define FM3     0x02
#define FM4     0x03
#define FM5     0x04




//外部変数


void m6951_i2c_send(unsigned char reg,unsigned char da)
{
    Wire.beginTransmission(RADIO_ADDR);
    Wire.write(reg); //内部アドレス
    Wire.write(da);
    Wire.endTransmission();

}

unsigned char m6951_i2c_read(unsigned char reg)
{
    unsigned char read_byte;
    Wire.beginTransmission(RADIO_ADDR);
    Wire.write(reg);
    Wire.endTransmission(false);
    Wire.requestFrom(RADIO_ADDR, 1);
    read_byte = Wire.read();
    Wire.endTransmission(true);
    return read_byte;
//delay(30);
}


 void m6951_i2c_tune(unsigned int mode)
{

    if(mode == FM) {        // FM mode
        m6951_i2c_send(CONFIG0,(POWER_ON|FM_MODE));
        delay(1);
        m6951_i2c_send(CONFIG0, (POWER_ON|FM_MODE|TUNE));    // tune 0->1
        delay(1);
        m6951_i2c_send(CONFIG0, (POWER_ON|FM_MODE));    // tune 1->0
        delay(8);
    } else {            // AM mode
        m6951_i2c_send(CONFIG0, (POWER_ON));
        delay(1);
        m6951_i2c_send(CONFIG0, (POWER_ON|TUNE));    // tune 0->1
        delay(1);
        m6951_i2c_send(CONFIG0, (POWER_ON));    // tune 1->0
        delay(8);
    }
}

 void m6951_i2c_Set_band(unsigned int band)
{
    if(band == AM) {
        m6951_i2c_send(CONFIG0, (POWER_ON));// AM mode
        m6951_i2c_send(CONFIG1, MW_M);// MW2
       }
    if(band == FM) {
        m6951_i2c_send(CONFIG0, (POWER_ON|FM_MODE));// FM mode
        m6951_i2c_send(CONFIG1, FM3);// FM4
        }
}


void m6951_i2c_set_freq(unsigned int band, unsigned int freq)
{
    unsigned int ch;

    if(band == FM){            // set fm freq
        ch = (((freq - 3000) * 10) / 25);
    }
    if(band == AM){    // set wm freq
        ch = (freq / 9) * 3;
    }
    m6951_i2c_send(CONFIG3, (ch & 0x00ff));
    m6951_i2c_send(CONFIG2, ((ch >> 8) | 0x40));
    m6951_i2c_tune(band);
}



void setup() {
  // put your setup code here, to run once:
  pinMode(P_ON,OUTPUT);
  Wire.begin();
  digitalWrite(P_ON,HIGH);
  delay(100);
  //DSP初期設定
  m6951_i2c_send(CONFIG0,POWER_ON);
//  m6951_i2c_send(CONFIG1,0x13);//M6951　初期値設定
  m6951_i2c_send(CONFIG7,0x31);//M6951　初期値設定
  m6951_i2c_send(CONFIG9,0x07);//M6951　初期値設定
  
  delay(100);

  //デフォルトチャンネル設定
  delay(100);

  m6951_i2c_Set_band(FM);
  m6951_i2c_set_freq(FM,8050);

  Serial.begin(9600);
  Serial.print("Radio program");
}

char readBuff[BUFSIZE];
int point = 0;

void loop() {
  // put your main code here, to run repeatedly:
  int rtn;
  unsigned char reg;
  int ch;
  int i;
  rtn = Serial.read();
  if(rtn != -1){
    if(rtn == '\n'){
      readBuff[point]=='\0';
      ch = atoi(readBuff);
      Serial.print(ch);
      m6951_i2c_set_freq(FM,ch);
      point =0;
    }
    else if(rtn == 'd'){
      for(i =0 ;i < 0x1c ; i++){
        reg = m6951_i2c_read(i);
        Serial.print(i,HEX);
        Serial.print(" ");      

        Serial.print(reg,HEX);
        Serial.print("\r\n");      
      }
    }
    else if(rtn == 'r'){
      reg = m6951_i2c_read(0);
      Serial.print(reg);
      Serial.print("\r\n");      
    }

    else {
      readBuff[point] = rtn;
      if(point >= (BUFSIZE-1)){
        point = point;
      }
      else {
        point ++;
      }
    }
    
  }

  
}
