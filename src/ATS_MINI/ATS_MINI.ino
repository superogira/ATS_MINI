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
int utcOffsetInSeconds = 25200;
const char* ssid = "WiFi1_SSID";
const char* password =  "WiFi1_Password";
const char* ssid2 = "WiFi2_SSID";
const char* password2 =  "WiFi2_Password";
String IPa;
String IPw;

const char *soft_ap_ssid          = "ATS-Mini";
const char *soft_ap_password      = NULL; // NULL for no password
const int   channel        = 10;    // WiFi Channel number between 1 and 13
const bool  hide_SSID      = false; // To disable SSID broadcast -> SSID will not appear in a basic WiFi scan
const int   max_connection = 3;     // Maximum simultaneous connected clients on the AP

WebServer server(80);
uint16_t ajaxInterval = 1000;

uint8_t aviationBandConverter = 0;

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

  WiFi.mode(WIFI_AP_STA); //Optional
  //Serial.println("Wait 100 ms for AP_START...");
  //delay(100);
  Serial.println("\n[*] Creating ESP32 AP");
  WiFi.softAP(soft_ap_ssid, soft_ap_password, channel, hide_SSID, max_connection);
  IPAddress ip(192, 168, 5, 5);
  IPAddress gateway(192,168,5,1); 
  IPAddress subnet(255, 255, 255, 0);
  WiFi.softAPConfig(ip, gateway, subnet);
  Serial.print("[+] AP Created with IP : ");
  IPa = WiFi.softAPIP().toString();
  Serial.println(IPa);
  tft.print("AP Mode IP : ");
  tft.println(IPa);
  
  WiFi.begin(ssid, password);
  Serial.printf("WiFi connecting to %s ", ssid);
  int wificheck = 0;
  tft.print("Connecting WiFi");
  while ((WiFi.status() != WL_CONNECTED) && wificheck <= 20){
    Serial.print(".");
    if((wificheck % 4) == 0){
      tft.print(".");
    }
    wificheck++;
    delay(500);
  }
  
  if(WiFi.status() != WL_CONNECTED){
    WiFi.disconnect();
    WiFi.begin(ssid2, password2);
    Serial.printf("\nWiFi connecting to %s ", ssid2);
    tft.print(" try2");
  }
  
  while ((WiFi.status() != WL_CONNECTED) && wificheck <= 40){
    Serial.print(".");
    if((wificheck % 4) == 0){
      tft.print(".");
    }
    wificheck++;
    delay(500);
  }
    
  if(WiFi.status() == WL_CONNECTED){
    Serial.println("\nConnected to the WiFi network");
    tft.println("Connected ");
    IPw = WiFi.localIP().toString();
    Serial.print("Local ESP32 IP:  ");
    Serial.println(IPw);  
    timeClient.update();
    
    ajaxInterval = 800;
    
    tft.print("WiFi IP : ");
    tft.print(IPw);
  } else {
    tft.println(" No WiFi connection");
    ajaxInterval = 3000;
  }

  server.on("/", handle_OnConnect);
  server.on("/radio", webradio);
  server.on("/data", []() {
    server.send(200, "text/plain", radio_data().c_str());
  });
  server.on("/setfrequency", webSetFreq);
  server.on("/setvolume", webSetVol);
  server.on("/config", webConfig);
  server.onNotFound(handle_NotFound);
  server.begin();
  Serial.println("\nHTTP server started\n");

  if(WiFi.status() == WL_CONNECTED){
    delay(2500);
  } else {
    delay(1500);
  }

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
    long freq;
    long khz;
    uint8_t mhz;
    long tail;
    
    spr.setTextDatum(MR_DATUM);
    if (isSSB()) {
      freq  = (uint32_t(currentFrequency) * 1000) + currentBFO; 
      khz   = freq / 1000;
      tail  = (freq % 1000);
      char skhz [32];
      char stail [32];
      sprintf(skhz, "%3.3u", khz);
      sprintf(stail, ".%3.3d", tail);
      spr.drawString(skhz,freq_offset_x,freq_offset_y,7);
      spr.setTextDatum(ML_DATUM);
      spr.drawString(stail,5+freq_offset_x,15+freq_offset_y,4);   
    }
    else {

      if (aviationBandConverter > 0 && currentMode == AM) {
        freq  = (long(aviationBandConverter * 1000000) + ((long(currentFrequency) * 1000)  + currentBFO));
        mhz   = freq / 1000000;
        tail  = (freq % 1000000) / 1000;
        Serial.println(freq);
        Serial.println(mhz);
        Serial.println(tail);
        char stail [32];
        sprintf(stail, ".%3.3d", tail);
        spr.drawNumber(mhz,freq_offset_x,freq_offset_y,7);
        spr.setTextDatum(ML_DATUM); 
        spr.drawString("AIR/CONV",125,40,2);
        spr.drawString(stail,5+freq_offset_x,15+freq_offset_y,4);
      } else {
        spr.drawNumber(currentFrequency,freq_offset_x,freq_offset_y,7);
        spr.setTextDatum(ML_DATUM);
        spr.drawString(".000",5+freq_offset_x,15+freq_offset_y,4);
      }
    }
    if (aviationBandConverter > 0 && currentMode == AM) {
      spr.drawString("MHz",funit_offset_x,funit_offset_y,4);
    } else {
      spr.drawString("kHz",funit_offset_x,funit_offset_y,4); 
    }
    
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
    

  if(WiFi.status() == WL_CONNECTED){
    timeClient.update();
    hours = timeClient.getHours();
    minutes = timeClient.getMinutes();
    sprintf(time_disp, "%2.2d:%2.2d", hours, minutes); 
  } else {
    // Run clock
    clock_time();    
  }
 


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
    Serial.print(getStrength());                  // S-Meter 
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
  server.send(200, "text/html", WebConfig()); 
}

void webradio() {
  server.send(200, "text/html", SendHTML()); 
}

void handle_NotFound(){
  server.send(404, "text/plain", "Not found");
}

void webSetFreq()
{
  if (server.arg("setFrequency") != "") {
    String webSetFrequency = server.arg("setFrequency");
    if (webSetFrequency.toFloat() <= 108) {
      rx.setFrequency(webSetFrequency.toFloat() * 100);
    } else {
      rx.setFrequency(webSetFrequency.toInt());
    }
  }
  if (server.arg("setBFO") != "") {
    String webSetBFO = server.arg("setBFO");
    Serial.println(webSetBFO); 
    currentFrequency = rx.getFrequency();
    currentBFO = (webSetBFO.toInt());
    updateBFO();
  }
  currentFrequency = rx.getFrequency();
  server.send(200); 
}

void webSetVol()
{
  Serial.println(server.arg("setVolume"));
  String webSetVolume= server.arg("setVolume");
  rx.setVolume(webSetVolume.toInt());
  server.send(200); 
}

void webConfig()
{
  String webWifiSSID = server.arg("setWifiSSID");
  if (webWifiSSID != "") {
    String webWifiPassword = server.arg("setWifiPassword");
    WiFi.disconnect();
    WiFi.begin(webWifiSSID, webWifiPassword);
    Serial.printf("WiFi connecting to %s\n", ssid);
    int wificheck = 0;
    while ((WiFi.status() != WL_CONNECTED) && wificheck <= 30){
      Serial.print(".");
      wificheck++;
      delay(500);
    }
      
    if(WiFi.status() == WL_CONNECTED){
      Serial.println("\nConnected to the WiFi network");
      IPw = WiFi.localIP().toString();
      Serial.print("WiFi IP: ");
      Serial.println(IPw);  
      
      ajaxInterval = 800;
    }
  }

  String webAviationBandConverter = server.arg("setAviationBandConverter");
  if (webAviationBandConverter != "") {
    aviationBandConverter = webAviationBandConverter.toInt();
  }

  String webUTCoffset = server.arg("setUTCoffsetSeconds");
  if (webUTCoffset != "") {
    utcOffsetInSeconds = webUTCoffset.toInt();
    timeClient.setTimeOffset(utcOffsetInSeconds);
    timeClient.update();
  }

  String webHours = server.arg("setHours");
  String webMinutes = server.arg("setMinutes");
  if ((webHours != "") && (webMinutes != "")) {
    time_hours = webHours.toInt();
    time_minutes = webMinutes.toInt();
  }

  String webSetBrightness = server.arg("setBrightness");
  if (webSetBrightness != "") {
    currentBrt = webSetBrightness.toInt();
    ledcWrite(0, currentBrt);
  }

  server.send(200, "text/html", SendHTML());
}

String radio_data() {
  String radioData;
  String stringTail = "\nkHz";
  String stringMode;
  if (currentMode == AM)
  {
    stringMode = "AM";
  }
  else if (currentMode == FM)
  {
    stringMode = "FM";
    stringTail = "\nMHz";
  }
  else if (currentMode == USB)
  {
    stringMode = "USB";
  }
  else
  {
    stringMode = "LSB";
  }

  long freq;
  uint16_t showFreq;
  uint16_t tail;
  String showTail;
  if (currentMode == FM)
  {
    freq = currentFrequency/100.00;
    showFreq   = int(freq);
    tail  = (currentFrequency % 100);
    if (tail < 10)
    {
      showTail = "0" + String(tail);
    } else {
      showTail = String(tail);
    }
  } else {
    if (aviationBandConverter > 0 && currentMode == AM) {
      stringTail = "\nMHz";
      
      freq  = (long(aviationBandConverter * 1000000) + ((long(currentFrequency) * 1000)  + currentBFO));
      showFreq   = freq / 1000000;
      showTail  = String((freq % 1000000) / 1000);
      
    } else {
      freq  = ((currentFrequency) * 1000) + currentBFO;
      showFreq   = freq / 1000;
      tail  =  (freq % 1000);
      if (tail < 10)
      {
        showTail = "00" + String(tail);
      } else if ((tail >= 10) && (tail < 100)) {
         showTail = "0" + String(tail);
      } else {
        showTail = String(tail);
      }
    }
  }
  radioData= stringMode + " " + String(showFreq) + " ." + showTail + stringTail + " " + String(getStrength()) + " " + String(rx.getVolume());
  return String(radioData);
}

String WebConfig(){
  String ptr = "<!DOCTYPE html>";
  ptr +="<html>";
  ptr +="<head>";
  ptr +="<title>ATS-Mini Config</title>";
  ptr +="<meta name='viewport' content='width=device-width, initial-scale=1'>";
  ptr +="<style>";
  ptr +="html {display: block; margin: 0px auto; text-align: center;color: #444444;}";
  ptr +="body{margin: 0px;} ";
  ptr +="h1 {margin: 10px auto 15px;} ";
  ptr +=".container{display: table;margin: 0 auto;}";
  ptr +="</style>";
  ptr +="</head>";
  ptr +="<body>";
  ptr +="<h1>ATS-Mini Config</h1>";
  ptr +="<div class='container'>";
  
  ptr +="<br><div id='setWifi'><form action=\"/config\" method=\"POST\"><label for=\"wifissid\">WiFi SSID : </label><input type=\"text\" name=\"setWifiSSID\" id=\"setwifissid\"><br><label for=\"wifipassword\">WiFi Password : </label><input type=\"password\" name=\"setWifiPassword\" id=\"setwifipassword\"><br><br><input type='submit' value='Connect WiFi (Only 2.4GHz)'></form></div><br>";
  ptr +="<br><div id='setAviationBand'><form action=\"/config\" method=\"POST\"><label for=\"aviationband\">Aviation Band Converter Model (100,110,0 for disable) : </label><input type=\"number\" min='0' max='110' name=\"setAviationBandConverter\" id=\"setaviationbandmodel\"> <input type='submit' value='Set Model'></form></div><br>";
  ptr +="<br><div id='setUTCoffset'><form action=\"/config\" method=\"POST\"><label for=\"utcoffset\">UTC Offset in seconds : </label><input type=\"number\" min='-43200' max='43200' name=\"setUTCoffsetSeconds\" id=\"setutcoffsetseconds\"> <input type='submit' value='Set UTC Offset'></form></div><br>";
  ptr +="<br><div id='setTime'><form action=\"/config\" method=\"POST\"><label for=\"hours\">Hours : </label><input type=\"number\" min='0' max='24' name=\"setHours\" id=\"sethours\" size=\"2\"> <label for=\"minutes\">Minutes : </label><input type=\"number\" min='0' max='60' name=\"setMinutes\" id=\"setminutes\" size=\"2\"> <input type='submit' value='Manual set time (if no internet)'></form></div><br>";
  ptr +="<br><div id='setBrightness'><form action=\"/config\" method=\"POST\"><label for=\"hours\">LCD Brightness (0-255) : </label><input type=\"number\" min='0' max='255' name=\"setBrightness\" id=\"setbrightness\" size=\"3\"> <input type='submit' value='Set Brightness'></form></div>";
  ptr +="***At brightness levels below 100%(255), switching from the PWM may cause power spikes and/or RFI***";
  ptr +="<br><br><br><br><div><form action=\"/radio\" method=\"POST\"><input type='submit' value='Radio'></form>";
  
  ptr +="</div>";
  ptr +="</body>";
  ptr +="</html>";
  return ptr;
}

String SendHTML(){
  String ptr = "<!DOCTYPE html>";
  ptr +="<html>";
  ptr +="<head>";
  ptr +="<title>SI4732 (ESP32-S3) ATS-Mini/Pocket Receiver Station</title>";
  ptr +="<meta name='viewport' content='width=device-width, initial-scale=1'>";
  ptr +="<style>";
  ptr +="html {display: block; margin: 0px auto; text-align: center;color: #444444;}";
  ptr +="body{margin: 0px;} ";
  ptr +="h1 {margin: 10px auto 15px;} ";
  ptr +=".side-by-side{display: table-cell;vertical-align: middle;position: relative;}";
  ptr +=".text{font-weight: 600;font-size: 19px;width: 150px;}";
  ptr +=".reading{font-weight: 300;font-size: 50px;padding-right: 25px;}";
  ptr +=".mode .reading{color: #3B97D3;}";
  ptr +=".frequecy .reading{color: #26B99A;}";
  ptr +=".volume .reading{color: #955BA5;}";
  ptr +=".superscript{font-size: 20px;font-weight: 600;position: absolute;top: 8px;}";
  ptr +=".data{padding: 5px;}";
  ptr +=".container{display: table;margin: 0 auto;}";
  ptr +=".icon{width:65px}";
  ptr +="</style>";
  ptr +="</head>";
  ptr +="<body>";
  ptr +="<h1>SI4732 (ESP32-S3) ATS-Mini/Pocket Receiver Station</h1>";
  ptr +="<div class='container'>";

  ptr +="<div class='data mode'>";
  ptr +="<div class='side-by-side icon'>";
  ptr +="<svg xmlns=\"http://www.w3.org/2000/svg\" enable-background=\"new 0 0 68 68\" viewBox=\"0 0 68 68\" id=\"radio\"><path fill=\"#DFE1E6\" d=\"M2.26,26.57531v36.44914c0,1.64339,1.33223,2.97562,2.97562,2.97562h57.52872 c1.64339,0,2.97562-1.33223,2.97562-2.97562V26.57531c0-1.64339-1.33224-2.97562-2.97562-2.97562H5.23562 C3.59223,23.59969,2.26,24.93192,2.26,26.57531z\"></path><path fill=\"#C7CAD0\" d=\"M65.74171,26.57816v36.45001c0,1.63995-1.33002,2.96997-2.98005,2.96997H5.23169 c-1.64001,0-2.97003-1.33002-2.97003-2.96997v-1.92999c36.88-1.68005,48.20001-27.31006,51.25-37.5h9.25 C64.41169,23.59818,65.74171,24.92814,65.74171,26.57816z\"></path><path fill=\"#C7CAD0\" d=\"M65.74646,26.57348v36.44995c0,1.64002-1.32996,2.97004-2.97998,2.97004H5.23651 c-1.64001,0-2.97003-1.33002-2.97003-2.97004v-1.92999c9.09998-0.40997,16.65002-2.28998,22.90002-5.02997 c6.87-3.01001,12.16998-7.07001,16.23999-11.39001c7.25-7.66998,10.64001-16.15997,12.10999-21.08002h9.25 C64.4165,23.59344,65.74646,24.92345,65.74646,26.57348z\"></path><path fill=\"#DFE1E6\" d=\"M9.34236,20.62406v2.97562h6.49622v-2.97562c0-0.5478-0.44408-0.99187-0.99187-0.99187h-4.51247 C9.78643,19.63219,9.34236,20.07627,9.34236,20.62406z\"></path><line x1=\"12.59\" x2=\"28.96\" y1=\"19.632\" y2=\"3.039\" fill=\"#DFE1E6\"></line><circle cx=\"12.721\" cy=\"51.374\" r=\"5\" fill=\"#FAC41D\"></circle><path fill=\"#F4A01E\" d=\"M17.72116,51.37468c0,2.75926-2.23847,4.9977-5.00227,4.9977c-2.70899,0-4.9109-2.15622-4.99313-4.84237 c0.70351,0.39286,1.50753,0.62128,2.36637,0.635h0.07308c1.95066,0,3.64551-1.11924,4.46322-2.75469 c0.34262-0.67609,0.53448-1.43442,0.53448-2.24301c0-0.05482,0-0.10963-0.00455-0.15991 C16.68872,47.8617,17.72116,49.49713,17.72116,51.37468z\"></path><path fill=\"#5EA7DC\" d=\"M61.27419,35.5912v-6.37248c0-0.5478-0.44408-0.99187-0.99187-0.99187H7.71297 c-0.5478,0-0.99187,0.44408-0.99187,0.99187v6.37248c0,0.54779,0.44408,0.99187,0.99187,0.99187h52.56934 C60.83012,36.58308,61.27419,36.139,61.27419,35.5912z\"></path><path fill=\"#3779BE\" d=\"M61.27885,29.21727v6.3778c0,0.54552-0.44636,0.99187-0.99187,0.99187H7.71764 c-0.55545,0-0.99187-0.44635-0.99187-0.99187v-2.45273c32.52354,0.23804,47.82622-2.12096,50.36463-4.91695h3.19659 C60.8325,28.2254,61.27885,28.67175,61.27885,29.21727z\"></path><circle cx=\"33.996\" cy=\"51.374\" r=\"10\" fill=\"#FFF\"></circle><path fill=\"#DFE1E6\" d=\"M43.99646,51.37346c0,5.51996-4.47998,10-10,10c-3.82996,0-7.14996-2.15002-8.82996-5.31 c6.87-3.01001,12.16998-7.07001,16.23999-11.39001C43.01648,46.44347,43.99646,48.79345,43.99646,51.37346z\"></path><path fill=\"#231F20\" d=\"M62.76416,22.59962H16.83838v-1.97559c0-1.05634-0.82916-1.91486-1.86908-1.97937L28.96484,4.45796 l0.32666,0.32672c0.19531,0.19531,0.45117,0.29297,0.70703,0.29297S30.51025,4.98,30.70557,4.78468 c0.39063-0.39014,0.39063-1.02344,0-1.41406l-1.04144-1.04169c-0.00067-0.00067-0.00085-0.00159-0.00085-0.00159 c-0.00128-0.00128-0.0022-0.00146-0.00281-0.00208l-1.03204-1.03229c-0.39063-0.39063-1.02344-0.39063-1.41406,0 c-0.39063,0.39014-0.39063,1.02344,0,1.41406l0.33649,0.33661l-15.3786,15.58868h-1.83777 c-1.09863,0-1.99219,0.89355-1.99219,1.9917v1.97559H5.23584c-2.19238,0-3.97559,1.7832-3.97559,3.97559v36.44922 c0,2.19238,1.7832,3.97559,3.97559,3.97559h57.52832c2.19238,0,3.97559-1.7832,3.97559-3.97559V26.57521 C66.73975,24.38283,64.95654,22.59962,62.76416,22.59962z M10.33447,20.63234c0.2834-0.00052,4.78304-0.00882,4.50391-0.0083 v1.97559h-4.49902L10.33447,20.63234z M64.73975,63.02443c0,1.08936-0.88672,1.97559-1.97559,1.97559H5.23584 c-1.08887,0-1.97559-0.88623-1.97559-1.97559V26.57521c0-1.08936,0.88672-1.97559,1.97559-1.97559 c9.54917,0,42.58106,0,57.52832,0c1.08887,0,1.97559,0.88623,1.97559,1.97559V63.02443z\"></path><path fill=\"#231F20\" d=\"M12.72119 45.37453c-3.30859 0-6 2.69141-6 6s2.69141 6 6 6 6-2.69141 6-6S16.02979 45.37453 12.72119 45.37453zM12.72119 55.37453c-2.20605 0-4-1.79443-4-4s1.79395-4 4-4 4 1.79443 4 4S14.92725 55.37453 12.72119 55.37453zM60.28271 27.22707H7.71338c-1.09863 0-1.99219.89355-1.99219 1.9917v6.37256c0 1.09814.89355 1.9917 1.99219 1.9917h52.56934c1.09766 0 1.99121-.89355 1.99121-1.9917v-6.37256C62.27393 28.12062 61.38037 27.22707 60.28271 27.22707zM60.27393 35.58302l-35.32227.00555V31.4131c0-.55225-.44727-1-1-1s-1 .44775-1 1v4.17584L7.72119 35.59132l-.00781-6.36426 52.56055-.0083V35.58302zM60.28271 35.58302c0 0 0 0 .00098 0C60.18353 35.58302 60.18255 35.58302 60.28271 35.58302zM33.99561 40.37453c-6.06543 0-11 4.93457-11 11s4.93457 11 11 11 11-4.93457 11-11S40.06104 40.37453 33.99561 40.37453zM25.05481 52.37599H42.9364c-.13202 1.18579-.49261 2.30328-1.04095 3.30518H26.09576C25.54742 54.67928 25.18683 53.56178 25.05481 52.37599zM25.05438 50.37599c.13159-1.18567.49164-2.30322 1.03955-3.30518h15.80334c.54791 1.00195.90796 2.11951 1.03955 3.30518H25.05438zM33.99561 42.37453c2.50922 0 4.7793 1.03436 6.41309 2.69629H27.58252C29.21631 43.40889 31.48639 42.37453 33.99561 42.37453zM33.99561 60.37453c-2.50775 0-4.77686-1.03314-6.4104-2.69336h12.8208C38.77246 59.34138 36.50336 60.37453 33.99561 60.37453z\"></path><g><circle cx=\"54.679\" cy=\"46.172\" r=\"1.5\" fill=\"#A3A6A8\"></circle><circle cx=\"54.679\" cy=\"56.577\" r=\"1.5\" fill=\"#A3A6A8\"></circle><circle cx=\"59.779\" cy=\"51.374\" r=\"1.5\" fill=\"#A3A6A8\"></circle><circle cx=\"49.579\" cy=\"51.374\" r=\"1.5\" fill=\"#A3A6A8\"></circle><g><circle cx=\"54.579\" cy=\"51.374\" r=\"1.5\" fill=\"#A3A6A8\"></circle></g></g></svg>";
  ptr +="</div>";
  ptr +="<div class='side-by-side text'>Mode : </div>";
  ptr +="<div class='side-by-side reading' id='Mode'>Please Wait";
  ptr +="<span class='superscript'></span></div>";
  ptr +="</div>";
  
  ptr +="<div class='data frequecy'>";
  ptr +="<div class='side-by-side icon'>";
  if(aviationBandConverter > 0 && currentMode == AM){
    ptr +="<svg xmlns=\"http://www.w3.org/2000/svg\" viewBox=\"0 0 32 32\" id=\"airplane\"><g><path fill=\"#1e88e5\" d=\"M14 24.3a1 1 0 0 0-1.55-.77l-4 2.64A1 1 0 0 0 8 27v2a1 1 0 0 0 1 1h4.91a1 1 0 0 0 1-1.22L14 24.86zM24 27v2a1 1 0 0 1-1 1H18a1 1 0 0 0 1-.75l1-4 0-.19.07-1.21 3.48 2.32A1 1 0 0 1 24 27z\"></path><path fill=\"#cfd8dc\" d=\"M9,12H6a1,1,0,0,0-1,1v4.11A1,1,0,0,0,6.53,18l3-1.89a1,1,0,0,0,.47-.85V13A1,1,0,0,0,9,12Z\"></path><path fill=\"#90a4ae\" d=\"M27,13v3.56l-5-3.15V13a1,1,0,0,1,1-1h3A1,1,0,0,1,27,13Z\"></path><path fill=\"#42a5f5\" d=\"M12.8,12.23a1,1,0,0,0-1,0L2.47,18.15A1,1,0,0,0,2,19v4a1,1,0,0,0,1,1l.22,0L13,21.78a1,1,0,0,0,.78-1l-.43-7.68A1,1,0,0,0,12.8,12.23Z\"></path><path fill=\"#fafafa\" d=\"M16,2V30H13.91a1,1,0,0,1-1-.78l-.92-4s0-.1,0-.16l-.07-1.21L11.83,22l-.52-9.45h0L11,7.06A5,5,0,0,1,16,2Z\"></path><path fill=\"#42a5f5\" d=\"M16,25v5H15V26A1,1,0,0,1,16,25Z\"></path><path fill=\"#b2ebf2\" d=\"M16,6V8H15a1,1,0,0,1,0-2Z\"></path><path fill=\"#1e88e5\" d=\"M17,26v4H16V25A1,1,0,0,1,17,26Z\"></path><path fill=\"#cfd8dc\" d=\"M21,7v.06l-.31,5.52h0L20.17,22l-.1,1.81L20,25.06l0,.19-1,4A1,1,0,0,1,18,30H17V26a1,1,0,0,0-1-1V8h1a1,1,0,0,0,0-2H16V2a5,5,0,0,1,5,5Z\"></path><path fill=\"#4dd0e1\" d=\"M18,7a1,1,0,0,1-1,1H16V6h1A1,1,0,0,1,18,7Z\"></path><path fill=\"#1e88e5\" d=\"M30,19v4a1,1,0,0,1-.38.78A1,1,0,0,1,29,24l-.22,0L20.17,22l.52-9.45,1.31.82,5,3.15,2.53,1.59A1,1,0,0,1,30,19Z\"></path><path fill=\"#263238\" d=\"M29.53,18.15,27,16.56V13a1,1,0,0,0-1-1H23a1,1,0,0,0-1,1v.41l-1.31-.82L21,7a5,5,0,1,0-10,.06l.31,5.53L10,13.41V13a1,1,0,0,0-1-1H6a1,1,0,0,0-1,1v3.56L2.47,18.15A1,1,0,0,0,2,19v4a1,1,0,0,0,1,1l.22,0L11.83,22l.1,1.81L8.45,26.17A1,1,0,0,0,8,27v2a1,1,0,0,0,1,1H23a1,1,0,0,0,1-1V27a1,1,0,0,0-.45-.83l-3.49-2.33.1-1.81L28.78,24,29,24a1,1,0,0,0,1-1V19A1,1,0,0,0,29.53,18.15ZM24,14h1v1.3l-1-.63ZM7,14H8v.67L7,15.3ZM4,21.75v-2.2l7.44-4.68L11.72,20Zm6,5.79,2.21-1.48L12.65,28H10ZM17,28V26a1,1,0,0,0-2,0v2h-.3L14,24.86,13,7a3,3,0,0,1,6-.06l-1,17.9L17.19,28Zm5,0H19.26l.5-2L22,27.54Zm6-6.25L20.28,20l.29-5.14L28,19.55Z\"></path><path fill=\"#263238\" d=\"M17,6H15a1,1,0,0,0,0,2h2a1,1,0,0,0,0-2Z\"></path></g></svg>";
  } else {
    ptr +="<svg xmlns='http://www.w3.org/2000/svg' width='60' height='60' id='voice-memo'><path d='M23.01 14a1 1 0 0 0-.965.705l-3.342 10.86-1.845-3.079a1 1 0 0 0-1.715 0L12.433 27H9a1 1 0 0 0-1 1 1 1 0 0 0 1 1h4a1 1 0 0 0 .858-.487L16 24.943l2.143 3.57a1 1 0 0 0 1.812-.218L23 18.398l3.045 9.897a1 1 0 0 0 1.883.076L31 20.69l3.073 7.68A1 1 0 0 0 35 29h16a1 1 0 0 0 1-1 1 1 0 0 0-1-1H35.678l-3.75-9.371a1 1 0 0 0-1.855 0l-2.95 7.373-3.168-10.297A1 1 0 0 0 23.01 14zM9 31a1 1 0 0 0-1 1 1 1 0 0 0 1 1h3.434l2.709 4.514a1 1 0 0 0 1.714 0l1.846-3.078 3.342 10.86a1 1 0 0 0 1.91 0L27.123 35l2.95 7.374a1 1 0 0 0 1.855 0L35.678 33H51a1 1 0 0 0 1-1 1 1 0 0 0-1-1H35a1 1 0 0 0-.928.628L31 39.31l-3.073-7.68a1 1 0 0 0-1.882.076L23 41.603l-3.045-9.897a1 1 0 0 0-1.813-.22L16 35.059l-2.143-3.571A1 1 0 0 0 13 31z' color='#000' style='font-feature-settings:normal;font-variant-alternates:normal;font-variant-caps:normal;font-variant-east-asian:normal;font-variant-ligatures:normal;font-variant-numeric:normal;font-variant-position:normal;font-variation-settings:normal;inline-size:0;isolation:auto;mix-blend-mode:normal;shape-margin:0;shape-padding:0;text-decoration-color:#000;text-decoration-line:none;text-decoration-style:solid;text-indent:0;text-orientation:mixed;text-transform:none'></path></svg>";
  }
  ptr +="</div>";
  ptr +="<div class='side-by-side text'>Frequency : </div>";
  ptr +="<div class='side-by-side reading' id='Frequency'>Please Wait";
  ptr +="</div>";
  ptr +="</div>";

  ptr +="<div class='data volume'>";
  ptr +="<div class='side-by-side icon'>";
  ptr +="<svg xmlns=\"http://www.w3.org/2000/svg\" viewBox=\"0 0 512 512\" id=\"volume\"><g><g><polygon fill=\"#00efd1\" points=\"273.7 68.5 273.7 443.5 110.31 337.62 110.78 336.89 110.78 175.11 110.31 174.38 273.7 68.5\"></polygon><polygon fill=\"#00acea\" points=\"110.78 175.11 110.78 336.89 110.31 337.62 109.19 336.89 45.02 336.89 45.02 175.11 109.19 175.11 110.31 174.38 110.78 175.11\"></polygon></g><g><path fill=\"#083863\" d=\"M353.656 303.72a10 10 0 1 0 13.137 15.08A85.125 85.125 0 0 0 396 254.59v-.183a85.354 85.354 0 0 0-29.2-64.445 10 10 0 0 0-13.141 15.076A65.336 65.336 0 0 1 376 254.38v.21A65.132 65.132 0 0 1 353.656 303.72zM407.842 366.275A10 10 0 1 0 421.479 380.9a174.649 174.649 0 0 0 40.384-56.126A169.8 169.8 0 0 0 476.98 254.4a171.7 171.7 0 0 0-55.563-126.6A10 10 0 1 0 407.9 142.542 151.662 151.662 0 0 1 456.98 254.38a149.955 149.955 0 0 1-13.344 62.166A154.768 154.768 0 0 1 407.842 366.275z\"></path><path fill=\"#083863\" d=\"M268.262,60.053,106.226,165H45.02A10.112,10.112,0,0,0,35,175.11V336.89A10.112,10.112,0,0,0,45.02,347h61.206L268.262,451.947a9.967,9.967,0,0,0,5.439,1.581,10.6,10.6,0,0,0,4.925-1.228A10.163,10.163,0,0,0,284,443.5V68.5a10.245,10.245,0,0,0-15.738-8.447ZM55,185h46V327H55ZM264,425.1,121,332.488V179.515L264,86.9Z\"></path></g></g></svg>";
  ptr +="</div>";
  ptr +="<div class='side-by-side text'>Volume : </div>";
  ptr +="<div class='side-by-side reading' id='Volume'>Please Wait";
  ptr +="</div>";
  ptr +="</div>";
  
  ptr +="<h1>S-METER</h1>";
  ptr +="<div class=\"chart\"><canvas id=\"gauge\"></canvas></div>";

  ptr +="<br><FORM id='setfrequency' METHOD='POST' action=''><div id='setFreq'></form></div>";
  ptr +="<br><FORM id='setbfo' METHOD='POST' action=''><div id='setBFO'></form></div>";
  ptr +="<div id='setVolume'><FORM id='setvolume' METHOD='POST' action=''><label for='Volume'>0 - 63 : </label><input type='number' min='0' max='63' name='setVolume'><input type='submit' value='Set Volume'></form></div><br>";

  if(WiFi.status() == WL_CONNECTED){
      ptr +="Access Point Mode - IP : ";
      ptr +="<a href=\"http://";
      ptr +=IPa;
      ptr +="/radio\">";
      ptr +=IPa;
      ptr +="</a>";
      ptr +="<br>WiFi Connected IP : ";
      ptr +="<a href=\"http://";
      ptr +=IPw;
      ptr +="/radio\">";
      ptr +=IPw;
      ptr +="</a>";
    } else {
      ptr +="Access Point Mode - IP : ";
      ptr +="<a href=\"http://";
      ptr +=IPa;
      ptr +="/radio\">";
      ptr +=IPa;
      ptr +="</a>";
      ptr +="<br>WiFi not connect";
    }
  ptr +="<br><br><div><form action=\"/\" method=\"POST\"><input type='submit' value='Config'></form>";
  
  ptr +="<script type=\"text/javascript\">";
  ptr +="/* gauge.min.js */(function(){var t,i,e,s,n,o,a,h,r,l,c,p,u,d,g=[].slice,m={}.hasOwnProperty,f=function(t,i){for(var e in i)m.call(i,e)&&(t[e]=i[e]);function s(){this.constructor=t}return s.prototype=i.prototype,t.prototype=new s,t.__super__=i.prototype,t},x=[].indexOf||function(t){for(var i=0,e=this.length;i<e;i++)if(i in this&&this[i]===t)return i;return-1};!function(){var t,i,e,s,n,o,a;for(e=0,n=(a=[\"ms\",\"moz\",\"webkit\",\"o\"]).length;e<n&&(o=a[e],!window.requestAnimationFrame);e++)window.requestAnimationFrame=window[o+\"RequestAnimationFrame\"],window.cancelAnimationFrame=window[o+\"CancelAnimationFrame\"]||window[o+\"CancelRequestAnimationFrame\"];t=null,s=0,i={},window.requestAnimationFrame?window.cancelAnimationFrame||(t=window.requestAnimationFrame,window.requestAnimationFrame=function(e,n){var o;return o=++s,t((function(){if(!i[o])return e()}),n),o},window.cancelAnimationFrame=function(t){return i[t]=!0}):(window.requestAnimationFrame=function(t,i){var e,s,n,o;return e=(new Date).getTime(),o=Math.max(0,16-(e-n)),s=window.setTimeout((function(){return t(e+o)}),o),n=e+o,s},window.cancelAnimationFrame=function(t){return clearTimeout(t)})}(),d=function(t){var i,e;for(t-=3600*(i=Math.floor(t/3600))+60*(e=Math.floor((t-3600*i)/60)),t+=\"\",e+=\"\";e.length<2;)e=\"0\"+e;for(;t.length<2;)t=\"0\"+t;return(i=i?i+\":\":\"\")+e+\":\"+t},p=function(){var t,i,e;return e=(i=1<=arguments.length?g.call(arguments,0):[])[0],t=i[1],l(e.toFixed(t))},u=function(t,i){var e,s,n;for(e in s={},t)m.call(t,e)&&(n=t[e],s[e]=n);for(e in i)m.call(i,e)&&(n=i[e],s[e]=n);return s},l=function(t){var i,e,s,n;for(s=(e=(t+=\"\").split(\".\"))[0],n=\"\",e.length>1&&(n=\".\"+e[1]),i=/(\\d+)(\\d{3})/;i.test(s);)s=s.replace(i,\"$1,$2\");return s+n},c=function(t){return\"#\"===t.charAt(0)?t.substring(1,7):t},s=function(t){function i(){return i.__super__.constructor.apply(this,arguments)}return f(i,t),i.prototype.displayScale=1,i.prototype.forceUpdate=!0,i.prototype.setTextField=function(t,i){return this.textField=t instanceof h?t:new h(t,i)},i.prototype.setMinValue=function(t,i){var e,s,n,o,a;if(this.minValue=t,null==i&&(i=!0),i){for(this.displayedValue=this.minValue,a=[],s=0,n=(o=this.gp||[]).length;s<n;s++)e=o[s],a.push(e.displayedValue=this.minValue);return a}},i.prototype.setOptions=function(t){return null==t&&(t=null),this.options=u(this.options,t),this.textField&&(this.textField.el.style.fontSize=t.fontSize+\"px\"),this.options.angle>.5&&(this.options.angle=.5),this.configDisplayScale(),this},i.prototype.configDisplayScale=function(){var t,i,e,s,n;return s=this.displayScale,!1===this.options.highDpiSupport?delete this.displayScale:(i=window.devicePixelRatio||1,t=this.ctx.webkitBackingStorePixelRatio||this.ctx.mozBackingStorePixelRatio||this.ctx.msBackingStorePixelRatio||this.ctx.oBackingStorePixelRatio||this.ctx.backingStorePixelRatio||1,this.displayScale=i/t),this.displayScale!==s&&(n=this.canvas.G__width||this.canvas.width,e=this.canvas.G__height||this.canvas.height,this.canvas.width=n*this.displayScale,this.canvas.height=e*this.displayScale,this.canvas.style.width=n+\"px\",this.canvas.style.height=e+\"px\",this.canvas.G__width=n,this.canvas.G__height=e),this},i.prototype.parseValue=function(t){return t=parseFloat(t)||Number(t),isFinite(t)?t:0},i}(r=function(){function t(t,e){null==t&&(t=!0),this.clear=null==e||e,t&&i.add(this)}return t.prototype.animationSpeed=32,t.prototype.update=function(t){var i;return null==t&&(t=!1),!(!t&&this.displayedValue===this.value)&&(this.ctx&&this.clear&&this.ctx.clearRect(0,0,this.canvas.width,this.canvas.height),i=this.value-this.displayedValue,Math.abs(i/this.animationSpeed)<=.001?this.displayedValue=this.value:this.displayedValue=this.displayedValue+i/this.animationSpeed,this.render(),!0)},t}()),h=function(){function t(t,i){this.el=t,this.fractionDigits=i}return t.prototype.render=function(t){return this.el.innerHTML=p(t.displayedValue,this.fractionDigits)},t}(),t=function(t){function i(t,e){if(this.elem=t,this.text=null!=e&&e,i.__super__.constructor.call(this),void 0===this.elem)throw new Error(\"The element isn't defined.\");this.value=1*this.elem.innerHTML,this.text&&(this.value=0)}return f(i,t),i.prototype.displayedValue=0,i.prototype.value=0,i.prototype.setVal=function(t){return this.value=1*t},i.prototype.render=function(){var t;return t=this.text?d(this.displayedValue.toFixed(0)):l(p(this.displayedValue)),this.elem.innerHTML=t},i}(r),a=function(t){function i(t){if(this.gauge=t,void 0===this.gauge)throw new Error(\"The element isn't defined.\");this.ctx=this.gauge.ctx,this.canvas=this.gauge.canvas,i.__super__.constructor.call(this,!1,!1),this.setOptions()}return f(i,t),i.prototype.displayedValue=0,i.prototype.value=0,i.prototype.options={strokeWidth:.035,length:.1,color:\"#000000\",iconPath:null,iconScale:1,iconAngle:0},i.prototype.img=null,i.prototype.setOptions=function(t){if(null==t&&(t=null),this.options=u(this.options,t),this.length=2*this.gauge.radius*this.gauge.options.radiusScale*this.options.length,this.strokeWidth=this.canvas.height*this.options.strokeWidth,this.maxValue=this.gauge.maxValue,this.minValue=this.gauge.minValue,this.animationSpeed=this.gauge.animationSpeed,this.options.angle=this.gauge.options.angle,this.options.iconPath)return this.img=new Image,this.img.src=this.options.iconPath},i.prototype.render=function(){var t,i,e,s,n,o,a,h,r;if(t=this.gauge.getAngle.call(this,this.displayedValue),h=Math.round(this.length*Math.cos(t)),r=Math.round(this.length*Math.sin(t)),o=Math.round(this.strokeWidth*Math.cos(t-Math.PI/2)),a=Math.round(this.strokeWidth*Math.sin(t-Math.PI/2)),i=Math.round(this.strokeWidth*Math.cos(t+Math.PI/2)),e=Math.round(this.strokeWidth*Math.sin(t+Math.PI/2)),this.ctx.beginPath(),this.ctx.fillStyle=this.options.color,this.ctx.arc(0,0,this.strokeWidth,0,2*Math.PI,!1),this.ctx.fill(),this.ctx.beginPath(),this.ctx.moveTo(o,a),this.ctx.lineTo(h,r),this.ctx.lineTo(i,e),this.ctx.fill(),this.img)return s=Math.round(this.img.width*this.options.iconScale),n=Math.round(this.img.height*this.options.iconScale),this.ctx.save(),this.ctx.translate(h,r),this.ctx.rotate(t+Math.PI/180*(90+this.options.iconAngle)),this.ctx.drawImage(this.img,-s/2,-n/2,s,n),this.ctx.restore()},i}(r),function(){function t(t){this.elem=t}t.prototype.updateValues=function(t){return this.value=t[0],this.maxValue=t[1],this.avgValue=t[2],this.render()},t.prototype.render=function(){var t,i;return this.textField&&this.textField.text(p(this.value)),0===this.maxValue&&(this.maxValue=2*this.avgValue),i=this.value/this.maxValue*100,t=this.avgValue/this.maxValue*100,$(\".bar-value\",this.elem).css({width:i+\"%\"}),$(\".typical-value\",this.elem).css({width:t+\"%\"})}}(),o=function(t){function e(t){var i,s;this.canvas=t,e.__super__.constructor.call(this),this.percentColors=null,\"undefined\"!=typeof G_vmlCanvasManager&&(this.canvas=window.G_vmlCanvasManager.initElement(this.canvas)),this.ctx=this.canvas.getContext(\"2d\"),i=this.canvas.clientHeight,s=this.canvas.clientWidth,this.canvas.height=i,this.canvas.width=s,this.gp=[new a(this)],this.setOptions()}return f(e,t),e.prototype.elem=null,e.prototype.value=[20],e.prototype.maxValue=80,e.prototype.minValue=0,e.prototype.displayedAngle=0,e.prototype.displayedValue=0,e.prototype.lineWidth=40,e.prototype.paddingTop=.1,e.prototype.paddingBottom=.1,e.prototype.percentColors=null,e.prototype.options={colorStart:\"#6fadcf\",colorStop:void 0,gradientType:0,strokeColor:\"#e0e0e0\",pointer:{length:.8,strokeWidth:.035,iconScale:1},angle:.15,lineWidth:.44,radiusScale:1,fontSize:40,limitMax:!1,limitMin:!1},e.prototype.setOptions=function(t){var i,s,n,o,a;for(null==t&&(t=null),e.__super__.setOptions.call(this,t),this.configPercentColors(),this.extraPadding=0,this.options.angle<0&&(o=Math.PI*(1+this.options.angle),this.extraPadding=Math.sin(o)),this.availableHeight=this.canvas.height*(1-this.paddingTop-this.paddingBottom),this.lineWidth=this.availableHeight*this.options.lineWidth,this.radius=(this.availableHeight-this.lineWidth/2)/(1+this.extraPadding),this.ctx.clearRect(0,0,this.canvas.width,this.canvas.height),s=0,n=(a=this.gp).length;s<n;s++)(i=a[s]).setOptions(this.options.pointer),i.render();return this.render(),this},e.prototype.configPercentColors=function(){var t,i,e,s,n,o,a;if(this.percentColors=null,void 0!==this.options.percentColors){for(this.percentColors=new Array,o=[],e=s=0,n=this.options.percentColors.length-1;0<=n?s<=n:s>=n;e=0<=n?++s:--s)a=parseInt(c(this.options.percentColors[e][1]).substring(0,2),16),i=parseInt(c(this.options.percentColors[e][1]).substring(2,4),16),t=parseInt(c(this.options.percentColors[e][1]).substring(4,6),16),o.push(this.percentColors[e]={pct:this.options.percentColors[e][0],color:{r:a,g:i,b:t}});return o}},e.prototype.set=function(t){var e,s,n,o,h,r,l,c,p;for(t instanceof Array||(t=[t]),s=n=0,l=t.length-1;0<=l?n<=l:n>=l;s=0<=l?++n:--n)t[s]=this.parseValue(t[s]);if(t.length>this.gp.length)for(s=o=0,c=t.length-this.gp.length;0<=c?o<c:o>c;s=0<=c?++o:--o)(e=new a(this)).setOptions(this.options.pointer),this.gp.push(e);else t.length<this.gp.length&&(this.gp=this.gp.slice(this.gp.length-t.length));for(s=0,r=0,h=t.length;r<h;r++)(p=t[r])>this.maxValue?this.options.limitMax?p=this.maxValue:this.maxValue=p+1:p<this.minValue&&(this.options.limitMin?p=this.minValue:this.minValue=p-1),this.gp[s].value=p,this.gp[s++].setOptions({minValue:this.minValue,maxValue:this.maxValue,angle:this.options.angle});return this.value=Math.max(Math.min(t[t.length-1],this.maxValue),this.minValue),i.add(this),i.run(this.forceUpdate),this.forceUpdate=!1},e.prototype.getAngle=function(t){return(1+this.options.angle)*Math.PI+(t-this.minValue)/(this.maxValue-this.minValue)*(1-2*this.options.angle)*Math.PI},e.prototype.getColorForPercentage=function(t,i){var e,s,n,o,a,h,r;if(0===t)e=this.percentColors[0].color;else for(e=this.percentColors[this.percentColors.length-1].color,n=o=0,h=this.percentColors.length-1;0<=h?o<=h:o>=h;n=0<=h?++o:--o)if(t<=this.percentColors[n].pct){!0===i?(r=this.percentColors[n-1]||this.percentColors[0],s=this.percentColors[n],a=(t-r.pct)/(s.pct-r.pct),e={r:Math.floor(r.color.r*(1-a)+s.color.r*a),g:Math.floor(r.color.g*(1-a)+s.color.g*a),b:Math.floor(r.color.b*(1-a)+s.color.b*a)}):e=this.percentColors[n].color;break}return\"rgb(\"+[e.r,e.g,e.b].join(\",\")+\")\"},e.prototype.getColorForValue=function(t,i){var e;return e=(t-this.minValue)/(this.maxValue-this.minValue),this.getColorForPercentage(e,i)},e.prototype.renderStaticLabels=function(t,i,e,s){var n,o,a,h,r,l,c,u,d,g;for(this.ctx.save(),this.ctx.translate(i,e),l=/\\d+\\.?\\d?/,r=(n=t.font||\"10px Times\").match(l)[0],u=n.slice(r.length),o=parseFloat(r)*this.displayScale,this.ctx.font=o+u,this.ctx.fillStyle=t.color||\"#000000\",this.ctx.textBaseline=\"bottom\",this.ctx.textAlign=\"center\",a=0,h=(c=t.labels).length;a<h;a++)void 0!==(g=c[a]).label?(!this.options.limitMin||g>=this.minValue)&&(!this.options.limitMax||g<=this.maxValue)&&(r=(n=g.font||t.font).match(l)[0],u=n.slice(r.length),o=parseFloat(r)*this.displayScale,this.ctx.font=o+u,d=this.getAngle(g.label)-3*Math.PI/2,this.ctx.rotate(d),this.ctx.fillText(p(g.label,t.fractionDigits),0,-s-this.lineWidth/2),this.ctx.rotate(-d)):(!this.options.limitMin||g>=this.minValue)&&(!this.options.limitMax||g<=this.maxValue)&&(d=this.getAngle(g)-3*Math.PI/2,this.ctx.rotate(d),this.ctx.fillText(p(g,t.fractionDigits),0,-s-this.lineWidth/2),this.ctx.rotate(-d));return this.ctx.restore()},e.prototype.renderTicks=function(t,i,e,s){var n,o,a,h,r,l,c,p,u,d,g,m,f,x,v,y,V,w,S,M;if(\"object\"==typeof t&&null!==t&&Object.keys(t).length>0){for(l=t.divisions||0,w=t.subDivisions||0,a=t.divColor||\"#fff\",x=t.subColor||\"#fff\",h=t.divLength||.7,y=t.subLength||.2,u=parseFloat(this.maxValue)-parseFloat(this.minValue),d=parseFloat(u)/parseFloat(t.divisions),v=parseFloat(d)/parseFloat(t.subDivisions),n=parseFloat(this.minValue),o=0+v,r=(p=u/400)*(t.divWidth||1),V=p*(t.subWidth||1),m=[],S=c=0,g=l+1;c<g;S=c+=1)this.ctx.lineWidth=this.lineWidth*h,f=this.lineWidth/2*(1-h),M=this.radius*this.options.radiusScale+f,this.ctx.strokeStyle=a,this.ctx.beginPath(),this.ctx.arc(0,0,M,this.getAngle(n-r),this.getAngle(n+r),!1),this.ctx.stroke(),o=n+v,n+=d,S!==t.divisions&&w>0?m.push(function(){var t,i,e;for(e=[],t=0,i=w-1;t<i;t+=1)this.ctx.lineWidth=this.lineWidth*y,f=this.lineWidth/2*(1-y),M=this.radius*this.options.radiusScale+f,this.ctx.strokeStyle=x,this.ctx.beginPath(),this.ctx.arc(0,0,M,this.getAngle(o-V),this.getAngle(o+V),!1),this.ctx.stroke(),e.push(o+=v);return e}.call(this)):m.push(void 0);return m}},e.prototype.render=function(){var t,i,e,s,n,o,a,h,r,l,c,p,u,d,g,m,f,x;if(f=this.canvas.width/2,e=this.canvas.height*this.paddingTop+this.availableHeight-(this.radius+this.lineWidth/2)*this.extraPadding,t=this.getAngle(this.displayedValue),this.textField&&this.textField.render(this),this.ctx.lineCap=\"butt\",l=this.radius*this.options.radiusScale,this.options.staticLabels&&this.renderStaticLabels(this.options.staticLabels,f,e,l),this.options.staticZones)for(this.ctx.save(),this.ctx.translate(f,e),this.ctx.lineWidth=this.lineWidth,s=0,o=(c=this.options.staticZones).length;s<o;s++)r=(x=c[s]).min,this.options.limitMin&&r<this.minValue&&(r=this.minValue),h=x.max,this.options.limitMax&&h>this.maxValue&&(h=this.maxValue),m=this.radius*this.options.radiusScale,x.height&&(this.ctx.lineWidth=this.lineWidth*x.height,u=this.lineWidth/2*(x.offset||1-x.height),m=this.radius*this.options.radiusScale+u),this.ctx.strokeStyle=x.strokeStyle,this.ctx.beginPath(),this.ctx.arc(0,0,m,this.getAngle(r),this.getAngle(h),!1),this.ctx.stroke();else void 0!==this.options.customFillStyle?i=this.options.customFillStyle(this):null!==this.percentColors?i=this.getColorForValue(this.displayedValue,this.options.generateGradient):void 0!==this.options.colorStop?(0===this.options.gradientType?(d=l-this.lineWidth/2,g=l+this.lineWidth/2,i=this.ctx.createRadialGradient(f,e,d,f,e,g)):i=this.ctx.createLinearGradient(0,0,f,0),i.addColorStop(0,this.options.colorStart),i.addColorStop(1,this.options.colorStop)):i=this.options.colorStart,this.ctx.strokeStyle=i,this.ctx.beginPath(),this.ctx.arc(f,e,l,(1+this.options.angle)*Math.PI,t,!1),this.ctx.lineWidth=this.lineWidth,this.ctx.stroke(),this.ctx.strokeStyle=this.options.strokeColor,this.ctx.beginPath(),this.ctx.arc(f,e,l,t,(2-this.options.angle)*Math.PI,!1),this.ctx.stroke(),this.ctx.save(),this.ctx.translate(f,e);for(this.options.renderTicks&&this.renderTicks(this.options.renderTicks,f,e,l),this.ctx.restore(),this.ctx.translate(f,e),n=0,a=(p=this.gp).length;n<a;n++)p[n].update(!0);return this.ctx.translate(-f,-e)},e}(s),e=function(t){function e(t){this.canvas=t,e.__super__.constructor.call(this),\"undefined\"!=typeof G_vmlCanvasManager&&(this.canvas=window.G_vmlCanvasManager.initElement(this.canvas)),this.ctx=this.canvas.getContext(\"2d\"),this.setOptions(),this.render()}return f(e,t),e.prototype.lineWidth=15,e.prototype.displayedValue=0,e.prototype.value=33,e.prototype.maxValue=80,e.prototype.minValue=0,e.prototype.options={lineWidth:.1,colorStart:\"#6f6ea0\",colorStop:\"#c0c0db\",strokeColor:\"#eeeeee\",shadowColor:\"#d5d5d5\",angle:.35,radiusScale:1},e.prototype.getAngle=function(t){return(1-this.options.angle)*Math.PI+(t-this.minValue)/(this.maxValue-this.minValue)*(2+this.options.angle-(1-this.options.angle))*Math.PI},e.prototype.setOptions=function(t){return null==t&&(t=null),e.__super__.setOptions.call(this,t),this.lineWidth=this.canvas.height*this.options.lineWidth,this.radius=this.options.radiusScale*(this.canvas.height/2-this.lineWidth/2),this},e.prototype.set=function(t){return this.value=this.parseValue(t),this.value>this.maxValue?this.options.limitMax?this.value=this.maxValue:this.maxValue=this.value:this.value<this.minValue&&(this.options.limitMin?this.value=this.minValue:this.minValue=this.value),i.add(this),i.run(this.forceUpdate),this.forceUpdate=!1},e.prototype.render=function(){var t,i,e,s,n,o;return t=this.getAngle(this.displayedValue),o=this.canvas.width/2,e=this.canvas.height/2,this.textField&&this.textField.render(this),s=this.radius-this.lineWidth/2,n=this.radius+this.lineWidth/2,(i=this.ctx.createRadialGradient(o,e,s,o,e,n)).addColorStop(0,this.options.colorStart),i.addColorStop(1,this.options.colorStop),this.ctx.strokeStyle=this.options.strokeColor,this.ctx.beginPath(),this.ctx.arc(o,e,this.radius,(1-this.options.angle)*Math.PI,(2+this.options.angle)*Math.PI,!1),this.ctx.lineWidth=this.lineWidth,this.ctx.lineCap=\"round\",this.ctx.stroke(),this.ctx.strokeStyle=i,this.ctx.beginPath(),this.ctx.arc(o,e,this.radius,(1-this.options.angle)*Math.PI,t,!1),this.ctx.stroke()},e}(s),n=function(t){function i(){return i.__super__.constructor.apply(this,arguments)}return f(i,t),i.prototype.strokeGradient=function(t,i,e,s){var n;return(n=this.ctx.createRadialGradient(t,i,e,t,i,s)).addColorStop(0,this.options.shadowColor),n.addColorStop(.12,this.options._orgStrokeColor),n.addColorStop(.88,this.options._orgStrokeColor),n.addColorStop(1,this.options.shadowColor),n},i.prototype.setOptions=function(t){var e,s,n,o;return null==t&&(t=null),i.__super__.setOptions.call(this,t),o=this.canvas.width/2,e=this.canvas.height/2,s=this.radius-this.lineWidth/2,n=this.radius+this.lineWidth/2,this.options._orgStrokeColor=this.options.strokeColor,this.options.strokeColor=this.strokeGradient(o,e,s,n),this},i}(e),i={elements:[],animId:null,addAll:function(t){var e,s,n,o;for(o=[],s=0,n=t.length;s<n;s++)e=t[s],o.push(i.elements.push(e));return o},add:function(t){if(x.call(i.elements,t)<0)return i.elements.push(t)},run:function(t){var e,s,n,o,a,h,r;if(null==t&&(t=!1),isFinite(parseFloat(t))||!0===t){for(e=!0,r=[],n=s=0,a=(h=i.elements).length;s<a;n=++s)h[n].update(!0===t)?e=!1:r.push(n);for(o=r.length-1;o>=0;o+=-1)n=r[o],i.elements.splice(n,1);return i.animId=e?null:requestAnimationFrame(i.run)}if(!1===t)return!0===i.animId&&cancelAnimationFrame(i.animId),i.animId=requestAnimationFrame(i.run)}},\"function\"==typeof window.define&&null!=window.define.amd?define((function(){return{Gauge:o,Donut:n,BaseDonut:e,TextRenderer:h,AnimationUpdater:i}})):\"undefined\"!=typeof module&&null!=module.exports?module.exports={Gauge:o,Donut:n,BaseDonut:e,TextRenderer:h,AnimationUpdater:i}:(window.Gauge=o,window.Donut=n,window.BaseDonut=e,window.TextRenderer=h,window.AnimationUpdater=i)}).call(this);";
  ptr +="</script>";
  ptr +="<script type=\"text/javascript\">";
  ptr +="/* Zepto v1.2.0 - zepto event ajax form ie - zeptojs.com/license */!function(t,e){'function'==typeof define&&define.amd?define(function(){return e(t)}):e(t)}(this,function(t){var e=function(){function $(t){return null==t?String(t):S[C.call(t)]||'object'}function F(t){return'function'==$(t)}function k(t){return null!=t&&t==t.window}function M(t){return null!=t&&t.nodeType==t.DOCUMENT_NODE}function R(t){return'object'==$(t)}function Z(t){return R(t)&&!k(t)&&Object.getPrototypeOf(t)==Object.prototype}function z(t){var e=!!t&&'length'in t&&t.length,n=r.type(t);return'function'!=n&&!k(t)&&('array'==n||0===e||'number'==typeof e&&e>0&&e-1 in t)}function q(t){return a.call(t,function(t){return null!=t})}function H(t){return t.length>0?r.fn.concat.apply([],t):t}function I(t){return t.replace(/::/g,'/').replace(/([A-Z]+)([A-Z][a-z])/g,'$1_$2').replace(/([a-z\\d])([A-Z])/g,'$1_$2').replace(/_/g,'-').toLowerCase()}function V(t){return t in l?l[t]:l[t]=new RegExp('(^|\\\\s)'+t+'(\\\\s|$)')}function _(t,e){return'number'!=typeof e||h[I(t)]?e:e+'px'}function B(t){var e,n;return c[t]||(e=f.createElement(t),f.body.appendChild(e),n=getComputedStyle(e,'').getPropertyValue('display'),e.parentNode.removeChild(e),'none'==n&&(n='block'),c[t]=n),c[t]}function U(t){return'children'in t?u.call(t.children):r.map(t.childNodes,function(t){return 1==t.nodeType?t:void 0})}function X(t,e){var n,r=t?t.length:0;for(n=0;r>n;n++)this[n]=t[n];this.length=r,this.selector=e||''}function J(t,r,i){for(n in r)i&&(Z(r[n])||L(r[n]))?(Z(r[n])&&!Z(t[n])&&(t[n]={}),L(r[n])&&!L(t[n])&&(t[n]=[]),J(t[n],r[n],i)):r[n]!==e&&(t[n]=r[n])}function W(t,e){return null==e?r(t):r(t).filter(e)}function Y(t,e,n,r){return F(e)?e.call(t,n,r):e}function G(t,e,n){null==n?t.removeAttribute(e):t.setAttribute(e,n)}function K(t,n){var r=t.className||'',i=r&&r.baseVal!==e;return n===e?i?r.baseVal:r:void(i?r.baseVal=n:t.className=n)}function Q(t){try{return t?'true'==t||('false'==t?!1:'null'==t?null:+t+''==t?+t:/^[\\[\\{]/.test(t)?r.parseJSON(t):t):t}catch(e){return t}}function tt(t,e){e(t);for(var n=0,r=t.childNodes.length;r>n;n++)tt(t.childNodes[n],e)}var e,n,r,i,O,P,o=[],s=o.concat,a=o.filter,u=o.slice,f=t.document,c={},l={},h={'column-count':1,columns:1,'font-weight':1,'line-height':1,opacity:1,'z-index':1,zoom:1},p=/^\\s*<(\\w+|!)[^>]*>/,d=/^<(\\w+)\\s*\\/?>(?:<\\/\\1>|)$/,m=/<(?!area|br|col|embed|hr|img|input|link|meta|param)(([\\w:]+)[^>]*)\\/>/gi,g=/^(?:body|html)$/i,v=/([A-Z])/g,y=['val','css','html','text','data','width','height','offset'],x=['after','prepend','before','append'],b=f.createElement('table'),E=f.createElement('tr'),j={tr:f.createElement('tbody'),tbody:b,thead:b,tfoot:b,td:E,th:E,'*':f.createElement('div')},w=/complete|loaded|interactive/,T=/^[\\w-]*$/,S={},C=S.toString,N={},A=f.createElement('div'),D={tabindex:'tabIndex',readonly:'readOnly','for':'htmlFor','class':'className',maxlength:'maxLength',cellspacing:'cellSpacing',cellpadding:'cellPadding',rowspan:'rowSpan',colspan:'colSpan',usemap:'useMap',frameborder:'frameBorder',contenteditable:'contentEditable'},L=Array.isArray||function(t){return t instanceof Array};return N.matches=function(t,e){if(!e||!t||1!==t.nodeType)return!1;var n=t.matches||t.webkitMatchesSelector||t.mozMatchesSelector||t.oMatchesSelector||t.matchesSelector;if(n)return n.call(t,e);var r,i=t.parentNode,o=!i;return o&&(i=A).appendChild(t),r=~N.qsa(i,e).indexOf(t),o&&A.removeChild(t),r},O=function(t){return t.replace(/-+(.)?/g,function(t,e){return e?e.toUpperCase():''})},P=function(t){return a.call(t,function(e,n){return t.indexOf(e)==n})},N.fragment=function(t,n,i){var o,s,a;return d.test(t)&&(o=r(f.createElement(RegExp.$1))),o||(t.replace&&(t=t.replace(m,'<$1></$2>')),n===e&&(n=p.test(t)&&RegExp.$1),n in j||(n='*'),a=j[n],a.innerHTML=''+t,o=r.each(u.call(a.childNodes),function(){a.removeChild(this)})),Z(i)&&(s=r(o),r.each(i,function(t,e){y.indexOf(t)>-1?s[t](e):s.attr(t,e)})),o},N.Z=function(t,e){return new X(t,e)},N.isZ=function(t){return t instanceof N.Z},N.init=function(t,n){var i;if(!t)return N.Z();if('string'==typeof t)if(t=t.trim(),'<'==t[0]&&p.test(t))i=N.fragment(t,RegExp.$1,n),t=null;else{if(n!==e)return r(n).find(t);i=N.qsa(f,t)}else{if(F(t))return r(f).ready(t);if(N.isZ(t))return t;if(L(t))i=q(t);else if(R(t))i=[t],t=null;else if(p.test(t))i=N.fragment(t.trim(),RegExp.$1,n),t=null;else{if(n!==e)return r(n).find(t);i=N.qsa(f,t)}}return N.Z(i,t)},r=function(t,e){return N.init(t,e)},r.extend=function(t){var e,n=u.call(arguments,1);return'boolean'==typeof t&&(e=t,t=n.shift()),n.forEach(function(n){J(t,n,e)}),t},N.qsa=function(t,e){var n,r='#'==e[0],i=!r&&'.'==e[0],o=r||i?e.slice(1):e,s=T.test(o);return t.getElementById&&s&&r?(n=t.getElementById(o))?[n]:[]:1!==t.nodeType&&9!==t.nodeType&&11!==t.nodeType?[]:u.call(s&&!r&&t.getElementsByClassName?i?t.getElementsByClassName(o):t.getElementsByTagName(e):t.querySelectorAll(e))},r.contains=f.documentElement.contains?function(t,e){return t!==e&&t.contains(e)}:function(t,e){for(;e&&(e=e.parentNode);)if(e===t)return!0;return!1},r.type=$,r.isFunction=F,r.isWindow=k,r.isArray=L,r.isPlainObject=Z,r.isEmptyObject=function(t){var e;for(e in t)return!1;return!0},r.isNumeric=function(t){var e=Number(t),n=typeof t;return null!=t&&'boolean'!=n&&('string'!=n||t.length)&&!isNaN(e)&&isFinite(e)||!1},r.inArray=function(t,e,n){return o.indexOf.call(e,t,n)},r.camelCase=O,r.trim=function(t){return null==t?'':String.prototype.trim.call(t)},r.uuid=0,r.support={},r.expr={},r.noop=function(){},r.map=function(t,e){var n,i,o,r=[];if(z(t))for(i=0;i<t.length;i++)n=e(t[i],i),null!=n&&r.push(n);else for(o in t)n=e(t[o],o),null!=n&&r.push(n);return H(r)},r.each=function(t,e){var n,r;if(z(t)){for(n=0;n<t.length;n++)if(e.call(t[n],n,t[n])===!1)return t}else for(r in t)if(e.call(t[r],r,t[r])===!1)return t;return t},r.grep=function(t,e){return a.call(t,e)},t.JSON&&(r.parseJSON=JSON.parse),r.each('Boolean Number String Function Array Date RegExp Object Error'.split(' '),function(t,e){S['[object '+e+']']=e.toLowerCase()}),r.fn={constructor:N.Z,length:0,forEach:o.forEach,reduce:o.reduce,push:o.push,sort:o.sort,splice:o.splice,indexOf:o.indexOf,concat:function(){var t,e,n=[];for(t=0;t<arguments.length;t++)e=arguments[t],n[t]=N.isZ(e)?e.toArray():e;return s.apply(N.isZ(this)?this.toArray():this,n)},map:function(t){return r(r.map(this,function(e,n){return t.call(e,n,e)}))},slice:function(){return r(u.apply(this,arguments))},ready:function(t){return w.test(f.readyState)&&f.body?t(r):f.addEventListener('DOMContentLoaded',function(){t(r)},!1),this},get:function(t){return t===e?u.call(this):this[t>=0?t:t+this.length]},toArray:function(){return this.get()},size:function(){return this.length},remove:function(){return this.each(function(){null!=this.parentNode&&this.parentNode.removeChild(this)})},each:function(t){return o.every.call(this,function(e,n){return t.call(e,n,e)!==!1}),this},filter:function(t){return F(t)?this.not(this.not(t)):r(a.call(this,function(e){return N.matches(e,t)}))},add:function(t,e){return r(P(this.concat(r(t,e))))},is:function(t){return this.length>0&&N.matches(this[0],t)},not:function(t){var n=[];if(F(t)&&t.call!==e)this.each(function(e){t.call(this,e)||n.push(this)});else{var i='string'==typeof t?this.filter(t):z(t)&&F(t.item)?u.call(t):r(t);this.forEach(function(t){i.indexOf(t)<0&&n.push(t)})}return r(n)},has:function(t){return this.filter(function(){return R(t)?r.contains(this,t):r(this).find(t).size()})},eq:function(t){return-1===t?this.slice(t):this.slice(t,+t+1)},first:function(){var t=this[0];return t&&!R(t)?t:r(t)},last:function(){var t=this[this.length-1];return t&&!R(t)?t:r(t)},find:function(t){var e,n=this;return e=t?'object'==typeof t?r(t).filter(function(){var t=this;return o.some.call(n,function(e){return r.contains(e,t)})}):1==this.length?r(N.qsa(this[0],t)):this.map(function(){return N.qsa(this,t)}):r()},closest:function(t,e){var n=[],i='object'==typeof t&&r(t);return this.each(function(r,o){for(;o&&!(i?i.indexOf(o)>=0:N.matches(o,t));)o=o!==e&&!M(o)&&o.parentNode;o&&n.indexOf(o)<0&&n.push(o)}),r(n)},parents:function(t){for(var e=[],n=this;n.length>0;)n=r.map(n,function(t){return(t=t.parentNode)&&!M(t)&&e.indexOf(t)<0?(e.push(t),t):void 0});return W(e,t)},parent:function(t){return W(P(this.pluck('parentNode')),t)},children:function(t){return W(this.map(function(){return U(this)}),t)},contents:function(){return this.map(function(){return this.contentDocument||u.call(this.childNodes)})},siblings:function(t){return W(this.map(function(t,e){return a.call(U(e.parentNode),function(t){return t!==e})}),t)},empty:function(){return this.each(function(){this.innerHTML=''})},pluck:function(t){return r.map(this,function(e){return e[t]})},show:function(){return this.each(function(){'none'==this.style.display&&(this.style.display=''),'none'==getComputedStyle(this,'').getPropertyValue('display')&&(this.style.display=B(this.nodeName))})},replaceWith:function(t){return this.before(t).remove()},wrap:function(t){var e=F(t);if(this[0]&&!e)var n=r(t).get(0),i=n.parentNode||this.length>1;return this.each(function(o){r(this).wrapAll(e?t.call(this,o):i?n.cloneNode(!0):n)})},wrapAll:function(t){if(this[0]){r(this[0]).before(t=r(t));for(var e;(e=t.children()).length;)t=e.first();r(t).append(this)}return this},wrapInner:function(t){var e=F(t);return this.each(function(n){var i=r(this),o=i.contents(),s=e?t.call(this,n):t;o.length?o.wrapAll(s):i.append(s)})},unwrap:function(){return this.parent().each(function(){r(this).replaceWith(r(this).children())}),this},clone:function(){return this.map(function(){return this.cloneNode(!0)})},hide:function(){return this.css('display','none')},toggle:function(t){return this.each(function(){var n=r(this);(t===e?'none'==n.css('display'):t)?n.show():n.hide()})},prev:function(t){return r(this.pluck('previousElementSibling')).filter(t||'*')},next:function(t){return r(this.pluck('nextElementSibling')).filter(t||'*')},html:function(t){return 0 in arguments?this.each(function(e){var n=this.innerHTML;r(this).empty().append(Y(this,t,e,n))}):0 in this?this[0].innerHTML:null},text:function(t){return 0 in arguments?this.each(function(e){var n=Y(this,t,e,this.textContent);this.textContent=null==n?'':''+n}):0 in this?this.pluck('textContent').join(''):null},attr:function(t,r){var i;return'string'!=typeof t||1 in arguments?this.each(function(e){if(1===this.nodeType)if(R(t))for(n in t)G(this,n,t[n]);else G(this,t,Y(this,r,e,this.getAttribute(t)))}):0 in this&&1==this[0].nodeType&&null!=(i=this[0].getAttribute(t))?i:e},removeAttr:function(t){return this.each(function(){1===this.nodeType&&t.split(' ').forEach(function(t){G(this,t)},this)})},prop:function(t,e){return t=D[t]||t,1 in arguments?this.each(function(n){this[t]=Y(this,e,n,this[t])}):this[0]&&this[0][t]},removeProp:function(t){return t=D[t]||t,this.each(function(){delete this[t]})},data:function(t,n){var r='data-'+t.replace(v,'-$1').toLowerCase(),i=1 in arguments?this.attr(r,n):this.attr(r);return null!==i?Q(i):e},val:function(t){return 0 in arguments?(null==t&&(t=''),this.each(function(e){this.value=Y(this,t,e,this.value)})):this[0]&&(this[0].multiple?r(this[0]).find('option').filter(function(){return this.selected}).pluck('value'):this[0].value)},offset:function(e){if(e)return this.each(function(t){var n=r(this),i=Y(this,e,t,n.offset()),o=n.offsetParent().offset(),s={top:i.top-o.top,left:i.left-o.left};'static'==n.css('position')&&(s.position='relative'),n.css(s)});if(!this.length)return null;if(f.documentElement!==this[0]&&!r.contains(f.documentElement,this[0]))return{top:0,left:0};var n=this[0].getBoundingClientRect();return{left:n.left+t.pageXOffset,top:n.top+t.pageYOffset,width:Math.round(n.width),height:Math.round(n.height)}},css:function(t,e){if(arguments.length<2){var i=this[0];if('string'==typeof t){if(!i)return;return i.style[O(t)]||getComputedStyle(i,'').getPropertyValue(t)}if(L(t)){if(!i)return;var o={},s=getComputedStyle(i,'');return r.each(t,function(t,e){o[e]=i.style[O(e)]||s.getPropertyValue(e)}),o}}var a='';if('string'==$(t))e||0===e?a=I(t)+':'+_(t,e):this.each(function(){this.style.removeProperty(I(t))});else for(n in t)t[n]||0===t[n]?a+=I(n)+':'+_(n,t[n])+';':this.each(function(){this.style.removeProperty(I(n))});return this.each(function(){this.style.cssText+=';'+a})},index:function(t){return t?this.indexOf(r(t)[0]):this.parent().children().indexOf(this[0])},hasClass:function(t){return t?o.some.call(this,function(t){return this.test(K(t))},V(t)):!1},addClass:function(t){return t?this.each(function(e){if('className'in this){i=[];var n=K(this),o=Y(this,t,e,n);o.split(/\\s+/g).forEach(function(t){r(this).hasClass(t)||i.push(t)},this),i.length&&K(this,n+(n?' ':'')+i.join(' '))}}):this},removeClass:function(t){return this.each(function(n){if('className'in this){if(t===e)return K(this,'');i=K(this),Y(this,t,n,i).split(/\\s+/g).forEach(function(t){i=i.replace(V(t),' ')}),K(this,i.trim())}})},toggleClass:function(t,n){return t?this.each(function(i){var o=r(this),s=Y(this,t,i,K(this));s.split(/\\s+/g).forEach(function(t){(n===e?!o.hasClass(t):n)?o.addClass(t):o.removeClass(t)})}):this},scrollTop:function(t){if(this.length){var n='scrollTop'in this[0];return t===e?n?this[0].scrollTop:this[0].pageYOffset:this.each(n?function(){this.scrollTop=t}:function(){this.scrollTo(this.scrollX,t)})}},scrollLeft:function(t){if(this.length){var n='scrollLeft'in this[0];return t===e?n?this[0].scrollLeft:this[0].pageXOffset:this.each(n?function(){this.scrollLeft=t}:function(){this.scrollTo(t,this.scrollY)})}},position:function(){if(this.length){var t=this[0],e=this.offsetParent(),n=this.offset(),i=g.test(e[0].nodeName)?{top:0,left:0}:e.offset();return n.top-=parseFloat(r(t).css('margin-top'))||0,n.left-=parseFloat(r(t).css('margin-left'))||0,i.top+=parseFloat(r(e[0]).css('border-top-width'))||0,i.left+=parseFloat(r(e[0]).css('border-left-width'))||0,{top:n.top-i.top,left:n.left-i.left}}},offsetParent:function(){return this.map(function(){for(var t=this.offsetParent||f.body;t&&!g.test(t.nodeName)&&'static'==r(t).css('position');)t=t.offsetParent;return t})}},r.fn.detach=r.fn.remove,['width','height'].forEach(function(t){var n=t.replace(/./,function(t){return t[0].toUpperCase()});r.fn[t]=function(i){var o,s=this[0];return i===e?k(s)?s['inner'+n]:M(s)?s.documentElement['scroll'+n]:(o=this.offset())&&o[t]:this.each(function(e){s=r(this),s.css(t,Y(this,i,e,s[t]()))})}}),x.forEach(function(n,i){var o=i%2;r.fn[n]=function(){var n,a,s=r.map(arguments,function(t){var i=[];return n=$(t),'array'==n?(t.forEach(function(t){return t.nodeType!==e?i.push(t):r.zepto.isZ(t)?i=i.concat(t.get()):void(i=i.concat(N.fragment(t)))}),i):'object'==n||null==t?t:N.fragment(t)}),u=this.length>1;return s.length<1?this:this.each(function(e,n){a=o?n:n.parentNode,n=0==i?n.nextSibling:1==i?n.firstChild:2==i?n:null;var c=r.contains(f.documentElement,a);s.forEach(function(e){if(u)e=e.cloneNode(!0);else if(!a)return r(e).remove();a.insertBefore(e,n),c&&tt(e,function(e){if(!(null==e.nodeName||'SCRIPT'!==e.nodeName.toUpperCase()||e.type&&'text/javascript'!==e.type||e.src)){var n=e.ownerDocument?e.ownerDocument.defaultView:t;n.eval.call(n,e.innerHTML)}})})})},r.fn[o?n+'To':'insert'+(i?'Before':'After')]=function(t){return r(t)[n](this),this}}),N.Z.prototype=X.prototype=r.fn,N.uniq=P,N.deserializeValue=Q,r.zepto=N,r}();return t.Zepto=e,void 0===t.$&&(t.$=e),function(e){function h(t){return t._zid||(t._zid=n++)}function p(t,e,n,r){if(e=d(e),e.ns)var i=m(e.ns);return(a[h(t)]||[]).filter(function(t){return t&&(!e.e||t.e==e.e)&&(!e.ns||i.test(t.ns))&&(!n||h(t.fn)===h(n))&&(!r||t.sel==r)})}function d(t){var e=(''+t).split('.');return{e:e[0],ns:e.slice(1).sort().join(' ')}}function m(t){return new RegExp('(?:^| )'+t.replace(' ',' .* ?')+'(?: |$)')}function g(t,e){return t.del&&!f&&t.e in c||!!e}function v(t){return l[t]||f&&c[t]||t}function y(t,n,i,o,s,u,f){var c=h(t),p=a[c]||(a[c]=[]);n.split(/\\s/).forEach(function(n){if('ready'==n)return e(document).ready(i);var a=d(n);a.fn=i,a.sel=s,a.e in l&&(i=function(t){var n=t.relatedTarget;return!n||n!==this&&!e.contains(this,n)?a.fn.apply(this,arguments):void 0}),a.del=u;var c=u||i;a.proxy=function(e){if(e=T(e),!e.isImmediatePropagationStopped()){e.data=o;var n=c.apply(t,e._args==r?[e]:[e].concat(e._args));return n===!1&&(e.preventDefault(),e.stopPropagation()),n}},a.i=p.length,p.push(a),'addEventListener'in t&&t.addEventListener(v(a.e),a.proxy,g(a,f))})}function x(t,e,n,r,i){var o=h(t);(e||'').split(/\\s/).forEach(function(e){p(t,e,n,r).forEach(function(e){delete a[o][e.i],'removeEventListener'in t&&t.removeEventListener(v(e.e),e.proxy,g(e,i))})})}function T(t,n){return(n||!t.isDefaultPrevented)&&(n||(n=t),e.each(w,function(e,r){var i=n[e];t[e]=function(){return this[r]=b,i&&i.apply(n,arguments)},t[r]=E}),t.timeStamp||(t.timeStamp=Date.now()),(n.defaultPrevented!==r?n.defaultPrevented:'returnValue'in n?n.returnValue===!1:n.getPreventDefault&&n.getPreventDefault())&&(t.isDefaultPrevented=b)),t}function S(t){var e,n={originalEvent:t};for(e in t)j.test(e)||t[e]===r||(n[e]=t[e]);return T(n,t)}var r,n=1,i=Array.prototype.slice,o=e.isFunction,s=function(t){return'string'==typeof t},a={},u={},f='onfocusin'in t,c={focus:'focusin',blur:'focusout'},l={mouseenter:'mouseover',mouseleave:'mouseout'};u.click=u.mousedown=u.mouseup=u.mousemove='MouseEvents',e.event={add:y,remove:x},e.proxy=function(t,n){var r=2 in arguments&&i.call(arguments,2);if(o(t)){var a=function(){return t.apply(n,r?r.concat(i.call(arguments)):arguments)};return a._zid=h(t),a}if(s(n))return r?(r.unshift(t[n],t),e.proxy.apply(null,r)):e.proxy(t[n],t);throw new TypeError('expected function')},e.fn.bind=function(t,e,n){return this.on(t,e,n)},e.fn.unbind=function(t,e){return this.off(t,e)},e.fn.one=function(t,e,n,r){return this.on(t,e,n,r,1)};var b=function(){return!0},E=function(){return!1},j=/^([A-Z]|returnValue$|layer[XY]$|webkitMovement[XY]$)/,w={preventDefault:'isDefaultPrevented',stopImmediatePropagation:'isImmediatePropagationStopped',stopPropagation:'isPropagationStopped'};e.fn.delegate=function(t,e,n){return this.on(e,t,n)},e.fn.undelegate=function(t,e,n){return this.off(e,t,n)},e.fn.live=function(t,n){return e(document.body).delegate(this.selector,t,n),this},e.fn.die=function(t,n){return e(document.body).undelegate(this.selector,t,n),this},e.fn.on=function(t,n,a,u,f){var c,l,h=this;return t&&!s(t)?(e.each(t,function(t,e){h.on(t,n,a,e,f)}),h):(s(n)||o(u)||u===!1||(u=a,a=n,n=r),(u===r||a===!1)&&(u=a,a=r),u===!1&&(u=E),h.each(function(r,o){f&&(c=function(t){return x(o,t.type,u),u.apply(this,arguments)}),n&&(l=function(t){var r,s=e(t.target).closest(n,o).get(0);return s&&s!==o?(r=e.extend(S(t),{currentTarget:s,liveFired:o}),(c||u).apply(s,[r].concat(i.call(arguments,1)))):void 0}),y(o,t,u,a,n,l||c)}))},e.fn.off=function(t,n,i){var a=this;return t&&!s(t)?(e.each(t,function(t,e){a.off(t,n,e)}),a):(s(n)||o(i)||i===!1||(i=n,n=r),i===!1&&(i=E),a.each(function(){x(this,t,i,n)}))},e.fn.trigger=function(t,n){return t=s(t)||e.isPlainObject(t)?e.Event(t):T(t),t._args=n,this.each(function(){t.type in c&&'function'==typeof this[t.type]?this[t.type]():'dispatchEvent'in this?this.dispatchEvent(t):e(this).triggerHandler(t,n)})},e.fn.triggerHandler=function(t,n){var r,i;return this.each(function(o,a){r=S(s(t)?e.Event(t):t),r._args=n,r.target=a,e.each(p(a,t.type||t),function(t,e){return i=e.proxy(r),r.isImmediatePropagationStopped()?!1:void 0})}),i},'focusin focusout focus blur load resize scroll unload click dblclick mousedown mouseup mousemove mouseover mouseout mouseenter mouseleave change select keydown keypress keyup error'.split(' ').forEach(function(t){e.fn[t]=function(e){return 0 in arguments?this.bind(t,e):this.trigger(t)}}),e.Event=function(t,e){s(t)||(e=t,t=e.type);var n=document.createEvent(u[t]||'Events'),r=!0;if(e)for(var i in e)'bubbles'==i?r=!!e[i]:n[i]=e[i];return n.initEvent(t,r,!0),T(n)}}(e),function(e){function p(t,n,r){var i=e.Event(n);return e(t).trigger(i,r),!i.isDefaultPrevented()}function d(t,e,n,i){return t.global?p(e||r,n,i):void 0}function m(t){t.global&&0===e.active++&&d(t,null,'ajaxStart')}function g(t){t.global&&!--e.active&&d(t,null,'ajaxStop')}function v(t,e){var n=e.context;return e.beforeSend.call(n,t,e)===!1||d(e,n,'ajaxBeforeSend',[t,e])===!1?!1:void d(e,n,'ajaxSend',[t,e])}function y(t,e,n,r){var i=n.context,o='success';n.success.call(i,t,o,e),r&&r.resolveWith(i,[t,o,e]),d(n,i,'ajaxSuccess',[e,n,t]),b(o,e,n)}function x(t,e,n,r,i){var o=r.context;r.error.call(o,n,e,t),i&&i.rejectWith(o,[n,e,t]),d(r,o,'ajaxError',[n,r,t||e]),b(e,n,r)}function b(t,e,n){var r=n.context;n.complete.call(r,e,t),d(n,r,'ajaxComplete',[e,n]),g(n)}function E(t,e,n){if(n.dataFilter==j)return t;var r=n.context;return n.dataFilter.call(r,t,e)}function j(){}function w(t){return t&&(t=t.split(';',2)[0]),t&&(t==c?'html':t==f?'json':a.test(t)?'script':u.test(t)&&'xml')||'text'}function T(t,e){return''==e?t:(t+'&'+e).replace(/[&?]{1,2}/,'?')}function S(t){t.processData&&t.data&&'string'!=e.type(t.data)&&(t.data=e.param(t.data,t.traditional)),!t.data||t.type&&'GET'!=t.type.toUpperCase()&&'jsonp'!=t.dataType||(t.url=T(t.url,t.data),t.data=void 0)}function C(t,n,r,i){return e.isFunction(n)&&(i=r,r=n,n=void 0),e.isFunction(r)||(i=r,r=void 0),{url:t,data:n,success:r,dataType:i}}function O(t,n,r,i){var o,s=e.isArray(n),a=e.isPlainObject(n);e.each(n,function(n,u){o=e.type(u),i&&(n=r?i:i+'['+(a||'object'==o||'array'==o?n:'')+']'),!i&&s?t.add(u.name,u.value):'array'==o||!r&&'object'==o?O(t,u,r,n):t.add(n,u)})}var i,o,n=+new Date,r=t.document,s=/<script\\b[^<]*(?:(?!<\\/script>)<[^<]*)*<\\/script>/gi,a=/^(?:text|application)\\/javascript/i,u=/^(?:text|application)\\/xml/i,f='application/json',c='text/html',l=/^\\s*$/,h=r.createElement('a');h.href=t.location.href,e.active=0,e.ajaxJSONP=function(i,o){if(!('type'in i))return e.ajax(i);var c,p,s=i.jsonpCallback,a=(e.isFunction(s)?s():s)||'Zepto'+n++,u=r.createElement('script'),f=t[a],l=function(t){e(u).triggerHandler('error',t||'abort')},h={abort:l};return o&&o.promise(h),e(u).on('load error',function(n,r){clearTimeout(p),e(u).off().remove(),'error'!=n.type&&c?y(c[0],h,i,o):x(null,r||'error',h,i,o),t[a]=f,c&&e.isFunction(f)&&f(c[0]),f=c=void 0}),v(h,i)===!1?(l('abort'),h):(t[a]=function(){c=arguments},u.src=i.url.replace(/\\?(.+)=\\?/,'?$1='+a),r.head.appendChild(u),i.timeout>0&&(p=setTimeout(function(){l('timeout')},i.timeout)),h)},e.ajaxSettings={type:'GET',beforeSend:j,success:j,error:j,complete:j,context:null,global:!0,xhr:function(){return new t.XMLHttpRequest},accepts:{script:'text/javascript, application/javascript, application/x-javascript',json:f,xml:'application/xml, text/xml',html:c,text:'text/plain'},crossDomain:!1,timeout:0,processData:!0,cache:!0,dataFilter:j},e.ajax=function(n){var u,f,s=e.extend({},n||{}),a=e.Deferred&&e.Deferred();for(i in e.ajaxSettings)void 0===s[i]&&(s[i]=e.ajaxSettings[i]);m(s),s.crossDomain||(u=r.createElement('a'),u.href=s.url,u.href=u.href,s.crossDomain=h.protocol+'//'+h.host!=u.protocol+'//'+u.host),s.url||(s.url=t.location.toString()),(f=s.url.indexOf('#'))>-1&&(s.url=s.url.slice(0,f)),S(s);var c=s.dataType,p=/\\?.+=\\?/.test(s.url);if(p&&(c='jsonp'),s.cache!==!1&&(n&&n.cache===!0||'script'!=c&&'jsonp'!=c)||(s.url=T(s.url,'_='+Date.now())),'jsonp'==c)return p||(s.url=T(s.url,s.jsonp?s.jsonp+'=?':s.jsonp===!1?'':'callback=?')),e.ajaxJSONP(s,a);var P,d=s.accepts[c],g={},b=function(t,e){g[t.toLowerCase()]=[t,e]},C=/^([\\w-]+:)\\/\\//.test(s.url)?RegExp.$1:t.location.protocol,N=s.xhr(),O=N.setRequestHeader;if(a&&a.promise(N),s.crossDomain||b('X-Requested-With','XMLHttpRequest'),b('Accept',d||'*/*'),(d=s.mimeType||d)&&(d.indexOf(',')>-1&&(d=d.split(',',2)[0]),N.overrideMimeType&&N.overrideMimeType(d)),(s.contentType||s.contentType!==!1&&s.data&&'GET'!=s.type.toUpperCase())&&b('Content-Type',s.contentType||'application/x-www-form-urlencoded'),s.headers)for(o in s.headers)b(o,s.headers[o]);if(N.setRequestHeader=b,N.onreadystatechange=function(){if(4==N.readyState){N.onreadystatechange=j,clearTimeout(P);var t,n=!1;if(N.status>=200&&N.status<300||304==N.status||0==N.status&&'file:'==C){if(c=c||w(s.mimeType||N.getResponseHeader('content-type')),'arraybuffer'==N.responseType||'blob'==N.responseType)t=N.response;else{t=N.responseText;try{t=E(t,c,s),'script'==c?(1,eval)(t):'xml'==c?t=N.responseXML:'json'==c&&(t=l.test(t)?null:e.parseJSON(t))}catch(r){n=r}if(n)return x(n,'parsererror',N,s,a)}y(t,N,s,a)}else x(N.statusText||null,N.status?'error':'abort',N,s,a)}},v(N,s)===!1)return N.abort(),x(null,'abort',N,s,a),N;var A='async'in s?s.async:!0;if(N.open(s.type,s.url,A,s.username,s.password),s.xhrFields)for(o in s.xhrFields)N[o]=s.xhrFields[o];for(o in g)O.apply(N,g[o]);return s.timeout>0&&(P=setTimeout(function(){N.onreadystatechange=j,N.abort(),x(null,'timeout',N,s,a)},s.timeout)),N.send(s.data?s.data:null),N},e.get=function(){return e.ajax(C.apply(null,arguments))},e.post=function(){var t=C.apply(null,arguments);return t.type='POST',e.ajax(t)},e.getJSON=function(){var t=C.apply(null,arguments);return t.dataType='json',e.ajax(t)},e.fn.load=function(t,n,r){if(!this.length)return this;var a,i=this,o=t.split(/\\s/),u=C(t,n,r),f=u.success;return o.length>1&&(u.url=o[0],a=o[1]),u.success=function(t){i.html(a?e('<div>').html(t.replace(s,'')).find(a):t),f&&f.apply(i,arguments)},e.ajax(u),this};var N=encodeURIComponent;e.param=function(t,n){var r=[];return r.add=function(t,n){e.isFunction(n)&&(n=n()),null==n&&(n=''),this.push(N(t)+'='+N(n))},O(r,t,n),r.join('&').replace(/%20/g,'+')}}(e),function(t){t.fn.serializeArray=function(){var e,n,r=[],i=function(t){return t.forEach?t.forEach(i):void r.push({name:e,value:t})};return this[0]&&t.each(this[0].elements,function(r,o){n=o.type,e=o.name,e&&'fieldset'!=o.nodeName.toLowerCase()&&!o.disabled&&'submit'!=n&&'reset'!=n&&'button'!=n&&'file'!=n&&('radio'!=n&&'checkbox'!=n||o.checked)&&i(t(o).val())}),r},t.fn.serialize=function(){var t=[];return this.serializeArray().forEach(function(e){t.push(encodeURIComponent(e.name)+'='+encodeURIComponent(e.value))}),t.join('&')},t.fn.submit=function(e){if(0 in arguments)this.bind('submit',e);else if(this.length){var n=t.Event('submit');this.eq(0).trigger(n),n.isDefaultPrevented()||this.get(0).submit()}return this}}(e),function(){try{getComputedStyle(void 0)}catch(e){var n=getComputedStyle;t.getComputedStyle=function(t,e){try{return n(t,e)}catch(r){return null}}}}(),e});";
  ptr +="</script>";
  ptr +="<script type=\"text/javascript\">";
  ptr +="var smeterGuage = 0; var lastMode = \"\"; $(function(){setInterval(function(){var getData=$.ajax({url:\"./data\",async:true,success:function(getData){const data = getData.split(\" \");$(\"div#Mode\").html(data[0]);$(\"div#Frequency\").html(data[1]+\"<span class='superscript' id='Tail'></span>\");$(\"span#Tail\").html(data[2]);$(\"div#Volume\").html(data[4]);smeterGuage = data[3];if (data[0] == \"FM\" && lastMode != \"FM\") {lastMode = \"FM\"; $(\"div#setFreq\").html(\"<label for='frequency'>64.00 or 6400 - 108 or 10800 : </label><input type='number' min='64' max='10800' step='.01' name='setFrequency'><input type='submit' value='Set Frequency'></form>\");$(\"div#setBFO\").html('');} else if (data[0] != \"FM\" && lastMode != \"OTHER\") {lastMode = \"OTHER\";$(\"div#setFreq\").html(\"<label for='frequency'>150 - 30000 : </label><input type='number' min='150' max='30000' name='setFrequency'><input type='submit' value='Set Frequency'></form>\");$(\"div#setBFO\").html(\"<label for='bfo'>0 - 999 : </label><input type='number' min='0' max='999' name='setBFO'><input type='submit' value='Set BFO'></form><br><br>\");} else {}}}).responseText;},";
  ptr +=ajaxInterval;
  ptr +=");});";
  ptr +="</script>";
  ptr +="<script type=\"text/javascript\">let opts = {angle: 0,lineWidth: 0.3,radiusScale: 1,pointer: {length: 0.65,strokeWidth: 0.05,color: '#000000'},staticZones: [{strokeStyle: \"#30B32D\", min: 0, max: 1, height: 0.2},{strokeStyle: \"#30B32D\", min: 1, max: 2, height: 0.2},{strokeStyle: \"#30B32D\", min: 2, max: 3, height: 0.3},{strokeStyle: \"#30B32D\", min: 3, max: 4, height: 0.3},{strokeStyle: \"#30B32D\", min: 4, max: 5, height: 0.4},{strokeStyle: \"#30B32D\", min: 5, max: 6, height: 0.4},{strokeStyle: \"#30B32D\", min: 6, max: 7, height: 0.5},{strokeStyle: \"#30B32D\", min: 7, max: 8, height: 0.5},{strokeStyle: \"#30B32D\", min: 8, max: 9, height: 0.6},{strokeStyle: \"#F03E3E\", min: 9, max: 11, height: 0.7},{strokeStyle: \"#F03E3E\", min: 11, max: 13, height: 0.8},{strokeStyle: \"#F03E3E\", min: 13, max: 15, height: 0.9},{strokeStyle: \"#F03E3E\", min: 15, max: 17, height: 1}],staticLabels: {font: \"12px sans-serif\",labels: [0, 1, 2, 3, 4, 5, 6, 7, 8, 9],color: \"#000000\",fractionDigits: 0},limitMax: true,limitMin: true,colorStart: '#6FADCF',colorStop: '#8FC0DA',strokeColor: '#E0E0E0',generateGradient: true,highDpiSupport: true};let target = document.querySelector('#gauge');let gaugeChart = new Gauge(target).setOptions(opts);gaugeChart.maxValue = 17;gaugeChart.setMinValue(0);gaugeChart.animationSpeed = 20;gaugeChart.set(smeterGuage);setInterval(function(){gaugeChart.set(smeterGuage)},";
  ptr +=ajaxInterval;
  ptr +=");";
  ptr +="</script>";
  ptr +="<script type=\"text/javascript\">$(function(){$(\"#setfrequency\").on(\"submit\",function(event){event.preventDefault();var dataSend = $(this).serialize();$.post(\"setfrequency\",dataSend,function(response){$(\"#setfrequency\")[0].reset();});});});</script>";
  ptr +="<script type=\"text/javascript\">$(function(){$(\"#setbfo\").on(\"submit\",function(event){event.preventDefault();var dataSend = $(this).serialize();$.post(\"setfrequency\",dataSend,function(response){$(\"#setbfo\")[0].reset();});});});</script>";
  ptr +="<script type=\"text/javascript\">$(function(){$(\"#setvolume\").on(\"submit\",function(event){event.preventDefault();var dataSend = $(this).serialize();$.post(\"setvolume\",dataSend,function(response){$(\"#setvolume\")[0].reset();});});});</script>";

  ptr +="</div>";
  ptr +="</body>";
  ptr +="</html>";
  return ptr;
}
