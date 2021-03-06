/*
This software, the ideas and concepts is Copyright (c) David Bird 2021 and beyond.
All rights to this software are reserved.
It is prohibited to redistribute or reproduce of any part or all of the software contents in any form other than the following:
 1. You may print or download to a local hard disk extracts for your personal and non-commercial use only.
 2. You may copy the content to individual third parties for their personal use, but only if you acknowledge the author David Bird as the source of the material.
 3. You may not, except with my express written permission, distribute or commercially exploit the content.
 4. You may not transmit it or store it in any other website or other form of electronic retrieval system for commercial purposes.
 5. You MUST include all of this copyright and permission notice ('as annotated') and this shall be included in all copies
 or substantial portions of the software and where the software use is visible to an end-user.
 
THE SOFTWARE IS PROVIDED "AS IS" FOR PRIVATE USE ONLY, IT IS NOT FOR COMMERCIAL USE IN WHOLE OR PART OR CONCEPT.

FOR PERSONAL USE IT IS SUPPLIED WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.

IN NO EVENT SHALL THE AUTHOR OR COPYRIGHT HOLDER BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
#include <SoftwareSerial.h>
#include <SPI.h>
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"

// For the ESP8266 D1 Mini use these connections
#define TFT_DC   D3
#define TFT_CS   D8
#define TFT_MOSI D7
#define TFT_RST  D4
#define TFT_CLK  D5
#define TFT_LED  3.3v
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST); // Using only hardware SPI for speed

// Assign names to common 16-bit color values:
#define BLACK    0x0000
#define BLUE     0x001F
#define RED      0xF800
#define GREEN    0x07E0
#define CYAN     0x07FF
#define YELLOW   0xFFE0
#define ORANGE   0xFD20
#define WHITE    0xFFFF

String sentence, current_time, lat, lon, azi, fix_quality, num_sats, hdop, altitude, track_true, track_magnetic, speed_knots, speed_kph, speed_mph, date, ordinal_bearing, unused;
float sats, flat, flon, faltitude, fspeed, fbearing;
const int centreX  = 230; // Location of the compass display on screen
const int centreY  = 120;
const int diameter = 70; // Size of the compass
int       dx = centreX, dy = centreY, last_dx = centreX, last_dy = centreY - diameter*0.85, hour, minute, second;

SoftwareSerial gps(D1, D2); // RX/TX Data Pins, Connect the GPS to D2 and D1 

void setup() {
  Serial.begin(115200);
  gps.begin(9600);        // Begin software serial port that will read the GPS data  
  tft.begin();            // Start the TFT display
  tft.setRotation(3);     // Rotate screen by 90°
  tft.setTextSize(2);     // Set medium text size
  tft.setTextColor(YELLOW); 
  tft.fillScreen(BLUE);
}

void loop() {
  if (gps.available()) {
    if (gps.find("GPGGA,")) { //GPGGA,225147.00,5121.87111,N,00207.72174,W,2,09,0.95,47.0,M,48.0,M,,0000*7B
      current_time = gps.readStringUntil(',');
      hour         = current_time.substring(0,2).toInt();
      minute       = current_time.substring(2,4).toInt();
      second       = current_time.substring(4,6).toInt();
      lat          = gps.readStringUntil(',');
      flat         = lat.toFloat()/100;
      String NS    = gps.readStringUntil(',');
      if (NS == "S") flat = -flat;
      lon          = gps.readStringUntil(',');
      flon         = lon.toFloat()/100;
      String EW    = gps.readStringUntil(',');
      if (EW == "W") flon = -flon;
      fix_quality  = gps.readStringUntil(','); // 0 = Invalid, 1 = GPS fix, 2 = DGPS fix
      num_sats     = gps.readStringUntil(','); 
      sats         = num_sats.toInt();
      hdop         = gps.readStringUntil(','); // Horizontal Dilution of Precision (HDOP) 1.5 Relative accuracy of horizontal position 
      altitude     = gps.readStringUntil(','); // Metres
      faltitude    = altitude.toFloat();
      unused       = gps.readStringUntil('\n');
    }
    if (gps.find("GPVTG,")) { //$GPVTG,,T,,M,0.162,N,0.300,K,A*25
      track_true      = gps.readStringUntil(','); // Track True
      track_true     += gps.readStringUntil(','); // Track Type
      track_magnetic  = gps.readStringUntil(','); // Track Magnetic
      track_magnetic += gps.readStringUntil(','); // Track Type
      speed_knots     = gps.readStringUntil(','); // Speed in knots
      speed_knots    += gps.readStringUntil(','); // Speed units N
      speed_kph       = gps.readStringUntil(','); // Speed in knots
      speed_kph      += gps.readStringUntil(','); // Speed units N
      speed_mph       = String(speed_kph.toFloat()* 0.621371); // Convert kph to MPH
      fbearing        = track_true.toFloat();
      ordinal_bearing = Bearing_to_Ordinal(fbearing);
      fspeed          = speed_kph.toFloat();
      unused          = gps.readStringUntil('\n');
    }
    if (gps.find("GPRMC,")) { //$GPRMC,233512.00,A,5121.87041,N,00207.73021,W,0.436,,051217,,,A*68
      for (int reading = 1; reading < 9; reading++){
        unused = gps.readStringUntil(','); // T
      }
      date = gps.readStringUntil(','); // Date
      date = date.substring(0,2)+"/"+date.substring(2,4)+"/"+date.substring(4,6);
    }
    Serial.println("Time\t\tLAT\tLON\tSATS\tAlt\tBearing\tSpeed(MPH)\tDate");
    Serial.println("------------------------------------------------------------------------");
    Serial.print(current_time.substring(0,2)+":"+current_time.substring(2,4)+":"+current_time.substring(4,6)+"\t");
    Serial.print(String(flat,3)+"\t");
    Serial.print(String(flon,3)+"\t");
    Serial.print(num_sats+"\t");
    Serial.print(altitude+"\t");
    Serial.print(String(fbearing)+"\t");
    Serial.print(String(fspeed)+"\t");
    Serial.println(date);
    Serial.println();
  }
  else
  {
    Serial.println("Waiting for GPS to acquire signal...");
  }
  OLED_display_GPS_data();
}

void OLED_display_GPS_data(){
  PrintText(60,0,"G6EJD GPS Compass",CYAN,2);
  tft.fillRect(45,40,90,19*4,BLUE);
  PrintText(0,45,"LAT:"+String(flat),YELLOW,2);
  PrintText(0,63,"LON:"+String(flon),YELLOW,2);
  PrintText(0,81,"ALT:"+String(faltitude,1)+"M",YELLOW,2);
  PrintText(0,99,"SAT:"+String(sats,0),YELLOW,2);
  tft.fillRect(80,220,120,18,BLUE);
  PrintText(10,220,"Speed:"+String(fspeed)+"kph",YELLOW,2);
  tft.fillRect(240,220,120,18,BLUE);
  tft.setCursor(200,220);
  tft.print("Azi: "+ordinal_bearing);
  Display_Compass();
  Display_Date_Time();
}

void Display_Compass() {
  int dxo, dyo, dxi, dyi;
  tft.setCursor(0,0);
  tft.drawCircle(centreX,centreY,diameter,WHITE);  // Draw compass circle
  for (float i = 0; i <360; i = i + 22.5) {
    dxo = diameter * cos((i-90)*3.14/180);
    dyo = diameter * sin((i-90)*3.14/180);
    dxi = dxo * 0.9;
    dyi = dyo * 0.9;
    tft.drawLine(dxo+centreX,dyo+centreY,dxi+centreX,dyi+centreY,WHITE);
  }
  PrintText((centreX-5),(centreY-diameter-18),"N",GREEN,2);
  PrintText((centreX-5),(centreY+diameter+5) ,"S",GREEN,2);
  PrintText((centreX+diameter+5),(centreY-5), "E",GREEN,2);
  PrintText((centreX-diameter-15),(centreY-5),"W",GREEN,2);
  dx = (0.85*diameter * cos((fbearing-90)*3.14/180)) + centreX; // calculate X position 
  dy = (0.85*diameter * sin((fbearing-90)*3.14/180)) + centreY; // calculate Y position 
  draw_arrow(last_dx,last_dy, centreX, centreY, 5, 5,BLUE);     // Erase last arrow      
  draw_arrow(dx,dy, centreX, centreY, 5, 5,YELLOW);             // Draw arrow in new position
  last_dx = dx; 
  last_dy = dy;
}

void draw_arrow(int x2, int y2, int x1, int y1, int alength, int awidth, int colour) {
  float distance;
  int dx, dy, x2o,y2o,x3,y3,x4,y4,k;
  distance = sqrt(pow((x1 - x2),2) + pow((y1 - y2), 2));
  dx = x2 + (x1 - x2) * alength / distance;
  dy = y2 + (y1 - y2) * alength / distance;
  k = awidth / alength;
  x2o = x2 - dx;
  y2o = dy - y2;
  x3 = y2o * k + dx;
  y3 = x2o * k + dy;
  x4 = dx - y2o * k;
  y4 = dy - x2o * k;
  tft.drawLine(x1, y1, x2, y2,colour);
  tft.drawLine(x1, y1, dx, dy,colour);
  tft.drawLine(x3, y3, x4, y4,colour);
  tft.drawLine(x3, y3, x2, y2,colour);
  tft.drawLine(x2, y2, x4, y4,colour);
} 

void Display_Date_Time(){
  PrintText(0,150,"Date/Time:",CYAN,2);
  tft.fillRect(0,165,130,19*2,BLUE);
  PrintText(0,168,(hour<10?"0":"")+String(hour)+":"+(minute<10?"0":"")+String(minute)+":"+(second<10?"0":"")+String(second),GREEN,2);
  PrintText(0,188,date,GREEN,2);
}

String Bearing_to_Ordinal(float bearing) {
  if (bearing >=    0.0  && bearing < 11.25  ) return "N";
  if (bearing >=  11.25  && bearing < 33.75 )  return "NNE";
  if (bearing >=  33.75  && bearing < 56.25 )  return "NE";
  if (bearing >=  56.25  && bearing < 78.75 )  return "ENE";
  if (bearing >=  78.75  && bearing < 101.25 ) return "E";
  if (bearing >=  101.25 && bearing < 123.75 ) return "ESE";
  if (bearing >=  123.75 && bearing < 146.25 ) return "SE";
  if (bearing >=  146.25 && bearing < 168.75 ) return "SSE";
  if (bearing >=  168.75 && bearing < 191.25 ) return "S";
  if (bearing >=  191.25 && bearing < 213.75 ) return "SSW";  
  if (bearing >=  213.75 && bearing < 236.25 ) return "SW";
  if (bearing >=  236.25 && bearing < 258.25 ) return "WSW";
  if (bearing >=  258.25 && bearing < 281.25 ) return "W";
  if (bearing >=  281.25 && bearing < 303.75 ) return "WNW";
  if (bearing >=  303.75 && bearing < 326.25 ) return "NW";
  if (bearing >=  326.25 && bearing < 348.75 ) return "NNW";
  if (bearing >=  348.75 && bearing < 360.00 ) return "N";
  else return "N";
}
/*
N    348.75   11.25
NNE   11.25   33.75
NE    33.75   56.25
ENE   56.25   78.75
E     78.75  101.25
ESE  101.25  123.75
SE   123.75  146.25
SSE  146.25  168.75
S    168.75  191.25
SSW  191.25  213.75
SW   213.75  236.25
WSW  236.25  258.75
W    258.75  281.25
WNW  281.25  303.75
NW   303.75  326.25
NNW  326.25  348.75
*/

void PrintText(int x, int y, String text, int colour, byte text_size){
  tft.setCursor(x,y);
  tft.setTextColor(colour); 
  tft.setTextSize(text_size);
  tft.print(text);
  tft.setTextColor(YELLOW); // Default colour
  tft.setTextSize(2);       // Default Text Size
}

/*
neo-6M Returns the following example sentences
$GPRMC,233512.00,A,5121.87041,N,00207.73021,W,0.436,,051217,,,A*68
$GPVTG,,T,,M,0.436,N,0.808,K,A*22
$GPGGA,233512.00,5121.87041,N,00207.73021,W,1,09,1.48,56.3,M,48.0,M,,*7D
$GPGSA,A,3,26,31,29,25,12,14,06,32,02,,,,2.16,1.48,1.57*00
$GPGSV,4,1,13,02,29,057,22,04,30,275,19,06,10,028,15,12,30,084,16*78
$GPGSV,4,2,13,14,34,249,24,21,01,174,,24,01,143,,25,69,075,35*7B
$GPGSV,4,3,13,26,13,274,22,29,73,172,17,31,51,297,28,32,22,228,25*78
$GPGSV,4,4,13,33,30,196,34*41
$GPGLL,5121.87041,N,00207.73021,W,233512.00,A,A*70
*/
