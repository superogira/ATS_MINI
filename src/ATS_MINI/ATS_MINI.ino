/*
  ------------------------------------------------------------------------------------------
  ATS_MINI
  ------------------------------------------------------------------------------------------
  This firmware is for use on the SI4732 (ESP32-S3) Mini/Pocket Receiver
  Based on the following sources:
    Ralph Xavier:      https://github.com/ralphxavier/SI4735
    PU2CLR, Ricardo:   https://github.com/pu2clr/SI4735
    Goshante:          https://github.com/goshante/ats20_ats_ex
  ------------------------------------------------------------------------------------------
  G8PTN, Dave (2025)

  ------------------------------------------------------------------------------------------
  NOTES
  ------------------------------------------------------------------------------------------ 
  - Special instructions for the TFT-eSPI library
    Ref: https://github.com/Xinyuan-LilyGO/T-Display-S3#quick-start

  - The code for this sketch was developed using Arduino IDE 1.8.19

  - The main changes from the Ralph Xavier firmware are as follows

    - User interface modified
      Removed the frequency scale
      Set "Volume" as the default adjustment parameter
      Modifed the S-Meter size and added labels
      All actions now use a single press of the rotary encoder button, with a 10s timeout
      Added status bar with indicators for Display and EEPROM write activity
      Added unit labels for "Step" and "BW"
      Added SSB tuning step options 10Hz, 25Hz, 50Hz, 0.1k and 0.5k
      Added background refresh of main screen
      	  
    - VFO/BFO tuning mechanism added based on Goshante ATS_EX firmware
      This provides "chuff" free tuning over a 28kHz span (+/- 14kHz)
      Compile option "BFO_MENU_EN" for debug purposes, manual BFO is not required

    - Modified the audio mute behaviour
      Previously the rx.setAudioMute() appeared to unmute when changing band
      The "Mute" option now toggles the volume level between 0 and previous value

    - Modified the battery monitoring function
      Uses set voltages for 25%, 50% and 75% with a configurable hysteresis voltage
      Added voltage reading to status bar
     
    - Added "Brightness" menu option
      This controls the PWM from 32 to 255 (full on) in steps of steps of 32
      When the brightness is set lower than 255, PSU or RFI noise may be present

    - Added "Calibration" menu option
      This allows the SI4732 reference clock offset to be compensated per band

    - Added Automatic Volume Control (AVC) menu option
      This allows the maximum audio gain to be adjusted

    - Settings for AGC/ATTN, SoftMute and AVC stored in EEPROM per mode
      AGC/ATTN (FM, AM, SSB), SoftMute (AM, SSB), AVC (AM, SSB)

    - Added GPIO1 (Output) control (0=FM, 1 = AM/SSB)

    - Modified FM steps options (50k, 100k, 200k, 1M)

    - Added "MODE" configuration per band (FM, AM, LSB, USB)

    - Added a REMOTE serial interface for debug control and monitoring
    

  ------------------------------------------------------------------------------------------
  PINOUT
  ------------------------------------------------------------------------------------------ 
  The pinout table is shown below.

  The relavent colums are ESP32-WROOM-1 "Pin Name" and "ATS-Mini Sketch Pin Definitions"
  
  ========================================================================================================================
  |     ESP32-WROOM-1      |  ATS-MINI Sketch      |  TFT_eSPI        |  xtronic.org  |  Comments                        |
  |  Pin #   |   Pin Name  |  Pin Definitions      |  Pin Definition  |  Schematic    |  Info                            |
  ========================================================================================================================
  |  1       |   GND       |                       |                  |  GND          |                                  |
  |  2       |   3V3       |                       |                  |  VCC_33       |                                  |
  |  3       |   EN        |                       |                  |  EN           |  RST Button                      |
  |  4       |   IO4       |  VBAT_MON             |                  |  BAT_ADC      |  Battery monitor                 |
  |  5       |   IO5       |                       |  TFT_RST         |  LCD_RES      |                                  |
  |  6       |   IO6       |                       |  TFT_CS          |  LCD_CS       |                                  |
  |  7       |   IO7       |                       |  TFT_DC          |  LCD_DC       |                                  |
  |  8       |   IO15      |  PIN_POWER_ON         |                  |  RADIO_EN     |  1= Radio LDO Enable             |
  |  9       |   IO16      |  RESET_PIN            |                  |  RST          |  SI4732 Reset                    |
  |  10      |   IO17      |  ESP32_I2C_SCL        |                  |  I2C_SCL      |  SI4732 Clock                    |
  |  11      |   IO18      |  ESP32_I2C_SDA        |                  |  I2C_SDA      |  SI4732 Data                     |
  |  12      |   IO8       |                       |  TFT_WR          |  LCD_WR       |                                  |
  |  13      |   IO19      |                       |                  |  USB_DM       |  USB_D- (CDC Port)               |
  |  14      |   IO20      |                       |                  |  USB_DP       |  USB_D+ (CDC Port)               |
  |  15      |   IO3       |  AUDIO_MUTE           |                  |  MUTE         |  1 = Mute L/R audio              |
  |  16      |   IO46      |                       |  TFT_D5          |  LCD_DS       |                                  |
  |  17      |   IO9       |                       |  TFT_RD          |  LCD_RD       |                                  |
  |  18      |   IO10      |  PIN_AMP_EN           |                  |  AMP_EN       |  1 = Audio Amp Enable            |
  |  19      |   IO11      |                       |                  |  NC           |  Spare                           |
  |  20      |   IO12      |                       |                  |  NC           |  Spare                           |
  |  21      |   IO13      |                       |                  |  NC           |  Spare                           |
  |  22      |   IO14      |                       |                  |  NC           |  Spare                           |
  |  23      |   IO21      |  ENCODER_PUSH_BUTTON  |                  |  SW           |  Rotary encoder SW signal        |
  |  24      |   IO47      |                       |  TFT_D6          |  LCD_D6       |                                  |
  |  25      |   IO48      |                       |  TFT_D7          |  LCD_D7       |                                  |
  |  26      |   IO45      |                       |  TFT_D4          |  LCD_D4       |                                  |
  |  27      |   IO0       |                       |                  |  GPIO0        |  BOOT button                     |
  |  28      |   IO35      |                       |                  |  NC           |  Used for OSPI PSRAM             |
  |  29      |   IO36      |                       |                  |  NC           |  Used for OSPI PSRAM             |
  |  30      |   IO37      |                       |                  |  NC           |  Used for OSPI PSRAM             |
  |  31      |   IO38      |  PIN_LCD_BL           |  TFT_BL          |  LCD_BL       |  Backlight control               |
  |  32      |   IO39      |                       |  TFT_D0          |  LCD_D0       |                                  |
  |  33      |   IO40      |                       |  TFT_D1          |  LCD_D1       |                                  |
  |  34      |   IO41      |                       |  TFT_D2          |  LCD_D2       |                                  |
  |  35      |   IO42      |                       |  TFT_D3          |  LCD_D2       |                                  |
  |  36      |   RXD0      |                       |                  |  NC           |  GPIO44                          |
  |  37      |   TXD0      |                       |                  |  NC           |  GPIO43                          |
  |  38      |   IO2       |  ENCODER_PIN_A        |                  |  A            |  Rotary encoder A signal         |
  |  39      |   IO1       |  ENCODER_PIN_B        |                  |  B            |  Rotary encoder B signal         |
  |  40      |   GND       |                       |                  |  GND          |                                  |
  |  41      |   EPAD      |                       |                  |  GND          |                                  |
  ========================================================================================================================

  ------------------------------------------------------------------------------------------
  VERSION SUMMARY (app_ver)
  ------------------------------------------------------------------------------------------ 

  ==========================================================================================
  | Date        | F/W Ver  |  app_ver  |  Comments                                         |
  ==========================================================================================
  | 03/03/2025  | v1.00    |  100      |  FreqUp/Dn modified, 1MHz FM step, SI4732 GPO1    |
  | 05/03/2025  | v1.01    |  101      |  Added MODE per band, Improved tuning speed       |
  |             |          |           |                                                   |
  ==========================================================================================  
  
*/

// =================================
// INCLUDE FILES
// =================================

#include <WiFi.h>
#include <WiFiUdp.h>
#include <WebServer.h>
#include <Wire.h>
#include <NTPClient.h>
#include <TFT_eSPI.h>            // https://github.com/Xinyuan-LilyGO/T-Display-S3#quick-start
#include "EEPROM.h"
#include <SI4735.h>
#include "Rotary.h"              // Disabled half-step mode
#include "patch_init.h"          // SSB patch for whole SSBRX initialization string


//Timezone Offset in seconds
const long utcOffsetInSeconds = 25200;
const char* ssid = "WiFi_SSID";
const char* password =  "WiFi_Password";

WebServer server(80);


// =================================
// PIN DEFINITIONS
// ================================= 

// SI4732/5 PINs
#define PIN_POWER_ON  15            // GPIO15   External LDO regulator enable (1 = Enable)
#define RESET_PIN     16            // GPIO16   SI4732/5 Reset
#define ESP32_I2C_SCL 17            // GPIO17   SI4732/5 Clock
#define ESP32_I2C_SDA 18            // GPIO18   SI4732/5 Data
#define AUDIO_MUTE     3            // GPIO3    Hardware L/R mute, controlled via SI4735 code (1 = Mute)
#define PIN_AMP_EN    10            // GPIO10   Hardware Audio Amplifer enable (1 = Enable)

// Display PINs
// All other pins are defined by the TFT_eSPI library
// Ref: User_Setup_Select.h
#define PIN_LCD_BL    38            // GPIO38   LCD backlight (PWM brightness control)

// Rotary Enconder PINs
#define ENCODER_PIN_A  2            // GPIO02   
#define ENCODER_PIN_B  1            // GPIO01
#define ENCODER_PUSH_BUTTON 21      // GPIO21

// Battery Monitor PIN
#define VBAT_MON  4                 // GPIO04


// =================================
// COMPILE CONSTANTS
// =================================

// Compile options (0 = Disable, 1 = Enable)
// BFO Menu option
#define BFO_MENU_EN 0         // Allows BFO menu option for debug

// Serial.print control
#define DEBUG1_PRINT 0        // Highest level - Primary information 
#define DEBUG2_PRINT 0        //               - Function call results
#define DEBUG3_PRINT 0        //               - Misc
#define DEBUG4_PRINT 0        // Lowest level  - EEPROM

// Remote Control
#define USE_REMOTE 1          // Allows basic serial control and monitoring

// Tune hold off enable (0 = Disable, 1 = Enable)
#define TUNE_HOLDOFF 1        // Whilst tuning holds off display update

// Display position control
// Added during development, code could be replaced with fixed values
#define menu_offset_x    0    // Menu horizontal offset
#define menu_offset_y   25    // Menu vertical offset
#define menu_delta_x    10    // Menu width delta
#define meter_offset_x 185    // Meter horizontal offset
#define meter_offset_y 115    // Meter vertical offset
#define freq_offset_x  260    // Frequency horizontal offset
#define freq_offset_y   75    // Frequency vertical offset
#define funit_offset_x 265    // Frequency Unit horizontal offset
#define funit_offset_y  55    // Frequency Unitvertical offset
#define mode_offset_x   95    // Mode horizontal offset
#define mode_offset_y  114    // Mode vertical offset  
#define vol_offset_x   120    // Volume horizontal offset
#define vol_offset_y   150    // Volume vertical offset
#define rds_offset_x    10    // RDS horizontal offset
#define rds_offset_y   158    // RDS vertical offset
#define batt_datum     240    // Battery meter x offset
#define clock_datum      6    // Clock x offset

// Battery Monitoring
#define BATT_ADC_READS          10  // ADC reads for average calculation (Maximum value = 16 to avoid rollover in average calculation)
#define BATT_ADC_FACTOR      1.702  // ADC correction factor used for the battery monitor
#define BATT_SOC_LEVEL1      3.680  // Battery SOC voltage for 25%
#define BATT_SOC_LEVEL2      3.780  // Battery SOC voltage for 50%
#define BATT_SOC_LEVEL3      3.880  // Battery SOC voltage for 75%
#define BATT_SOC_HYST_2      0.020  // Battery SOC hyteresis voltage divided by 2

// SI473/5 and UI
#define MIN_ELAPSED_TIME         5  // 300
#define MIN_ELAPSED_RSSI_TIME  200  // RSSI check uses IN_ELAPSED_RSSI_TIME * 6 = 1.2s
#define ELAPSED_COMMAND      10000  // time to turn off the last command controlled by encoder. Time to goes back to the VFO control // G8PTN: Increased time and corrected comment
#define ELAPSED_CLICK         1500  // time to check the double click commands
#define DEFAULT_VOLUME          35  // change it for your favorite sound volume
#define STRENGTH_CHECK_TIME   1500  // Not used
#define RDS_CHECK_TIME         250  // Increased from 90

#define BACKGROUND_REFRESH_TIME 5000    // Background screen refresh time. Covers the situation where there are no other events causing a refresh
#define TUNE_HOLDOFF_TIME         90    // Timer to hold off display whilst tuning

// Band Types
#define FM_BAND_TYPE 0
#define MW_BAND_TYPE 1
#define SW_BAND_TYPE 2
#define LW_BAND_TYPE 3

// Modes
#define FM  0
#define LSB 1
#define USB 2
#define AM  3

// Menu Options
#define BAND         0
#define MODE         1
#define VOLUME       2
#define STEP         3
#define BW           4
#define MUTE         5
#define AGC_ATT      6
#define SOFTMUTE     7
#define AVC          8
#define SPARE1       9
#define SEEKUP      10
#define SEEKDOWN    11
#define CALIBRATION 12
#define BRIGHTNESS  13
#if BFO_MENU_EN
#define BFO         14
#else
#define SPARE2      14
#endif


#define TFT_MENU_BACK TFT_BLACK              // 0x01E9
#define TFT_MENU_HIGHLIGHT_BACK TFT_BLUE

#define EEPROM_SIZE     512
#define STORE_TIME    10000                  // Time of inactivity to make the current receiver status writable (10s)
                      

// =================================
// CONSTANTS AND VARIABLES
// =================================

// SI4732/5 patch
const uint16_t size_content = sizeof ssb_patch_content; // see patch_init.h


// EEPROM 
// ====================================================================================================================================================
// Update F/W version comment as required   F/W VER    Function                                                           Locn (dec)            Bytes
// ====================================================================================================================================================
const uint8_t  app_id  = 65;          //               EEPROM ID.  If EEPROM read value mismatch, reset EEPROM            eeprom_address        1
const uint16_t app_ver = 101;         //     v1.01     EEPROM VER. If EEPROM read value mismatch (older), reset EEPROM    eeprom_ver_address    2
const int eeprom_address = 0;         //               EEPROM start address
const int eeprom_set_address = 256;   //               EEPROM setting base adddress
const int eeprom_setp_address = 272;  //               EEPROM setting (per band) base adddress
const int eeprom_ver_address = 496;   //               EEPROM version base adddress

long storeTime = millis();
bool itIsTimeToSave = false;

bool bfoOn = false;
bool ssbLoaded = false;
char bfo[18]="0000";
bool muted = false;
int8_t agcIdx = 0;
uint8_t disableAgc = 0;
int8_t agcNdx = 0;
int8_t softMuteMaxAttIdx = 4;
uint8_t countClick = 0;

uint8_t seekDirection = 1;
bool seekStop = false;        // G8PTN: Added flag to abort seeking on rotary encoder detection

bool cmdBand = false;
bool cmdVolume = false;
bool cmdAgc = false;
bool cmdBandwidth = false;
bool cmdStep = false;
bool cmdMode = false;
bool cmdMenu = false;
bool cmdSoftMuteMaxAtt = false;
bool cmdCal = false;
bool cmdBrt = false;
bool cmdAvc = false;

bool fmRDS = false;

int16_t currentBFO = 0;
long elapsedRSSI = millis();
long elapsedButton = millis();

long lastStrengthCheck = millis();
long lastRDSCheck = millis();

long elapsedClick = millis();
long elapsedCommand = millis();
volatile int encoderCount = 0;
uint16_t currentFrequency;

const uint16_t currentBFOStep = 10;

char sAgc[15];

// G8PTN: Main additional variables
// BFO and Calibration limits (BFOMax + CALMax <= 16000)
const int BFOMax = 14000;               // Maximum range for currentBFO = +/- BFOMax
const int CALMax =  2000;               // Maximum range for currentCAL = +/- CALMax

// AGC/ATTN index per mode (FM/AM/SSB)
int8_t FmAgcIdx = 0;                    // Default FM  AGGON  : Range = 0 to 37, 0 = AGCON, 1 - 27 = ATTN 0 to 26 
int8_t AmAgcIdx = 0;                    // Default AM  AGCON  : Range = 0 to 37, 0 = AGCON, 1 - 37 = ATTN 0 to 36
int8_t SsbAgcIdx = 0;                   // Default SSB AGCON  : Range = 0 to 1,  0 = AGCON,      1 = ATTN 0

// AVC index per mode (AM/SSB)
int8_t AmAvcIdx = 48;                   // Default AM  = 48 (as per AN332), range = 12 to 90 in steps of 2
int8_t SsbAvcIdx = 48;                  // Default SSB = 48, range = 12 to 90 in steps of 2

// SoftMute index per mode (AM/SSB)
int8_t AmSoftMuteIdx = 4;               // Default AM  = 4, range = 0 to 32
int8_t SsbSoftMuteIdx = 4;              // Default SSB = 4, range = 0 to 32

// Button checking
unsigned long pb1_timer = millis();     // Push button timer
int pb1_current;                        // Push button current state
int pb1_last;                           // Push button last state (after debounce)
uint16_t pb1_shift;                     // Debounce shift register
bool pb1_pressed = false;               // Push button pressed

// Status bar icon flags
bool screen_toggle = false;             // Toggle when drawsprite is called
bool eeprom_wr_flag = false;            // Flag indicating EEPROM write request

// Firmware controlled mute
uint8_t mute_vol_val = 0;               // Volume level when mute is applied

// Menu options
int16_t currentCAL = 0;                 // Calibration offset, +/- 1000Hz in steps of 10Hz
uint16_t currentBrt = 128;              // Display brightness, range = 32 to 255 in steps of 32
int8_t currentAVC = 48;                 // Selected AVC, range = 12 to 90 in steps of 2

// Background screen refresh
uint32_t background_timer = millis();   // Background screen refresh timer.
uint32_t tuning_timer = millis();       // Tuning hold off timer.
bool tuning_flag = false;               // Flag to indicate tuning

// Battery monitoring
uint16_t adc_read_total = 0;            // Total ADC count
uint16_t adc_read_avr;                  // Average ADC count = adc_read_total / BATT_ADC_READS
float adc_volt_avr;                     // Average ADC voltage with correction
uint8_t batt_soc_state = 255;           // State machine used for battery state of charge (SOC) detection with hysteresis (Default = Illegal state)

// Time
uint32_t clock_timer = 0;
uint8_t time_seconds = 0;
uint8_t time_minutes = 0;
uint8_t time_hours = 0;
char time_disp [16];

// Remote serial
#if USE_REMOTE
uint32_t g_remote_timer = millis();
uint8_t g_remote_seqnum = 0;
#endif


// Tables

// Menu Description
#if BFO_MENU_EN
// With BFO           <---- 00 ----> <---- 01 ----> <---- 02 ----> <---- 03 ----> <---- 04 ----> <---- 05 ----> <---- 06 ----> <---- 07 ----> <---- 08 ----> <---- 09 ---->
const char *menu[] = {    "Band",        "Mode",       "Volume",       "Step",     "Bandwidth",      "Mute",      "AGC/ATTN",    "SoftMute",       "AVC",       "Spare 1",


//                    <---- 10 ----> <---- 11 ----> <---- 12 ----> <---- 13 ----> <---- 14 ----> 
                         "Seek Up",     "Seek Dn",   "Calibration", "Brightness",     "BFO"     };


#else
// Without BFO        <---- 00 ----> <---- 01 ----> <---- 02 ----> <---- 03 ----> <---- 04 ----> <---- 05 ----> <---- 06 ----> <---- 07 ----> <---- 08 ----> <---- 09 ---->
const char *menu[] = {    "Band",        "Mode",       "Volume",       "Step",     "Bandwidth",      "Mute",      "AGC/ATTN",    "SoftMute",       "AVC",       "Spare 1",

//                    <---- 10 ----> <---- 11 ----> <---- 12 ----> <---- 13 ----> <---- 14 ---->
                         "Seek Up",     "Seek Dn",   "Calibration", "Brightness",    "Spare 2"  };

#endif

int8_t menuIdx = VOLUME;
const int lastMenu = (sizeof menu / sizeof(char *)) - 1;
int8_t currentMenuCmd = -1;

typedef struct
{
  uint8_t idx;      // SI473X device bandwidth index
  const char *desc; // bandwidth description
} Bandwidth;

int8_t bwIdxSSB = 4;
const int8_t maxSsbBw = 5;
Bandwidth bandwidthSSB[] = {
  {4, "0.5k"},
  {5, "1.0k"},
  {0, "1.2k"},
  {1, "2.2k"},
  {2, "3.0k"},
  {3, "4.0k"}
};
const int lastBandwidthSSB = (sizeof bandwidthSSB / sizeof(Bandwidth)) - 1;

int8_t bwIdxAM = 4;
const int8_t maxAmBw = 6;
Bandwidth bandwidthAM[] = {
  {4, "1.0k"},
  {5, "1.8k"},
  {3, "2.0k"},
  {6, "2.5k"},
  {2, "3.0k"},
  {1, "4.0k"},
  {0, "6.0k"}
};
const int lastBandwidthAM = (sizeof bandwidthAM / sizeof(Bandwidth)) - 1;

int8_t bwIdxFM = 0;
const int8_t maxFmBw = 4;
Bandwidth bandwidthFM[] = {
    {0, "Auto"}, // Automatic - default
    {1, "110k"}, // Force wide (110 kHz) channel filter.
    {2, "84k"},
    {3, "60k"},
    {4, "40k"}};
const int lastBandwidthFM = (sizeof bandwidthFM / sizeof(Bandwidth)) - 1;


int tabAmStep[] = {1,      // 0   AM/SSB   (kHz)
                   5,      // 1   AM/SSB   (kHz)
                   9,      // 2   AM/SSB   (kHz)
                   10,     // 3   AM/SSB   (kHz)
                   50,     // 4   AM       (kHz)
                   100,    // 5   AM       (kHz)
                   1000,   // 6   AM       (kHz)
                   10,     // 7   SSB      (Hz)
                   25,     // 8   SSB      (Hz)
                   50,     // 9   SSB      (Hz)
                   100,    // 10  SSB      (Hz)
                   500};   // 11  SSB      (Hz)

uint8_t AmTotalSteps = 7;                          // Total AM steps
uint8_t AmTotalStepsSsb = 4;                       // G8PTN: Original : AM(LW/MW) 1k, 5k, 9k, 10k, 50k        : SSB 1k, 5k, 9k, 10k
//uint8_t AmTotalStepsSsb = 5;                     // G8PTN: Option 1 : AM(LW/MW) 1k, 5k, 9k, 10k, 100k       : SSB 1k, 5k, 9k, 10k, 50k
//uint8_t AmTotalStepsSsb = 6;                     // G8PTN: Option 2 : AM(LW/MW) 1k, 5k, 9k, 10k, 100k , 1M  : SSB 1k, 5k, 9k, 10k, 50k, 100k
//uint8_t AmTotalStepsSsb = 7;                     // G8PTN: Invalid option (Do not use)
uint8_t SsbTotalSteps = 5;                         // SSB sub 1kHz steps 
volatile int8_t idxAmStep = 3;


const char *AmSsbStepDesc[] = {"1k", "5k", "9k", "10k", "50k", "100k", "1M", "10Hz", "25Hz", "50Hz", "0.1k", "0.5k"};


int tabFmStep[] = {5, 10, 20, 100};                             // G8PTN: Added 1MHz step
const int lastFmStep = (sizeof tabFmStep / sizeof(int)) - 1;
int idxFmStep = 1;

const char *FmStepDesc[] = {"50k", "100k", "200k", "1M"};


uint16_t currentStepIdx = 1;


const char *bandModeDesc[] = {"FM", "LSB", "USB", "AM"};
const int lastBandModeDesc = (sizeof bandModeDesc / sizeof(char *)) - 1;
uint8_t currentMode = FM;


/**
 *  Band data structure
 */
typedef struct
{
  const char *bandName;   // Band description
  uint8_t bandType;       // Band type (FM, MW or SW)
  uint16_t minimumFreq;   // Minimum frequency of the band
  uint16_t maximumFreq;   // maximum frequency of the band
  uint16_t currentFreq;   // Default frequency or current frequency
  int8_t currentStepIdx;  // Idex of tabStepAM:  Defeult frequency step (See tabStepAM)
  int8_t bandwidthIdx;    // Index of the table bandwidthFM, bandwidthAM or bandwidthSSB;
} Band;

/*
   Band table
   YOU CAN CONFIGURE YOUR OWN BAND PLAN. Be guided by the comments.
   To add a new band, all you have to do is insert a new line in the table below. No extra code will be needed.
   You can remove a band by deleting a line if you do not want a given band. 
   Also, you can change the parameters of the band.
   ATTENTION: You have to RESET the eeprom after adding or removing a line of this table. 
              Turn your receiver on with the encoder push button pressed at first time to RESET the eeprom content.  
*/
Band band[] = {
    {"VHF", FM_BAND_TYPE, 6400, 10800, 10390, 1, 0},
    {"MW1", MW_BAND_TYPE, 150, 1720, 810, 3, 4},
    {"MW2", MW_BAND_TYPE, 531, 1701, 783, 2, 4},
    {"MW2", MW_BAND_TYPE, 1700, 3500, 2500, 1, 4},
    {"80M", MW_BAND_TYPE, 3500, 4000, 3700, 0, 4},
    {"SW1", SW_BAND_TYPE, 4000, 5500, 4885, 1, 4},
    {"SW2", SW_BAND_TYPE, 5500, 6500, 6000, 1, 4},
    {"40M", SW_BAND_TYPE, 6500, 7300, 7100, 0, 4},
    {"SW3", SW_BAND_TYPE, 7200, 8000, 7200, 1, 4},
    {"SW4", SW_BAND_TYPE, 9000, 11000, 9500, 1, 4},
    {"SW5", SW_BAND_TYPE, 11100, 13000, 11900, 1, 4},
    {"SW6", SW_BAND_TYPE, 13000, 14000, 13500, 1, 4},
    {"20M", SW_BAND_TYPE, 14000, 15000, 14200, 0, 4},
    {"SW7", SW_BAND_TYPE, 15000, 17000, 15300, 1, 4},
    {"SW8", SW_BAND_TYPE, 17000, 18000, 17500, 1, 4},
    {"15M", SW_BAND_TYPE, 20000, 21400, 21100, 0, 4},
    {"SW9", SW_BAND_TYPE, 21400, 22800, 21500, 1, 4},
    {"CB ", SW_BAND_TYPE, 26000, 28000, 27500, 0, 4},
    {"10M", SW_BAND_TYPE, 28000, 30000, 28400, 0, 4},
    {"ALL", SW_BAND_TYPE, 150, 30000, 15000, 0, 4} // All band. LW, MW and SW (from 150kHz to 30MHz)
};                                             

const int lastBand = (sizeof band / sizeof(Band)) - 1;
int bandIdx = 0;

//int tabStep[] = {1, 5, 10, 50, 100, 500, 1000};
//const int lastStep = (sizeof tabStep / sizeof(int)) - 1;


// Calibration (per band). Size needs to be the same as band[]
// Defaults
int16_t bandCAL[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

// Mode (per band). Size needs to be the same as band[] and mode needs to be appropriate for bandType
// Example bandType = FM_BAND_TYPE, bandMODE = FM. All other BAND_TYPE's, bandMODE = AM/LSB/USB
// Defaults
uint8_t bandMODE[] = {FM, AM, AM, AM, LSB, AM, AM, LSB, AM, AM, AM, AM, USB, AM, AM, USB, AM, AM, USB, AM};

char *rdsMsg;
char *stationName;
char *rdsTime;
char bufferStationName[50];
char bufferRdsMsg[100];
char bufferRdsTime[32];

uint8_t rssi = 0;
uint8_t snr = 0;
uint8_t volume = DEFAULT_VOLUME;


// SSB Mode detection
bool isSSB()
{
    return currentMode > FM && currentMode < AM;    // This allows for adding CW mode as well as LSB/USB if required
}


// Generation of step value
int getSteps()
{
    if (isSSB())
    {
        if (idxAmStep >= AmTotalSteps)
            return tabAmStep[idxAmStep];            // SSB: Return in Hz used for VFO + BFO tuning

        return tabAmStep[idxAmStep] * 1000;         // SSB: Return in Hz used for VFO + BFO tuning
    }

    if (idxAmStep >= AmTotalSteps)                  // AM: Set to 0kHz if step is from the SSB Hz values 
        idxAmStep = 0;

    return tabAmStep[idxAmStep];                    // AM: Return value in KHz for SI4732 step
}


// Generate last step index
int getLastStep()
{
  // Debug
  #if DEBUG2_PRINT
  Serial.print("Info: getLastStep() >>> AmTotalSteps = ");
  Serial.print(AmTotalSteps);
  Serial.print(", SsbTotalSteps = ");
  Serial.print(SsbTotalSteps);
  Serial.print(", isSSB = ");
  Serial.println(isSSB());
  #endif
  
  if (isSSB())
    return AmTotalSteps + SsbTotalSteps - 1;
  else if (bandIdx == LW_BAND_TYPE || bandIdx == MW_BAND_TYPE)    // G8PTN; Added in place of check in doStep() for LW/MW step limit
    return AmTotalStepsSsb;
  else
    return AmTotalSteps - 1;
}


// Devices class declarations
Rotary encoder = Rotary(ENCODER_PIN_B, ENCODER_PIN_A);      // G8PTN: Corrected mapping based on rotary library


TFT_eSPI tft = TFT_eSPI();
TFT_eSprite spr = TFT_eSprite(&tft);

SI4735 rx;


// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds);

int hours = 0;
int minutes = 0;

void setup()
{
  // Enable Serial. G8PTN: Added
  Serial.begin(115200);
  delay(1000);

  WiFi.mode(WIFI_STA); //Optional
  WiFi.begin(ssid, password);
  Serial.println("\nConnecting");
  while(WiFi.status() != WL_CONNECTED){
    Serial.print(".");
    delay(500);
  }
  Serial.println("\nConnected to the WiFi network");
  Serial.print("Local ESP32 IP: ");
  Serial.println(WiFi.localIP());
  timeClient.update();

  server.on("/", handle_OnConnect);
  server.onNotFound(handle_NotFound);
  server.begin();
  Serial.println("HTTP server started");

  // Audio Amplifier Enable. G8PTN: Added
  // Initally disable the audio amplifier until the SI4732 has been setup
  pinMode(PIN_AMP_EN, OUTPUT);
  digitalWrite(PIN_AMP_EN, LOW);

  // SI4732 VDD Enable
  pinMode(PIN_POWER_ON, OUTPUT);
  digitalWrite(PIN_POWER_ON, HIGH);

  // Encoder pins. Enable internal pull-ups
  pinMode(ENCODER_PUSH_BUTTON, INPUT_PULLUP); 
  pinMode(ENCODER_PIN_A, INPUT_PULLUP);
  pinMode(ENCODER_PIN_B, INPUT_PULLUP);

  // The line below may be necessary to setup I2C pins on ESP32
  Wire.begin(ESP32_I2C_SDA, ESP32_I2C_SCL);

  // TFT display setup
  tft.begin();
  tft.setRotation(3);
  tft.fillScreen(TFT_BLACK);
  spr.createSprite(320,170);
  spr.setTextDatum(MC_DATUM);
  spr.setSwapBytes(true);
  spr.setFreeFont(&Orbitron_Light_24);
  spr.setTextColor(TFT_WHITE,TFT_BLACK);

  // TFT display brightness control (PWM)
  // Note: At brightness levels below 100%, switching from the PWM may cause power spikes and/or RFI
  ledcSetup(0, 16000, 8);           // Port 0, 16kHz, 8-bit
  ledcAttachPin(PIN_LCD_BL, 0);     // Pin assignment
  ledcWrite(0, 255);                // Default value 255 = 100%)

  // Display startup information
  tft.setTextSize(2);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);

  uint16_t ver_major = (app_ver / 100);
  uint16_t ver_minor = (app_ver % 100);
  char fw_ver [16];
  sprintf(fw_ver, "F/W: v%1.1d.%2.2d", ver_major, ver_minor);
  tft.println("ATS-Mini Receiver");
  tft.println(fw_ver);
  tft.println();
  tft.println("To reset EEPROM");
  tft.println("Press+Hold ENC Button");
  tft.println();     
  tft.println("IP Address : ");
  tft.println(WiFi.localIP());
  delay(3000);

  // EEPROM
  // Note: Use EEPROM.begin(EEPROM_SIZE) before use and EEPROM.begin.end after use to free up memory and avoid memory leaks
  EEPROM.begin(EEPROM_SIZE);

  // Press and hold Encoder button to force an EEPROM reset
  // Indirectly forces the reset by setting app_id = 0 (Detectected in the subsequent check for app_id and app_ver)
  // Note: EEPROM reset is recommended after firmware updates
  if (digitalRead(ENCODER_PUSH_BUTTON) == LOW)
  {
    EEPROM.write(eeprom_address, 0);
    EEPROM.commit();
    tft.setTextColor(TFT_RED, TFT_BLACK);
    tft.print("EEPROM Resetting");
    delay(1000);
  }

  EEPROM.end();

  // G8PTN: Moved this to later, to avoid interrupt action
  /*
  // ICACHE_RAM_ATTR void rotaryEncoder(); see rotaryEncoder implementation below.
  attachInterrupt(digitalPinToInterrupt(ENCODER_PIN_A), rotaryEncoder, CHANGE);
  attachInterrupt(digitalPinToInterrupt(ENCODER_PIN_B), rotaryEncoder, CHANGE);
  */

  // Check for SI4732 connected on I2C interface
  // If the SI4732 is not detected, then halt with no further processing
  rx.setI2CFastModeCustom(100000);
  
  int16_t si4735Addr = rx.getDeviceI2CAddress(RESET_PIN); // Looks for the I2C bus address and set it.  Returns 0 if error

  if ( si4735Addr == 0 ) {
    tft.setTextSize(2);
    tft.setTextColor(TFT_RED, TFT_BLACK);
    tft.println("Si4735 not detected");
    while (1);
  }  
  
  rx.setup(RESET_PIN, MW_BAND_TYPE);
  // Comment the line above and uncomment the three lines below if you are using external ref clock (active crystal or signal generator)
  // rx.setRefClock(32768);
  // rx.setRefClockPrescaler(1);   // will work with 32768  
  // rx.setup(RESET_PIN, 0, MW_BAND_TYPE, SI473X_ANALOG_AUDIO, XOSCEN_RCLK);

  // Attached pin to allows SI4732 library to mute audio as required to minimise loud clicks
  rx.setAudioMuteMcuPin(AUDIO_MUTE);  
  
  cleanBfoRdsInfo();
  
  delay(300);
  
  // Audio Amplifier Enable. G8PTN: Added
  // After the SI4732 has been setup, enable the audio amplifier
  digitalWrite(PIN_AMP_EN, HIGH);

  
  // Checking the EEPROM content
  // Checks app_id (which covers manual reset) and app_ver which allows for automatic reset
  // The app_ver is equivalent to a F/W version.

  // Debug
  // Read all EEPROM locations
  #if DEBUG4_PRINT
  EEPROM.begin(EEPROM_SIZE);
  Serial.println("**** EEPROM READ: Pre Check");
  for (int i = 0; i <= (EEPROM_SIZE - 1); i++){
    Serial.print(EEPROM.read(i));
    delay(10);
    Serial.print("\t");
    if (i%16 == 15) Serial.println();
  }
  Serial.println("****");
  EEPROM.end();
  #endif

  // Perform check against app_id and app_ver
  uint8_t  id_read;
  uint16_t ver_read;

  EEPROM.begin(EEPROM_SIZE);
  id_read = EEPROM.read(eeprom_address);
  ver_read  = EEPROM.read(eeprom_ver_address) << 8;
  ver_read |= EEPROM.read(eeprom_ver_address + 1);
  EEPROM.end();

  if ((id_read == app_id) && (ver_read == app_ver)) {
    readAllReceiverInformation();                        // Load EEPROM values
  }
  else {
    saveAllReceiverInformation();                        // Set EEPROM to defaults
    rx.setVolume(volume);                                // Set initial volume after EEPROM reset
    ledcWrite(0, currentBrt);                            // Set initial brightness after EEPROM reset
  }

  // Debug
  // Read all EEPROM locations
  #if DEBUG4_PRINT
  EEPROM.begin(EEPROM_SIZE);
  Serial.println("**** START READ: Post check actions");
  for (int i = 0; i <= (EEPROM_SIZE - 1); i++){
    Serial.print(EEPROM.read(i));
    delay(10);
    Serial.print("\t");
    if (i%16 == 15) Serial.println();
  }
  Serial.println("****");
  EEPROM.end();
  #endif

  // ** SI4732 STARTUP **
  // Uses values from EEPROM (Last stored or defaults after EEPROM reset) 
  useBand();
  
  showStatus();
  drawSprite();

  // Interrupt actions for Rotary encoder
  // Note: Moved to end of setup to avoid inital interrupt actions
  // ICACHE_RAM_ATTR void rotaryEncoder(); see rotaryEncoder implementation below.
  attachInterrupt(digitalPinToInterrupt(ENCODER_PIN_A), rotaryEncoder, CHANGE);
  attachInterrupt(digitalPinToInterrupt(ENCODER_PIN_B), rotaryEncoder, CHANGE);
}


/**
 * Prints a given content on display 
 */
void print(uint8_t col, uint8_t lin, const GFXfont *font, uint8_t textSize, const char *msg) {
  tft.setCursor(col,lin);
  tft.setTextSize(textSize);
  tft.setTextColor(TFT_RED, TFT_BLACK);
  tft.println(msg);
}

void printParam(const char *msg) {
 tft.fillScreen(TFT_BLACK);
 print(0,10,NULL,2, msg);
 }

/*
   writes the conrrent receiver information into the eeprom.
   The EEPROM.update avoid write the same data in the same memory position. It will save unnecessary recording.
*/
void saveAllReceiverInformation()
{
  eeprom_wr_flag = true;
  int addr_offset;
  int16_t currentBFOs = (currentBFO % 1000);            // G8PTN: For SSB ensures BFO value is valid wrt band[bandIdx].currentFreq = currentFrequency;

  EEPROM.begin(EEPROM_SIZE);

  EEPROM.write(eeprom_address, app_id);                 // Stores the app id;
  EEPROM.write(eeprom_address + 1, rx.getVolume());     // Stores the current Volume
  EEPROM.write(eeprom_address + 2, bandIdx);            // Stores the current band
  EEPROM.write(eeprom_address + 3, fmRDS);              // G8PTN: Not used
  EEPROM.write(eeprom_address + 4, currentMode);        // Stores the current Mode (FM / AM / LSB / USB). Now per mode, leave for compatibility
  EEPROM.write(eeprom_address + 5, currentBFOs >> 8);   // G8PTN: Stores the current BFO % 1000 (HIGH byte)
  EEPROM.write(eeprom_address + 6, currentBFOs & 0XFF); // G8PTN: Stores the current BFO % 1000 (LOW byte)
  EEPROM.commit();

  addr_offset = 7;

  // G8PTN: Commented out the assignment
  // - The line appears to be required to ensure the band[bandIdx].currentFreq = currentFrequency
  // - Updated main code to ensure that this should occur as required with frequency, band or mode changes
  // - The EEPROM reset code now calls saveAllReceiverInformation(), which is the correct action, this line
  //   must be disabled otherwise band[bandIdx].currentFreq = 0 (where bandIdx = 0; by default) on EEPROM reset
  //band[bandIdx].currentFreq = currentFrequency;

  for (int i = 0; i <= lastBand; i++)
  {
    EEPROM.write(addr_offset++, (band[i].currentFreq >> 8));   // Stores the current Frequency HIGH byte for the band
    EEPROM.write(addr_offset++, (band[i].currentFreq & 0xFF)); // Stores the current Frequency LOW byte for the band
    EEPROM.write(addr_offset++, band[i].currentStepIdx);       // Stores current step of the band
    EEPROM.write(addr_offset++, band[i].bandwidthIdx);         // table index (direct position) of bandwidth
    EEPROM.commit();
  }

  // G8PTN: Added
  addr_offset = eeprom_set_address;
  EEPROM.write(addr_offset++, currentBrt >> 8);         // Stores the current Brightness value (HIGH byte)
  EEPROM.write(addr_offset++, currentBrt & 0XFF);       // Stores the current Brightness value (LOW byte)
  EEPROM.write(addr_offset++, FmAgcIdx);                // Stores the current FM AGC/ATTN index value
  EEPROM.write(addr_offset++, AmAgcIdx);                // Stores the current AM AGC/ATTN index value
  EEPROM.write(addr_offset++, SsbAgcIdx);               // Stores the current SSB AGC/ATTN index value
  EEPROM.write(addr_offset++, AmAvcIdx);                // Stores the current AM AVC index value
  EEPROM.write(addr_offset++, SsbAvcIdx);               // Stores the current SSB AVC index value
  EEPROM.write(addr_offset++, AmSoftMuteIdx);           // Stores the current AM SoftMute index value
  EEPROM.write(addr_offset++, SsbSoftMuteIdx);          // Stores the current SSB SoftMute index value
  EEPROM.commit();

  addr_offset = eeprom_setp_address;
  for (int i = 0; i <= lastBand; i++)
  {
    EEPROM.write(addr_offset++, (bandCAL[i] >> 8));     // Stores the current Calibration value (HIGH byte) for the band
    EEPROM.write(addr_offset++, (bandCAL[i] & 0XFF));   // Stores the current Calibration value (LOW byte) for the band
    EEPROM.write(addr_offset++,  bandMODE[i]);          // Stores the current Mode value for the band
    EEPROM.commit();
  }  

  addr_offset = eeprom_ver_address;  
  EEPROM.write(addr_offset++, app_ver >> 8);            // Stores app_ver (HIGH byte)
  EEPROM.write(addr_offset++, app_ver & 0XFF);          // Stores app_ver (LOW byte)
  EEPROM.commit();

  EEPROM.end();
}

/**
 * reads the last receiver status from eeprom. 
 */
void readAllReceiverInformation()
{
  uint8_t volume;
  int addr_offset;
  int bwIdx;
  EEPROM.begin(EEPROM_SIZE);

  volume = EEPROM.read(eeprom_address + 1); // Gets the stored volume;
  bandIdx = EEPROM.read(eeprom_address + 2);
  fmRDS = EEPROM.read(eeprom_address + 3);                // G8PTN: Not used
  currentMode = EEPROM.read(eeprom_address + 4);          // G8PTM: Reads stored Mode. Now per mode, leave for compatibility
  currentBFO = EEPROM.read(eeprom_address + 5) << 8;      // G8PTN: Reads stored BFO value (HIGH byte)
  currentBFO |= EEPROM.read(eeprom_address + 6);          // G8PTN: Reads stored BFO value (HIGH byte)

  addr_offset = 7;
  for (int i = 0; i <= lastBand; i++)
  {
    band[i].currentFreq = EEPROM.read(addr_offset++) << 8;
    band[i].currentFreq |= EEPROM.read(addr_offset++);
    band[i].currentStepIdx = EEPROM.read(addr_offset++);
    band[i].bandwidthIdx = EEPROM.read(addr_offset++);
  }

  // G8PTN: Added
  addr_offset = eeprom_set_address;
  currentBrt      = EEPROM.read(addr_offset++) << 8;      // Reads stored Brightness value (HIGH byte)
  currentBrt     |= EEPROM.read(addr_offset++);           // Reads stored Brightness value (LOW byte)
  FmAgcIdx        = EEPROM.read(addr_offset++);           // Reads stored FM AGC/ATTN index value
  AmAgcIdx        = EEPROM.read(addr_offset++);           // Reads stored AM AGC/ATTN index value
  SsbAgcIdx       = EEPROM.read(addr_offset++);           // Reads stored SSB AGC/ATTN index value
  AmAvcIdx        = EEPROM.read(addr_offset++);           // Reads stored AM AVC index value
  SsbAvcIdx       = EEPROM.read(addr_offset++);           // Reads stored SSB AVC index value
  AmSoftMuteIdx   = EEPROM.read(addr_offset++);           // Reads stored AM SoftMute index value
  SsbSoftMuteIdx  = EEPROM.read(addr_offset++);           // Reads stored SSB SoftMute index value

  addr_offset = eeprom_setp_address;
  for (int i = 0; i <= lastBand; i++)
  {
    bandCAL[i]    = EEPROM.read(addr_offset++) << 8;      // Reads stored Calibration value (HIGH byte) per band
    bandCAL[i]   |= EEPROM.read(addr_offset++);           // Reads stored Calibration value (LOW byte) per band
    bandMODE[i]   = EEPROM.read(addr_offset++);           // Reads stored Mode value per band
  }

  EEPROM.end();

  // G8PTN: Added
  ledcWrite(0, currentBrt);

  currentFrequency = band[bandIdx].currentFreq;
  currentMode = bandMODE[bandIdx];                       // G8PTN: Added to support mode per band
  
  if (band[bandIdx].bandType == FM_BAND_TYPE)
  {
    currentStepIdx = idxFmStep = band[bandIdx].currentStepIdx;
    rx.setFrequencyStep(tabFmStep[currentStepIdx]);
  }
  else
  {
    currentStepIdx = idxAmStep = band[bandIdx].currentStepIdx;
    rx.setFrequencyStep(tabAmStep[currentStepIdx]);
  }

  bwIdx = band[bandIdx].bandwidthIdx;

  if (isSSB())
  {
    loadSSB();
    bwIdxSSB = (bwIdx > 5) ? 5 : bwIdx;
    rx.setSSBAudioBandwidth(bandwidthSSB[bwIdxSSB].idx);
    // If audio bandwidth selected is about 2 kHz or below, it is recommended to set Sideband Cutoff Filter to 0.
    if (bandwidthSSB[bwIdxSSB].idx == 0 || bandwidthSSB[bwIdxSSB].idx == 4 || bandwidthSSB[bwIdxSSB].idx == 5)
      rx.setSSBSidebandCutoffFilter(0);
    else
      rx.setSSBSidebandCutoffFilter(1);
      updateBFO();
  }
  else if (currentMode == AM)
  {
    bwIdxAM = bwIdx;
    rx.setBandwidth(bandwidthAM[bwIdxAM].idx, 1);
  }
  else
  {
    bwIdxFM = bwIdx;
    rx.setFmBandwidth(bandwidthFM[bwIdxFM].idx);
  }

  if (currentBFO > 0)
    sprintf(bfo, "+%4.4d", currentBFO);
  else
    sprintf(bfo, "%4.4d", currentBFO);

  delay(50);
  rx.setVolume(volume);
}

/*
 * To store any change into the EEPROM, it is needed at least STORE_TIME  milliseconds of inactivity.
 */
void resetEepromDelay()
{
  elapsedCommand = storeTime = millis();
  itIsTimeToSave = true;
}

/**
    Set all command flags to false
    When all flags are disabled (false), the encoder controls the frequency
*/
void disableCommands()
{
  cmdBand = false;
  bfoOn = false;
  cmdVolume = false;
  cmdAgc = false;
  cmdBandwidth = false;
  cmdStep = false;
  cmdMode = false;
  cmdMenu = false;
  cmdSoftMuteMaxAtt = false;
  countClick = 0;
  // showCommandStatus((char *) "VFO ");
  cmdCal = false;
  cmdBrt = false;
  cmdAvc = false;  
  
}

/**
 * Reads encoder via interrupt
 * Use Rotary.h and  Rotary.cpp implementation to process encoder via interrupt
 * if you do not add ICACHE_RAM_ATTR declaration, the system will reboot during attachInterrupt call. 
 * With ICACHE_RAM_ATTR macro you put the function on the RAM.
 */
ICACHE_RAM_ATTR void  rotaryEncoder()
{ // rotary encoder events
  uint8_t encoderStatus = encoder.process();
  if (encoderStatus) {
    encoderCount = (encoderStatus == DIR_CW) ? 1 : -1;
    seekStop = true;  // G8PTN: Added flag
  }
}

/**
 * Shows frequency information on Display
 */
void showFrequency()
{
  char tmp[15];
  sprintf(tmp, "%5.5u", currentFrequency);
  drawSprite();
  // showMode();
}

/**
 * Shows the current mode
 */
void showMode() {
  drawSprite();    
}

/**
 * Shows some basic information on display
 */
void showStatus()
{
  showFrequency();
  showRSSI();
}

/**
 *  Shows the current Bandwidth status
 */
void showBandwidth()
{
  drawSprite();
}

/**
 *   Shows the current RSSI and SNR status
 */
void showRSSI()
{
  char sMeter[10];
  sprintf(sMeter, "S:%d ", rssi);
  drawSprite();
}

/**
 *    Shows the current AGC and Attenuation status
 */
void showAgcAtt()
{
  // lcd.clear();
  //rx.getAutomaticGainControl();             // G8PTN: Read back value is not used
  if (agcNdx == 0 && agcIdx == 0)
    strcpy(sAgc, "AGC ON");
  else
    sprintf(sAgc, "ATT: %2.2d", agcNdx);

  drawSprite();

}

/**
 *   Shows the current step
 */
void showStep()
{
  drawSprite();
}

/**
 *  Shows the current BFO value
 */
void showBFO()
{
  
  if (currentBFO > 0)
    sprintf(bfo, "+%4.4d", currentBFO);
  else
    sprintf(bfo, "%4.4d", currentBFO);
  drawSprite();
  elapsedCommand = millis();
}

/*
 *  Shows the volume level on LCD
 */
void showVolume()
{
drawSprite();
}

/**
 * Show Soft Mute 
 */
void showSoftMute()
{
  drawSprite();
}

/**
 *   Sets Band up (1) or down (!1)
 */
void setBand(int8_t up_down)
{
  // G8PTN: Reset BFO when changing band and store frequency
  band[bandIdx].currentFreq = currentFrequency + (currentBFO / 1000);  
  currentBFO = 0;

  band[bandIdx].currentStepIdx = currentStepIdx;
  if (up_down == 1)                                            // G8PTN: Corrected direction
    bandIdx = (bandIdx < lastBand) ? (bandIdx + 1) : 0;
  else
    bandIdx = (bandIdx > 0) ? (bandIdx - 1) : lastBand;

  // G8PTN: Added to support mode per band
  currentMode = bandMODE[bandIdx];
  if (isSSB())
  {
    if (ssbLoaded == false)
    {
      // Only loadSSB if not already loaded
      spr.fillSmoothRoundRect(80,40,160,40,4,TFT_WHITE);
      spr.fillSmoothRoundRect(81,41,158,38,4,TFT_MENU_BACK);
      spr.drawString("Loading SSB",160,62,4);
      spr.pushSprite(0,0);
    
      loadSSB();
      ssbLoaded = true;  
    }
  } 
  else {
    // If not SSB
    ssbLoaded = false;    
  }

  
  useBand();
  delay(MIN_ELAPSED_TIME); // waits a little more for releasing the button.
  elapsedCommand = millis();
}

/**
 * Switch the radio to current band
 */
void useBand()
{
  currentMode = bandMODE[bandIdx];                  // G8PTN: Added to support mode per band
  if (band[bandIdx].bandType == FM_BAND_TYPE)
  {
    currentMode = FM;
    rx.setTuneFrequencyAntennaCapacitor(0);
    rx.setFM(band[bandIdx].minimumFreq, band[bandIdx].maximumFreq, band[bandIdx].currentFreq, tabFmStep[band[bandIdx].currentStepIdx]);
    rx.setSeekFmLimits(band[bandIdx].minimumFreq, band[bandIdx].maximumFreq);
    bfoOn = ssbLoaded = false;
    bwIdxFM = band[bandIdx].bandwidthIdx;
    rx.setFmBandwidth(bandwidthFM[bwIdxFM].idx);
    rx.setFMDeEmphasis(1);
    rx.RdsInit();
    rx.setRdsConfig(1, 2, 2, 2, 2);
    rx.setGpioCtl(1,0,0);   // G8PTN: Enable GPIO1 as output
    rx.setGpio(0,0,0);      // G8PTN: Set GPIO1 = 0
  }
  else
  {
    // set the tuning capacitor for SW or MW/LW
    rx.setTuneFrequencyAntennaCapacitor((band[bandIdx].bandType == MW_BAND_TYPE || band[bandIdx].bandType == LW_BAND_TYPE) ? 0 : 1);
    if (ssbLoaded)
    {
      // Configure SI4732 for SSB
      rx.setSSB(
        band[bandIdx].minimumFreq,
        band[bandIdx].maximumFreq,
        band[bandIdx].currentFreq,
        0,                                                  // SI4732 step is not used for SSB! 
        currentMode);
     
      rx.setSSBAutomaticVolumeControl(1);                   // G8PTN: Always enabled
      //rx.setSsbSoftMuteMaxAttenuation(softMuteMaxAttIdx); // G8PTN: Commented out
      if   (band[bandIdx].bandwidthIdx > 5) bwIdxSSB = 5;   // G8PTN: Limit value
      else bwIdxSSB = band[bandIdx].bandwidthIdx;
      rx.setSSBAudioBandwidth(bandwidthSSB[bwIdxSSB].idx);
      updateBFO();                                          // G8PTN: If SSB is loaded update BFO
    }
    else
    {
      currentMode = AM;
      rx.setAM(
        band[bandIdx].minimumFreq,
        band[bandIdx].maximumFreq,
        band[bandIdx].currentFreq,
        band[bandIdx].currentStepIdx >= AmTotalSteps ? 1 : tabAmStep[band[bandIdx].currentStepIdx]);   // Set to 1kHz
      
      bfoOn = false;
      bwIdxAM = band[bandIdx].bandwidthIdx;
      rx.setBandwidth(bandwidthAM[bwIdxAM].idx, 1);
      //rx.setAmSoftMuteMaxAttenuation(softMuteMaxAttIdx); //Soft Mute for AM or SSB
    }
    rx.setGpioCtl(1,0,0);   // G8PTN: Enable GPIO1 as output
    rx.setGpio(1,0,0);      // G8PTN: Set GPIO1 = 1
    rx.setSeekAmLimits(band[bandIdx].minimumFreq, band[bandIdx].maximumFreq); // Consider the range all defined current band
    rx.setSeekAmSpacing(5); // Max 10kHz for spacing

  }
  
  // G8PTN: Added
  // Call doSoftMute(0), 0 = No incr/decr action (eqivalent to getSoftMute)
  // This gets softMuteMaxAttIdx based on mode (AM, SSB)  
  doSoftMute(0);
  
  // Call doAgc(0), 0 = No incr/decr action (eqivalent to getAgc)
  // This gets disableAgc and agcNdx values based on mode (FM, AM , SSB)  
  doAgc(0);

  // Call doAvc(0), 0 = No incr/decr action (eqivalent to getAvc)
  // This gets currentAVC values based on mode (AM, SSB)
  doAvc(0);
  
  delay(100);

  // Default
  currentFrequency = band[bandIdx].currentFreq;
  currentStepIdx = band[bandIdx].currentStepIdx;    // Default. Need to modify for AM/SSB as required


  if (currentMode == FM)
      idxFmStep = band[bandIdx].currentStepIdx;
  else
  {
    // Default for AM/SSB
    idxAmStep = band[bandIdx].currentStepIdx;

    
    // Update depending on currentMode and currentStepIdx
    // If outside SSB step ranges
    if (isSSB() && currentStepIdx >= AmTotalStepsSsb && currentStepIdx <AmTotalSteps)
    {
      currentStepIdx = 0;;
      idxAmStep = 0;
      band[bandIdx].currentStepIdx = 0;      
    }

    // If outside AM step ranges
    if (currentMode == AM && currentStepIdx >= AmTotalSteps)
    {
      currentStepIdx = 0;;
      idxAmStep = 0;
      band[bandIdx].currentStepIdx = 0;      
    }
   
  }

  /*
  // G8PTN: Why is this required?
  if ((bandIdx == LW_BAND_TYPE || bandIdx == MW_BAND_TYPE)
      && idxAmStep > AmTotalStepsSsb)
      idxAmStep = AmTotalStepsSsb;
  */

  // Debug
  #if DEBUG2_PRINT
  Serial.print("Info: useBand() >>> currentStepIdx = ");
  Serial.print(currentStepIdx);
  Serial.print(", idxAmStep = ");
  Serial.print(idxAmStep);
  Serial.print(", band[bandIdx].currentStepIdx = ");
  Serial.print(band[bandIdx].currentStepIdx);
  Serial.print(", currentMode = ");
  Serial.println(currentMode);
  #endif

  // Store mode
  bandMODE[bandIdx] = currentMode;               // G8PTN: Added to support mode per band
  
  rssi = 0;
  snr = 0;
  cleanBfoRdsInfo();
  showStatus();
}


void loadSSB() {
  rx.setI2CFastModeCustom(400000); // You can try rx.setI2CFastModeCustom(700000); or greater value
  rx.loadPatch(ssb_patch_content, size_content, bandwidthSSB[bwIdxSSB].idx);
  rx.setI2CFastModeCustom(100000);
  ssbLoaded = true; 
}

/**
 *  Switches the Bandwidth
 */
void doBandwidth(int8_t v)
{
    if (isSSB())
    {
      bwIdxSSB = (v == 1) ? bwIdxSSB + 1 : bwIdxSSB - 1;

      if (bwIdxSSB > maxSsbBw)
        bwIdxSSB = 0;
      else if (bwIdxSSB < 0)
        bwIdxSSB = maxSsbBw;

      rx.setSSBAudioBandwidth(bandwidthSSB[bwIdxSSB].idx);
      // If audio bandwidth selected is about 2 kHz or below, it is recommended to set Sideband Cutoff Filter to 0.
      if (bandwidthSSB[bwIdxSSB].idx == 0 || bandwidthSSB[bwIdxSSB].idx == 4 || bandwidthSSB[bwIdxSSB].idx == 5)
        rx.setSSBSidebandCutoffFilter(0);
      else
        rx.setSSBSidebandCutoffFilter(1);

      band[bandIdx].bandwidthIdx = bwIdxSSB;
    }
    else if (currentMode == AM)
    {
      bwIdxAM = (v == 1) ? bwIdxAM + 1 : bwIdxAM - 1;

      if (bwIdxAM > maxAmBw)
        bwIdxAM = 0;
      else if (bwIdxAM < 0)
        bwIdxAM = maxAmBw;

      rx.setBandwidth(bandwidthAM[bwIdxAM].idx, 1);
      band[bandIdx].bandwidthIdx = bwIdxAM;
      
    } else {
    bwIdxFM = (v == 1) ? bwIdxFM + 1 : bwIdxFM - 1;
    if (bwIdxFM > maxFmBw)
      bwIdxFM = 0;
    else if (bwIdxFM < 0)
      bwIdxFM = maxFmBw;

    rx.setFmBandwidth(bandwidthFM[bwIdxFM].idx);
    band[bandIdx].bandwidthIdx = bwIdxFM;
  }
  showBandwidth();
  delay(MIN_ELAPSED_TIME); // waits a little more for releasing the button.
}

/**
 * Show cmd on display. It means you are setting up something.  
 */
void showCommandStatus(char * currentCmd)
{
  spr.drawString(currentCmd,38,14,2);
  drawSprite();
}

/**
 * Show menu options
 */
void showMenu() {
  drawSprite();
}

/**
 *  AGC and attenuattion setup
 */
void doAgc(int8_t v) {

  // G8PTN: Modified to have separate AGC/ATTN per mode (FM, AM, SSB)
  if (currentMode == FM) {
    if      (v == 1)   FmAgcIdx ++;
    else if (v == -1)  FmAgcIdx --;

    // Limit range
    if (FmAgcIdx < 0)
      FmAgcIdx = 27;
    else if (FmAgcIdx > 27)
      FmAgcIdx = 0;

    // Select
    agcIdx = FmAgcIdx;
  }

  else if (isSSB()) {
    if      (v == 1)   SsbAgcIdx ++;
    else if (v == -1)  SsbAgcIdx --;
    
    // Limit range
    if (SsbAgcIdx < 0)
      SsbAgcIdx = 1;
    else if (SsbAgcIdx > 1)
      SsbAgcIdx = 0;

    // Select
    agcIdx = SsbAgcIdx;    
  }

  else {
    if      (v == 1)   AmAgcIdx ++;
    else if (v == -1)  AmAgcIdx --;
 
    // Limit range
    if (AmAgcIdx < 0)
      AmAgcIdx = 37;
    else if (AmAgcIdx > 37)
      AmAgcIdx = 0;

    // Select
    agcIdx = AmAgcIdx;  
  }

  // Process agcIdx to generate disableAgc and agcIdx
  // agcIdx     0 1 2 3 4 5 6  ..... n    (n:    FM = 27, AM = 37, SSB = 1)
  // agcNdx     0 0 1 2 3 4 5  ..... n -1 (n -1: FM = 26, AM = 36, SSB = 0) 
  // disableAgc 0 1 1 1 1 1 1  ..... 1 
  disableAgc = (agcIdx > 0);     // if true, disable AGC; else, AGC is enabled
  if (agcIdx > 1)
    agcNdx = agcIdx - 1;
  else
    agcNdx = 0;  

  // Configure SI4732/5
  rx.setAutomaticGainControl(disableAgc, agcNdx); // if agcNdx = 0, no attenuation

  // Only call showAgcAtt() if incr/decr action (allows the doAgc(0) to act as getAgc)
  if (v != 0) showAgcAtt();
  
  delay(MIN_ELAPSED_TIME); // waits a little more for releasing the button.
  elapsedCommand = millis();
}


/**
 * Switches the current step
 */
void doStep(int8_t v)
{
    if ( currentMode == FM ) {
      idxFmStep = (v == 1) ? idxFmStep + 1 : idxFmStep - 1;
      if (idxFmStep > lastFmStep)
        idxFmStep = 0;
      else if (idxFmStep < 0)
        idxFmStep = lastFmStep;
        
      currentStepIdx = idxFmStep;
      rx.setFrequencyStep(tabFmStep[currentStepIdx]);      
    }
    
    else {
      idxAmStep = (v == 1) ? idxAmStep + 1 : idxAmStep - 1;
      if (idxAmStep > getLastStep())
        idxAmStep = 0;
      else if (idxAmStep < 0)
        idxAmStep = getLastStep();

      //SSB Step limit
      else if (isSSB() && idxAmStep >= AmTotalStepsSsb && idxAmStep < AmTotalSteps)
          idxAmStep = v == 1 ? AmTotalSteps : AmTotalStepsSsb - 1;

      // G8PTN: Reduced steps for LW/MW now covered in getLastStep()
      /*
      //LW/MW Step limit
      else if ((bandIdx == LW_BAND_TYPE || bandIdx == MW_BAND_TYPE)
          && v == 1 && idxAmStep > AmTotalStepsSsb && idxAmStep < AmTotalSteps)
          idxAmStep = AmTotalSteps;
      else if ((bandIdx == LW_BAND_TYPE || bandIdx == MW_BAND_TYPE)
          && v != 1 && idxAmStep > AmTotalStepsSsb && idxAmStep < AmTotalSteps)
          idxAmStep = AmTotalStepsSsb;
      */

      if (!isSSB() || isSSB() && idxAmStep < AmTotalSteps)
      {
          currentStepIdx = idxAmStep;
          rx.setFrequencyStep(tabAmStep[idxAmStep]);
      }

      /*
      if (!isSSB())
          rx.setSeekAmSpacing((band[bandIdx].currentStepIdx >= AmTotalSteps) ? 1 : tabStep[band[bandIdx].currentStepIdx]);
      */
          
      //showStep();

      currentStepIdx = idxAmStep;
      //rx.setFrequencyStep(tabAmStep[currentStepIdx]);
      rx.setSeekAmSpacing(5); // Max 10kHz for spacing
    }
    
    // Debug
    #if DEBUG2_PRINT
    int temp_LastStep = getLastStep(); 
    Serial.print("Info: doStep() >>> currentStepIdx = ");
    Serial.print(currentStepIdx);
    Serial.print(", getLastStep() = ");
    Serial.println(temp_LastStep);
    #endif
    
    band[bandIdx].currentStepIdx = currentStepIdx;
    showStep();
    elapsedCommand = millis();
}

/**
 * Switches to the AM, LSB or USB modes
 */
void doMode(int8_t v)
{
  currentMode = bandMODE[bandIdx];               // G8PTN: Added to support mode per band
  
  if (currentMode != FM)                         // Nothing to do if FM mode
  {
    if (v == 1)  { // clockwise
      if (currentMode == AM)
      {
        // If you were in AM mode, it is necessary to load SSB patch (every time)

        spr.fillSmoothRoundRect(80,40,160,40,4,TFT_WHITE);
        spr.fillSmoothRoundRect(81,41,158,38,4,TFT_MENU_BACK);
        spr.drawString("Loading SSB",160,62,4);
        spr.pushSprite(0,0);
        
        loadSSB();
        ssbLoaded = true;
        currentMode = LSB;
      }
      else if (currentMode == LSB)
        currentMode = USB;
      else if (currentMode == USB)
      {
        currentMode = AM;
        bfoOn = ssbLoaded = false;
        
        // G8PTN: When exiting SSB mode update the current frequency and BFO
        currentFrequency = currentFrequency + (currentBFO / 1000);
        currentBFO = 0;
      }
    } else { // and counterclockwise
      if (currentMode == AM)
      {
        // If you were in AM mode, it is necessary to load SSB patch (every time)

        spr.fillSmoothRoundRect(80,40,160,40,4,TFT_WHITE);
        spr.fillSmoothRoundRect(81,41,158,38,4,TFT_MENU_BACK);
        spr.drawString("Loading SSB",160,62,4);
        spr.pushSprite(0,0);
        
        loadSSB();
        ssbLoaded = true;
        currentMode = USB;
      }
      else if (currentMode == USB)
        currentMode = LSB;
      else if (currentMode == LSB)
      {
        currentMode = AM;
        bfoOn = ssbLoaded = false;
        
        // G8PTN: When exiting SSB mode update the current frequency and BFO
        currentFrequency = currentFrequency + (currentBFO / 1000);
        currentBFO = 0;
      }
    }
    
    band[bandIdx].currentFreq = currentFrequency;
    band[bandIdx].currentStepIdx = currentStepIdx;
    bandMODE[bandIdx] = currentMode;                      // G8PTN: Added to support mode per band
    useBand();
  }
  delay(MIN_ELAPSED_TIME); // waits a little more for releasing the button.
  elapsedCommand = millis();
}

/**
 * Sets the audio volume
 */
void doVolume( int8_t v ) {
  if ( v == 1)
    rx.volumeUp();
  else
    rx.volumeDown();

  showVolume();
  delay(MIN_ELAPSED_TIME); // waits a little more for releasing the button.
}

/**
 *  This function is called by the seek function process.  G8PTN: Added
 */
bool checkStopSeeking() {
  // Checks the seekStop flag
  return seekStop;  // returns true if the user rotates the encoder
}

/**
 *  This function is called by the seek function process.
 */
void showFrequencySeek(uint16_t freq)
{
  currentFrequency = freq;
  showFrequency();
}

/**
 *  Find a station. The direction is based on the last encoder move clockwise or counterclockwise
 */
void doSeek()
{
  if (isSSB()) return; // It does not work for SSB mode
  
  rx.seekStationProgress(showFrequencySeek, checkStopSeeking, seekDirection);   // G8PTN: Added checkStopSeeking
  currentFrequency = rx.getFrequency();
  
}

/**
 * Sets the Soft Mute Parameter
 */
void doSoftMute(int8_t v)
{
  // G8PTN: Modified to have separate SoftMute per mode (AM, SSB)
  // Only allow for AM and SSB modes
  if (currentMode != FM) {

    if (isSSB()) {
      if      (v == 1)   SsbSoftMuteIdx ++;
      else if (v == -1)  SsbSoftMuteIdx --;
    
      // Limit range
      if (SsbSoftMuteIdx < 0)
        SsbSoftMuteIdx = 32;
      else if (SsbSoftMuteIdx > 32)
        SsbSoftMuteIdx = 0;

      // Select
      softMuteMaxAttIdx = SsbSoftMuteIdx;
    }

    else {
      if      (v == 1)   AmSoftMuteIdx ++;
      else if (v == -1)  AmSoftMuteIdx --;
    
      // Limit range
      if (AmSoftMuteIdx < 0)
        AmSoftMuteIdx = 32;
      else if (AmSoftMuteIdx > 32)
        AmSoftMuteIdx = 0;

      // Select
      softMuteMaxAttIdx = AmSoftMuteIdx;
    }
  
  rx.setAmSoftMuteMaxAttenuation(softMuteMaxAttIdx);

  // Only call showSoftMute() if incr/decr action (allows the doSoftMute(0) to act as getSoftMute)
  if (v != 0) showSoftMute();

  elapsedCommand = millis();
  }
}

/**
 *  Menu options selection
 */
void doMenu( int8_t v) {
  menuIdx = (v == 1) ? menuIdx + 1 : menuIdx - 1;               // G8PTN: Corrected direction

  if (menuIdx > lastMenu)
    menuIdx = 0;
  else if (menuIdx < 0)
    menuIdx = lastMenu;

  showMenu();
  delay(MIN_ELAPSED_TIME); // waits a little more for releasing the button.
  elapsedCommand = millis();
}


/**
 * Starts the MENU action process
 */
void doCurrentMenuCmd() {
  disableCommands();
  switch (currentMenuCmd) {
     case VOLUME:                   // VOLUME
      if(muted) {
        rx.setVolume(mute_vol_val);
        muted = false;
      }
      cmdVolume = true;
      showVolume();
      break;
    case STEP:                      // STEP
      cmdStep = true;
      showStep();
      break;
    case MODE:                      // MODE
      cmdMode = true;
      showMode();
      break;
    #if BFO_MENU_EN                 // BFO
    case BFO:
      if (isSSB()) {
        bfoOn = true;
        showBFO();
      }
      showFrequency();
      break;
    #endif     
    case BW:                        // BW
      cmdBandwidth = true;
      showBandwidth();
      break;
    case AGC_ATT:                   // AGC/ATT
      cmdAgc = true;
      showAgcAtt();
      break;
    case SOFTMUTE:                  // SOFTMUTE
      if (currentMode != FM) {
        cmdSoftMuteMaxAtt = true;         
      }
      showSoftMute(); 
      break;
    case SEEKUP:                    // SEEKUP
      seekStop = false;             // G8PTN: Flag is set by rotary encoder and cleared on seek entry
      seekDirection = 1;
      doSeek();
      break;  
    case SEEKDOWN:                  // SEEKDOWN
      seekStop = false;             // G8PTN: Flag is set by rotary encoder and cleared on seek entry
      seekDirection = 0;
      doSeek();
      break;    
    case BAND:                      // BAND
      cmdBand = true;
      drawSprite();  
      break;
    case MUTE:                      // MUTE
      muted=!muted;
      if (muted)
      {
        mute_vol_val = rx.getVolume();
        rx.setVolume(0);
      }     
      else rx.setVolume(mute_vol_val);
      drawSprite();  
      break;

    // G8PTN: Added
    case CALIBRATION:               // CALIBRATION
      if (isSSB()) {
        cmdCal = true;
        currentCAL = bandCAL[bandIdx];
      }
      showCal();
      break;

    // G8PTN: Added
    case BRIGHTNESS:                // BRIGHTNESS
      cmdBrt = true;
      showBrt(); 
      break;

    // G8PTN: Added
    case AVC:                       // AVC
      if (currentMode != FM) { 
        cmdAvc = true;    
      }
      showAvc(); 
      break;

    default:
      showStatus();
      break;
  }
  currentMenuCmd = -1;
  elapsedCommand = millis();
}

/**
 * Return true if the current status is Menu command
 */
bool isMenuMode() {
  return (cmdMenu | cmdStep | cmdBandwidth | cmdAgc | cmdVolume | cmdSoftMuteMaxAtt | cmdMode | cmdBand | cmdCal | cmdBrt | cmdAvc);     // G8PTN: Added cmdBand, cmdCal, cmdBrt and cmdAvc
}

uint8_t getStrength() {
  if (currentMode != FM) {
    //dBuV to S point conversion HF
    if ((rssi >= 0) and (rssi <=  1)) return  1;  // S0
    if ((rssi >  1) and (rssi <=  2)) return  2;  // S1         // G8PTN: Corrected table
    if ((rssi >  2) and (rssi <=  3)) return  3;  // S2
    if ((rssi >  3) and (rssi <=  4)) return  4;  // S3
    if ((rssi >  4) and (rssi <= 10)) return  5;  // S4
    if ((rssi > 10) and (rssi <= 16)) return  6;  // S5
    if ((rssi > 16) and (rssi <= 22)) return  7;  // S6
    if ((rssi > 22) and (rssi <= 28)) return  8;  // S7
    if ((rssi > 28) and (rssi <= 34)) return  9;  // S8
    if ((rssi > 34) and (rssi <= 44)) return 10;  // S9
    if ((rssi > 44) and (rssi <= 54)) return 11;  // S9 +10
    if ((rssi > 54) and (rssi <= 64)) return 12;  // S9 +20
    if ((rssi > 64) and (rssi <= 74)) return 13;  // S9 +30
    if ((rssi > 74) and (rssi <= 84)) return 14;  // S9 +40
    if ((rssi > 84) and (rssi <= 94)) return 15;  // S9 +50
    if  (rssi > 94)                   return 16;  // S9 +60
    if  (rssi > 95)                   return 17;  //>S9 +60
  }
  else
  {
    //dBuV to S point conversion FM
    if  (rssi >= 0  and (rssi <=  1)) return  1;               // G8PTN: Corrected table
    if ((rssi >  1) and (rssi <=  2)) return  7;  // S6
    if ((rssi >  2) and (rssi <=  8)) return  8;  // S7
    if ((rssi >  8) and (rssi <= 14)) return  9;  // S8
    if ((rssi > 14) and (rssi <= 24)) return 10;  // S9
    if ((rssi > 24) and (rssi <= 34)) return 11;  // S9 +10
    if ((rssi > 34) and (rssi <= 44)) return 12;  // S9 +20
    if ((rssi > 44) and (rssi <= 54)) return 13;  // S9 +30
    if ((rssi > 54) and (rssi <= 64)) return 14;  // S9 +40
    if ((rssi > 64) and (rssi <= 74)) return 15;  // S9 +50
    if  (rssi > 74)                   return 16;  // S9 +60
    if  (rssi > 76)                   return 17;  //>S9 +60
    // newStereoPilot=si4735.getCurrentPilot();
  }
}    

// G8PTN: Alternative layout
void drawMenu() {
  if (cmdMenu) {
    spr.fillSmoothRoundRect(1+menu_offset_x,1+menu_offset_y,76+menu_delta_x,110,4,TFT_RED);
    spr.fillSmoothRoundRect(2+menu_offset_x,2+menu_offset_y,74+menu_delta_x,108,4,TFT_MENU_BACK);
    spr.setTextColor(TFT_WHITE,TFT_MENU_BACK);
    
    char label_menu [16];
    sprintf(label_menu, "Menu %2.2d/%2.2d", (menuIdx + 1), (lastMenu + 1));
    //spr.drawString("Menu",38+menu_offset_x+(menu_delta_x/2),14+menu_offset_y,2);
    spr.drawString(label_menu,38+menu_offset_x+(menu_delta_x/2),14+menu_offset_y,2);
    
    spr.setTextFont(0);
    spr.setTextColor(0xBEDF,TFT_MENU_BACK);
    spr.fillRoundRect(6+menu_offset_x,24+menu_offset_y+(2*16),66+menu_delta_x,16,2,0x105B);
    for(int i=-2;i<3;i++){
      if (i==0) spr.setTextColor(0xBEDF,0x105B);
      else spr.setTextColor(0xBEDF,TFT_MENU_BACK);
      spr.drawString(menu[abs((menuIdx+lastMenu+1+i)%(lastMenu+1))],38+menu_offset_x+(menu_delta_x/2),64+menu_offset_y+(i*16),2);
    }
  } else {
    spr.setTextColor(TFT_WHITE,TFT_MENU_BACK);    
    spr.fillSmoothRoundRect(1+menu_offset_x,1+menu_offset_y,76+menu_delta_x,110,4,TFT_RED);
    spr.fillSmoothRoundRect(2+menu_offset_x,2+menu_offset_y,74+menu_delta_x,108,4,TFT_MENU_BACK);
    spr.drawString(menu[menuIdx],38+menu_offset_x+(menu_delta_x/2),14+menu_offset_y,2);
    spr.setTextFont(0);
    spr.setTextColor(0xBEDF,TFT_MENU_BACK);
    // spr.fillRect(6,24+(2*16),67,16,0xBEDF);
    spr.fillRoundRect(6+menu_offset_x,24+menu_offset_y+(2*16),66+menu_delta_x,16,2,0x105B);

    //G8PTN: Added to reduce calls to getLastStep()
    int temp_LastStep = getLastStep();
    
    for(int i=-2;i<3;i++){
      if (i==0) spr.setTextColor(0xBEDF,0x105B);
      else spr.setTextColor(0xBEDF,TFT_MENU_BACK);
      if (cmdMode)
        if (currentMode == FM) {
          if (i==0) spr.drawString(bandModeDesc[abs((currentMode+lastBandModeDesc+1+i)%(lastBandModeDesc+1))],38+menu_offset_x+(menu_delta_x/2),64+menu_offset_y+(i*16),2);
        }          
        else spr.drawString(bandModeDesc[abs((currentMode+lastBandModeDesc+1+i)%(lastBandModeDesc+1))],38+menu_offset_x+(menu_delta_x/2),64+menu_offset_y+(i*16),2);
      if (cmdStep)
        if (currentMode == FM) spr.drawString(FmStepDesc[abs((currentStepIdx+lastFmStep+1+i)%(lastFmStep+1))],38+menu_offset_x+(menu_delta_x/2),64+menu_offset_y+(i*16),2);
        else spr.drawString(AmSsbStepDesc[abs((currentStepIdx+temp_LastStep+1+i)%(temp_LastStep+1))],38+menu_offset_x+(menu_delta_x/2),64+menu_offset_y+(i*16),2);
      if (cmdBand) spr.drawString(band[abs((bandIdx+lastBand+1+i)%(lastBand+1))].bandName,38+menu_offset_x+(menu_delta_x/2),64+menu_offset_y+(i*16),2);
      if (cmdBandwidth) {
        if (isSSB())
        {
          spr.drawString(bandwidthSSB[abs((bwIdxSSB+lastBandwidthSSB+1+i)%(lastBandwidthSSB+1))].desc,38+menu_offset_x+(menu_delta_x/2),64+menu_offset_y+(i*16),2);
          // bw = (char *)bandwidthSSB[bwIdxSSB].desc;
          // showBFO();
        }
        else if (currentMode == AM)
        {
          spr.drawString(bandwidthAM[abs((bwIdxAM+lastBandwidthAM+1+i)%(lastBandwidthAM+1))].desc,38+menu_offset_x+(menu_delta_x/2),64+menu_offset_y+(i*16),2);
        }
        else
        {
          spr.drawString(bandwidthFM[abs((bwIdxFM+lastBandwidthFM+1+i)%(lastBandwidthFM+1))].desc,38+menu_offset_x+(menu_delta_x/2),64+menu_offset_y+(i*16),2);
        }
      }
    }
    if (cmdVolume) {
      spr.setTextColor(0xBEDF,TFT_MENU_BACK);
      spr.fillRoundRect(6+menu_offset_x,24+menu_offset_y+(2*16),66+menu_delta_x,16,2,TFT_MENU_BACK);
      spr.drawNumber(rx.getVolume(),38+menu_offset_x+(menu_delta_x/2),60+menu_offset_y,7);
    }
    if (cmdAgc) {
      spr.setTextColor(0xBEDF,TFT_MENU_BACK);
      spr.fillRoundRect(6+menu_offset_x,24+menu_offset_y+(2*16),66+menu_delta_x,16,2,TFT_MENU_BACK);
      // rx.getAutomaticGainControl();             // G8PTN: Read back value is not used
      if (agcNdx == 0 && agcIdx == 0) {
        spr.setFreeFont(&Orbitron_Light_24);
        spr.drawString("AGC",38+menu_offset_x+(menu_delta_x/2),48+menu_offset_y);
        spr.drawString("On",38+menu_offset_x+(menu_delta_x/2),72+menu_offset_y);
        spr.setTextFont(0);
      } else {
        sprintf(sAgc, "%2.2d", agcNdx);
        spr.drawString(sAgc,38+menu_offset_x+(menu_delta_x/2),60+menu_offset_y,7);
      }
    }        
    if (cmdSoftMuteMaxAtt) {
      spr.setTextColor(0xBEDF,TFT_MENU_BACK);
      spr.fillRoundRect(6+menu_offset_x,24+menu_offset_y+(2*16),66+menu_delta_x,16,2,TFT_MENU_BACK);
      spr.drawString("Max Attn",38+menu_offset_x+(menu_delta_x/2),32+menu_offset_y,2);
      spr.drawNumber(softMuteMaxAttIdx,38+menu_offset_x+(menu_delta_x/2),60+menu_offset_y,4);
      spr.drawString("dB",38+menu_offset_x+(menu_delta_x/2),90+menu_offset_y,4);
    }

    // G8PTN: Added
    if (cmdCal) {
      spr.setTextColor(0xBEDF,TFT_MENU_BACK);
      spr.fillRoundRect(6+menu_offset_x,24+menu_offset_y+(2*16),66+menu_delta_x,16,2,TFT_MENU_BACK);
      spr.drawNumber(currentCAL,38+menu_offset_x+(menu_delta_x/2),60+menu_offset_y,4);
      spr.drawString("Hz",38+menu_offset_x+(menu_delta_x/2),90+menu_offset_y,4);
    }

    // G8PTN: Added
    if (cmdBrt) {
      spr.setTextColor(0xBEDF,TFT_MENU_BACK);
      spr.fillRoundRect(6+menu_offset_x,24+menu_offset_y+(2*16),66+menu_delta_x,16,2,TFT_MENU_BACK);
      spr.drawNumber(currentBrt,38+menu_offset_x+(menu_delta_x/2),60+menu_offset_y,4);
    }

    // G8PTN: Added
    if (cmdAvc) {
      spr.setTextColor(0xBEDF,TFT_MENU_BACK);
      spr.fillRoundRect(6+menu_offset_x,24+menu_offset_y+(2*16),66+menu_delta_x,16,2,TFT_MENU_BACK);
      spr.drawString("Max Gain",38+menu_offset_x+(menu_delta_x/2),32+menu_offset_y,2);
      spr.drawNumber(currentAVC,38+menu_offset_x+(menu_delta_x/2),60+menu_offset_y,4);
      spr.drawString("dB",38+menu_offset_x+(menu_delta_x/2),90+menu_offset_y,4);
    }
        
    spr.setTextColor(TFT_WHITE,TFT_BLACK);
  }
}



// G8PTN: Alternative layout
void drawSprite()
{
  spr.fillSprite(TFT_BLACK);
  spr.setTextColor(TFT_WHITE,TFT_BLACK);

  // Status bar
  spr.fillRect(0,0,320,22,TFT_BLUE);

  // Time
  spr.setTextColor(TFT_WHITE,TFT_BLUE);
  spr.setTextDatum(ML_DATUM);
  spr.drawString(time_disp,clock_datum,12,2);
  spr.setTextColor(TFT_WHITE,TFT_BLACK);

  // Screen activity icon
  screen_toggle = !screen_toggle;
  spr.drawCircle(clock_datum+50,11,6,TFT_WHITE);
  if (screen_toggle) spr.fillCircle(clock_datum+50,11,5,TFT_BLACK);
  else               spr.fillCircle(clock_datum+50,11,5,TFT_GREEN);  

  // EEPROM write request icon
  spr.drawCircle(clock_datum+70,11,6,TFT_WHITE);
  if (eeprom_wr_flag){
    spr.fillCircle(clock_datum+70,11,5,TFT_RED);
    eeprom_wr_flag = false;
  }
  else spr.fillCircle(clock_datum+70,11,5,TFT_BLACK); 

  if (currentMode == FM) {
    spr.setTextDatum(MR_DATUM);
    spr.drawFloat(currentFrequency/100.00,2,freq_offset_x,freq_offset_y,7);
    spr.setTextDatum(ML_DATUM);
    spr.drawString("MHz",funit_offset_x,funit_offset_y,4);
    spr.setTextDatum(MC_DATUM);
  }
  else {
    spr.setTextDatum(MR_DATUM);
    if (isSSB()) {
      uint32_t freq  = (uint32_t(currentFrequency) * 1000) + currentBFO;
      uint16_t khz   = freq / 1000;
      uint16_t tail  = (freq % 1000);
      char skhz [32];
      char stail [32];
      sprintf(skhz, "%3.3u", khz);
      sprintf(stail, ".%3.3d", tail);
      spr.drawString(skhz,freq_offset_x,freq_offset_y,7);
      spr.setTextDatum(ML_DATUM);
      spr.drawString(stail,5+freq_offset_x,15+freq_offset_y,4);   
    }
    else {
      spr.drawNumber(currentFrequency,freq_offset_x,freq_offset_y,7);
      spr.setTextDatum(ML_DATUM);
      spr.drawString(".000",5+freq_offset_x,15+freq_offset_y,4);
     
    }
    spr.drawString("kHz",funit_offset_x,funit_offset_y,4); 
    spr.setTextDatum(MC_DATUM);
  }
  
  //if (isMenuMode() or cmdBand) drawMenu();
  if (isMenuMode()) drawMenu();                      // G8PTN: Removed cmdBand, now part of isMenuMode() 
  else {
    countClick = 0;
    spr.setTextDatum(ML_DATUM);
    spr.setTextColor(TFT_WHITE,TFT_MENU_BACK);    
    spr.fillSmoothRoundRect(1+menu_offset_x,1+menu_offset_y,76+menu_delta_x,110,4,TFT_WHITE);
    spr.fillSmoothRoundRect(2+menu_offset_x,2+menu_offset_y,74+menu_delta_x,108,4,TFT_MENU_BACK);
    spr.drawString("Band:",6+menu_offset_x,64+menu_offset_y+(-3*16),2);    
    spr.drawString(band[bandIdx].bandName,48+menu_offset_x,64+menu_offset_y+(-3*16),2);
    spr.drawString("Mode:",6+menu_offset_x,64+menu_offset_y+(-2*16),2);    
    spr.drawString(bandModeDesc[currentMode],48+menu_offset_x,64+menu_offset_y+(-2*16),2);    
    spr.drawString("Step:",6+menu_offset_x,64+menu_offset_y+(-1*16),2);
    if (currentMode == FM) spr.drawString(FmStepDesc[currentStepIdx],48+menu_offset_x,64+menu_offset_y+(-1*16),2);
    else spr.drawString(AmSsbStepDesc[currentStepIdx],48+menu_offset_x,64+menu_offset_y+(-1*16),2);
    spr.drawString("BW:",6+menu_offset_x,64+menu_offset_y+(0*16),2);        
    if (isSSB())
    {
      spr.drawString(bandwidthSSB[bwIdxSSB].desc,48+menu_offset_x,64+menu_offset_y+(0*16),2);
    }
    else if (currentMode == AM)
    {
      spr.drawString(bandwidthAM[bwIdxAM].desc,48+menu_offset_x,64+menu_offset_y+(0*16),2);
    }
    else
    {
      spr.drawString(bandwidthFM[bwIdxFM].desc,48+menu_offset_x,64+menu_offset_y+(0*16),2);
    }
    if (agcNdx == 0 && agcIdx == 0) {
      spr.drawString("AGC:",6+menu_offset_x,64+menu_offset_y+(1*16),2);        
      spr.drawString("On",48+menu_offset_x,64+menu_offset_y+(1*16),2);
    } else {
      sprintf(sAgc, "%2.2d", agcNdx);
      spr.drawString("ATTN:",6+menu_offset_x,64+menu_offset_y+(1*16),2);        
      spr.drawString(sAgc,48+menu_offset_x,64+menu_offset_y+(1*16),2);
    }

    /*
    spr.drawString("BFO:",6+menu_offset_x,64+menu_offset_y+(2*16),2);
    if (isSSB()) {
      spr.setTextDatum(MR_DATUM);
      spr.drawString(bfo,74+menu_offset_x,64+menu_offset_y+(2*16),2);
    }
    else spr.drawString("Off",48+menu_offset_x,64+menu_offset_y+(2*16),2);
    spr.setTextDatum(MC_DATUM);
    */

    spr.drawString("VOL:",6+menu_offset_x,64+menu_offset_y+(2*16),2);
    if (muted) {
      //spr.setTextDatum(MR_DATUM);
      spr.setTextColor(TFT_WHITE,TFT_RED);
      spr.drawString("Muted",48+menu_offset_x,64+menu_offset_y+(2*16),2);
      spr.setTextColor(TFT_WHITE,TFT_BLACK);
    }
    else spr.drawNumber(rx.getVolume(),48+menu_offset_x,64+menu_offset_y+(2*16),2);
    spr.setTextDatum(MC_DATUM);
    
  }

  if (bfoOn) {
    spr.setTextColor(TFT_WHITE,TFT_BLACK);
    spr.setTextDatum(ML_DATUM);
    spr.drawString("BFO:",10,158,4);
    spr.drawString(bfo,80,158,4);
    spr.setTextDatum(MC_DATUM);    
  
  }


  // S-Meter
  for(int i=0;i<getStrength();i++)
    if (i<10)
      // Option 1 - Variable heaght bars
      //spr.fillRect(244+(i*4),80-(i*1),2,4+(i*1),0x3526);
      // Option 2 - Fixed heaght bars
      //spr.fillRect(1+meter_offset_x+(i*8),1+meter_offset_y,3,20,0x3526);     // Option 2a 
      spr.fillRect(1+meter_offset_x+(i*8),1+meter_offset_y,3,20,TFT_GREEN);    // Option 2b
    else
      // Option 1 - Variable heaght bars
      //spr.fillRect(244+(i*4),80-(i*1),2,4+(i*1),TFT_RED);
      // Option 2 - Fixed heaght bars
      spr.fillRect(1+meter_offset_x+(i*8),1+meter_offset_y,3,20,TFT_RED);

  // S-Meter Scale
  //spr.drawLine(1+meter_offset_x,25+meter_offset_y,5+meter_offset_x+(15*8),25+meter_offset_y,TFT_DARKGREY);  // Option 2a
  //spr.setTextColor(TFT_DARKGREY,TFT_BLACK);                                                                 // Option 2a
  spr.drawLine(1+meter_offset_x,25+meter_offset_y,5+meter_offset_x+(15*8),25+meter_offset_y,TFT_WHITE);       // Option 2b
  spr.setTextColor(TFT_WHITE,TFT_BLACK);                                                                      // Option 2b
  
  spr.drawString("S",1+meter_offset_x,45+meter_offset_y,2);
  for(int i=0;i<16;i++)
    {
      if (i%2) {
        //spr.drawLine(2+meter_offset_x+(i*8),25+meter_offset_y,2+meter_offset_x+(i*8),35+meter_offset_y,TFT_DARKGREY);  // Option 2a
        spr.drawLine(2+meter_offset_x+(i*8),25+meter_offset_y,2+meter_offset_x+(i*8),35+meter_offset_y,TFT_WHITE);       // Option 2b
        if (i < 10)  spr.drawNumber(i,2+meter_offset_x+(i*8),45+meter_offset_y,2);
        if (i == 13) spr.drawString("+40",2+meter_offset_x+(i*8),45+meter_offset_y,2);
      }
    }
  spr.setTextColor(TFT_WHITE,TFT_BLACK);


  if (currentMode == FM) {
    spr.fillSmoothRoundRect(1+mode_offset_x,1+mode_offset_y,76,22,4,TFT_WHITE);
    spr.fillSmoothRoundRect(2+mode_offset_x,2+mode_offset_y,74,20,4,TFT_BLACK);
    if (rx.getCurrentPilot()) {
      //spr.setTextColor(TFT_RED,TFT_BLACK);                                       // STEREO Option 1
      spr.fillSmoothRoundRect(2+mode_offset_x,2+mode_offset_y,74,20,4,TFT_RED);    // STEREO Option 2
      spr.setTextColor(TFT_WHITE,TFT_RED);                                         // STEREO Option 2
      spr.drawString("STEREO",38+mode_offset_x,11+mode_offset_y,2);
      spr.setTextColor(TFT_WHITE,TFT_BLACK);
    } else spr.drawString("MONO",38+mode_offset_x,11+mode_offset_y,2);

    // spr.setTextColor(TFT_MAGENTA,TFT_BLACK);
    spr.setTextDatum(ML_DATUM);
    spr.drawString(bufferStationName,rds_offset_x,rds_offset_y,4);
    spr.setTextDatum(MC_DATUM);
    // spr.setTextColor(TFT_WHITE,TFT_BLACK);    
  }
    /*
    else {
    spr.fillSmoothRoundRect(1+mode_offset_x,1+mode_offset_y,76,22,4,TFT_WHITE);
    spr.fillSmoothRoundRect(2+mode_offset_x,2+mode_offset_y,74,20,4,TFT_BLACK);  
    spr.drawString(bandModeDesc[currentMode],38+mode_offset_x,11+mode_offset_y,2);
  }
  */

#if TUNE_HOLDOFF
  // Update if not tuning
  if (tuning_flag == false) {
    batteryMonitor();
    spr.pushSprite(0,0);
  }  
#else
  // No hold off
  batteryMonitor();
  spr.pushSprite(0,0);
#endif
  
}


void cleanBfoRdsInfo()
{
  bufferStationName[0]='\0';
}

void showRDSMsg()
{
  rdsMsg[35] = bufferRdsMsg[35] = '\0';
  if (strcmp(bufferRdsMsg, rdsMsg) == 0)
    return;
}

void showRDSStation()
{
  if (strcmp(bufferStationName, stationName) == 0 ) return;
  cleanBfoRdsInfo();
  strcpy(bufferStationName, stationName);
  drawSprite();
}

void showRDSTime()
{
  if (strcmp(bufferRdsTime, rdsTime) == 0)
    return;
}

void checkRDS()
{
  rx.getRdsStatus();
  if (rx.getRdsReceived())
  {
    if (rx.getRdsSync() && rx.getRdsSyncFound())
    {
      rdsMsg = rx.getRdsText2A();
      stationName = rx.getRdsText0A();
      rdsTime = rx.getRdsTime();
      // if ( rdsMsg != NULL )   showRDSMsg();
      if (stationName != NULL)         
          showRDSStation();
      // if ( rdsTime != NULL ) showRDSTime();
    }
  }
}


/***************************************************************************************
** Function name:           batteryMonitor
** Description:             Check Battery Level and Draw to level icon
***************************************************************************************/
// Check Battery Level
void batteryMonitor() {

  // Read ADC and calculate average  
  adc_read_total = 0;                 // Reset to 0 at start of calculation
  for (int i = 0; i < 10; i++) {
    adc_read_total += analogRead(VBAT_MON);
  }
  adc_read_avr = (adc_read_total / BATT_ADC_READS);

  // Calculated average voltage with correction factor
  adc_volt_avr = adc_read_avr * BATT_ADC_FACTOR / 1000;

  // State machine
  // SOC (%)      batt_soc_state
  //  0 to 25           0
  // 25 to 50           1
  // 50 to 75           2
  // 75 to 100          3

  switch (batt_soc_state) {
    case 0:
      if      (adc_volt_avr > (BATT_SOC_LEVEL1 + BATT_SOC_HYST_2)) batt_soc_state = 1;   // State 0 > 1
      break;

    case 1:
      if      (adc_volt_avr > (BATT_SOC_LEVEL2 + BATT_SOC_HYST_2)) batt_soc_state = 2;   // State 1 > 2
      else if (adc_volt_avr < (BATT_SOC_LEVEL1 - BATT_SOC_HYST_2)) batt_soc_state = 0;   // State 1 > 0
      break;

    case 2:
      if      (adc_volt_avr > (BATT_SOC_LEVEL3 + BATT_SOC_HYST_2)) batt_soc_state = 3;   // State 2 > 3
      else if (adc_volt_avr < (BATT_SOC_LEVEL2 - BATT_SOC_HYST_2)) batt_soc_state = 1;   // State 2 > 1
      break;

    case 3:
      if      (adc_volt_avr < (BATT_SOC_LEVEL3 - BATT_SOC_HYST_2)) batt_soc_state = 2;   // State 3 > 2
      break;

    default:
      if      (batt_soc_state > 3) batt_soc_state = 0;                                   // State (Illegal) > 0
      else    batt_soc_state = batt_soc_state;                                           // Keep current state
      break;   
    }

    // Debug
    #if DEBUG3_PRINT
    Serial.print("Info: batteryMonitor() >>> batt_soc_state = "); Serial.print(batt_soc_state); Serial.print(", ");
    Serial.print("ADC count (average) = "); Serial.print(adc_read_avr); Serial.print(", "); 
    Serial.print("ADC voltage (average) = "); Serial.println(adc_volt_avr);
    #endif

  // SOC display information
  // Variable: chargeLevel = pixel width, batteryLevelColor = Colour of level
  int chargeLevel;
  uint16_t batteryLevelColor;

  if (batt_soc_state == 0 ) {
    chargeLevel=7;
    batteryLevelColor=TFT_RED;
  }
  if (batt_soc_state == 1 ) {
    chargeLevel=14;
    batteryLevelColor=TFT_GREEN;
  }
  if (batt_soc_state == 2 ) {
    chargeLevel=21;
    batteryLevelColor=TFT_GREEN;
  }
  if (batt_soc_state == 3 ) {
    chargeLevel=28;
    batteryLevelColor=TFT_GREEN;
  }

  // Set display information
  spr.fillRect(batt_datum,5,30,14,TFT_WHITE);
  spr.fillRect(batt_datum + 1,6,28,12,TFT_BLACK);
  spr.fillRect(batt_datum + 30,7,3,10,TFT_WHITE);
  spr.fillRect(batt_datum + 1,6,chargeLevel,12,batteryLevelColor);

  spr.setTextColor(TFT_WHITE,TFT_BLUE);
  spr.setTextDatum(ML_DATUM);

  // The hardware has a load sharing circuit to allow simultaneous charge and power
  // With USB(5V) connected the voltage reading will be approx. VBUS - Diode Drop = 4.65V
  // If the average voltage is greater than 4.3V, show "EXT" on the display
  if (adc_volt_avr > 4.3) {
    spr.drawString("EXT",batt_datum + 45,12,2);
  }
  else {
    spr.drawFloat(adc_volt_avr,2,batt_datum + 37,12,2);
    spr.drawString("V",batt_datum + 70,12,2);
  }

  // Debug
  //spr.drawNumber(batt_soc_state,batt_datum - 10,12,2);

  spr.setTextColor(TFT_WHITE,TFT_BLACK);
  spr.setTextDatum(MC_DATUM);
  //spr.pushSprite(0,0);            // G8PTN: Not needed
  
}


/***************************************************************************************
** Description:   In SSB mode tuning uses VFO and BFO
**                - Algorithm from ATS-20_EX Goshante firmware
***************************************************************************************/
// Tuning algorithm
void doFrequencyTuneSSB()
{
    //const int BFOMax = 16000;    G8PTN: Moved to a global variable
    int step = encoderCount == 1 ? getSteps() : getSteps() * -1;
    int newBFO = currentBFO + step;
    int redundant = 0;

    if (newBFO > BFOMax)
    {
        redundant = (newBFO / BFOMax) * BFOMax;
        currentFrequency += redundant / 1000;
        newBFO -= redundant;
    }
    else if (newBFO < -BFOMax)
    {
        redundant = ((abs(newBFO) / BFOMax) * BFOMax);
        currentFrequency -= redundant / 1000;
        newBFO += redundant;
    }

    currentBFO = newBFO;
    updateBFO();

    if (redundant != 0)
      
    {
        clampSSBBand();                                   // G8PTN: Added          
        rx.setFrequency(currentFrequency);
        //agcSetFunc(); //Re-apply to remove noize        // G8PTN: Commented out
        currentFrequency = rx.getFrequency();
        //band[bandIdx].currentFreq = currentFrequency;   // G8PTN: Commented out, covered below
    }

    band[bandIdx].currentFreq = currentFrequency + (currentBFO / 1000);     // Update band table currentFreq

    //g_lastFreqChange = millis();
    //g_previousFrequency = 0; //Force EEPROM update
    if (clampSSBBand()) {
      // Clamp frequency to band limits                  // Automatically done by function call
      //showFrequency();                                 // This action is not required
      
      // Debug
      #if DEBUG1_PRINT
      Serial.println("Info: clampSSBBand() >>> SSB Band Clamp !");
      #endif     
    }

}

// Clamp SSB tuning to band limits
bool clampSSBBand()
{
    uint16_t freq = currentFrequency + (currentBFO / 1000);

    // Special case to cover SSB frequency negative!
    bool SsbFreqNeg = false; 
    if (currentFrequency & 0x8000)
      SsbFreqNeg = true;

    // Priority to minimum check to cover SSB frequency negative
    bool upd = false;
    if (freq < band[bandIdx].minimumFreq || SsbFreqNeg)
    {
        currentFrequency = band[bandIdx].maximumFreq;
        upd = true;
    }
    else if (freq > band[bandIdx].maximumFreq)
    {
        currentFrequency = band[bandIdx].minimumFreq;
        upd = true;
    }

    if (upd)
    {
        band[bandIdx].currentFreq = currentFrequency;    // Update band table currentFreq
        rx.setFrequency(currentFrequency);
        currentBFO = 0;
        updateBFO();
        return true;
    }

    return false;
}


void updateBFO()
{
    // To move frequency forward, need to move the BFO backwards, so multiply by -1
    currentCAL = bandCAL[bandIdx];    // Select from table
    rx.setSSBBfo((currentBFO + currentCAL) * -1);
      
    // Debug
    #if DEBUG2_PRINT
    Serial.print("Info: updateBFO() >>> ");
    Serial.print("currentBFO = ");
    Serial.print(currentBFO);
    Serial.print(", currentCAL = ");
    Serial.print(currentCAL);
    Serial.print(", rx.setSSBbfo() = ");
    Serial.println((currentBFO + currentCAL) * -1);
    #endif
}


void doCal( int16_t v ) {
  currentCAL = bandCAL[bandIdx];    // Select from table
  if ( v == 1) {
    currentCAL = currentCAL + 10;
    if (currentCAL > CALMax) currentCAL = CALMax;
  }

  else {
    currentCAL = currentCAL - 10;
    if (currentCAL < -CALMax) currentCAL = -CALMax;
  }
  bandCAL[bandIdx] = currentCAL;    // Store to table

  // If in SSB mode set the SI4732/5 BFO value
  // This adjustments the BFO whilst in the calibration menu
  if (isSSB()) updateBFO();
  
  showCal();
  delay(MIN_ELAPSED_TIME); // waits a little more for releasing the button.
}

void showCal()
{
drawSprite();
}


void doBrt( uint16_t v ) {
  if ( v == 1) {
    currentBrt = currentBrt + 32;
    if (currentBrt > 255) currentBrt = 255;
  }

  else {
    if (currentBrt == 255) currentBrt = currentBrt - 31;
    else currentBrt = currentBrt - 32;
    if (currentBrt < 32) currentBrt = 32;
  }

  ledcWrite(0, currentBrt);
  showBrt();
  delay(MIN_ELAPSED_TIME); // waits a little more for releasing the button.
}

void showBrt()
{
drawSprite();
}


void doAvc(int16_t v) {
  // Only allow for AM and SSB modes
  if (currentMode != FM) {
    
    if (isSSB()) {
      if      (v == 1)   SsbAvcIdx += 2;
      else if (v == -1)  SsbAvcIdx -= 2;
    
      // Limit range
      if (SsbAvcIdx < 12)
        SsbAvcIdx = 90;
      else if (SsbAvcIdx > 90)
        SsbAvcIdx = 12;

      // Select
      currentAVC = SsbAvcIdx;
    }

    else {
      if      (v == 1)   AmAvcIdx += 2;
      else if (v == -1)  AmAvcIdx -= 2;
    
      // Limit range
      if (AmAvcIdx < 12)
        AmAvcIdx = 90;
      else if (AmAvcIdx > 90)
        AmAvcIdx = 12;

      // Select
      currentAVC = AmAvcIdx;
    }

  // Configure SI4732/5
  rx.setAvcAmMaxGain(currentAVC);

  // Only call showAvc() if incr/decr action (allows the doAvc(0) to act as getAvc)
  if (v != 0) showAvc();

  delay(MIN_ELAPSED_TIME); // waits a little more for releasing the button.
  }
}

void showAvc()
{
drawSprite();
}


void button_check()
{
  // G8PTN: Added
  // Push button detection
  // Only execute every 10 ms
  if ((millis() - pb1_timer) > 10) {
    pb1_timer = millis();  
    pb1_current = digitalRead(ENCODER_PUSH_BUTTON);              // Read pin value
    pb1_shift = (pb1_shift << 1 ) | (pb1_current) | 0xFF00;      // Shift register
  
    if ((pb1_shift == 0xFF00) && (pb1_last == 0)) {              // Falling edge
      pb1_last = 1;
      pb1_pressed = true;

      // Debug
      #if DEBUG2_PRINT
      Serial.println("Info: button_check() >>> Button Pressed");
      #endif
    }

    else if ((pb1_shift == 0xFFFF) && (pb1_last == 1)) {         // Rising edge
      pb1_last = 0;

      // Debug
      #if DEBUG2_PRINT
      Serial.println("Info: button_check() >>> Button Released");
      #endif
    }
  }
}

void clock_time()
{
  if ((micros() - clock_timer) >= 1000000) {
    clock_timer = micros();
    time_seconds++;    
    if (time_seconds >= 60) {
      time_seconds = 0;
      time_minutes ++;

      if (time_minutes >= 60) {
        time_minutes = 0;
        time_hours++;

        if (time_hours >= 24) {
          time_hours = 0;
        }
      }
    }

    // Format for display HH:MM (24 hour format)
    sprintf(time_disp, "%2.2d:%2.2d", time_hours, time_minutes);  
  }
}

/**
 * Main loop
 */
void loop()
{
  
  // Check if the encoder has moved.
  if (encoderCount != 0)
  {
    // G8PTN: The manual BFO adjusment is not required with the doFrequencyTuneSSB method, but leave for debug
    if (bfoOn & isSSB())
    {
      currentBFO = (encoderCount == 1) ? (currentBFO + currentBFOStep) : (currentBFO - currentBFOStep);
      // G8PTN: Clamp range to +/- BFOMax (as per doFrequencyTuneSSB)
      if (currentBFO >  BFOMax) currentBFO =  BFOMax;
      if (currentBFO < -BFOMax) currentBFO = -BFOMax;  
      band[bandIdx].currentFreq = currentFrequency + (currentBFO / 1000);     // G8PTN; Calculate frequency value to store in EEPROM    
      updateBFO();
      showBFO();
    }
    else if (cmdMenu)
      doMenu(encoderCount);
    else if (cmdMode)
      doMode(encoderCount);
    else if (cmdStep)
      doStep(encoderCount);
    else if (cmdAgc)
      doAgc(encoderCount);
    else if (cmdBandwidth)
      doBandwidth(encoderCount);
    else if (cmdVolume)
      doVolume(encoderCount);
    else if (cmdSoftMuteMaxAtt)
      doSoftMute(encoderCount);
    else if (cmdBand)
      setBand(encoderCount);

    // G8PTN: Added commands
    else if (cmdCal)
      doCal(encoderCount);
    else if (cmdBrt)
      doBrt(encoderCount);
    else if (cmdAvc)
      doAvc(encoderCount);    

    // G8PTN: Added SSB tuning
    else if (isSSB()) {

#if TUNE_HOLDOFF
      // Tuning timer to hold off (SSB) display updates
      tuning_flag = true;
      tuning_timer = millis();
      #if DEBUG3_PRINT
      Serial.print("Info: TUNE_HOLDOFF SSB (Set) >>> ");
      Serial.print("tuning_flag = ");
      Serial.print(tuning_flag);
      Serial.print(", millis = ");
      Serial.println(millis());
      #endif      
#endif
      
      doFrequencyTuneSSB();
      currentFrequency = rx.getFrequency();
      
      // Debug
      #if DEBUG1_PRINT
      Serial.print("Info: SSB >>> ");
      Serial.print("currentFrequency = ");
      Serial.print(currentFrequency);
      Serial.print(", currentBFO = ");
      Serial.print(currentBFO);
      Serial.print(", rx.setSSBbfo() = ");
      Serial.println((currentBFO + currentCAL) * -1);
      #endif
      
      showFrequency();
    }
    else {

#if TUNE_HOLDOFF
      // Tuning timer to hold off (FM/AM) display updates
      tuning_flag = true;
      tuning_timer = millis();
      #if DEBUG3_PRINT
      Serial.print("Info: TUNE_HOLDOFF FM/AM (Set) >>> ");
      Serial.print("tuning_flag = ");     
      Serial.print(tuning_flag);
      Serial.print(", millis = ");
      Serial.println(millis());
      #endif    
#endif

      // G8PTN: Used in place of rx.frequencyUp() and rx.frequencyDown()
      if (currentMode == FM)
        currentFrequency += tabFmStep[currentStepIdx] * encoderCount;       // FM Up/Down
      else
        currentFrequency += tabAmStep[currentStepIdx] * encoderCount;       // AM Up/Down

      // Band limit checking
      uint16_t bMin = band[bandIdx].minimumFreq;                            // Assign lower band limit
      uint16_t bMax = band[bandIdx].maximumFreq;                            // Assign upper band limit

      // Special case to cover AM frequency negative!
      bool AmFreqNeg = false; 
      if ((currentMode == AM) && (currentFrequency & 0x8000))
        AmFreqNeg = true;

      // Priority to minimum check to cover AM frequency negative
      if ((currentFrequency < bMin) || AmFreqNeg)
        currentFrequency = bMax;                                           // Lower band limit or AM frequency negative
      else if (currentFrequency > bMax)                                  
        currentFrequency = bMin;                                           // Upper band limit

      rx.setFrequency(currentFrequency);                                   // Set new frequency

      /*
      if (encoderCount == 1)
      {
        rx.frequencyUp();
      }
      else
      {
        rx.frequencyDown();
      }
      */
      
      if (currentMode == FM) cleanBfoRdsInfo();
      // Show the current frequency only if it has changed
      currentFrequency = rx.getFrequency();
      band[bandIdx].currentFreq = currentFrequency;            // G8PTN: Added to ensure update of currentFreq in table for AM/FM
      
      // Debug
      #if DEBUG1_PRINT
      Serial.print("Info: AM/FM >>> currentFrequency = ");
      Serial.print(currentFrequency);
      Serial.print(", currentBFO = ");
      Serial.println(currentBFO);                              // Print to check the currentBFO value
      //Serial.print(", rx.setSSBbfo() = ");                   // rx.setSSBbfo() will not have been written
      //Serial.println((currentBFO + currentCAL) * -1);        // rx.setSSBbfo() will not have been written
      #endif 
        
      showFrequency();
    }

    encoderCount = 0;
    resetEepromDelay();
    delay(MIN_ELAPSED_TIME);
    elapsedCommand = millis();
  }
  else
  {
    // G8PTN: Modified to use new button detection. Disable band menu on single push. Default to volume option
    //if (digitalRead(ENCODER_PUSH_BUTTON) == LOW)
    if (pb1_pressed)
    {
      pb1_pressed = false;
      //while (digitalRead(ENCODER_PUSH_BUTTON) == LOW) { }
      countClick++;
      if (cmdMenu)
      {
        currentMenuCmd = menuIdx;
        doCurrentMenuCmd();
      }
      //else if (countClick == 1)
      else if (countClick >= 1)                   // G8PTN: All actions now done on single press
      { // If just one click, you can select the band by rotating the encoder
        if (isMenuMode())
        {
          disableCommands();
          showStatus();
          showCommandStatus((char *)"VFO ");
        }
        else if (bfoOn) {
          bfoOn = false;
          showStatus();
        }
        else
        {
          //cmdBand = !cmdBand;
          cmdMenu = !cmdMenu;
          menuIdx = VOLUME;          
          currentMenuCmd = menuIdx;
          drawSprite();
        }
      }
      else                                       // G8PTN: Not used
      { // GO to MENU if more than one click in less than 1/2 seconds.
        cmdMenu = !cmdMenu;
        if (cmdMenu)
          showMenu();
      }
      delay(MIN_ELAPSED_TIME);
      elapsedCommand = millis();
    }
  }

  // Show RSSI status only if this condition has changed
  if ((millis() - elapsedRSSI) > MIN_ELAPSED_RSSI_TIME * 6)
  {
    // Debug
    #if DEBUG3_PRINT
    Serial.println("Info: loop() >>> Checking signal information");
    #endif
    
    rx.getCurrentReceivedSignalQuality();
    snr= rx.getCurrentSNR();
    int aux = rx.getCurrentRSSI();
    
    // Debug
    #if DEBUG3_PRINT
    Serial.print("Info: loop() >>> RSSI = ");
    Serial.println(rssi);
    #endif 
    
    //if (rssi != aux && !isMenuMode())
    if (rssi != aux)                            // G8PTN: Based on 1.2s update, always allow S-Meter
    {
      // Debug
      #if DEBUG3_PRINT
      Serial.println("Info: loop() >>> RSI diff detected");
      #endif
      
      rssi = aux;
      showRSSI();
    }
    elapsedRSSI = millis();
  }

  // Disable commands control
  if ((millis() - elapsedCommand) > ELAPSED_COMMAND)
  {
    if (isSSB())
    {
      bfoOn = false;
      // showBFO();
      disableCommands();
      showStatus();
    } 
    //else if (isMenuMode() or cmdBand) {
    else if (isMenuMode()) {                     // G8PTN: Removed cmdBand, now part of isMenuMode()
      disableCommands();
      showStatus();
    } 
    elapsedCommand = millis();
  }

  if ((millis() - elapsedClick) > ELAPSED_CLICK)
  {
    countClick = 0;
    elapsedClick = millis();
  }

  if ((millis() - lastRDSCheck) > RDS_CHECK_TIME) {
    if ((currentMode == FM) and (snr >= 12)) checkRDS();
    lastRDSCheck = millis();
  }  

  // Show the current frequency only if it has changed
  if (itIsTimeToSave)
  {
    if ((millis() - storeTime) > STORE_TIME)
    {
      saveAllReceiverInformation();
      storeTime = millis();
      itIsTimeToSave = false;
    }
  }

  // Check for button activity
  // In this case used for falling edge detection
  button_check();

  // Periodically refresh the main screen
  // This covers the case where there is nothing else triggering a refresh
  if ((millis() - background_timer) > BACKGROUND_REFRESH_TIME) {
    background_timer = millis();
    if (!isMenuMode()) showStatus(); 
  }

#if TUNE_HOLDOFF
  // Check if tuning flag is set
  if (tuning_flag == true) {
    if ((millis() - tuning_timer) > TUNE_HOLDOFF_TIME) {
      tuning_flag = false;
      showFrequency();
      #if DEBUG3_PRINT
      Serial.print("Info: TUNE_HOLDOFF FM/AM (Reset) >>> ");
      Serial.print("tuning_flag = ");
      Serial.print(tuning_flag);
      Serial.print(", millis = ");
      Serial.println(millis()); 
      #endif 
    } 
  }
#endif
    
  // Run clock
  //clock_time();
  timeClient.update();
  hours = timeClient.getHours();
  minutes = timeClient.getMinutes();
  sprintf(time_disp, "%2.2d:%2.2d", hours, minutes);  


#if USE_REMOTE
  // REMOTE Serial - Experimental

  if (millis() - g_remote_timer >= 500)
  {
    g_remote_timer = millis();

    // Increment diagnostic sequence number
    g_remote_seqnum ++;

    // Prepare information ready to be sent
    int remote_volume  = rx.getVolume();

    // S-Meter conditional on compile option
    rx.getCurrentReceivedSignalQuality();
    uint8_t remote_rssi = rx.getCurrentRSSI();

    // Remote serial
    Serial.print(app_ver);                      // Firmware version
    Serial.print(",");
    
    Serial.print(currentFrequency);             // Frequency (KHz)
    Serial.print(",");
    Serial.print(currentBFO);                   // Frequency (Hz x 1000)
    Serial.print(",");

    Serial.print(bandIdx);                      // Band
    Serial.print(",");
    Serial.print(currentMode);                  // Mode
    Serial.print(",");
    Serial.print(currentStepIdx);               // Step (FM/AM/SSB)
    Serial.print(",");
    Serial.print(bwIdxFM);                      // Bandwidth (FM)
    Serial.print(",");
    Serial.print(bwIdxAM);                      // Bandwidth (AM)
    Serial.print(",");
    Serial.print(bwIdxSSB);                     // Bandwidth (SSB)
    Serial.print(",");
    Serial.print(agcIdx);                       // AGC/ATTN (FM/AM/SSB)
    Serial.print(",");

    Serial.print(cmdBand);                      // Band command mode
    Serial.print(",");
    Serial.print(cmdMode);                      // Mode command mode
    Serial.print(",");
    Serial.print(cmdStep);                      // Step command mode
    Serial.print(",");
    Serial.print(cmdBandwidth);                 // Bandwidth command mode
    Serial.print(",");
    Serial.print(cmdAgc);                       // AGC/ATTN command mode
    Serial.print(",");

    Serial.print(remote_volume);                // Volume
    Serial.print(",");
    Serial.print(remote_rssi);                  // RSSI 
    Serial.print(",");  
    Serial.print(adc_read_avr);                 // V_BAT/2 (ADC average value)
    Serial.print(",");            
    Serial.println(g_remote_seqnum);            // Sequence number
  }
    
  if (Serial.available() > 0)
  {
    char key = Serial.read();
    switch (key)
    {         
        case 'U':                              // Encoder Up
          encoderCount ++;
          break;
        case 'D':                              // Encoder Down
          encoderCount --;
          break;
        case 'P':                              // Encoder Push Button
          pb1_pressed = true;
          break;
          
        case 'B':                              // Band Up
          setBand(1);
          break;
        case 'b':                              // Band Down
          setBand(-1);
          break;

        case 'M':                              // Mode Up
          doMode(1);
          break;
        case 'm':                              // Mode Down
          doMode(-1);
          break;

        case 'S':                              // Step Up
          doStep(1);
          break;
        case 's':                              // Step Down
          doStep(-1);
          break;

        case 'W':                              // Bandwidth Up
          doBandwidth(1);
          break;
        case 'w':                              // Bandwidth Down
          doBandwidth(-1);
          break;

        case 'A':                              // AGC/ATTN Up
          doAgc(1);
          break;
        case 'a':                              // AGC/ATTN Down
          doAgc(-1);
          break;

        case 'V':                              // Volume Up
          doVolume(1);
          break;
        case 'v':                              // Volume Down
          doVolume(-1);
          break;

        case 'L':                              // Backlight Up
          doBrt(1);
          break;
        case 'l':                              // Backlight Down
          doBrt(-1);
          break;

        default:
          break;           
    }
  }
  
#endif

  server.handleClient();
  // Add a small default delay in the main loop
  delay(5);
}

void handle_OnConnect() {
  String stringMode;

  if (currentMode == AM)
  {
    stringMode = "AM";
  }
  else if (currentMode == FM)
  {
    stringMode = "FM";
  }
  else if (currentMode == USB)
  {
    stringMode = "USB";
  }
  else
  {
    stringMode = "LSB";
  }
  
  String stringBFO;
  if (currentBFO < 100)
  {
    stringBFO = ".0";
  } else {
    stringBFO = ".";
  }
  server.send(200, "text/html", SendHTML(currentFrequency,stringBFO,currentBFO,stringMode)); 
}

void handle_NotFound(){
  server.send(404, "text/plain", "Not found");
}


String SendHTML(int currentFrequency, String stringBFO, int currentBFO, String stringMode){
  String ptr = "<!DOCTYPE html>";
  ptr +="<html>";
  ptr +="<head>";
  ptr +="<title>SI4732 (ESP32-S3) ATS-Mini/Pocket Receiver</title>";
  ptr +="<meta name='viewport' content='width=device-width, initial-scale=1.0'>";
  ptr +="<meta http-equiv='refresh' content='60'>";
  ptr +="<link href='https://fonts.googleapis.com/css?family=Open+Sans:300,400,600' rel='stylesheet'>";
  ptr +="<style>";
  ptr +="html { font-family: 'Open Sans', sans-serif; display: block; margin: 0px auto; text-align: center;color: #444444;}";
  ptr +="body{margin: 0px;} ";
  ptr +="h1 {margin: 50px auto 30px;} ";
  ptr +=".side-by-side{display: table-cell;vertical-align: middle;position: relative;}";
  ptr +=".text{font-weight: 600;font-size: 19px;width: 200px;}";
  ptr +=".reading{font-weight: 300;font-size: 50px;padding-right: 25px;}";
  ptr +=".frequecy .reading{color: #3B97D3;}";
  ptr +=".superscript{font-size: 17px;font-weight: 600;position: absolute;top: 10px;}";
  ptr +=".data{padding: 10px;}";
  ptr +=".container{display: table;margin: 0 auto;}";
  ptr +=".icon{width:65px}";
  ptr +="</style>";
  ptr +="</head>";
  ptr +="<body>";
  ptr +="<h1>SI4732 (ESP32-S3) ATS-Mini/Pocket Receiver</h1>";
  ptr +="<div class='container'>";

  ptr +="<div class='data frequecy'>";
  ptr +="<div class='side-by-side icon'>";
  ptr +="<svg xmlns='http://www.w3.org/2000/svg' enable-background='new 0 0 512 512' viewBox='0 0 512 512' id='radio'><g><g><linearGradient id='a' x1='284.966' x2='226.578' y1='-205.851' y2='880.166' gradientUnits='userSpaceOnUse'><stop offset='0' stop-color='#eae9fe'></stop><stop offset='.424' stop-color='#eeeefe'></stop><stop offset='.897' stop-color='#fbfbff'></stop><stop offset='1' stop-color='#fff'></stop></linearGradient><path fill='url(#a)' d='M468.836,264.275c-9.501-7.393-20.976-13.763-25.395-24.961 c-6.01-15.229,3.596-31.571,9.261-46.931c17.297-46.899-4.903-102.439-44.732-132.645 c-75.982-57.623-224.302-37.425-245.028,42.219c-8.768,33.692,6.941,72.131-9.131,103.013 c-20.278,38.966-74.073,41.594-108.719,58.921C2.169,285.358-6.419,332.1,24.365,367.686 c11.304,13.068,25.769,22.07,36.136,36.348c15.001,20.659,23.415,43.504,43.486,60.545c28.263,23.996,75.517,29.946,100.538,2.586 c14.089-15.407,18.674-38.106,34.054-52.225c20.245-18.586,52.242-16.086,78.004-6.514c25.762,9.571,49.654,25.026,76.789,29.381 c42.426,6.81,87.917-18.085,105.053-57.489S502.748,290.664,468.836,264.275z'></path><path fill='#ededed' d='M355.96,97.671L144.802,200.654c-2.397,1.198-4.953,1.757-7.589,1.757 c-1.838,0-3.755-0.319-5.593-0.879c-4.394-1.518-7.829-4.634-9.907-8.789c-1.997-4.154-2.317-8.788-0.799-13.182 c1.518-4.315,4.634-7.83,8.789-9.827l0.879-0.479l216.91-90.359c2.556-1.039,5.353-1.039,7.909,0 c2.477,1.039,4.474,3.036,5.513,5.593C363.071,89.442,360.834,95.275,355.96,97.671z'></path><path fill='#2769fd' d='M442.724,236.526v193.421c0,17.657-14.301,31.957-31.957,31.957H91.194 c-17.657,0-31.957-14.301-31.957-31.957V236.526c0-17.657,14.301-31.957,31.957-31.957h319.573 C428.423,204.568,442.724,218.869,442.724,236.526z'></path><path fill='#ededed' d='M169.39,197.144v7.424h-66.084v-7.424c0-18.248,14.793-33.042,33.042-33.042h0 C154.597,164.102,169.39,178.896,169.39,197.144z'></path><path fill='#e1e7ee' d='M271.243 333.293c0 50.519-40.961 91.593-91.593 91.593-50.519 0-91.593-41.075-91.593-91.593 0-50.633 41.075-91.707 91.593-91.707C230.283 241.586 271.243 282.661 271.243 333.293zM410.765 243.017v46.528c0 4.412-3.577 7.989-7.989 7.989h-90.18c-4.412 0-7.989-3.577-7.989-7.989v-46.528c0-4.412 3.577-7.989 7.989-7.989h90.18C407.188 235.028 410.765 238.605 410.765 243.017z'></path><path fill='#ff5859' d='M355.2,274.754v22.78h-17.786v-22.78c0-4.921,3.972-8.893,8.893-8.893S355.2,269.833,355.2,274.754 z'></path> <ellipse cx='357.686' cy='378.178' fill='#f9bb40' rx='45.256' ry='45.257' transform='rotate(-27.659 357.692 378.186)'></ellipse><path fill='#ededed' d='M331.267,339.16l19.181,19.181c3.494,3.494,3.494,9.104-0.001,12.599 c-3.494,3.494-9.103,3.494-12.598-0.001l-19.181-19.181c-3.494-3.494-3.495-9.103,0-12.598 C322.163,335.666,327.773,335.665,331.267,339.16z'></path><path fill='#ff5859' d='M396.086,84.13c0,21.174-17.13,38.377-38.304,38.377c-21.247,0-38.378-17.204-38.378-38.377 s17.13-38.377,38.378-38.377C378.956,45.753,396.086,62.956,396.086,84.13z'></path><path fill='#332e8e' d='M91.194,466.072h319.572c19.919,0,36.126-16.206,36.126-36.125V236.526 c0-19.92-16.207-36.126-36.126-36.126H173.559v-3.256c0-1.924-0.192-3.8-0.476-5.647l155.978-76.071 c7.577,6.959,17.645,11.248,28.72,11.248c23.419,0,42.473-19.086,42.473-42.545s-19.053-42.545-42.473-42.545 c-23.46,0-42.546,19.086-42.546,42.545c0,1.219,0.082,2.418,0.183,3.612l-174.17,72.555c-1.608-0.213-3.236-0.361-4.901-0.361 c-20.516,0-37.209,16.691-37.209,37.209v3.256h-7.945c-19.919,0-36.126,16.206-36.126,36.126v193.421 C55.068,449.866,71.276,466.072,91.194,466.072z M357.782,49.921c18.823,0,34.137,15.346,34.137,34.209 s-15.314,34.209-34.137,34.209c-18.863,0-34.21-15.346-34.21-34.209S338.918,49.921,357.782,49.921z M316.985,96.12 c1.374,4.667,3.537,8.987,6.32,12.84l-152.442,74.348c-3.315-8.238-9.483-15.018-17.289-19.115L316.985,96.12z M107.475,197.144 c0-15.921,12.954-28.873,28.873-28.873c15.922,0,28.876,12.952,28.876,28.873v3.256h-57.749V197.144z M63.404,236.526 c0-15.324,12.468-27.79,27.79-27.79h12.113h66.084h241.375c15.322,0,27.79,12.467,27.79,27.79v193.421 c0,15.322-12.468,27.789-27.79,27.789H91.194c-15.322,0-27.79-12.467-27.79-27.789V236.526z'></path><path fill='#332e8e' d='M179.651 429.054c52.802 0 95.761-42.958 95.761-95.761 0-52.865-42.958-95.875-95.761-95.875s-95.761 43.01-95.761 95.875C83.89 386.096 126.848 429.054 179.651 429.054zM179.651 245.754c48.206 0 87.425 39.269 87.425 87.539 0 48.206-39.219 87.425-87.425 87.425s-87.425-39.219-87.425-87.425C92.226 285.024 131.445 245.754 179.651 245.754zM312.596 301.702c33.514 0 56.706 0 90.179 0 6.705 0 12.159-5.453 12.159-12.156v-46.529c0-6.703-5.454-12.156-12.159-12.156h-90.179c-6.702 0-12.156 5.453-12.156 12.156v46.529C300.44 296.249 305.894 301.702 312.596 301.702zM341.583 293.366v-18.611c0-2.606 2.119-4.725 4.724-4.725s4.724 2.119 4.724 4.725v18.611H341.583zM308.775 243.016c0-2.107 1.715-3.82 3.82-3.82h11.25v18.674c0 2.302 1.867 4.168 4.168 4.168 2.301 0 4.168-1.865 4.168-4.168v-18.674h21.338v8.527c0 2.302 1.867 4.168 4.168 4.168 2.301 0 4.168-1.865 4.168-4.168v-8.527h21.338V256c0 2.302 1.867 4.168 4.168 4.168 2.301 0 4.168-1.865 4.168-4.168v-16.804h11.247c2.108 0 3.823 1.714 3.823 3.82v46.529c0 2.107-1.715 3.82-3.823 3.82h-43.409v-18.611c0-7.201-5.858-13.061-13.06-13.061s-13.06 5.86-13.06 13.061v18.611h-20.652c-2.106 0-3.82-1.714-3.82-3.82V243.016zM333.147 335.342c-5.129-4.146-12.661-3.895-17.425.871-4.761 4.761-5.017 12.276-.887 17.405-10.812 18.876-8.199 43.403 7.904 59.507 19.297 19.3 50.595 19.301 69.894 0 19.301-19.295 19.301-50.594 0-69.895C376.551 327.146 352.09 324.405 333.147 335.342zM321.615 342.108c1.879-1.881 4.826-1.88 6.705-.001l19.181 19.182c1.85 1.847 1.885 4.819 0 6.703-1.88 1.879-4.824 1.876-6.705 0l-19.181-19.181C319.735 346.931 319.735 343.987 321.615 342.108zM386.739 407.232c-16.042 16.042-42.065 16.042-58.107 0-12.777-12.778-15.261-31.9-7.642-47.258l13.913 13.914c5.099 5.093 13.395 5.099 18.492-.001 5.125-5.125 5.128-13.367 0-18.492l-13.962-13.962c15.47-7.756 34.565-5.051 47.307 7.692C402.784 365.168 402.781 391.19 386.739 407.232z'></path></g></g></svg>";
  ptr +="</div>";
  ptr +="<div class='side-by-side text'>Mode : </div>";
  ptr +="<div class='side-by-side reading'>";
  ptr +=(String)stringMode;
  ptr +="<span class='superscript'></span></div>";
  ptr +="</div>";
  
  ptr +="<div class='data frequecy'>";
  ptr +="<div class='side-by-side icon'>";
  ptr +="</div>";
  ptr +="<div class='side-by-side text'>Frequency : </div>";
  ptr +="<div class='side-by-side reading'>";
  ptr +=(int)currentFrequency;
  ptr +="<span class='superscript'>";
  ptr +=(String)stringBFO;
  ptr +=(int)currentBFO;
  ptr +=" kHz</span></div>";
  ptr +="</div>";
  
  ptr +="</div>";
  ptr +="</body>";
  ptr +="</html>";
  return ptr;
}
