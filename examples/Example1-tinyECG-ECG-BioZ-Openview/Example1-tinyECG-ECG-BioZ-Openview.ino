//////////////////////////////////////////////////////////////////////////////////////////
//
//    Demo code for the TinyECG breakout board
//
//    This example plots the ECG through serial UART on openview processing GUI.
//    GUI URL: https://github.com/Protocentral/protocentral_openview.git
//
//    Arduino connections:
//
//  |Tiny ECG pin label| Pin Function         |Adafruit QT PY ESP32-C3 Connection|
//  |----------------- |:--------------------:|---------------------------------:|
//  | MISO             | Slave Out            |  MISO                            |
//  | MOSI             | Slave In             |  MOSI                            |
//  | SCLK             | Serial Clock         |  SCK                             |
//  | CS               | Chip Select          |  6                               |
//  | 3V3              | Digital VDD          |  3V                              |
//  | GND              | Digital Gnd          |  Gnd                             | 
//  | FCLK             | 32K CLOCK            |  -                               |
//  | INTB             | Interrupt1           |  A0                              | 
//  | INT2B            | Interrupt2           |  -                               | 
//
//    This software is licensed under the MIT License(http://opensource.org/licenses/MIT).
//
//   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT
//   NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
//   IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
//   WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
//   SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//
//   For information on how to use, visit https://github.com/Protocentral/protocentral_max30001
//
/////////////////////////////////////////////////////////////////////////////////////////

#include <SPI.h>
#include "protocentral_max30001.h"

volatile char DataPacket[DATA_LEN];
const char DataPacketFooter[2] = {ZERO, CES_CMDIF_PKT_STOP};
const char DataPacketHeader[5] = {CES_CMDIF_PKT_START_1, CES_CMDIF_PKT_START_2, DATA_LEN, ZERO, CES_CMDIF_TYPE_DATA};

signed long ecg_data;
signed long bioz_data;
bool BioZSkipSample = false;

MAX30001 max30001(MAX30001_CS_PIN);

void sendData(signed long ecg_sample, signed long bioz_sample, bool _bioZSkipSample)
{

  DataPacket[0] = ecg_sample;
  DataPacket[1] = ecg_sample >> 8;
  DataPacket[2] = ecg_sample >> 16;
  DataPacket[3] = ecg_sample >> 24;

  DataPacket[4] = bioz_sample;
  DataPacket[5] = bioz_sample >> 8;
  DataPacket[6] = bioz_sample >> 16;
  DataPacket[7] = bioz_sample >> 24;

  if (_bioZSkipSample == false)
  {
    DataPacket[8] = 0x00;
  }
  else
  {
    DataPacket[8] = 0xFF;
  }

  DataPacket[9] = 0x00; // max30001.heartRate >> 8;
  DataPacket[10] = 0x00;
  DataPacket[11] = 0x00;

  // Send packet header (in ProtoCentral OpenView format)
  for (int i = 0; i < 5; i++)
  {
    Serial.write(DataPacketHeader[i]);
  }

  // Send the data payload
  for (int i = 0; i < DATA_LEN; i++) // transmit the data
  {
    Serial.write(DataPacket[i]);
  }

  // Send packet footer (in ProtoCentral OpenView format)
  for (int i = 0; i < 2; i++)
  {
    Serial.write(DataPacketFooter[i]);
  }
}


void setup()
{
  Serial.begin(57600); // Serial begin
   
  SPI.begin(MAX30001_SCK_PIN,MAX30001_MISO_PIN,MAX30001_MOSI_PIN,MAX30001_CS_PIN);

  bool ret = max30001.max30001ReadInfo();
  if (ret)
  {
    Serial.println("MAX 30001 read ID Success");
  }
  else
  {
    while (!ret)
    {
      // stay here untill the issue is fixed.
      ret = max30001.max30001ReadInfo();
      Serial.println("Failed to read ID, please make sure all the pins are connected");
      delay(5000);
    }
  }

  Serial.println("Initialising the chip ...");
  max30001.BeginECGBioZ(); // initialize MAX30001
}

void loop()
{
  ecg_data = max30001.getECGSamples();
  if (BioZSkipSample == false)
  {
    bioz_data = max30001.getBioZSamples();
    sendData(ecg_data, bioz_data, BioZSkipSample);
    BioZSkipSample = true;
  }
  else
  {
    bioz_data = 0x00;
    sendData(ecg_data, bioz_data, BioZSkipSample);
    BioZSkipSample = false;
  }
  delay(MAX30001_DELAY_SAMPLES);
}
