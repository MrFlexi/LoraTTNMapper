#pragma once

#if (USE_CAMERA)

#include "globals.h"
#include "camera.h"
#include <base64.h>

/***************************************
 *  Board select
 **************************************/
#define T_Camera_PLUS_VERSION

/***************************************
 *  Function
 **************************************/
#define DEFAULT_MEASUR_MILLIS 3000 /* Get sensor time by default (ms)*/

/***************************************
 *  Netcup Server
 **************************************/
String serverName = "api.szaroletta.de";
//String serverPath = "/upload_and_detect";
String serverPath = "/add";
const int serverPort = 5000;

String macAddress = "";
String ipAddress = "";

//  Camera functions
camera_fb_t *capture()
{
    camera_fb_t *fb = NULL;
    esp_err_t res = ESP_OK;
    fb = esp_camera_fb_get();
    return fb;
}

void showCameraImageTFT()
{
    camera_fb_t *fb = NULL;
    fb = esp_camera_fb_get();
    if (!fb || fb->format != PIXFORMAT_JPEG)
    {
        ESP_LOGE(TAG, "Camera capture failed");
        esp_camera_fb_return(fb);
        return;
    }
    else
    {
        ESP_LOGI(TAG, "Camera captured, size: %d",fb->len);
        TJpgDec.drawJpg(0, 0, (const uint8_t *)fb->buf, fb->len);
        esp_camera_fb_return(fb);
        tft.drawString("MrFlexi PlantServ", 20, 10);
    }
}


String sendPhoto()
{
    String getAll;
    String getBody;

    delay(1000);
    Serial.println();
    ESP_LOGI(TAG, "Smile.....");
    camera_fb_t *fb = NULL;
    fb = esp_camera_fb_get();
    //dataBuffer.data.image_buffer = base64::encode(fb->buf,fb->len);;
    if (!fb)
    {
        ESP_LOGE(TAG, "Camera capture failed");
    }

    uint16_t imageLen = fb->len;
    ESP_LOGI(TAG, "ImageSize: %d", imageLen);
    if (imageLen > 100)
    {
        if (WiFi.status() == WL_CONNECTED)
        {
            Serial.println("Connecting to server: " + serverName);

            if (wifiClient.connect(serverName.c_str(), serverPort))
            {
                Serial.println("Connection successful!");
                String head = "--MrFlexi\r\nContent-Disposition: form-data; name=\"image\"; filename=\"image.jpg\"\r\nContent-Type: image/jpeg\r\n\r\n";
                String tail = "\r\n--MrFlexi--\r\n";

                uint16_t extraLen = head.length() + tail.length();
                uint16_t totalLen = imageLen + extraLen;

                wifiClient.println("POST " + serverPath + " HTTP/1.1");
                wifiClient.println("Host: " + serverName);
                wifiClient.println("Content-Length: " + String(totalLen));
                wifiClient.println("Content-Type: multipart/form-data; boundary=MrFlexi");
                wifiClient.println();
                wifiClient.print(head);

                uint8_t *fbBuf = fb->buf;
                size_t fbLen = fb->len;
                ESP_LOGI(TAG, "HTTP POST");
                for (size_t n = 0; n < fbLen; n = n + 1024)
                {
                    if (n + 1024 < fbLen)
                    {
                        wifiClient.write(fbBuf, 1024);
                        fbBuf += 1024;
                    }
                    else if (fbLen % 1024 > 0)
                    {
                        size_t remainder = fbLen % 1024;
                        wifiClient.write(fbBuf, remainder);
                    }
                }
                ESP_LOGI(TAG, "POST completed");
                wifiClient.print(tail);

                int timoutTimer = 1000;
                long startTimer = millis();
                boolean state = false;

                ESP_LOGI(TAG, "GetServerResponse");
                while ((startTimer + timoutTimer) > millis())
                {
                    Serial.print(".");
                    delay(50);
                    while (wifiClient.available())
                    {
                        char c = wifiClient.read();
                        if (c == '\n')
                        {
                            if (getAll.length() == 0)
                            {
                                state = true;
                            }
                            getAll = "";
                        }
                        else if (c != '\r')
                        {
                            getAll += String(c);
                        }
                        if (state == true)
                        {
                            getBody += String(c);
                        }
                        startTimer = millis();
                    }
                    if (getBody.length() > 0)
                    {
                        break;
                    }
                }

                Serial.println((millis() - startTimer));
                ESP_LOGI(TAG, "HTTP Response");
                Serial.println(getBody);
            }
            else
            {
                ESP_LOGE(TAG, "Connection to %s failed", serverName);
            }
        }
        else
        {
            ESP_LOGE(TAG, "No Wifi ");
        }
    }
    else
    {
        ESP_LOGE(TAG, "Image Size is 0");
    }
    esp_camera_fb_return(fb);
    wifiClient.stop();
    ESP_LOGI(TAG, "WifiClient Stop");
};



#if defined(SDCARD_CS_PIN)
#include <SD.h>
#endif
bool setupSDCard()
{
    /*
        T-CameraPlus Board, SD shares the bus with the LCD screen.
        It does not need to be re-initialized after the screen is initialized.
        If the screen is not initialized, the initialization SPI bus needs to be turned on.
    */
    // SPI.begin(TFT_SCLK_PIN, TFT_MISO_PIN, TFT_MOSI_PIN);

#if defined(SDCARD_CS_PIN)
    if (!SD.begin(SDCARD_CS_PIN))
    {
        tft.setTextColor(TFT_RED);
        tft.drawString("SDCard begin failed", tft.width() / 2, tft.height() / 2 - 20);
        tft.setTextColor(TFT_WHITE);
        return false;
    }
    else
    {
        String cardInfo = String(((uint32_t)SD.cardSize() / 1024 / 1024));
        tft.setTextColor(TFT_GREEN);
        tft.drawString("SDcardSize=[" + cardInfo + "]MB", tft.width() / 2, tft.height() / 2 + 92);
        tft.setTextColor(TFT_WHITE);

        Serial.print("SDcardSize=[");
        Serial.print(cardInfo);
        Serial.println("]MB");
    }
#endif
    return true;
}

bool setupCamera()
{
    camera_config_t config;

#if defined(Y2_GPIO_NUM)
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
    config.pixel_format = PIXFORMAT_JPEG;
    // init with high specs to pre-allocate larger buffers
    if (psramFound())
    {
        Serial.printf("Psram found");
        config.frame_size = FRAMESIZE_VGA;
        //config.frame_size = FRAMESIZE_240X240;
        config.jpeg_quality = 10;
        config.fb_count = 2;
    }
    else
    {
        Serial.printf("NO psram found");
        config.frame_size = FRAMESIZE_240X240;;
        config.jpeg_quality = 12;
        config.fb_count = 1;
    }
#endif

    // camera init
    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK)
    {
        Serial.printf("Camera init failed with error 0x%x\n", err);
        return false;
    }

    sensor_t *s = esp_camera_sensor_get();
    // initial sensors are flipped vertically and colors are a bit saturated
    if (s->id.PID == OV3660_PID)
    {
        //s->set_vflip(s, 1);       // flip it back
        //s->set_brightness(s, 1);  // up the blightness just a bit
        //s->set_saturation(s, -2); // lower the saturation
    }
    // drop down frame size for higher initial frame rate
    // s->set_framesize(s, FRAMESIZE_QVGA);

    return true;
}

void setupNetwork()
{

    ipAddress = WiFi.localIP().toString();
#if (HAS_TFT_DISPLAY)
    tft.drawString("ipAddress:", tft.width() / 2, tft.height() / 2 + 50);
    tft.drawString(ipAddress, tft.width() / 2, tft.height() / 2 + 72);
#endif
}

void setupCam()
{
//Create ring buffer
    RingbufHandle_t buf_handle;
    buf_handle = xRingbufferCreate(1028, RINGBUF_TYPE_NOSPLIT);
    if (buf_handle == NULL) {
        printf("Failed to create ring buffer\n");
    }
    //status = setupSDCard();
    //Serial.print("setupSDCard status ");
    //Serial.println(status);

    setupCamera();
    setupNetwork();
    //startCameraServer();
}

#endif