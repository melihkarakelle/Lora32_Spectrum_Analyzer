#include <SPI.h>
#include <LoRa.h>
#include <Wire.h>  
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>


#define SCK     5    // GPIO5  -- SX1278's SCK
#define MISO    19   // GPIO19 -- SX1278's MISO
#define MOSI    27   // GPIO27 -- SX1278's MOSI
#define SS      18   // GPIO18 -- SX1278's CS
#define RST     23   // GPIO14 -- SX1278's RESET
#define DI0     26   // GPIO26 -- SX1278's IRQ(Interrupt Request)
#define BAND    868E6

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32


Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

int rssi_val[128] ;
uint8_t channel_no = 0;

long main_frequency = 830000000;
long frequency = 0;

long SignalBandwidth = 500E3;

int max_rssi = 0;
long max_rssi_frequency = 0;

void displaytop(){
  //display.setRotation(0);
  display.setCursor(0,0); 
  display.print((float)(main_frequency/1000000),1);
  display.print("          ");
  display.println((float)((main_frequency+ (SignalBandwidth * 127))/1000000),1);
  
  display.print((float)(max_rssi_frequency/1000000),1);
  display.print("  ");
  display.print(max_rssi);
  display.print("dBm");
}


void setup() {

  Serial.begin(115200);
  while (!Serial);
  Serial.println();
  Serial.println("LoRa Receiver Callback");
  SPI.begin(SCK,MISO,MOSI,SS);
  LoRa.setPins(SS,RST,DI0);  
  if (!LoRa.begin(frequency)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }
  LoRa.receive();
  Serial.println("init ok");
  
    if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  // Show initial display buffer contents on the screen --
  // the library initializes this with an Adafruit splash screen.
  display.display();
  delay(500); // Pause for 2 seconds

  // Clear the buffer
  display.clearDisplay();

  display.setTextSize(1);             // Normal 1:1 pixel scale
  //display.setRotation(3);
  display.setTextColor(SSD1306_WHITE);        // Draw white text
  display.setCursor(0,0);             // Start at top-left corner
  display.println(F("Starting..."));
  display.display();

  LoRa.setSignalBandwidth(SignalBandwidth);

}

void loop() {

  frequency = main_frequency + (SignalBandwidth * channel_no);

  LoRa.setFrequency(frequency);
  LoRa.idle();
  LoRa.receive();
  delayMicroseconds(600);
  
  rssi_val[channel_no] = LoRa.rssi();

  if (rssi_val[channel_no] > max_rssi) 
     {
      max_rssi = rssi_val[channel_no] ;
      max_rssi_frequency = frequency; 
     }
  

  display.drawLine(channel_no, 0, channel_no, 64, SSD1306_BLACK);
  display.drawLine(channel_no, 64, channel_no, 64-((120+rssi_val[channel_no])/1),  SSD1306_WHITE);
  

  channel_no++;
  if (channel_no>127) {channel_no = 0;  displaytop(); display.display(); max_rssi = -200;}

 
}
