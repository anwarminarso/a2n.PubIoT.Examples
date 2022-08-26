
// Generated using a2n Google Device Code Generator v0.1 beta
// Source code ini adalah template dasar untuk device yang terintegrasi dengan Simple IoT Platform, Google Home dan Google Assistant

#include <Arduino.h>
#include <ArduinoJSON.h>
#include <ESPWiFiMqttWrapper.h>
#include <Adafruit_NeoPixel.h>


//Code ini support untuk ESP32 dan ESP8266
//Membutuhkan library ESPWiFiMqttWrapper, ArduinoJSON dan library PubSubClient
//https://www.arduino.cc/reference/en/libraries/espwifimqttwrapper/
//https://www.arduino.cc/reference/en/libraries/arduinojson/
//https://www.arduino.cc/reference/en/libraries/pubsubclient/
ESPWiFiMqttWrapper wrapper;

#define PIN			18		//pin 18
#define NUMPIXELS	3		//Jumlah LED
Adafruit_NeoPixel strip(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);


const char* MQTT_Server		= "iot.a2n.tech"; // server MQTT
const char* MQTT_username	= "YOUR_EMAIL@DOMAIN.COM"; // login user anda pada aplikasi
const char* MQTT_password	= "YOUR_PASSWORD"; // password login anda pada aplikasi

const char* WiFi_SSID		= "YOUR_WIFI_SSID"; // nama wifi anda
const char* WiFi_Password	= "YOUR_WIFI_PASSWORD"; // password wifi anda
const char* WiFi_HostName	= "yourHostName"; // nama host name device ini

const char* MQTT_HEARTBEAT_TOPIC = "/user/XXX/myRGB";			// sesuai path user (digenerate dari webapp)
const char* MQTT_STATE_TOPIC = "/user/XXX/myRGB/state";		// sesuai path user (digenerate dari webapp)
const char* MQTT_COMMAND_TOPIC = "/user/XXX/myRGB/command";	// sesuai path user (digenerate dari webapp)

uint32_t HeartbeatInterval = 60000; // setiap 60 detik
enum colorModes { RGB };

typedef union colorRgb_s {
	uint32_t value; // Spectrum RGB value as a decimal integer.
	struct argb_t	// component-wise representation
	{
		uint8_t b;	// (reversed for intel storage order)
		uint8_t g;
		uint8_t r;
		uint8_t a;
	} argb;
} colorRgb_t;
typedef struct spectrumHsv_s {
	float hue; // hue min 0.0f, max 360
	float saturation; // saturation min 0.0f, max 1.0f
	float value; // value min 0.0f, max 1.0f
} spectrumHsv_t;
typedef struct deviceState_s {
	bool on;
	int brightness;
	colorModes colorMode = colorModes::RGB; // current Color mode
	colorRgb_t colorRGB; // The current color RGB currently being used on the device.
	spectrumHsv_t test;
} deviceState_t;

deviceState_t currentState;

enum commandType { OnOff, BrightnessAbsolute, BrightnessRelative, ColorAbsolute, UNKNOWN };

commandType getCommandType(const char* value) {
	if (strcmp(value, "OnOff") == 0) {
		return commandType::OnOff;
	}
	else if (strcmp(value, "BrightnessAbsolute") == 0) {
		return commandType::BrightnessAbsolute;
	}
	else if (strcmp(value, "BrightnessRelative") == 0) {
		return commandType::BrightnessRelative;
	}
	else if (strcmp(value, "ColorAbsolute") == 0) {
		return commandType::ColorAbsolute;
	}
	return commandType::UNKNOWN;
}

void publishState() {
	DynamicJsonDocument doc(512);
	String message = "";

	doc["on"] = currentState.on;
	doc["brightness"] = currentState.brightness;
	switch (currentState.colorMode)
	{
	case RGB:
		doc["color"]["spectrumRgb"] = currentState.colorRGB.value;
		break;
	default:
		break;
	}

	serializeJson(doc, message);

	wrapper.publish(MQTT_STATE_TOPIC, message.c_str());
}
void OnCommand(JsonDocument& doc) {
	if (doc["command"].isNull())
		return;
	commandType cmdType = getCommandType(doc["command"]);
	JsonVariant data = doc["data"];

	switch (cmdType) {
		case OnOff:
			{
				if (!data["on"].is<bool>())
					return;
				currentState.on = data["on"];
				// currentState.on adalah tipe data boolean, valuenya true atau false
				// value true dapat disebut HIGH, atau value false dapat disebut LOW
				// atau bisa juga value true dapat disebut 1, dan sebaliknya value false dapat disebut 0
				// Contoh set Skalar/GPIO pin 3:
				// pinMode(3, currentState.on);
				/* YOUR CODE HERE */
				strip.clear();
				if (currentState.on) {
					uint8_t val = map(currentState.brightness, 0, 100, 0, 255);
					strip.setBrightness(val);
					for (int i = 0; i < strip.numPixels(); i++)
						strip.setPixelColor(i, currentState.colorRGB.argb.r, currentState.colorRGB.argb.g, currentState.colorRGB.argb.b);
				}
				else {
					strip.setBrightness(0);
				}
				strip.show();
			}
			break;
		case BrightnessAbsolute:
			{
				if (!data["brightness"].isNull() && data["brightness"].is<int>()) {
					currentState.brightness = data["brightness"];
				}
				else
					return;
				currentState.brightness = constrain(currentState.brightness, 0, 100);

				// Eksekusi perintah brightness
				// Contoh penggunaan DAC ESP32 pada pin 25
				// int analogValue = map(currentState.brightness, 0, 100, 0, 255);
				// dacWrite(25, analogValue);
				/* YOUR CODE HERE */
				if (currentState.on) {
					strip.clear();
					uint8_t val = map(currentState.brightness, 0, 100, 0, 255);
					strip.setBrightness(val);
					for (int i = 0; i < strip.numPixels(); i++)
						strip.setPixelColor(i, currentState.colorRGB.argb.r, currentState.colorRGB.argb.g, currentState.colorRGB.argb.b);
					strip.show();
				}
			}
			break;
		case BrightnessRelative:
			{
				if (!data["brightnessRelativePercent"].isNull() && data["brightnessRelativePercent"].is<long>()) {
					int brightnessRelativePercent = data["brightnessRelativePercent"];
					currentState.brightness += brightnessRelativePercent;
				}
				else if (!data["brightnessRelativeWeight"].isNull() && data["brightnessRelativeWeight"].is<long>()) {
					int brightnessRelativeWeight = data["brightnessRelativeWeight"];
					currentState.brightness *= brightnessRelativeWeight;
				}
				else
					return;
				currentState.brightness = constrain(currentState.brightness, 0, 100);

				// Eksekusi perintah brightness
				// Contoh penggunaan DAC ESP32 pada pin 25
				// dacWrite(25, analogValue);
				/* YOUR CODE HERE */

				if (currentState.on) {
					strip.clear();
					uint8_t val = map(currentState.brightness, 0, 100, 0, 255);
					strip.setBrightness(val);
					for (int i = 0; i < strip.numPixels(); i++)
						strip.setPixelColor(i, currentState.colorRGB.argb.r, currentState.colorRGB.argb.g, currentState.colorRGB.argb.b);
					strip.show();
				}
			}
			break;
		case ColorAbsolute:
			{
				if (!data["color"]["spectrumRGB"].isNull() && data["color"]["spectrumRGB"].is<long>()) {
					currentState.colorMode = colorModes::RGB;
					currentState.colorRGB.value = data["color"]["spectrumRGB"];
				}
				else
					return;

				/* YOUR CODE HERE */
				if (currentState.on) {
					strip.clear();
					uint8_t val = map(currentState.brightness, 0, 100, 0, 255);
					strip.setBrightness(val);
					for (int i = 0; i < strip.numPixels(); i++)
						strip.setPixelColor(i, currentState.colorRGB.argb.r, currentState.colorRGB.argb.g, currentState.colorRGB.argb.b);
					strip.show();
				}
			}
			break;
		default:
			return;
	}

	publishState();
}

void initDevice() {
	// seting pin mode, sensor, dll
	// disini
	strip.begin();
	strip.show();            // Turn OFF all pixels ASAP
	strip.setBrightness(0); // Set BRIGHTNESS to about 1/5 (max = 255)
}
void setup() {
	Serial.begin(115200);

	initDevice();

	// enable debugger to Serial
	wrapper.setDebugger(&Serial);
	wrapper.setWiFi(WiFi_HostName, WiFi_SSID, WiFi_Password);
	wrapper.setMqttServer(MQTT_Server, MQTT_username, MQTT_password);
	wrapper.initWiFi();

	// Heartbeat, untuk update device status online
	wrapper.setPublisher(MQTT_HEARTBEAT_TOPIC, HeartbeatInterval, [&] {
		Serial.println("HEARTBEAT");
		return ""; // message kosong
		});
	wrapper.setSubscription(MQTT_COMMAND_TOPIC, [&](const char* message) {
		Serial.print("Message Received : ");
		Serial.print(message);
		Serial.println();

		if (sizeof(message) > 0) {
			DynamicJsonDocument doc(512);
			deserializeJson(doc, message);
			OnCommand(doc);
		}
		});


	wrapper.initMqtt();
}

void loop() {
	wrapper.loop();
}