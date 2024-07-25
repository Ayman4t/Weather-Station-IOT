
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "ESP32_MailClient.h"
#include <Adafruit_Sensor.h>

#include <DHT.h>
#include <Adafruit_BMP085.h>
#include <SD.h> 
// Function prototype
#define POWER_PIN 32  // ESP32's pin GPIO32 that provides the power to the rain sensor
#define DO_PIN 13     // ESP32's pin GPIO13 connected to DO pin of the rain sensor

const int sdCardCSPin = 5;
Adafruit_BMP085 bmp;
// Replace with your network credentials
const char* ssid = "Ayman";
const char* password = "123456789";

#define DHTPIN 21   // Digital pin connected to the DHT sensor

#define DHTTYPE    DHT11     // DHT 11

DHT dht(DHTPIN, DHTTYPE);
// To send Emails using Gmail on port 465 (SSL)
#define emailSenderAccount    "ahmedsalem0.1.3.2.4@gmail.com"
#define emailSenderPassword   "flru cgpf qhao cgys"
#define smtpServer            "smtp.gmail.com"
#define smtpServerPort        465
#define emailSubject          "[ALERT] Weather Station"

// Default Recipient Email Address
String inputMessage = "ahmedsalem0.1.3.2.4@gmail.com";
String enableEmailChecked = "checked";
String inputMessage2 = "true";
// Default Thresholds Values
String inputMessage3 = "25.0";
String lastTemperature;
String inputMessage4 = "110000.00";
String lastPressure;

String lastaltitude;
String inputMessage5 = "90.00";
String lastHumidity;


String readDHTTemperature() {
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();
  // Check if any reads failed and exit early (to try again).
  if (isnan(t)) {    
    Serial.println("Failed to read from DHT sensor!");
    return "--";
  }
  else {
    Serial.println(t);
    return String(t);
  }
}
String readRainSensor() {

    int rainValue = digitalRead(DO_PIN);  // Replace DO_PIN with the actual pin connected to the rain sensor
    String rainStatus = (rainValue == HIGH) ? "😄😄😄" : "😭😭😭";
    return rainStatus;
}
float readPressure() {
  if (!bmp.begin()) {
    Serial.println("Failed to read from BMP sensor!");
    return -1.0; // Return a negative value to indicate an error
  } else {
    float pressure = bmp.readPressure();
    Serial.println("Pressure: " + String(pressure) + " Pa");

    // Calculate altitude in meters
    float altitude = bmp.readAltitude();
    Serial.println("Altitude: " + String(altitude) + " meters");

    return pressure;
  }
}

String readDHTHumidity() {
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  if (isnan(h)) {
    Serial.println("Failed to read from DHT sensor!");
    return "--";
  }
  else {
    Serial.println(h);
    return String(h);
  }
}

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML>
<html>

<head>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <link rel="stylesheet" href="https://use.fontawesome.com/releases/v5.7.2/css/all.css"
        integrity="sha384-fnmOCqbTlWIlj8LyTjo7mOUStjsKC4pOpQbqyi7RrhN7udi9RwhKkMHpvLbHG9Sr" crossorigin="anonymous">
    <style>
        html {
            font-family: Arial;
            display: inline-block;
            margin: 0px auto;
            text-align: center;
            background-color: beige;
        }

        form {
            margin: 4px;
            top: 44px;
            position: relative;
            left: 0rem;
        }

        label {
            display: block;
            /* margin-left: 513px; */
            white-space: inherit;
            text-align: left;
            top: 18px;
            position: relative;

            left: 34rem;
        }

        input {

            white-space: inherit;
            text-align: left;
            position: absolute;
        }

        /* misc */
        a {
            text-decoration: none;

        }

        /* header style */
        

        header a {

            font-size: 2.3rem;
            color: aliceblue;
            margin: 0.7rem;
            border-radius: 0.5rem;
            background-color: #4f4e4d;
            display: flex;
            justify-content: center;

        }

        header :hover {

            color: orange;

        }

        p {
            font-size: 3.0rem;
        }

        .units {
            font-size: 1.2rem;
        }

        .unit {
            font-size: 1.2rem;
        }


        .dht-labels {
            font-size: 1.2rem;
            vertical-align: middle;
            padding-bottom: 15px;
        }

        .btn {

            margin: 0.5rem;
            padding: 0.25rem 1.2rem;
            font-size: .8rem;
            border-radius: 0.5rem;
            background-color: white;
            cursor: pointer;
            left: 43rem;
            top: 12rem;
        }

        .btn:hover {
            background-color: orange;
        }

        div {
            position: relative;
            top: 90px;
        }

        hr {
            height: 4px;
            /* border-width: 13px; */
            color: orange;
            background-color: orange;
            position: relative;
            top: 6rem;
        }
    </style>
</head>

<body>
    <header>
        <a href="#">Weather Station</a>
        <a href="#">T024</a>
        
    </header>
    <form action="/get">
        <label>Email Address</label>
        <input type="email" name="email_input" value="%EMAIL_INPUT%" required><br>
        <label>Enable Email Notification</label>
        <input type="checkbox" name="enable_email_input" value="true" %ENABLE_EMAIL%><br>
        <label>Temperature Threshold</label>
        <input type="number" step="0.1" name="threshold_input1" value="%THRESHOLD1%" required><br>
        <label>Pressure Threshold</label>
        <input type="number" step="0.1" name="threshold_input2" value="%THRESHOLD2%" required><br>
        <label>Humidity Threshold</label>
        <input type="number" step="0.1" name="threshold_input3" value="%THRESHOLD3%" required><br>
        <input class="btn" type="submit" value="Submit">
    </form>

    <hr>
    <div>
    <p>
        <i class="fas fa-tint" style="color:#00add6;"></i>
        <span class="dht-labels">Rain</span>
        <span id="rain">%RAINSSENSOR%</span>
    </p>
    <p>
        <i class="fas fa-thermometer-half" style="color:#059e8a;"></i>
        <span class="dht-labels">Temperature</span>
        <span id="temperature">%TEMPERATURE%</span>
        <sup class="units">&deg;C</sup>
    </p>
   <p>
  <i class="fas fa-compress" style="color:#FFA500;"></i>
  <span class="dht-labels">Pressure</span>
  <span id="pressure">%PRESSURE%</span>
  <sup class="unit">Pa</sup>
</p>
<p>
  <i class="fas fa-arrows-alt-v" style="color:#A52A2A;"></i>
  <span class="dht-labels">Altitude</span>
  <span id="altitude">%ALTITUDE%</span>
  <sup class="unit">m</sup>
</p>
    <p>
        <i class="fas fa-tint" style="color:#00add6;"></i>
        <span class="dht-labels">Humidity</span>
        <span id="humidity">%HUMIDITY%</span>
        <sup class="units">%</sup>
    </p>
</div>
</body>
<script>
    setInterval(function () {
        var xhttp = new XMLHttpRequest();
        xhttp.onreadystatechange = function () {
            if (this.readyState == 4 && this.status == 200) {
                document.getElementById("temperature").innerHTML = this.responseText;
            }
        };
        xhttp.open("GET", "/temperature", true);
        xhttp.send();
    }, 10000);

    setInterval(function () {
        var xhttp = new XMLHttpRequest();
        xhttp.onreadystatechange = function () {
            if (this.readyState == 4 && this.status == 200) {
                document.getElementById("humidity").innerHTML = this.responseText;
            }
        };
        xhttp.open("GET", "/humidity", true);
        xhttp.send();
    }, 10000);

    setInterval(function () {
        var xhttp = new XMLHttpRequest();
        xhttp.onreadystatechange = function () {
            if (this.readyState == 4 && this.status == 200) {
                document.getElementById("pressure").innerHTML = this.responseText;
            }
        };
        xhttp.open("GET", "/pressure", true);
        xhttp.send();
    }, 10000);
    setInterval(function () {
    var xhttp = new XMLHttpRequest();
    xhttp.onreadystatechange = function () {
        if (this.readyState == 4 && this.status == 200) {
            document.getElementById("rain").innerHTML = this.responseText;
        }
    };
    xhttp.open("GET", "/rain", true);
    xhttp.send();
}, 10000);
setInterval(function () {
    var xhttp = new XMLHttpRequest();
    xhttp.onreadystatechange = function () {
        if (this.readyState == 4 && this.status == 200) {
            document.getElementById("pressure").innerHTML = this.responseText;
        }
    };
    xhttp.open("GET", "/pressure", true);
    xhttp.send();
}, 10000);

setInterval(function () {
    var xhttp = new XMLHttpRequest();
    xhttp.onreadystatechange = function () {
        if (this.readyState == 4 && this.status == 200) {
            document.getElementById("altitude").innerHTML = this.responseText;
        }
    };
    xhttp.open("GET", "/altitude", true);
    xhttp.send();
}, 10000);

</script>

</html>)rawliteral";

void notFound(AsyncWebServerRequest *request) {
  request->send(404, "text/plain", "Not found");
}

AsyncWebServer server(80);
// Replaces placeholder with DHT values

String processor(const String& var){
  if(var == "TEMPERATURE"){
    return readDHTTemperature();
  }
  else if(var == "HUMIDITY"){
    return readDHTHumidity();
  }
  else if(var == "PRESSURE"){
    return String(readPressure());
  }
  else if(var == "ALTITUDE"){
    return String(bmp.readAltitude());
  }
  else if(var == "EMAIL_INPUT"){
    return inputMessage;
  }
  else if(var == "ENABLE_EMAIL"){
    return enableEmailChecked;
  }
  else if(var == "THRESHOLD1"){
    return inputMessage3;
  }
  else if(var == "THRESHOLD2"){
    return inputMessage4;
  }
  else if(var == "THRESHOLD3"){
    return inputMessage5;
  }
  return String();
}

// Flag variable to keep track if email notification was sent or not
bool emailSent1 = false;
bool emailSent2 = false;
bool emailSent3 = false;
const char* PARAM_INPUT_1 = "email_input";
const char* PARAM_INPUT_2 = "enable_email_input";
const char* PARAM_INPUT_3 = "threshold_input1";
const char* PARAM_INPUT_4 = "threshold_input2";
const char* PARAM_INPUT_5 = "threshold_input3";
// Interval between sensor readings. Learn more about timers: https://RandomNerdTutorials.com/esp32-pir-motion-sensor-interrupts-timers/
unsigned long previousMillis = 0;     
const long interval = 5000;    

// The Email Sending data object contains config and data to send
SMTPData smtpData;

void setup(){
  // Serial port for debugging purposes
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  pinMode(POWER_PIN, OUTPUT);  // configure the power pin pin as an OUTPUT
  pinMode(DO_PIN, INPUT);
  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  if (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("WiFi Failed!");
    return;
  }
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }
  Serial.println();
  Serial.print("ESP IP Address: http://");
  Serial.println(WiFi.localIP());
  dht.begin();
// Initialize SD card
  if (!SD.begin(sdCardCSPin)) {
    Serial.println("Card Mount Failed");
    return;
  }
  Serial.println("SD card initialized");

 

  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html, processor);
  });
  server.on("/temperature", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", readDHTTemperature().c_str());
  });
  server.on("/humidity", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", readDHTHumidity().c_str());
  });
  server.on("/pressure", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", String(readPressure()).c_str());
  });
  server.on("/altitude", HTTP_GET, [](AsyncWebServerRequest *request){
  request->send_P(200, "text/plain", String(bmp.readAltitude()).c_str());
});
  server.on("/rain", HTTP_GET, [](AsyncWebServerRequest *request){
    // Call a function to read the rain sensor value and return it as plain text
    request->send_P(200, "text/plain", readRainSensor().c_str());
});
server.on("/get", HTTP_GET, [] (AsyncWebServerRequest *request) {
    // GET email_input value on <ESP_IP>/get?email_input=<inputMessage>
    if (request->hasParam(PARAM_INPUT_1)) {
      inputMessage = request->getParam(PARAM_INPUT_1)->value();
      // GET enable_email_input value on <ESP_IP>/get?enable_email_input=<inputMessage2>
      if (request->hasParam(PARAM_INPUT_2)) {
        inputMessage2 = request->getParam(PARAM_INPUT_2)->value();
        enableEmailChecked = "checked";
      }
      else {
        inputMessage2 = "false";
        enableEmailChecked = "";
      }
      // GET threshold_input value on <ESP_IP>/get?threshold_input=<inputMessage3>
      if (request->hasParam(PARAM_INPUT_3)) {
        inputMessage3 = request->getParam(PARAM_INPUT_3)->value();
      }
      if (request->hasParam(PARAM_INPUT_4)) {
        inputMessage4 = request->getParam(PARAM_INPUT_4)->value();
      }
      if (request->hasParam(PARAM_INPUT_5)) {
        inputMessage5 = request->getParam(PARAM_INPUT_5)->value();
      }
    }
    else {
      inputMessage = "No message sent";
    }
    Serial.println(inputMessage);
    Serial.println(inputMessage2);
    Serial.println(inputMessage3);
    Serial.println(inputMessage4);
    Serial.println(inputMessage5);
    request->send(200, "text/html", "HTTP GET request sent to your ESP.<br><a href=\"/\">Return to Home Page</a>");
  });
  server.onNotFound(notFound);
  
  // Start server
  server.begin();
}

void saveDataToSDCard(String data) {
  // Open the file for writing in append mode
  File dataFile = SD.open("/weather station data.txt", FILE_APPEND);

  // If the file is available, write to it
  if (dataFile) {
    dataFile.println(data);
    dataFile.close();
    Serial.println("Data appended to SD card");
  } else {
    Serial.println("Error opening data.txt for writing");
  }
}
bool sendEmailNotification(String emailMessage);

void loop(){
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;   
    lastTemperature = readDHTTemperature();
    lastPressure = readPressure();
    lastaltitude = bmp.readAltitude();
    lastHumidity =readDHTHumidity();
    
    float temperature = lastTemperature.toFloat();  
    float pressure = lastPressure.toFloat();  
    float altitude =lastaltitude.toFloat();
    float humidity = lastHumidity.toFloat(); 
    //rain sensor
  digitalWrite(POWER_PIN, HIGH);  // turn the rain sensor's power  ON
  delay(10);                      // wait 10 milliseconds

  int rain_state = digitalRead(DO_PIN);

  digitalWrite(POWER_PIN, LOW);  // turn the rain sensor's power OFF

  //append data to sd card across save data to sd card 
  if (rain_state == HIGH)
     {
        Serial.println("The rain is NOT detected");

      String dataToSave = "temperature :" + lastTemperature +  "\n" + "pressure :"  + lastPressure +  "\n" + "altitude :"  + lastaltitude + "\n" + "humidity :"  + lastHumidity+"\nrain :"+0;
     saveDataToSDCard(dataToSave);
      }
  else
      {
        Serial.println("The rain is detected");

      String dataToSave = "temperature :" + lastTemperature +  "\n" + "pressure :"  + lastPressure + "\n"  + "altitude :"  + lastaltitude + "\n" + "humidity :"  + lastHumidity+"\nrain :"+1;
     saveDataToSDCard(dataToSave);
      }
  
    // Check if temperature is above threshold and if it needs to send the Email alert
    if(temperature > inputMessage3.toFloat() && inputMessage2 == "true" && !emailSent1){
      String emailMessage = String("Temperature above threshold. Current temperature: ") + 
                            String(temperature) + String("C");
      if(sendEmailNotification(emailMessage)) {
        Serial.println(emailMessage);
        emailSent1 = true;
      }
      else {
        Serial.println("Email failed to send");
      }  
      //  digitalWrite(ledPin, HIGH);  
    }
    // Check if temperature is below threshold and if it needs to send the Email alert
    else if((temperature < inputMessage3.toFloat()) && inputMessage2 == "true" && emailSent1) {
      String emailMessage = String("Temperature below threshold. Current temperature: ") + 
                            String(temperature) + String(" C");
      if(sendEmailNotification(emailMessage)) {
        Serial.println(emailMessage);
        emailSent1 = false;
      }
      else {
        Serial.println("Email failed to send");
      }
       
    }
    // Check if temperature is above threshold and if it needs to send the Email alert
     if(pressure > inputMessage4.toFloat() && inputMessage2 == "true" && !emailSent2){
      String emailMessage2 = String("pressure above threshold. Current pressure: ") + 
                            String(pressure) + String("pa");
      if(sendEmailNotification(emailMessage2)) {
        Serial.println(emailMessage2);
        emailSent2 = true;
      }
      else {
        Serial.println("Email failed to send");
      }    
    }
    // Check if temperature is below threshold and if it needs to send the Email alert
    else if((pressure < inputMessage4.toFloat()) && inputMessage2 == "true" && emailSent2) {
      String emailMessage2 = String("pressure below threshold. Current pressure: ") + 
                            String(pressure) + String(" pa");
      if(sendEmailNotification(emailMessage2)) {
        Serial.println(emailMessage2);
        emailSent2 = false;
      }
      else {
        Serial.println("Email failed to send");
      }
    }
    // Check if temperature is above threshold and if it needs to send the Email alert
    if(humidity > inputMessage5.toFloat() && inputMessage2 == "true" && !emailSent3){
      String emailMessage3 = String("humidity above threshold. Current humidity: ") + 
                            String(humidity) + String("%");
      if(sendEmailNotification(emailMessage3)) {
        Serial.println(emailMessage3);
        emailSent3 = true;
      }
      else {
        Serial.println("Email failed to send");
      }    
    }
    // Check if temperature is below threshold and if it needs to send the Email alert
    else if((humidity < inputMessage5.toFloat()) && inputMessage2 == "true" && emailSent3) {
      String emailMessage3 = String("humidity below threshold. Current humidity: ") + 
                            String(humidity) + String("%");
      if(sendEmailNotification(emailMessage3)) {
        Serial.println(emailMessage3);
        emailSent3 = false;
        
      }
      else {
        Serial.println("Email failed to send");
      }
    }
  }
     
  
}
// Callback function to get the Email sending status
void sendCallback(SendStatus msg) {
  // Print the current status
  Serial.println(msg.info());

  // Do something when complete
  if (msg.success()) {
    Serial.println("----------------");
  }
}
bool sendEmailNotification(String emailMessage){
  // Set the SMTP Server Email host, port, account and password
  smtpData.setLogin(smtpServer, smtpServerPort, emailSenderAccount, emailSenderPassword);

  // For library version 1.2.0 and later which STARTTLS protocol was supported,the STARTTLS will be 
  // enabled automatically when port 587 was used, or enable it manually using setSTARTTLS function.
  //smtpData.setSTARTTLS(true);

  // Set the sender name and Email
  smtpData.setSender("ESP32", emailSenderAccount);

  // Set Email priority or importance High, Normal, Low or 1 to 5 (1 is highest)
  smtpData.setPriority("High");

  // Set the subject
  smtpData.setSubject(emailSubject);

  // Set the message with HTML format
  smtpData.setMessage(emailMessage, true);

  // Add recipients
  smtpData.addRecipient(inputMessage);

  smtpData.setSendCallback(sendCallback);

  // Start sending Email, can be set callback function to track the status
  if (!MailClient.sendMail(smtpData)) {
    Serial.println("Error sending Email, " + MailClient.smtpErrorReason());
    return false;
  }
  // Clear all data from Email object to free memory
  smtpData.empty();
  return true;
}

