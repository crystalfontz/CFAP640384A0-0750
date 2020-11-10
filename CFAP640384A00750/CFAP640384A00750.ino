//=============================================================================
// "Arduino" example program for Crystalfontz ePaper. 
//
// This project is for the CFAP640384A0-0750 :
//
//   https://www.crystalfontz.com/product/cfap640384a00750
//
// It was written against a Seeduino v4.2 @3.3v. An Arduino UNO modified to
// operate at 3.3v should also work.
//-----------------------------------------------------------------------------
// This is free and unencumbered software released into the public domain.
// 
// Anyone is free to copy, modify, publish, use, compile, sell, or
// distribute this software, either in source code form or as a compiled
// binary, for any purpose, commercial or non-commercial, and by any
// means.
// 
// In jurisdictions that recognize copyright laws, the author or authors
// of this software dedicate any and all copyright interest in the
// software to the public domain. We make this dedication for the benefit
// of the public at large and to the detriment of our heirs and
// successors. We intend this dedication to be an overt act of
// relinquishment in perpetuity of all present and future rights to this
// software under copyright law.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
// IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
// OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
// OTHER DEALINGS IN THE SOFTWARE.
// 
// For more information, please refer to <http://unlicense.org/>
//=============================================================================
// Connecting the Arduino to the display
//
// ARDUINO |Adapter |Wire Color |Function
// --------+--------+-----------+--------------------
// D2      |19      |Yellow     |BS1 Not Used
// D3      |17      |Green      |Busy Line
// D4      |18      |Brown      |Reset Line
// D5      |15      |Purple     |Data/Command Line
// D10     |16      |Blue       |Chip Select Line
// D11     |14      |White      |MOSI
// D13     |13      |Orange     |Clock
// 3.3V    |5       |Red        |Power
// GND     |3       |Black      |Ground
//
// Short the following pins on the adapter board:
// GND  -> BS2
// RESE -> 3ohms
//=============================================================================
//Connecting the Arduino to the SD card
//
// ARDUINO  |Wire Color |Function
// ---------+-----------+--------------------
// D8       |Blue       |CS
// D11      |Green      |MOSI
// D12      |Purple     |MISO
// D13      |Brown      |CLK
//
//
//=============================================================================
// Creating image data arrays
//
// Bmp_to_epaper is code that will aid in creating bitmaps necessary from .bmp files if desired.
// The code can be downloaded from the Crystalfontz website: 
//    https://www.crystalfontz.com/product/bmptoepaper-bitmap-to-epaper-software
// or it can be downloaded from github: 
//    https://github.com/crystalfontz/bmp_to_epaper
//=============================================================================

// The display is SPI, include the library header.
#include <SPI.h>
#include <SD.h>
#include <avr/io.h>

// Include LUTs
#include "LUTs_for_CFAP640384A00750.h"
//Include Images
#include "Images_for_CFAP640384A00750.h"

#define ePaper_RST_0  (digitalWrite(EPD_RESET, LOW))
#define ePaper_RST_1  (digitalWrite(EPD_RESET, HIGH))
#define ePaper_CS_0   (digitalWrite(EPD_CS, LOW))
#define ePaper_CS_1   (digitalWrite(EPD_CS, HIGH))
#define ePaper_DC_0   (digitalWrite(EPD_DC, LOW))
#define ePaper_DC_1   (digitalWrite(EPD_DC, HIGH))

#define EPD_READY   3
#define EPD_RESET   4
#define EPD_DC      5
#define EPD_CS      10
#define SD_CS       8

#define HRES  640
#define VRES  384

//=============================================================================
//this function will take in a byte and send it to the display with the 
//command bit low for command transmission
void writeCMD(uint8_t command)
{
	ePaper_DC_0;
	ePaper_CS_0;
	SPI.transfer(command);
	ePaper_CS_1;
}

//this function will take in a byte and send it to the display with the 
//command bit high for data transmission
void writeData(uint8_t data)
{
	ePaper_DC_1;
	ePaper_CS_0;
	SPI.transfer(data);
	ePaper_CS_1;
}

//===========================================================================
void setup(void)
{
	//Debug port / Arduino Serial Monitor (optional)
	Serial.begin(9600);
  Serial.println("setup started");
	// Configure the pin directions   
	pinMode(EPD_CS, OUTPUT);
	pinMode(EPD_RESET, OUTPUT);
	pinMode(EPD_DC, OUTPUT);
	pinMode(EPD_READY, INPUT);
  pinMode(SD_CS, OUTPUT);

  digitalWrite(SD_CS, LOW);

  if (!SD.begin(SD_CS))
  {
    Serial.println("SD could not initialize");
  }

	//Set up SPI interface
	SPI.beginTransaction(SPISettings(2000000, MSBFIRST, SPI_MODE0));
	SPI.begin();


	//reset driver
	ePaper_RST_0;
	delay(200);
	ePaper_RST_1;
	delay(200);

  initEPD();
	Serial.println("setup complete");
}

void initEPD()
{

  //-----------------------------------------------------------------------------
  //more detail on the following commands and additional commands not used here	
  //can be found on the CFAP640384A0-0750 datasheet on the Crystalfontz website	
  //-----------------------------------------------------------------------------


  //Power Setting
  writeCMD(0x01);
  writeData(0x37);
  writeData(0x00);
  writeData(0x08);
  writeData(0x08);

  //Booster Soft Start
  writeCMD(0x06);
  writeData(0xc7);
  writeData(0xcc);
  writeData(0x28);

  //Power On
  writeCMD(0x04);
  Serial.println("before wait");
  //wait until powered on
  while (0 == digitalRead(EPD_READY));
  Serial.println("after wait");

  //Panel Setting 
  writeCMD(0x00);
  writeData(0xc3);

  //PLL Control
  writeCMD(0x30);
  writeData(0x3a);

  //Resolution
  writeCMD(0x61);
#define WIDTH (640)
  writeData(WIDTH>>8);	  //MSB
  writeData(WIDTH&0xff);	//LSB
#define HEIGHT (384)
  writeData(HEIGHT>>8);	  //MSB
  writeData(HEIGHT&0xff);	//LSB

  //VCM DC Settings
  writeCMD(0x82);
  writeData(0x1e);

  //Vcom and data interval setting
  writeCMD(0x50);
  writeData(0x67);

  //flash define
  writeCMD(0xe5);
  writeData(0x03);

}

void setRegisterLUT()
{
	//set LUTs
	//The following block allows the LUTs to be changed.
	//In order for these LUTs to take effect, command 0x00 must have bit 5 set to "1"
	//set panel setting to call LUTs from the register
	writeCMD(0x00);
	writeData(0xeb);

	//VCOM_LUT_LUTC
	writeCMD(0x20);
	for (int i = 0; i < 44; i++)
	{
		writeData(pgm_read_byte(&VCOM_LUT_LUTC[i]));
	}
	//W2W_LUT_LUTWW
	writeCMD(0x21);
	for (int i = 0; i < 42; i++)
	{
		writeData(pgm_read_byte(&W2W_LUT_LUTWW[i]));
	}
	//B2W_LUT_LUTBW_LUTR
	writeCMD(0x22);
	for (int i = 0; i < 42; i++)
	{
		writeData(pgm_read_byte(&B2W_LUT_LUTBW_LUTR[i]));
	}
	//B2B_LUT_LUTBB_LUTB
	writeCMD(0x23);
	for (int i = 0; i < 42; i++)
	{
		writeData(pgm_read_byte(&B2B_LUT_LUTBB_LUTB[i]));
	}
	//W2B_LUT_LUTWB_LUTW
	writeCMD(0x24);
	for (int i = 0; i < 42; i++)
	{
		writeData(pgm_read_byte(&W2B_LUT_LUTWB_LUTW[i]));
	}
}

void setOTPLUT()
{
	//set panel setting to call LUTs from OTP
	writeCMD(0x00);
	writeData(0xcb);
}
//================================================================================
void show_BMPs_in_root(void)
{
  Serial.println("in BMP function");
  delay(1);
  File
    root_dir;
  root_dir = SD.open("/");
  if (0 == root_dir)
  {
    Serial.println("show_BMPs_in_root: Can't open \"root\"");
    return;
  }

  File
    bmp_file;

  Serial.println("made it after root dir check");
  while (1)
  {
    bmp_file = root_dir.openNextFile();
    if (0 == bmp_file)
    {
      // no more files, break out of while()
      // root_dir will be closed below.
      break;
    }
    Serial.println("made it after check for file");
    //Skip directories (what about volume name?)
    if (0 == bmp_file.isDirectory())
    {
      //The file name must include ".BMP"
      if (0 != strstr(bmp_file.name(), ".BMP"))
      {
        uint32_t size = bmp_file.size();
        //print out the size of the display to see what it is for reference
        Serial.println(size);
        //check to make sure a valid bitmap is put in. Should be 737334-737336
        if ( 737330 <= size)
        {
          clear();
          //Power on
          writeCMD(0x04);
          while (0 == digitalRead(EPD_READY));

          //Start transmission of first set of data
          writeCMD(0x10);
          //Jump over BMP header
          bmp_file.seek(54);
          //grab one row of pixels from the SD card at a time
          static uint8_t quarter_line[160 * 3];
          for (int line = 0; line < 384*4; line++)
          {

            //Set the LCD to the left of this line. BMPs store data
            //to have the image drawn from the other end, uncomment the line below

            //read a line from the SD card
            bmp_file.read(quarter_line, 160 * 3);

            //send the line to the display
            send_pixels_BW(160 * 3, quarter_line);

          }
          Serial.println("before refresh wait");
          //refresh the display
          writeCMD(0x11);
          writeCMD(0x12);
          delay(100);
          while (0 == digitalRead(EPD_READY));
          //power off while we wait
          writeCMD(0x02);
          Serial.println("after refresh wait");
          delay(5000);
          while (1);
        }
      }
    }
    //Release the BMP file handle
    bmp_file.close();
    //Give a bit to let them see it
    delay(1000);
  }
  //Release the root directory file handle
  root_dir.close();
  //SPI.beginTransaction(SPISettings(12000000, MSBFIRST, SPI_MODE0));
}

//================================================================================
void Load_Flash_Image_To_Display_RAM_RLE(uint16_t width_pixels,
  uint16_t height_pixels,
  const uint8_t *BW_image)
{
  //Index into *image, that works with pgm_read_byte()

  //Get width_bytes from width_pixel, rounding up
  uint8_t
    width_bytes;
  width_bytes = (width_pixels + 7) >> 3;

  //Make sure the display is not busy before starting a new command.
  while (0 == digitalRead(EPD_READY));
  //Select the controller   
  ePaper_CS_0;

  //Aim at the command register
  ePaper_DC_0;
  //Write the command: DATA START TRANSMISSION 2 (DTM2) (R13H)
  //  Display Start transmission 2
  //  (DTM2, Red Data)
  //
  // This command starts transmitting data and write them into SRAM. To complete
  // data transmission, command DSP (Data transmission Stop) must be issued. Then
  // the chip will start to send data/VCOM for panel.
  //  * In B/W mode, this command writes “NEW” data to SRAM.
  //  * In B/W/Red mode, this command writes “Red” data to SRAM.
  SPI.transfer(0x10);
  //Pump out the Red data.
  ePaper_DC_1;
  uint8_t count = 0;
  uint8_t byteRead = 0;
  uint8_t byteSent[4];
  for (int i = 0; i < MONO_ARRAY_SIZE; i = i + 2)
  {
    count = pgm_read_byte(&BW_image[i]);
    byteRead = pgm_read_byte(&BW_image[i + 1]);

    uint8_t x = 0;
    for (int j = 7; j > 0; j = j - 2)
    {
      byteSent[x] = 0x00;
      if (((byteRead >> j) & 0x01) == 1)
      {
        byteSent[x] = 0x30;
      }
      if (((byteRead >> (j - 1)) & 0x01) == 1)
      {
        byteSent[x] |= 0x03;
      }
      x++;
    }

    for (uint8_t j = 0; j < count; j++)
    {
      for (uint8_t k = 0; k < 4; k++)
      {
        SPI.transfer(byteSent[k]);
      }
    }
  }

  //Aim back at the command register
  ePaper_DC_0;
  //Write the command: DATA STOP (DSP) (R11H)
  SPI.transfer(0x11);
  //Write the command: Display Refresh (DRF)   
  SPI.transfer(0x12);
  //Deslect the controller   
  ePaper_CS_1;
}

//================================================================================
void clear()
{
  //Power on
  writeCMD(0x04);
  while (0 == digitalRead(EPD_READY));

  writeCMD(0x10);
  for (long i = 0; i < 122880; i++)
  {
    writeData(0x00);
  }

  //refresh the display
  writeCMD(0x11);
  writeCMD(0x12);
  delay(100);
  Serial.print("refreshing . . . ");
  while (0 == digitalRead(EPD_READY));
  Serial.println("refresh complete");
}



//================================================================================
void send_pixels_BW(uint8_t byteCount, uint8_t *dataPtr)
{
  uint8_t data;
  uint8_t red;
  uint8_t green;
  uint8_t blue;
  while (byteCount != 0)
  {
    uint8_t data = 0;
    red = *dataPtr;
    dataPtr++;
    byteCount--;
    green = *dataPtr;
    dataPtr++;
    byteCount--;
    blue = *dataPtr;
    dataPtr++;
    byteCount--;

    if (127 >= ((red*.21) + (green*.72) + (blue*.07)))
    {
      data = 0x30;
    }

      red = *dataPtr;
      dataPtr++;
      byteCount--;
      green = *dataPtr;
      dataPtr++;
      byteCount--;
      blue = *dataPtr;
      dataPtr++;
      byteCount--;
      //data = data << 1;

      if (127 >= ((red*.21) + (green*.72) + (blue*.07)))
      {
        data = data | 0x03;
      }

      writeData(data);
  }
}

void powerON()
{
  writeCMD(0x04);
}

void powerOff()
{
  writeCMD(0x02);
  writeCMD(0x03);
  writeData(0x00);
}

//=============================================================================
#define waittime          10000
#define white             0
#define checks            0
#define zebra             0
#define black             0
#define splashscreenRLE   1
#define showBMPs          0
void loop()
{
  Serial.println("top of loop");

#if white
  //power on the display
  powerON();
  while (0 == digitalRead(EPD_READY));

	writeCMD(0x10);
	for (long i = 0; i < 122880; i++)
	{
 		writeData(0x00);
	}

	//refresh the display
  writeCMD(0x11);
	writeCMD(0x12);
  delay(100);
  Serial.println("before refresh wait");
	while (0 == digitalRead(EPD_READY));
  Serial.println("after refresh wait");
  //for maximum power conservation, power off the EPD
  powerOff();
  delay(waittime);
#endif

#if checks
  //power on the display
  powerON();
  while (0 == digitalRead(EPD_READY));

  writeCMD(0x10);
  uint16_t
    x,y,z;
  for(y=0;y<HEIGHT;y++)
    {
    for(x=0;x<WIDTH/4;x++)
      {
      z = y & 0x0002;
      if(z != 0)
        {
        writeData(0x00);
        writeData(0x33);
        }
      else
        {
        writeData(0x33);
        writeData(0x00);
        }
      }
    }
  //refresh the display
  writeCMD(0x11);
  writeCMD(0x12);
  delay(100);
  Serial.println("before refresh wait");
  while (0 == digitalRead(EPD_READY));
  Serial.println("after refresh wait");
  //for maximum power conservation, power off the EPD
  powerOff();
  delay(waittime);
#endif

#if zebra
  //power on the display
  powerON();
  while (0 == digitalRead(EPD_READY));

  writeCMD(0x10);
  for (int i = 0; i < 7680 * 4; i++)
  {
    writeData(0x33);
  }
  for (int i = 0; i < 7680 * 4; i++)
  {
    writeData(0x00);
  }
  for (int i = 0; i < 7680 * 4; i++)
  {
    writeData(0x33);
  }
  for (int i = 0; i < 7680 * 4; i++)
  {
    writeData(0x00);
  }

  //refresh the display
  writeCMD(0x11);
  writeCMD(0x12);
  delay(100);
  Serial.println("before refresh wait");
  while (0 == digitalRead(EPD_READY));
    Serial.println("after refresh wait");

  //for maximum power conservation, power off the EPD
  powerOff();
  delay(waittime);

  //Power on
  writeCMD(0x04);
  while (0 == digitalRead(EPD_READY));

  writeCMD(0x10);
  for (int i = 0; i < 7680 * 4; i++)
  {
    writeData(0x00);
  }
  for (int i = 0; i < 7680 * 4; i++)
  {
    writeData(0x33);
  }
  for (int i = 0; i < 7680 * 4; i++)
  {
    writeData(0x00);
  }
  for (int i = 0; i < 7680 * 4; i++)
  {
    writeData(0x33);
  }

  //refresh the display
  writeCMD(0x11);
  writeCMD(0x12);
  delay(100);
  Serial.println("before refresh wait");
  while (0 == digitalRead(EPD_READY));
  Serial.println("after refresh wait");
  //for maximum power conservation, power off the EPD
  powerOff();
  delay(waittime);
  clear();
#endif


#if black
  //power on the display
  powerON();
  while (0 == digitalRead(EPD_READY));

  writeCMD(0x10);
  for (long i = 0; i < 122880; i++)
  {
    writeData(0x33);
  }

  //refresh the display
  writeCMD(0x11);
  writeCMD(0x12);
  delay(100);
  Serial.println("before refresh wait");
  while (0 == digitalRead(EPD_READY));
  Serial.println("after refresh wait");
  //power off while we wait
  //for maximum power conservation, power off the EPD
  powerOff();
  delay(waittime);
#endif

#if splashscreenRLE
  //on the Seeeduino, there is not enough flash memory to store this data 
  //but if another uP with more flash is used, this function can be utilized
  //power on the display
  powerON();
  //load an image to the display
  Load_Flash_Image_To_Display_RAM_RLE(HRES, VRES, Mono_1BPP);


  Serial.print("refreshing . . . ");
  while (0 == digitalRead(EPD_READY));
  Serial.println("refresh complete");
  //for maximum power conservation, power off the EPD
  powerOff();
  delay(waittime);
#endif

#if showBMPs
  //power on the display
  powerON();
  //send over 
  writeCMD(0x00);
  writeData(0xcb);
  show_BMPs_in_root();
  writeCMD(0x00);
  writeData(0xc3);
  powerOff();
  delay(waittime);
#endif

}
//=============================================================================
