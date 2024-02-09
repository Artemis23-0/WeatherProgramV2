#include <M5Core2.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include "EGR425_Phase1_weather_bitmap_images.h"
#include "WiFi.h"
#include <NTPClient.h>
////////////////////////////////////////////////////////////////////
// State
////////////////////////////////////////////////////////////////////

enum Screen { S_WEATHER, S_ZIP_EDIT };
enum Temp { T_Fahrenheit, T_Celsius };
static bool stateChangedThisLoop = false;
static bool zipChangedThisLoop = false;
static bool tempChangedThisLoop = false;

////////////////////////////////////////////////////////////////////
// Variables
////////////////////////////////////////////////////////////////////
// TODO 3: Register for openweather account and get API key
String urlOpenWeather = "https://api.openweathermap.org/data/2.5/weather?";
String apiKey = "0f4c9cbe9b2dc5d1b4445b61a58f2dd9";

// TODO 1: WiFi variables
String wifiNetworkName = "CBU-LANCERS";
String wifiPassword = "LiveY0urPurp0se";

// Time variables
unsigned long lastTime = 0;
unsigned long timerDelay = 5000;  // 5000; 5 minutes (300,000ms) or 5 seconds (5,000ms)
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", -28800);

// LCD variables
int sWidth;
int sHeight;
int sFifthWidth = sWidth / 5;

// Weather/zip variables
String strWeatherIcon;
String strWeatherDesc;
String cityName;
double tempNow;
double tempNowC;
double tempMin;
double tempMinC; 
double tempMax;
double tempMaxC;
String timeOfLastUpdate;



// Screen Variables
static Screen screen = S_WEATHER;
static Temp tempState = T_Fahrenheit;

//#region Buttons
ButtonColors onCol = {BLACK, WHITE, WHITE};
ButtonColors offCol = {TFT_DARKCYAN, BLACK, NODRAW};

//ZipCode Variables of an array consisting of the different numbers
char numbers [10] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};
// array for top row of buttons to change zip code
Button N1U(15, 45, 50, 50, false, "^", offCol, onCol);
Button N2U(75, 45, 50, 50, false, "^", offCol, onCol);
Button N3U(135, 45, 50, 50, false, "^", offCol, onCol);
Button N4U(195, 45, 50, 50, false, "^", offCol, onCol);
Button N5U(255, 45, 50, 50, false, "^", offCol, onCol);

Button topButtons [5] = { N1U, N2U, N3U, N4U, N5U };
// array for bottom row of buttons to change zip code
//Button N1D(5, sHeight - 10, 10, 10, "First Number - Bottom");
Button N1D(15, 175, 50, 50, false, "v", offCol, onCol);
Button N2D(75, 175, 50, 50, false, "v", offCol, onCol);
Button N3D(135, 175, 50, 50, false, "v", offCol, onCol);
Button N4D(195, 175, 50, 50, false, "v", offCol, onCol);
Button N5D(255, 175, 50, 50, false, "v", offCol, onCol);

Button bottomButtons [5] = { N1D, N2D, N3D, N4D, N5D };

char number1 = numbers[9];
char number2 = numbers[2];
char number3 = numbers[5];
char number4 = numbers[0];
char number5 = numbers[4];

//zipCode = number1 + number2 + number3 + number4 + number5;

String zipCode = (String)number1 + (String)number2 + (String)number3 + (String)number4 + (String)number5;

////////////////////////////////////////////////////////////////////
// Method header declarations
////////////////////////////////////////////////////////////////////
String httpGETRequest(const char* serverName);
void drawWeatherImage(String iconId, int resizeMult);
void fetchWeatherDetails();
void drawWeatherDisplay();
void drawZipDisplay();
void down1Pressed(Event& e);
void down2Pressed(Event& e);
void down3Pressed(Event& e);
void down4Pressed(Event& e);
void down5Pressed(Event& e);
void up1Pressed(Event& e);
void up2Pressed(Event& e);
void up3Pressed(Event& e);
void up4Pressed(Event& e);
void up5Pressed(Event& e);
void hideButtons();
String epoch_to_timestamp(long epoch); 

///////////////////////////////////////////////////////////////
// Put your setup code here, to run once
///////////////////////////////////////////////////////////////
void setup() {
    // Initialize the device
    M5.begin();
    M5.Buttons.setFont(FSS18);
    
    N1U.addHandler(up1Pressed, E_TAP);
    N2U.addHandler(up2Pressed, E_TAP);
    N3U.addHandler(up3Pressed, E_TAP);
    N4U.addHandler(up4Pressed, E_TAP);
    N5U.addHandler(up5Pressed, E_TAP);

    N1D.addHandler(down1Pressed, E_TAP);
    N2D.addHandler(down2Pressed, E_TAP);
    N3D.addHandler(down3Pressed, E_TAP);
    N4D.addHandler(down4Pressed, E_TAP);
    N5D.addHandler(down5Pressed, E_TAP);
    
    // Set screen orientation and get height/width 
    sWidth = M5.Lcd.width();
    sHeight = M5.Lcd.height();

    // TODO 2: Connect to WiFi
    WiFi.begin(wifiNetworkName.c_str(), wifiPassword.c_str());
    Serial.printf("Connecting to %s", wifiNetworkName.c_str());
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.print("\n\nConnected to WiFi network with IP address: ");
    Serial.println(WiFi.localIP());
    timeClient.begin();
}

//pressed down buttons
void down1Pressed(Event& e) {
    int idx = number1 - '0';
    if (idx == 0) {
        number1 = numbers[9];
    } else {
        number1 = numbers[idx - 1];
    }
    zipCode = (String)number1 + (String)number2 + (String)number3 + (String)number4 + (String)number5;
    zipChangedThisLoop = true;
}

void down2Pressed(Event& e) {
    int idx = number2 - '0';
    if (idx == 0) {
        number2 = numbers[9];
    } else {
        number2 = numbers[idx - 1];
    }
    zipCode = (String)number1 + (String)number2 + (String)number3 + (String)number4 + (String)number5;
    zipChangedThisLoop = true;
}

void down3Pressed(Event& e) {
    int idx = number3 - '0';
    if (idx == 0) {
        number3 = numbers[9];
    } else {
        number3 = numbers[idx - 1];
    }
    zipCode = (String)number1 + (String)number2 + (String)number3 + (String)number4 + (String)number5;
    zipChangedThisLoop = true;
}

void down4Pressed(Event& e) {
    int idx = number4 - '0';
    if (idx == 0) {
        number4 = numbers[9];
    } else {
        number4 = numbers[idx - 1];
    }
    zipCode = (String)number1 + (String)number2 + (String)number3 + (String)number4 + (String)number5;
    zipChangedThisLoop = true;
}

void down5Pressed(Event& e) {
    int idx = number5 - '0';
    if (idx == 0) {
        number5 = numbers[9];
    } else {
        number5 = numbers[idx - 1];
    }
    zipCode = (String)number1 + (String)number2 + (String)number3 + (String)number4 + (String)number5;
    zipChangedThisLoop = true;
}

//pressed up buttons
void up1Pressed(Event& e) {
    int idx = number1 - '0';
    if (idx == 9) {
        number1 = numbers[0];
    } else {
        number1 = numbers[idx + 1];
    }
    zipCode = (String)number1 + (String)number2 + (String)number3 + (String)number4 + (String)number5;
    zipChangedThisLoop = true;
}

void up2Pressed(Event& e) {
    int idx = number2 - '0';
    if (idx == 9) {
        number2 = numbers[0];
    } else {
        number2 = numbers[idx + 1];
    }
    zipCode = (String)number1 + (String)number2 + (String)number3 + (String)number4 + (String)number5;
    zipChangedThisLoop = true;
}

void up3Pressed(Event& e) {
    int idx = number3 - '0';
    if (idx == 9) {
        number3 = numbers[0];
    } else {
        number3 = numbers[idx + 1];
    }
    zipCode = (String)number1 + (String)number2 + (String)number3 + (String)number4 + (String)number5;
    zipChangedThisLoop = true;
}

void up4Pressed(Event& e) {
    int idx = number4 - '0';
    if (idx == 9) {
        number4 = numbers[0];
    } else {
        number4 = numbers[idx + 1];
    }
    zipCode = (String)number1 + (String)number2 + (String)number3 + (String)number4 + (String)number5;
    zipChangedThisLoop = true;
}


void up5Pressed(Event& e) {
    int idx = number5 - '0';
    if (idx == 9) {
        number5 = numbers[0];
    } else {
        number5 = numbers[idx + 1];
    }
    zipCode = (String)number1 + (String)number2 + (String)number3 + (String)number4 + (String)number5;
    zipChangedThisLoop = true;
}
//#endregion
///////////////////////////////////////////////////////////////
// Put your main code here, to run repeatedly
///////////////////////////////////////////////////////////////
void loop() {
    // Update states
    M5.update();
    timeClient.update();

    if (M5.BtnB.wasPressed()) {
        if (screen == S_WEATHER) {
            screen = S_ZIP_EDIT;
        } else {
            screen = S_WEATHER;
        }
        stateChangedThisLoop = true;
        lastTime = millis();
    }

    if(M5.BtnA.wasPressed()) {
        if (tempState == T_Fahrenheit) {
            tempState = T_Celsius;
        } else {
            tempState = T_Fahrenheit;
        }
        tempChangedThisLoop = true;
        lastTime = millis();
    }

    // Only execute every so often
    if ((millis() - lastTime) > timerDelay) {
        if (WiFi.status() == WL_CONNECTED) {
            if (screen == S_WEATHER) {
                fetchWeatherDetails();
                drawWeatherDisplay();
            } 
        } else {
            Serial.println("WiFi Disconnected");
        }
        // Update the last time to NOW
        lastTime = millis();
    }
    
    if (stateChangedThisLoop) {
        if (screen == S_WEATHER) {
            fetchWeatherDetails();
            drawWeatherDisplay();
        } else {
            drawZipDisplay();
        } 
    }

    if (zipChangedThisLoop && screen == S_ZIP_EDIT) {
        drawZipDisplay();
    }

    if (tempChangedThisLoop && screen == S_WEATHER) {
        drawWeatherDisplay();
    }

    tempChangedThisLoop = false;
    zipChangedThisLoop = false;
    stateChangedThisLoop = false;
}

String epoch_to_timestamp(long epoch) {
    unsigned long hours = (epoch % 86400L) / 3600;
    bool isPM = false;
    if (hours >= 12) {
        hours -= 12;
        isPM = true;
    }
    if (hours == 0) {
        hours = 12;
    }
    String hoursStr = hours < 10 ? "0" + String(hours) : String(hours);
    unsigned long minutes = (epoch % 3600) / 60;
    String minuteStr = minutes < 10 ? "0" + String(minutes) : String(minutes);

    unsigned long seconds = epoch % 60;
    String secondStr = seconds < 10 ? "0" + String(seconds) : String(seconds);
    String amPmStr = isPM ? "PM" : "AM";

    return hoursStr + ":" + minuteStr + ":" + secondStr + " " + amPmStr;
}

void hideButtons() {
    N1U.hide(); 
    N2U.hide();
    N3U.hide(); 
    N4U.hide();
    N5U.hide(); 
    
    N1D.hide();
    N2D.hide(); 
    N3D.hide();
    N4D.hide(); 
    N5D.hide();
    M5.Lcd.fillScreen(TFT_BLACK);
}

/////////////////////////////////////////////////////////////////
// Update the display based on the zip variables defined
// at the top of the screen.
/////////////////////////////////////////////////////////////////
void drawZipDisplay() {
    //////////////////////////////////////////////////////////////////
    // Draw background - neutral tones
    //////////////////////////////////////////////////////////////////
    uint16_t primaryTextColor = TFT_BLACK;
    M5.Lcd.fillScreen(TFT_LIGHTGREY);

    //////////////////////////////////////////////////////////////////
    // Draw the title
    //////////////////////////////////////////////////////////////////
    int pad = 10;
    M5.Lcd.setCursor(pad, pad);
    M5.Lcd.setTextColor(primaryTextColor);
    M5.Lcd.setTextSize(1);
    M5.Lcd.drawString("Enter Your Zipcode", sWidth / 2, pad + 10);
    M5.Lcd.setTextSize(3);

    //////////////////////////////////////////////////////////////////
    // Draw the Zip Boxes
    //////////////////////////////////////////////////////////////////
    
    //Draw Buttons
    M5.Buttons.draw();
    
    //TODO: Draw the zip numbers

    // Draw boxes for zip numbers
    for (int i = 0; i < zipCode.length(); i++) {
        String zipNum = zipCode.substring(i, i+1);
        M5.Lcd.drawRect(15 + (60 * i), 105, 50, 50, TFT_BLACK);
        M5.Lcd.setCursor(25 + (60 * i), 150);
        M5.Lcd.print(zipNum);
    }

    //TODO: handle changing zip number
}

/////////////////////////////////////////////////////////////////
// This method fetches the weather details from the OpenWeather
// API and saves them into the fields defined above
/////////////////////////////////////////////////////////////////
void fetchWeatherDetails() {
    //////////////////////////////////////////////////////////////////
    // Hardcode the specific city,state,country into the query
    // Examples: https://api.openweathermap.org/data/2.5/weather?zip=92504,usa&units=imperial&appid=YOUR_API_KEY
    //////////////////////////////////////////////////////////////////
    //String serverURL = urlOpenWeather + "q=sacramento,ca,usa&units=imperial&appid=" + apiKey;
    String serverURL = urlOpenWeather + "zip=" + zipCode + ",us&units=imperial&appid=" + apiKey;
    //Serial.println(serverURL); // Debug print
    

    //////////////////////////////////////////////////////////////////
    // Make GET request and store reponse
    //////////////////////////////////////////////////////////////////
    String response = httpGETRequest(serverURL.c_str());
    //Serial.print(response); // Debug print
    
    //////////////////////////////////////////////////////////////////
    // Import ArduinoJSON Library and then use arduinojson.org/v6/assistant to
    // compute the proper capacity (this is a weird library thing) and initialize
    // the json object
    //////////////////////////////////////////////////////////////////
    const size_t jsonCapacity = 768+250;
    DynamicJsonDocument objResponse(jsonCapacity);

    //////////////////////////////////////////////////////////////////
    // Deserialize the JSON document and test if parsing succeeded
    //////////////////////////////////////////////////////////////////
    DeserializationError error = deserializeJson(objResponse, response);
    if (error) {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.f_str());
        return;
    }
    //serializeJsonPretty(objResponse, Serial); // Debug print

    //////////////////////////////////////////////////////////////////
    // Parse Response to get the weather description and icon
    //////////////////////////////////////////////////////////////////
    JsonArray arrWeather = objResponse["weather"];
    JsonObject objWeather0 = arrWeather[0];
    String desc = objWeather0["main"];
    String icon = objWeather0["icon"];
    String city = objResponse["name"];

    // ArduinoJson library will not let us save directly to these
    // variables in the 3 lines above for unknown reason
    strWeatherDesc = desc;
    strWeatherIcon = icon;
    cityName = city;

    // Parse response to get the temperatures
    JsonObject objMain = objResponse["main"];
    tempNow = objMain["temp"];
    tempMin = objMain["temp_min"];
    tempMax = objMain["temp_max"];
    Serial.printf("NOW: %.1f F and %s\tMIN: %.1f F\tMax: %.1f F\n", tempNow, strWeatherDesc, tempMin, tempMax);
    long epoch = timeClient.getEpochTime();
    timeOfLastUpdate = epoch_to_timestamp(epoch);
}

/////////////////////////////////////////////////////////////////
// Update the display based on the weather variables defined
// at the top of the screen.
/////////////////////////////////////////////////////////////////
void drawWeatherDisplay() {
    //////////////////////////////////////////////////////////////////
    // Draw background - light blue if day time and navy blue of night
    //////////////////////////////////////////////////////////////////
    hideButtons();
    Serial.print("after hide buttons");
    uint16_t primaryTextColor;
    if (strWeatherIcon.indexOf("d") >= 0) {
        M5.Lcd.fillScreen(TFT_CYAN);
        primaryTextColor = TFT_BLACK;
    } else {
        M5.Lcd.fillScreen(TFT_NAVY);
        primaryTextColor = TFT_BLACK;
    }
    
    //////////////////////////////////////////////////////////////////
    // Draw the icon on the right side of the screen - the built in 
    // drawBitmap method works, but we cannot scale up the image
    // size well, so we'll call our own method
    //////////////////////////////////////////////////////////////////
    //M5.Lcd.drawBitmap(0, 0, 100, 100, myBitmap, TFT_BLACK);
    drawWeatherImage(strWeatherIcon, 3.25);
    
    //////////////////////////////////////////////////////////////////
    // Draw the temperatures and city name
    //////////////////////////////////////////////////////////////////
    int pad = 10;
    M5.Lcd.setCursor(pad, pad + 20);
    M5.Lcd.setTextColor(TFT_BLUE);
    M5.Lcd.setTextSize(2);
    if (tempState == T_Celsius) {
        double tempMinC = ((tempMin - 32.0) * 5.0)/9.0;
        M5.Lcd.printf("LO:%0.fC\n", tempMinC);
    } else {
        M5.Lcd.printf("LO:%0.fF\n", tempMin);
    }

    M5.Lcd.setCursor(180, pad + 20);
    M5.Lcd.setTextColor(TFT_RED);
    M5.Lcd.setTextSize(2);
    if (tempState == T_Celsius) {
        double tempMaxC = ((tempMax - 32.0) * 5.0)/9.0;
        M5.Lcd.printf("HI:%0.fC\n", tempMaxC);
    } else {
        M5.Lcd.printf("HI:%0.fF\n", tempMax);
    }

    M5.Lcd.setCursor(sWidth / 3, sHeight / 2);
    M5.Lcd.setTextColor(primaryTextColor);
    M5.Lcd.setTextSize(3);

    if (tempState == T_Celsius) {
        double tempNowC = ((tempNow - 32.0) * 5.0)/9.0;
        M5.Lcd.printf("%0.fC\n", tempNowC); 
    } else {
        M5.Lcd.printf("%0.fF\n", tempNow);
    }

    if (cityName.length() > 8) {
        M5.Lcd.setTextSize(2);
    } else {
        M5.Lcd.setTextSize(3); 
    }
    M5.Lcd.setTextColor(primaryTextColor);
    M5.Lcd.drawString(cityName.c_str(), sWidth / 2, M5.Lcd.getCursorY() - 30);
    Serial.print(cityName);

    M5.Lcd.setTextSize(1); 
    M5.Lcd.setTextColor(TFT_BLACK);
    M5.Lcd.drawString(timeOfLastUpdate, sWidth, sHeight);

    M5.Lcd.setCursor(sWidth / 3, sHeight / 2 + 50);
    M5.Lcd.setTextColor(primaryTextColor);
}

/////////////////////////////////////////////////////////////////
// This method takes in a URL and makes a GET request to the
// URL, returning the response. 
/////////////////////////////////////////////////////////////////
String httpGETRequest(const char* serverURL) {
    
    // Initialize client
    HTTPClient http;
    http.begin(serverURL);

    // Send HTTP GET request and obtain response
    int httpResponseCode = http.GET();
    String response = http.getString();

    // Check if got an error
    if (httpResponseCode > 0)
        Serial.printf("HTTP Response code: %d\n", httpResponseCode);
    else {
        Serial.printf("HTTP Response ERROR code: %d\n", httpResponseCode);
        Serial.printf("Server Response: %s\n", response);
    }

    // Free resources and return response
    http.end();
    return response;
}

/////////////////////////////////////////////////////////////////
// This method takes in an image icon string (from API) and a 
// resize multiple and draws the corresponding image (bitmap byte
// arrays found in EGR425_Phase1_weather_bitmap_images.h) to scale (for 
// example, if resizeMult==2, will draw the image as 200x200 instead
// of the native 100x100 pixels) on the right-hand side of the
// screen (centered vertically). 
/////////////////////////////////////////////////////////////////
void drawWeatherImage(String iconId, int resizeMult) {

    // Get the corresponding byte array
    const uint16_t * weatherBitmap = getWeatherBitmap(iconId);

    // Compute offsets so that the image is centered vertically and is
    // right-aligned
    int yOffset = -(resizeMult * imgSqDim - M5.Lcd.height()) / 2;
    // int xOffset = sWidth - (imgSqDim*resizeMult*.8); // Right align (image doesn't take up entire array)
    //int xOffset = (M5.Lcd.width() / 2) - (imgSqDim * resizeMult / 2); // center horizontally
    
    // Iterate through each pixel of the imgSqDim x imgSqDim (100 x 100) array
    for (int y = 0; y < imgSqDim; y++) {
        for (int x = 0; x < imgSqDim; x++) {
            // Compute the linear index in the array and get pixel value
            int pixNum = (y * imgSqDim) + x;
            uint16_t pixel = weatherBitmap[pixNum];

            // If the pixel is black, do NOT draw (treat it as transparent);
            // otherwise, draw the value
            if (pixel != 0) {
                // 16-bit RBG565 values give the high 5 pixels to red, the middle
                // 6 pixels to green and the low 5 pixels to blue as described
                // here: http://www.barth-dev.de/online/rgb565-color-picker/
                byte red = (pixel >> 11) & 0b0000000000011111;
                red = red << 3;
                byte green = (pixel >> 5) & 0b0000000000111111;
                green = green << 2;
                byte blue = pixel & 0b0000000000011111;
                blue = blue << 3;

                // Scale image; for example, if resizeMult == 2, draw a 2x2
                // filled square for each original pixel
                for (int i = 0; i < resizeMult; i++) {
                    for (int j = 0; j < resizeMult; j++) {
                        int xDraw = x * resizeMult + i;
                        int yDraw = y * resizeMult + j + yOffset;
                        M5.Lcd.drawPixel(xDraw, yDraw, M5.Lcd.color565(red, green, blue));
                    }
                }
            }
        }
    }
}
//////////////////////////////////////////////////////////////////////////////////
// For more documentation see the following links:
// https://github.com/m5stack/m5-docs/blob/master/docs/en/api/
// https://docs.m5stack.com/en/api/core2/lcd_api
//////////////////////////////////////////////////////////////////////////////////