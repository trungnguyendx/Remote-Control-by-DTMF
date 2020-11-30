#include<String.h>

#include <REGX52.H>
 //Khai bao chan quet nut nhan
#define col0 P2_0 //cot
#define col1 P2_1
#define col2 P2_2
#define col3 P2_3
#define row0 P2_4 //hang
#define row1 P2_5
#define row2 P2_6
#define row3 P2_7

//Khai bao chan giao tiep LCD16x2 4bit
#define LCD_RS P3_7
#define LCD_EN P3_6

#define LCD_D4 P3_5
#define LCD_D5 P3_4
#define LCD_D6 P3_2
#define LCD_D7 P3_1

//buzzer
#define buzz P1_6

//DTMF
#define STD P3_3
//Giao tiep ds1307
#define SCL P1_4
#define SDA P1_5

//Khai bao thiet bi dieu khien
#define TB1 P3_0
#define TB2 P1_7
//Khai bao hang so

//Khai bao bien toan cuc

char gio, phut, giay, ngay, thang, nam, thu; //phuc vu hien thi thoi gian
char gioontb1, phutontb1, gioontb2, phutontb2; //phuc vu tu dong bat tat thiet bi
char gioofftb1, phutofftb1, gioofftb2, phutofftb2;

char Character1[8]= {14, 17, 17, 17, 31, 31, 31, 0 };	//Icon khoa
char Character2[8]= {14, 17, 1, 1, 31, 31, 31, 0 };  //Icon mo khoa
bit lock = 1;
KtLock = 0; // trang thai khoa
bit trangthaicaidatontb1 = 0, trangthaicaidatontb2 = 0; //trang thai tu dong bat tat 
bit trangthaicaidatofftb1 = 0, trangthaicaidatofftb2 = 0;
char key, MODE; //ma phim, MODE dieu khien
bit DTMF = 0; //che do DTMF
char Pas[4]; //Mat Khau da luu
char PasNhap[4]; //mat khau nhap
bit Cancel = 0; //cancel khi nhap thieu pass

char dem1 = 0, dem2 = 0; //phuc vu auto thoat
bit TimeOut = 0;
/*****************Ham delay*********************/

void delay_us(unsigned int t) {
  unsigned int i;
  for (i = 0; i < t; i++);
}
void delay_ms(unsigned int t) {
  unsigned int i, j;
  for (i = 0; i < t; i++)
    for (j = 0; j < 123; j++);
}

//buzzer
void bip() {
  unsigned char n;
  for (n = 0; n < 150; n++) {
    buzz = 0;
    delay_us(30);
    buzz = 1;
    delay_us(30); // 100us = 10khz
  }
  //buzz=1; delay_ms(150); buzz=0;

}

//Cac ham phuc vu giao tiep I2C
void I2C_Start(void) {
  SDA = 1;
  SCL = 1; // dua SCL va SDA len muc 1
  SDA = 0; // keo SDA xuong 0
  delay_ms(1); // delay 1 khoang thoi gian ngan
  SCL = 0; // keo SCL xuong 0  ( qua trinh Start ket thuc)
  SDA = 1; // dua SDA len muc 1 
}

void I2C_STOP() {
  SDA = 0; // cho SDA xuong 0 de chuan bi dua len 1
  SCL = 1; // dua SCL len muc 1 
  SDA = 1; // SDA duoc dua len muc 1. Qua trinh Stop hoan tat
}

bit I2C_Write(unsigned char dat) // khai bao ham truyen du lieu va bien dat  
{
  unsigned char i; // khai bao bien i
  bit outbit; // khai bao bit 

  for (i = 1; i <= 8; i++) //vong lap for lap 8 lan tuong duong 8 bit ( 1 byte)
  {
    outbit = dat & 0x80; // gán bien outbit cho byte can truen & 0x80 ; 
    SDA = outbit; // cho SDA bang voi gia tri cua outbit ( 1 hoac 0)
    dat = dat << 1; // dich trai 1 lan
    SCL = 1;
    SCL = 0; // tao 1 xung clock tren chan SCL de dich gia tri 1 hoac 0 tren chan SDA vao slaver
  }
  SDA = 1;
  SCL = 1; // dua SDA va SCL len muc 1 chuan bi doc bit ack
  outbit = SDA; // lay bit ack tu chan SDA
  SCL = 0; // cho SCL ve 0	 
  return (outbit); // lay gia tri ve
}

unsigned char I2C_Read(bit ack) {
  unsigned char i, dat;
  bit inbit;
  dat = 0;
  for (i = 1; i <= 8; i++) // moi lan doc 1 bit. 8 lan doc = 1 byte
  {
    SCL = 1; // cho SCL len =1 , slaver se gui 1 bit vao chan SDA
    inbit = SDA; // lay gia tri cua chan SDA gán vao inbit
    dat = dat << 1; // dich byte data sang trái  1 lan
    dat = dat | inbit; // muc dich la gán giá tri cua bit inbit vao byte dat
    SCL = 0; // cho SLC xuong muc 0 san sang cho lan doc bit tiep thep
  }
  if (ack) SDA = 0; // cac cau lenh  nay dung de doc bit ack 
  else SDA = 1;
  SCL = 1;
  SCL = 0;
  SDA = 1;
  return (dat);
}

//Ham ghi du lieu vao ds1307
void DSl3O7_Write(unsigned char add, unsigned char dat) {
  I2C_Start(); // bat dau ghi	
  I2C_Write(0xd0); //0xd0 la dia chi cua ds1307
  I2C_Write(add); // gia tri can ghi
  I2C_Write(((dat / 10) << 4) | (dat % 10)); // do du lieu trong ds la BCD nen ta can chuyen ca gia tri sang BCD ( day cau lenh chuyen du lieu sang BCD)
  I2C_STOP(); // ket thuc qua trinh truyen du lieu
}

void DSl3O7_Write_bt(unsigned char add, unsigned char dat) {
  I2C_Start(); // bat dau ghi	
  I2C_Write(0xd0); //0xd0 la dia chi cua ds1307
  I2C_Write(add); // gia tri can ghi
  I2C_Write(dat); // do du lieu trong ds la BCD nen ta can chuyen ca gia tri sang BCD ( day cau lenh chuyen du lieu sang BCD)
  I2C_STOP(); // ket thuc qua trinh truyen du lieu
}

// Ham doc du lieu thoi gian trong ds
unsigned char DSl3O7_Read(unsigned char add) {
  unsigned char dat;

  I2C_Start(); // bat dau doc

  I2C_Write(0xd0); // dau tien gui lenh ghi du lieu(ghi dia chi can lay du lieu trong DS1307)
  I2C_Write(add); // Dia chi ma ta muon doc ( vi du, muon doc giay thi ta ghi dia chi 0x00)
  I2C_Start(); // bat dau doc du lieu
  I2C_Write(0xd1); // gui ma lenh doc du lieu tu dia chi(add)
  dat = I2C_Read(0); // doc xong thi luu gia tri da doc dc vao dat
  I2C_STOP(); // ket thuc qua trinh doc du lieu
  dat = (dat & 0x0f) + (dat >> 4) * 10; // du lieu doc ra o dang BCD nen chuyen sang he 10					  
  return (dat); // tra ve gia tri da doc duoc
}

unsigned char DSl3O7_Read_bt(unsigned char add) {
  unsigned char dat;

  I2C_Start(); // bat dau doc

  I2C_Write(0xd0); // dau tien gui lenh ghi du lieu(ghi dia chi can lay du lieu trong DS1307)
  I2C_Write(add); // Dia chi ma ta muon doc ( vi du, muon doc giay thi ta ghi dia chi 0x00)
  I2C_Start(); // bat dau doc du lieu
  I2C_Write(0xd1); // gui ma lenh doc du lieu tu dia chi(add)
  dat = I2C_Read(0); // doc xong thi luu gia tri da doc dc vao dat
  I2C_STOP(); // ket thuc qua trinh doc du lieu		  
  return (dat); // tra ve gia tri da doc duoc
}

void laythoigiantrongds() // muc dich cua ham nay la lay gia tri thoi gian trong ds1307
{
  gio = DSl3O7_Read(0x02); // doc gia tri trong thanh ghi 0x02 ( chính là thanh ghi gio)
  phut = DSl3O7_Read(0x01); // doc gia tri trong thanh ghi 0x01 ( chính là thanh ghi phut)
  giay = DSl3O7_Read(0x00); // doc gia tri trong thanh ghi 0x00 ( chính là thanh ghi giay)
  thu = DSl3O7_Read(0x03);
  ngay = DSl3O7_Read(0x04); // doc gia tri trong thanh ghi 0x04 ( chính là thanh ghi ngay) 
  thang = DSl3O7_Read(0x05); // doc gia tri trong thanh ghi 0x05 ( chính là thanh ghi thang) 
  nam = DSl3O7_Read(0x06); // doc gia tri trong thanh ghi 0x06 ( chính là thanh ghi nam)
}

void LayThoiGianHenGio() {
  gioontb1 = DSl3O7_Read(0x20);
  phutontb1 = DSl3O7_Read(0x21);
  gioontb2 = DSl3O7_Read(0x22);
  phutontb2 = DSl3O7_Read(0x23);
  gioofftb1 = DSl3O7_Read(0x24);
  phutofftb1 = DSl3O7_Read(0x25);
  gioofftb2 = DSl3O7_Read(0x26);
  phutofftb2 = DSl3O7_Read(0x26);
}
/*******Ham quet matrix phim nhan 4x4******/
unsigned char check_but() //Kiem tra nut nhan
{
  row0 = row1 = row2 = row3 = 0;
  if (!col0 || !col1 || !col2 || !col3) return 1;
  return 0;
}
void scan_row(unsigned char r) //Quet hang
{
  row0 = row1 = row2 = row3 = 1;
  if (r == 0) row0 = 0;
  else if (r == 1) row1 = 0;
  else if (r == 2) row2 = 0;
  else if (r == 3) row3 = 0;
}

unsigned char check_col() //Kiem tra cot
{
  unsigned char c = 0;
  if (!col0) c = 1;
  else if (!col1) c = 2;
  else if (!col2) c = 3;
  else if (!col3) c = 4;
  return c;
}

unsigned char get_key() //Ham tra ve vi tri nut 1-16
{
  unsigned char row, col, scancode = 0xff;
  if (check_but()) {
    delay_ms(10); //debouce 5ms
    while (check_but()) {
      for (row = 0; row < 4; row++) {
        scan_row(row); //Quet hang
        col = check_col(); //Lay vi tri cot
        if (col > 0) scancode = ((row * 4) + col); //Tra ve vi tri nut duoc nhan
      }
    }
  }
  switch (scancode) {
  case 4: //MENU
    scancode = 12;
    break;
  case 5:
    scancode = 4;
    break;
  case 6:
    scancode = 5;
    break;
  case 7:
    scancode = 6;
    break;
  case 8: //LOCK
    scancode = 13;
    break;
  case 9:
    scancode = 7;
    break;
  case 10:
    scancode = 8;
    break;
  case 11:
    scancode = 9;
    break;
  case 12: //CANCEL
    scancode = 14;
    break;
  case 13: //*
    scancode = 15;
    break;
  case 14:
    scancode = 0;
    break;
  case 15: //#
    scancode = 16;
    break;
  case 16: //OK
    scancode = 17;
    break;
  default:
    break;
  }
  return scancode;
}

unsigned char GiaiMaCodeDTMF() //giai ma phim DTMF
{
  unsigned char ma = 20; //khong nhan phim gi
  while (STD == 0) {
    ma = P1 & 0x0f;
    switch (ma) {
    case 10: //phim 0	
      ma = 0;
      break;
    case 11: //phim *
      ma = 11;
      break;
    case 12: //phim #
      ma = 12;
      break;
    default:
      break;
    }
  }
  return ma;
}
/**************Giao tiep LCD 16x2 4bit**********************/
void LCD_Enable(void) {
  LCD_EN = 1;
  delay_us(3);
  LCD_EN = 0;
  delay_us(50);
}

void LCD_Send4Bit(unsigned char Data) {
  LCD_D4 = Data & 0x01;
  LCD_D5 = (Data >> 1) & 1;
  LCD_D6 = (Data >> 2) & 1;
  LCD_D7 = (Data >> 3) & 1;
}

void LCD_SendCommand(unsigned char command) {	
  LCD_Send4Bit(command >> 4); /* Gui 4 bit cao */
  LCD_Enable();
  LCD_Send4Bit(command); /* Gui 4 bit thap*/
  LCD_Enable();
}
void LCD_Clear() {
  LCD_SendCommand(0x01);
  delay_us(10);
}

void LCD_Gotoxy(unsigned char col, unsigned char row) {
  unsigned char address;
  if (!row) address = (0x80 + col);
  else address = (0xc0 + col);
  delay_us(1000);
  LCD_SendCommand(address);
  delay_us(50);
}

void LCD_PutChar(unsigned char Data) {
  LCD_RS = 1;
  LCD_SendCommand(Data);
  LCD_RS = 0;
}

void LCD_Puts(char * s) {
  while ( * s) {
    LCD_PutChar( * s);
    s++;
  }
}

void LCDBuildChar(unsigned char loc, unsigned char *p) {
     unsigned char i;
     if(loc<8)                                 //If valid address
	 {
         LCD_SendCommand(0x40+(loc*8));               //Write to CGRAM
         for(i=0;i<8;i++)
            LCD_PutChar(p[i]);                   //Write the character pattern to CGRAM
     }
	 LCD_SendCommand(0x80);                           //shift back to DDRAM location 0
}

void LCD_Init() {
  LCD_Send4Bit(0x00);
  delay_ms(20);
  LCD_RS = 0;
  LCD_Send4Bit(0x03);
  LCD_Enable();
  delay_ms(5);
  LCD_Enable();
  delay_us(100);
  LCD_Enable();
  LCD_Send4Bit(0x02);
  LCD_Enable();
  LCD_SendCommand(0x28);
  LCD_SendCommand(0x0c);
  LCD_SendCommand(0x06);
  LCDBuildChar(0, Character1); 
  LCDBuildChar(1, Character2); 
  LCD_SendCommand(0x01);
}

//Hien thi thoi gian thuc
void HienThoiGian() {
  laythoigiantrongds();
  LCD_Gotoxy(0, 0);
  LCD_PutChar(gio / 10 + 0x30);
  LCD_PutChar(gio % 10 + 0x30);
  LCD_Puts(":");

  LCD_PutChar(phut / 10 + 0x30);
  LCD_PutChar(phut % 10 + 0x30);
  LCD_PutChar(32);
  if(lock) LCD_PutChar(0); else LCD_PutChar(1);
  //LCD_Puts(":");

  //LCD_PutChar(giay/10+0x30);
  //LCD_PutChar(giay%10+0x30);

  LCD_Gotoxy(8, 0);
  LCD_PutChar(ngay / 10 + 0x30);
  LCD_PutChar(ngay % 10 + 0x30);
  LCD_Puts("/");

  LCD_PutChar(thang / 10 + 0x30);
  LCD_PutChar(thang % 10 + 0x30);
  LCD_Puts("/");

  LCD_PutChar(nam / 10 + 0x30);
  LCD_PutChar(nam % 10 + 0x30);

}
void HienGioCaiDat(unsigned char Gio) {
  LCD_Gotoxy(0, 1);

  LCD_PutChar(Gio / 10 + 0x30);
  LCD_PutChar(Gio % 10 + 0x30);
}

void HienPhutCaiDat(unsigned char Gio, unsigned char Phut) {
  LCD_Gotoxy(0, 1);
  LCD_PutChar(Gio / 10 + 0x30);
  LCD_PutChar(Gio % 10 + 0x30);
  LCD_Puts(":");

  LCD_PutChar(Phut / 10 + 0x30);
  LCD_PutChar(Phut % 10 + 0x30);
}

void HienGiayCaiDat(unsigned char Gio, unsigned char Phut, unsigned char Giay) {
  LCD_Gotoxy(0, 1);
  LCD_PutChar(Gio / 10 + 0x30);
  LCD_PutChar(Gio % 10 + 0x30);
  LCD_Puts(":");

  LCD_PutChar(Phut / 10 + 0x30);
  LCD_PutChar(Phut % 10 + 0x30);
  LCD_Puts(":");

  LCD_PutChar(Giay / 10 + 0x30);
  LCD_PutChar(Giay % 10 + 0x30);
}

void HienNgayCaiDat() {
  LCD_Gotoxy(0, 1);
  LCD_PutChar(ngay / 10 + 0x30);
  LCD_PutChar(ngay % 10 + 0x30);
}
void HienThangCaiDat() {
  LCD_Gotoxy(0, 1);
  LCD_PutChar(ngay / 10 + 0x30);
  LCD_PutChar(ngay % 10 + 0x30);
  LCD_Puts("/");

  LCD_PutChar(thang / 10 + 0x30);
  LCD_PutChar(thang % 10 + 0x30);

}
void HienNamCaiDat() {
  LCD_Gotoxy(0, 1);
  LCD_PutChar(ngay / 10 + 0x30);
  LCD_PutChar(ngay % 10 + 0x30);
  LCD_Puts("/");

  LCD_PutChar(thang / 10 + 0x30);
  LCD_PutChar(thang % 10 + 0x30);
  LCD_Puts("/");

  LCD_PutChar(nam / 10 + 0x30);
  LCD_PutChar(nam % 10 + 0x30);
}
void HienTrangThai(unsigned Gio, unsigned Phut, bit tt) {
  HienPhutCaiDat(Gio, Phut);
  LCD_Gotoxy(8, 1);
  if (tt == 0) LCD_Puts("OFF");
  else LCD_Puts("ON ");
}
void HienTrangThai1(bit TB) {
  LCD_Gotoxy(7, 1);
  if (TB == 1) LCD_Puts("OFF");
  else LCD_Puts("ON ");
}
void CaiThoiGian() {
  unsigned char mp, mode;
  if (MODE == 4) {
    mode = 1;
    LCD_Clear();
    while (MODE == 4) {
      if (TimeOut == 1) {
        TimeOut = 0;
        goto exit;
      }
      LCD_Gotoxy(0, 0);
      LCD_Puts("Cai thoi gian");
      if (mode == 1) HienGioCaiDat(gio);
      else if (mode == 2) HienPhutCaiDat(gio, phut);
      else if (mode == 3) HienGiayCaiDat(gio, phut, giay);
      else if (mode == 4) HienNgayCaiDat();
      else if (mode == 5) HienThangCaiDat();
      else if (mode == 6) HienNamCaiDat();
      mp = get_key();
      if (mp < 10) {
        bip();
        if (mode == 1) //gio
        {
          HienGioCaiDat(gio);
          gio = (gio % 10) * 10 + mp;
          if (gio > 23) gio = mp;
        } else if (mode == 2) //phut
        {

          HienPhutCaiDat(gio, phut);
          phut = (phut % 10) * 10 + mp;
          if (phut > 59) phut = mp;
        } else if (mode == 3) //giay
        {
          HienGiayCaiDat(gio, phut, giay);
          giay = (giay % 10) * 10 + mp;
          if (giay > 59) giay = mp;
        } else if (mode == 4) //ngay
        {
          HienNgayCaiDat();
          ngay = (ngay % 10) * 10 + mp;
          if (ngay > 31) ngay = mp;
        } else if (mode == 5) //thang
        {
          HienThangCaiDat();
          thang = (thang % 10) * 10 + mp;
          if (thang > 12) thang = mp;
        } else if (mode == 6) //nam
        {
          HienNamCaiDat();
          nam = (nam % 10) * 10 + mp;
        }
      } else if (mp == 14) //phim CANCEL 
      {
        exit:;
        bip();bip();bip();
        //Bat Timer
        TR1 = 1;
        MODE = 0;
        LCD_Clear();
        break;
      }
      else if (mp == 12) {
        LCD_Clear();
        MODE++;
        dem1 = dem2 = 0;
        TR0 = 1;
        bip();
      } //phim Menu
      else if (mp == 17) //phim ok
      {
        DSl3O7_Write(0x02, gio);
        DSl3O7_Write(0x01, phut);
        DSl3O7_Write(0x00, giay);
        DSl3O7_Write(0x04, ngay);
        DSl3O7_Write(0x05, thang);
        DSl3O7_Write(0x06, nam);
        bip();
        mode++;
        if (mode == 4) {
          LCD_Gotoxy(0, 1);
          LCD_Puts("             ");

        }
        if (mode == 7) {
          bip();
          bip();
          bip();
          //Bat Timer
          MODE = 0;
          TR1 = 1;
          LCD_Clear();
          break;
        }

      }
    }

  }
}

void KiemTraTrangThaiThietBi() {
  LCD_Gotoxy(0, 1);
  if (TB1 == 1 && TB2 == 1) LCD_Puts("TB1:OFF  TB2:OFF");
  else if (TB1 == 1 && TB2 == 0) LCD_Puts("TB1:OFF   TB2:ON ");
  else if (TB1 == 0 && TB2 == 1) LCD_Puts("TB1:ON   TB2:OFF");
  else if (TB1 == 0 && TB2 == 0) LCD_Puts("TB1:ON   TB2:ON ");
}
////////////////////////////////////////////////////////////////////////////////
void NhapMatKhau() {
  unsigned char mp;
  char i = 0;
  while (i < 4 && !DTMF) {
    if (TimeOut == 1) {
      TimeOut = 0;
      goto exit;
    }
    mp = get_key();
    if (mp < 10) {
      if (i != 3) bip();
      PasNhap[i] = mp;
      LCD_Gotoxy(6 + i, 1);
      LCD_PutChar(mp + 0x30);
      delay_ms(200);
      LCD_Gotoxy(6 + i, 1);
      LCD_Puts("*");
      i++;
    } else if (mp == 14) //phim CANCEL
    {
      exit:;
      Cancel = 1;
      break;
    }
    else if (mp == 12 && MODE == 5) //phim Menu
    {
      LCD_Clear();
      bip();
      bip();
      bip();
      MODE = 0;
      TR1 = 1;
      break;
    }
  }
}

bit CheckPas(char a[], char b[]) {
  char i;
  char m = sizeof(a) / sizeof(a[0]), n = sizeof(b) / sizeof(b[0]);

  if (m != n) {
    return 0;
  }
  for (i = 0; i < 4; i++) {
    if (a[i] != b[i]) {
      return 0;
    }
  }
  return 1;
}

void KiemTraMatKhau() {
  if (KtLock == 1) // phim lock
  {
    KtLock = 0;
    MODE = 0;
    TR1 = 0; //tat Timer
    saipass: ;
    LCD_Clear();
    LCD_Gotoxy(1, 0);
    LCD_Puts("Nhap mat khau");
    LCD_Gotoxy(0, 1);
    NhapMatKhau();
    if (DTMF) goto exit;
    while (lock == 1) //dang khoa
    {
      if (TimeOut == 1) {
        TimeOut = 0;
        goto exit;
      }
      if ((get_key() == 14) || Cancel == 1) //phim CANCEL
      {
        exit:;
        Cancel = 0;
        bip();bip();bip();
        LCD_Clear();
        TR1 = 1;
        break;
      }

      if (CheckPas(Pas, PasNhap)) {
        bip();
        delay_ms(100);
        bip();
        LCD_Clear();
        LCD_Gotoxy(4, 0);
        LCD_Puts("Welcome!");
        delay_ms(1000);
        LCD_Clear();
        lock = 0;
        TR1 = 1;
      } else {

        LCD_Clear();
        LCD_Gotoxy(2, 0);
        LCD_Puts("Sai mat khau!");
        bip();
        bip();
        bip();
        delay_ms(1000);
        goto saipass;
      }
    }
  }
}

void HenGioTB1() {
  unsigned char mp, mode;
  if (MODE == 2) {
    mode = 1;
    LCD_Clear();
    LCD_Gotoxy(0, 0);
    LCD_Puts("Hen gio TB1");
    delay_ms(1500);
    LCD_Clear();
    while (MODE == 2) {
      if (TimeOut == 1) {
        TimeOut = 0;
        goto exit;
      }
      LCD_Gotoxy(0, 0);
      if (mode < 4) LCD_Puts("Auto ON");
      else LCD_Puts("Auto OFF");

      if (mode == 1) HienGioCaiDat(gioontb1);
      else if (mode == 2) HienPhutCaiDat(gioontb1, phutontb1);
      else if (mode == 3) HienTrangThai(gioontb1, phutontb1, trangthaicaidatontb1); // trang thai cai dat

      else if (mode == 4) HienGioCaiDat(gioofftb1);
      else if (mode == 5) HienPhutCaiDat(gioofftb1, phutofftb1);
      else if (mode == 6) HienTrangThai(gioofftb1, phutofftb1, trangthaicaidatofftb1); // trang thai cai dat

      mp = get_key();
      if (mp < 10) {
        bip();
        if (mode == 1) //gio
        {
          HienGioCaiDat(gioontb1);
          gioontb1 = (gioontb1 % 10) * 10 + mp;
          if (gioontb1 > 23) gioontb1 = mp;
        } else if (mode == 2) //phut
        {
          HienPhutCaiDat(gioontb1, phutontb1);
          phutontb1 = (phutontb1 % 10) * 10 + mp;
          if (phutontb1 > 59) phutontb1 = mp;
        } else if (mode == 4) //gio
        {
          HienGioCaiDat(gioofftb1);
          gioofftb1 = (gioofftb1 % 10) * 10 + mp;
          if (gioofftb1 > 23) gioofftb1 = mp;
        } else if (mode == 5) //phut
        {
          HienPhutCaiDat(gioofftb1, phutofftb1);
          phutofftb1 = (phutofftb1 % 10) * 10 + mp;
          if (phutofftb1 > 59) phutofftb1 = mp;
        }
      } else if (mp == 14) //phim CANCEL 
      {
        exit:;
        bip();bip();bip();
        //Bat Timer
        TR1 = 1;
        MODE = 0;
        LCD_Clear();
        break;
      }
      else if (mp == 12) {
        MODE++;
        bip();
        dem1 = dem2 = 0;
        TR0 = 1;
      } //phim Menu		   {LCD_Clear();MODE++;dem1=dem2=0; TR0=1;bip();}
      else if (mp == 15 && mode == 3) {
        bip();
        trangthaicaidatontb1 = 1;
      } else if (mp == 16 && mode == 3) {
        bip();
        trangthaicaidatontb1 = 0;
      } else if (mp == 15 && mode == 6) {
        bip();
        trangthaicaidatofftb1 = 1;
      } else if (mp == 16 && mode == 6) {
        bip();
        trangthaicaidatofftb1 = 0;
      } else if (mp == 17) //phim ok
      {
        DSl3O7_Write(0x20, gioontb1);
        DSl3O7_Write(0x21, phutontb1);
        DSl3O7_Write(0x22, gioofftb1);
        DSl3O7_Write(0x23, phutofftb1);
        DSl3O7_Write(0x24, trangthaicaidatontb1);
        DSl3O7_Write(0x25, trangthaicaidatofftb1);
        bip();
        mode++;
        if (mode == 4) {
          LCD_Clear();
        }
        if (mode == 7) {
          bip();
          bip();
          bip();
          //Bat Timer
          MODE = 0;
          TR1 = 1;
          LCD_Clear();
          break;
        }

      }
    }

  }
}

void HenGioTB2() {
  unsigned char mp, mode;
  if (MODE == 3) {
    mode = 1;
    LCD_Clear();
    LCD_Gotoxy(0, 0);
    LCD_Puts("Hen gio TB2");
    delay_ms(1500);
    LCD_Clear();
    while (MODE == 3) {
      if (TimeOut == 1) {
        TimeOut = 0;
        goto exit;
      }
      LCD_Gotoxy(0, 0);
      if (mode < 4) LCD_Puts("Auto ON");
      else LCD_Puts("Auto OFF");

      if (mode == 1) HienGioCaiDat(gioontb2);
      else if (mode == 2) HienPhutCaiDat(gioontb2, phutontb2);
      else if (mode == 3) HienTrangThai(gioontb2, phutontb2, trangthaicaidatontb2); // trang thai cai dat

      else if (mode == 4) HienGioCaiDat(gioofftb2);
      else if (mode == 5) HienPhutCaiDat(gioofftb2, phutofftb2);
      else if (mode == 6) HienTrangThai(gioofftb2, phutofftb2, trangthaicaidatofftb2); // trang thai cai dat

      mp = get_key();
      if (mp < 10) {
        bip();
        if (mode == 1) //gio
        {
          HienGioCaiDat(gioontb2);
          gioontb2 = (gioontb2 % 10) * 10 + mp;
          if (gioontb2 > 23) gioontb2 = mp;
        } else if (mode == 2) //phut
        {
          HienPhutCaiDat(gioontb2, phutontb2);
          phutontb2 = (phutontb2 % 10) * 10 + mp;
          if (phutontb1 > 59) phutontb2 = mp;
        } else if (mode == 4) //gio
        {
          HienGioCaiDat(gioofftb2);
          gioofftb2 = (gioofftb2 % 10) * 10 + mp;
          if (gioofftb2 > 23) gioofftb2 = mp;
        } else if (mode == 5) //phut
        {
          HienPhutCaiDat(gioofftb2, phutofftb2);
          phutofftb2 = (phutofftb2 % 10) * 10 + mp;
          if (phutofftb2 > 59) phutofftb2 = mp;
        }
      } else if (mp == 14) //phim CANCEL 
      {
        exit:;
        bip();bip();bip();
        //Bat Timer
        TR1 = 1;
        MODE = 0;
        LCD_Clear();
        break;
      }
      else if (mp == 12) {
        MODE++;
        bip();
        dem1 = dem2 = 0;
        TR0 = 1;
      } //phim Menu
      else if (mp == 15 && mode == 3) {
        bip();
        trangthaicaidatontb2 = 1;
      } else if (mp == 16 && mode == 3) {
        bip();
        trangthaicaidatontb2 = 0;
      } else if (mp == 15 && mode == 6) {
        bip();
        trangthaicaidatofftb2 = 1;
      } else if (mp == 16 && mode == 6) {
        bip();
        trangthaicaidatofftb2 = 0;
      } else if (mp == 17) //phim ok
      {
        DSl3O7_Write(0x26, gioontb2);
        DSl3O7_Write(0x27, phutontb2);
        DSl3O7_Write(0x28, gioofftb2);
        DSl3O7_Write(0x29, phutofftb2);
        DSl3O7_Write(0x2a, trangthaicaidatontb2);
        DSl3O7_Write(0x2b, trangthaicaidatofftb2);

        bip();
        mode++;
        if (mode == 4) {
          LCD_Clear();
        }
        if (mode == 7) {
          bip();
          bip();
          bip();
          //Bat Timer
          MODE = 0;
          TR1 = 1;
          LCD_Clear();
          break;
        }

      }
    }

  }
}
void KiemTraThoiGianTuDong() {
  if ((gioontb1 != gioofftb1) || (phutontb1 != phutofftb1)) {
    if (trangthaicaidatontb1 == 1 && gio == gioontb1 && phut == phutontb1 && giay < 2) {
      TB1 = 0;
      bip();
      delay_ms(1000);
    }
    if (trangthaicaidatofftb1 == 1 && gio == gioofftb1 && phut == phutofftb1 && giay < 2) {
      TB1 = 1;
      bip();
      delay_ms(100);
      bip();
      delay_ms(1000);
    }
    delay_ms(1000);
  }
  if ((gioontb2 != gioofftb2) || (phutontb2 != phutofftb2)) {
    if (trangthaicaidatontb2 == 1 && gio == gioontb2 && phut == phutontb2 && giay < 3) {
      TB2 = 0;
      bip();
      delay_ms(1000);
    }
    if (trangthaicaidatofftb2 == 1 && gio == gioofftb2 && phut == phutofftb2 && giay < 4) {
      TB2 = 1;
      bip();
      delay_ms(100);
      bip();
      delay_ms(1000);
    }
  }
}

void CaiMatKhau() {
  char i;
  if (MODE == 5) // phim lock
  {

    LCD_Clear();
    LCD_Gotoxy(0, 0);
    LCD_Puts("Cai mat khau");
    delay_ms(1500);
    saipass: ;
    LCD_Clear();
    LCD_Gotoxy(0, 0);
    LCD_Puts("Nhap mat khau cu");
    LCD_Gotoxy(0, 1);
    NhapMatKhau();
    while (MODE == 5) {
      if (TimeOut == 1) {
        TimeOut = 0;
        goto exit;
      }
      if ((get_key() == 14) || Cancel == 1) //phim CANCEL
      {
        exit:;
        Cancel = 0;
        bip();bip();bip();
        LCD_Clear();
        MODE = 0;
        TR1 = 1;
        break;
      }
      if (get_key() == 12) {
        LCD_Clear();
        MODE = 0;
        TR1 = 1;
        bip();
        bip();
        bip();
        break;
      }
      if (CheckPas(Pas, PasNhap)) {
        bip();
        delay_ms(100);
        bip();
        LCD_Clear();
        LCD_Gotoxy(0, 0);
        LCD_Puts("Mat khau moi");
        LCD_Gotoxy(0, 1);
        NhapMatKhau();
        if (Cancel == 1) {
          LCD_Clear();
          Cancel = 0;
          bip();
          MODE = 0;
          TR1 = 1;
          break;
        }
        for (i = 0; i < 4; i++) {
          Pas[i] = PasNhap[i];
          DSl3O7_Write(0x30 + i, PasNhap[i]);
        }
        bip();
        delay_ms(100);
        bip();
        TR1 = 1;
        MODE = 0;
        LCD_Clear();
        break;
      } else {
        bip();
        bip();
        bip();
        goto saipass;
      }
    }
  }
}

void DocMatKhau() {
  char i;
  for (i = 0; i < 4; i++) Pas[i] = DSl3O7_Read(0x30 + i);
}

void DieuKhienThietBi() {
  unsigned char mp, mode;
  if (MODE == 1 && lock == 0) {
    //Tat Timer
    TR1 = 0;
    mode = 1;
    LCD_Clear();
    LCD_Gotoxy(0, 0);
    LCD_Puts("Dieu khien TB");
    delay_ms(1500);
    LCD_Clear();
    while (MODE == 1) {
      if (TimeOut == 1) {
        TimeOut = 0;
        goto exit;
      }
      LCD_Gotoxy(0, 0);
      if (mode == 1) LCD_Puts("Thiet Bi 1");
      else LCD_Puts("Thiet Bi 2");

      if (mode == 1) HienTrangThai1(TB1);
      else if (mode == 2) HienTrangThai1(TB2);
      mp = get_key();

      if (mp == 14) //phim CANCEL 
      {
        exit:;
        bip();bip();bip();
        //Bat Timer
        TR1 = 1;
        MODE = 0;
        LCD_Clear();
        break;
      }
      else if (mp == 12) {
        MODE++;
        bip();
        dem1 = dem2 = 0;
        TR0 = 1;
      } //phim Menu
      else if (mp == 15 && mode == 1) {
        bip();
        TB1 = 0;
      } else if (mp == 16 && mode == 1) {
        bip();
        delay_ms(100);
        bip();
        TB1 = 1;
      } else if (mp == 15 && mode == 2) {
        bip();
        TB2 = 0;
      } else if (mp == 16 && mode == 2) {
        bip();
        delay_ms(100);
        bip();
        TB2 = 1;
      } else if (mp == 17) //phim ok
      {
        bip();
        mode++;
        if (mode == 3) mode = 1;
        if (mode == 2) {
          LCD_Clear();
        }
      }
    }

  }
}
void NhapMatKhauDTMF() {
  unsigned char mp;
  char i = 0;
  LCD_Gotoxy(6, 1);
  while (i < 4) {
    if (TimeOut == 1) {
      Cancel = 1;
      TimeOut = 0;
      break;
    }
    if (get_key() == 14) {
      Cancel = 1;
      break;
    }
    mp = GiaiMaCodeDTMF();
    if (mp < 10) {
      if (i != 3) bip();
      PasNhap[i] = mp;
      LCD_Puts("*");
      i++;
    }
  }
}
void BatTatDTMF() {
  bit tb1 = 0, tb2 = 0;
  char mp;
  while (DTMF == 1) {
    LCD_Gotoxy(3, 0);
    LCD_Puts("DTMF Mode");
    KiemTraTrangThaiThietBi();
    if (get_key() == 14) goto thoat; //phim CANCEL
    if (TimeOut == 1) {
      TimeOut = 0;
      goto thoat;
    }
    mp = GiaiMaCodeDTMF();
    if (mp == 1) //so 1
    {
      tb1 = 1;
      tb2 = 0;
      if (TB1 == 0) bip();
      else {
        bip();
        delay_ms(100);
        bip();
      }
    }
    if (mp == 2) //so 2
    {
      tb1 = 0;
      tb2 = 1;
      if (TB2 == 0) bip();
      else {
        bip();
        delay_ms(100);
        bip();
      }
    }

    if (mp == 13 || mp == 11) //phim *, bat thiet bi
    {
      if (tb1 == 1) {
        //tb1=0;
        TB1 = 0;
        bip();
      } else
      if (tb2 == 1) {
        //tb2=0;
        TB2 = 0;
        bip();
      }
    }
    if (mp == 14 || mp == 12) //phim #, tat thiet bi
    {
      if (tb1 == 1) {
        //tb1 = 0;
        TB1 = 1;
        bip();
        delay_ms(100);
        bip();
      }
      if (tb2 == 1) {
        //tb2 = 0;
        TB2 = 1;
        bip();
        delay_ms(100);
        bip();
      }
    }
    if (mp == 0) //phim 0, tat 2 thiet bi
    {
      TB1 = TB2 = 1;
      tb2 = tb1 = 0;
      bip();
      delay_ms(100);
      bip();
    }
    if (mp == 8) //phim 8, bat 2 thiet bi
    {
      TB1 = 0;
      bip();
      delay_ms(2000);
      TB2 = 0;
      bip();
    }
    if (mp == 5) //thoat
    {
      thoat:;
      tb1 = tb2 = 0;
      DTMF = 0;
    }
  }
}

void DieuKhienDTMF() {
  if (DTMF == 1) {
    EX1 = 0; //CAM NGAT NGOAI
    TR1 = 0; //tat Timer1 	
    saipass: ;
    LCD_Clear();
    LCD_Gotoxy(3, 0);
    LCD_Puts("DTMF Mode");
    LCD_Gotoxy(0, 1);
    NhapMatKhauDTMF();
    while (DTMF == 1) {
      if (TimeOut == 1) {
        TimeOut = 0;
        goto exit;
      }
      if ((get_key() == 14) || Cancel == 1) //phim CANCEL
      {
        exit:;
        Cancel = 0;
        bip();bip();bip();
        LCD_Clear();
        DTMF = 0;
        TR1 = 1;IE1 = 0;
        EX1 = 1;
        break;
      }

      if (CheckPas(Pas, PasNhap)) {
        bip();
        delay_ms(100);
        bip();
        LCD_Clear();
        while (DTMF == 1) {
          BatTatDTMF();
        }
        bip();
        bip();
        bip();
        delay_ms(1000);
        LCD_Clear();
        TR1 = 1;
        IE1 = 0;
        EX1 = 1;

      } else {
        bip();
        bip();
        bip();
        bip();
        bip();
        bip();
        goto saipass;
      }
    }
  }
}
void main() {

  //Set up Thiet bi
  TB1 = 1;
  TB2 = 1;
  //Setup RAM ds1307
  //Gio bat tat tu dong thiet bi
  gioontb1 = DSl3O7_Read(0x20);
  phutontb1 = DSl3O7_Read(0x21);
  gioofftb1 = DSl3O7_Read(0x22);
  phutofftb1 = DSl3O7_Read(0x23);
  trangthaicaidatontb1 = DSl3O7_Read(0x24);
  trangthaicaidatofftb1 = DSl3O7_Read(0x25);

  gioontb2 = DSl3O7_Read(0x26);
  phutontb2 = DSl3O7_Read(0x27);
  gioofftb2 = DSl3O7_Read(0x28);
  phutofftb2 = DSl3O7_Read(0x29);
  trangthaicaidatontb2 = DSl3O7_Read(0x2a);
  trangthaicaidatofftb2 = DSl3O7_Read(0x2b);

  //Set up Mat khau
  if (DSl3O7_Read(0x00)==80) {
    DSl3O7_Write(0x30, 1);
    DSl3O7_Write(0x31, 6);
    DSl3O7_Write(0x32, 0);
    DSl3O7_Write(0x33, 3);
  }

  //Setup Timer
  //Timer 0,1
  TMOD = 0x11;
  TH0 = 0X3C; //50ms
  TL0 = 0XB0;
  TH1 = 0X3C; //50ms
  TL1 = 0XB0;
  EA = 1;
  ET1 = 1;
  ET0 = 1;
  TR1 = 1;
  //Ngat ngoai 1
  EX1 = 1;
  IT1 = 1;

  //Setup LCD  
  LCD_Init();
  delay_ms(100);
  LCD_Clear();
  bip();
  LCD_Gotoxy(1,0);
  LCD_Puts("DTMF Controller");
  LCD_Gotoxy(6,1);
  LCD_Puts("V1.0 ");
  delay_ms(1500);
  LCD_Clear();
  while (1) {
    DocMatKhau();
	HienThoiGian();
    KiemTraMatKhau();
    DieuKhienThietBi();
    HenGioTB1();
    HenGioTB2();
    CaiThoiGian();
    CaiMatKhau();
    KiemTraTrangThaiThietBi();
    KiemTraThoiGianTuDong();
    DieuKhienDTMF();
  }	
}

void Timer1(void) interrupt TF1_VECTOR {
  TR1 = 0;
  TH1 = 0X3C; //50ms
  TL1 = 0XB0;
  key = get_key();
  if (key == 13) //Phim Lock
  {
    if (lock == 0) //khoa
    {
      lock = 1;
      bip();
      bip();
      bip();
    } else {
      bip();
      KtLock = 1;
      dem1 = dem2 = 0;
      TimeOut = 0;
      TR0 = 1; //bat timer0 TimeOut	

    }
  } else if (key == 12) // Phim Menu
  {
    bip();
    MODE = 1;
    dem1 = dem2 = 0;
    TimeOut = 0;
    TR0 = 1; //bat timer0 TimeOut
  }
  TR1 = 1;

}

void Timer0(void) interrupt TF0_VECTOR {
  TR0 = 0;
  TH0 = 0X3C; //50ms
  TL0 = 0XB0;
  dem1++;
  if (dem1 == 20) //duoc 1 s
  {
    dem1 = 0;
    dem2++;
    if (dem2 == 120) //2p
    {
      dem1 = dem2 = 0;
      TimeOut = 1;
      lock = 1;
    }
  }
  if (TimeOut == 0) TR0 = 1;
}

void NgatNgoai1(void) interrupt 2 {
  bip();
  delay_ms(100);
  bip();
  delay_ms(100);
  bip();
  DTMF = 1;
  while (STD == 0);
  MODE = 0;
  dem1 = dem2 = 0;
  TimeOut = 0;
  TR0 = 1; //bat timer0 TimeOut                                                                      
}


