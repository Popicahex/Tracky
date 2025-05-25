#include "esp_camera.h"
#include "FS.h"                
#include "SD_MMC.h"            
#include "soc/soc.h"           
#include "soc/rtc_cntl_reg.h"  
#include "driver/rtc_io.h"

// Pinout pentru RHYX-M21-45 & OV2640
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22


unsigned long photoCounter = 0;


void initMicroSDCard(){
  Serial.println("Inițializare SD Card...");
  if(!SD_MMC.begin()){
    Serial.println("Eroare la montarea SD Cardului");
    return;
  }
  uint8_t cardType = SD_MMC.cardType();
  if(cardType == CARD_NONE){
    Serial.println("Nu există un card SD conectat");
    return;
  }
  Serial.println("SD Card montat cu succes!");
}

String getPictureFilename(){
  String filename;
  fs::FS &fs = SD_MMC;
  File file;
  do {

    filename = "/foto_" + String(photoCounter++) + ".pgm";
    file = fs.open(filename, FILE_READ); 
    file.close();  
  } while (file);  


  return filename; 
}

void takeSavePhoto(){
  camera_fb_t * fb = esp_camera_fb_get();
  
  if(!fb) {
    Serial.println("Captura camerei a eșuat");
    delay(1000);
    ESP.restart();
  }

  
  String path = getPictureFilename();
  Serial.printf("Fișier PGM: %s\n", path.c_str());

 
  fs::FS &fs = SD_MMC; 
  File file = fs.open(path.c_str(), FILE_WRITE);
  if(!file){
    Serial.println("Eroare la deschiderea fișierului pentru scriere");
  } else {
 
    file.printf("P5\n%d %d\n255\n", fb->width, fb->height);

    file.write(fb->buf, fb->len);
    Serial.printf("Imagine salvată: %s\n", path.c_str());
    file.close();
  }

  esp_camera_fb_return(fb); 
}


void configCamera(){
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_GRAYSCALE; // Format de culoare grayscale (alb-negru)
  


  if(psramFound()){
    config.frame_size = FRAMESIZE_UXGA;  
    config.jpeg_quality = 10; 
    config.fb_count = 1;
  } else {
    config.frame_size = FRAMESIZE_UXGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }

  
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Eroare la inițializarea camerei: 0x%x", err);
    return;
  }
}

void setup() {
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); 
  Serial.begin(115200);
  delay(1000);


  Serial.print("Inițializare cameră...");
  configCamera();
  Serial.println("Camera inițializată cu succes!");

  initMicroSDCard();
}

void loop() {    
  takeSavePhoto();
  delay(3000); 
}
