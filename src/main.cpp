#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <Preferences.h>
#include <LittleFS.h>
#include <Update.h>
#include <time.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>
#include <AnimatedGIF.h>
#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>
#include "web_ui.h"

#ifndef MATRIX_E_PIN
#define MATRIX_E_PIN -1
#endif

static WebServer web(80);
static Preferences prefs;
static WiFiClient mqttNet;
static PubSubClient mqtt(mqttNet);
static AnimatedGIF decoder;
static MatrixPanel_I2S_DMA *matrix = nullptr;
static File gifFile;
static File uploadFile;
static String deviceId, adminPassword, apPassword;
static String ssid, wifiPassword, mqttHost, mqttUser, mqttPassword, timezone;
static uint16_t mqttPort = 1883;
static String mode = "clock", message = "HOLA", selectedGif, activeGif;
static uint8_t brightness = 96;
static int8_t matrixEPin = MATRIX_E_PIN;
static bool gifOpen = false, uploadError = false, otaError = false;
static bool apActive = false;
static String uploadName;
static size_t uploadBytes = 0;
static unsigned long lastDraw = 0, nextFrame = 0, lastMqtt = 0, lastConnect = 0, lastWifiRetry = 0;

static bool validGifName(const String &s) {
  if (s.length() < 5 || s.length() > 36 || !s.endsWith(".gif")) return false;
  for (size_t i = 0; i < s.length(); ++i) {
    char c = s[i];
    if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
          (c >= '0' && c <= '9') || c == '-' || c == '_' || c == '.')) return false;
  }
  return s != "..gif";
}

static String baseTopic() { return "mortymel/" + deviceId; }
static bool authed() {
  if (web.authenticate("admin", adminPassword.c_str())) return true;
  web.requestAuthentication(BASIC_AUTH, "Mortymel Matrix");
  return false;
}
static void jsonReply(int status, const JsonDocument &doc) {
  String body; serializeJson(doc, body);
  web.send(status, "application/json; charset=utf-8", body);
}
static void errorReply(int status, const char *msg) {
  JsonDocument d; d["error"] = msg; jsonReply(status, d);
}
static bool bodyJson(JsonDocument &d) {
  if (web.arg("plain").length() > 2048 || deserializeJson(d, web.arg("plain"))) {
    errorReply(400, "JSON inválido o demasiado largo"); return false;
  }
  return true;
}
static void saveDisplay() {
  prefs.putString("mode", mode);
  prefs.putString("message", message);
  prefs.putString("gif", selectedGif);
  prefs.putUChar("brightness", brightness);
  if (matrix) matrix->setBrightness8(brightness);
  lastDraw = 0;
}

static void publishState() {
  if (!mqtt.connected()) return;
  auto b = baseTopic();
  mqtt.publish((b + "/state/mode").c_str(), mode.c_str(), true);
  mqtt.publish((b + "/state/text").c_str(), message.c_str(), true);
  mqtt.publish((b + "/state/brightness").c_str(), String(brightness).c_str(), true);
  mqtt.publish((b + "/state/status").c_str(), matrix ? "ready" : "no_panel_profile", true);
}
static void discovery(const char *kind, const char *key, JsonDocument &d) {
  d["unique_id"] = deviceId + "_" + key;
  d["name"] = key;
  d["device"]["identifiers"][0] = deviceId;
  d["device"]["name"] = "Mortymel Matrix " + deviceId;
  d["availability_topic"] = baseTopic() + "/availability";
  String out; serializeJson(d, out);
  mqtt.publish(("homeassistant/" + String(kind) + "/" + deviceId + "/" + key + "/config").c_str(), out.c_str(), true);
}
static void publishDiscovery() {
  auto b = baseTopic();
  {
    JsonDocument d; d["command_topic"] = b + "/set/mode";
    d["state_topic"] = b + "/state/mode";
    d["options"][0] = "clock"; d["options"][1] = "text"; d["options"][2] = "gif";
    discovery("select", "Escena", d);
  }
  {
    JsonDocument d; d["command_topic"] = b + "/set/text";
    d["state_topic"] = b + "/state/text"; d["max"] = 80;
    discovery("text", "Mensaje", d);
  }
  {
    JsonDocument d; d["command_topic"] = b + "/set/brightness";
    d["state_topic"] = b + "/state/brightness";
    d["min"] = 1; d["max"] = 255;
    discovery("number", "Brillo", d);
  }
  {
    JsonDocument d; d["state_topic"] = b + "/state/status";
    discovery("sensor", "Estado", d);
  }
}
static void onMqtt(char *topic, byte *payload, unsigned int len) {
  if (len > 80) return;
  String cmd; cmd.reserve(len);
  for (unsigned int i = 0; i < len; ++i) cmd += char(payload[i]);
  auto b = baseTopic();
  if (String(topic) == b + "/set/mode") {
    if (cmd != "clock" && cmd != "text" && cmd != "gif") return;
    mode = cmd;
  } else if (String(topic) == b + "/set/text") {
    message = cmd;
  } else if (String(topic) == b + "/set/brightness") {
    int value = cmd.toInt();
    if (value < 1 || value > 255) return;
    brightness = value;
  } else return;
  saveDisplay(); publishState();
}
static void maintainMqtt() {
  if (!mqttHost.length() || WiFi.status() != WL_CONNECTED) { if (mqtt.connected()) mqtt.disconnect(); return; }
  if (!mqtt.connected() && millis() - lastConnect > 5000) {
    lastConnect = millis();
    mqtt.setServer(mqttHost.c_str(), mqttPort);
    auto b = baseTopic();
    bool ok = mqtt.connect(deviceId.c_str(), mqttUser.c_str(), mqttPassword.c_str(),
                           (b + "/availability").c_str(), 0, true, "offline");
    if (ok) {
      mqtt.publish((b + "/availability").c_str(), "online", true);
      mqtt.subscribe((b + "/set/#").c_str());
      publishDiscovery(); publishState();
    }
  }
  if (mqtt.connected()) {
    mqtt.loop();
    if (millis() - lastMqtt > 30000) { lastMqtt = millis(); publishState(); }
  }
}

static void *gifOpenFile(const char *name, int32_t *size) {
  gifFile = LittleFS.open(name, "r");
  if (!gifFile) return nullptr;
  *size = gifFile.size(); return &gifFile;
}
static void gifCloseFile(void *handle) { static_cast<File *>(handle)->close(); }
static int32_t gifReadFile(GIFFILE *file, uint8_t *buf, int32_t len) {
  File *f = static_cast<File *>(file->fHandle);
  int32_t count = f->read(buf, len);
  file->iPos = f->position(); return count;
}
static int32_t gifSeekFile(GIFFILE *file, int32_t pos) {
  File *f = static_cast<File *>(file->fHandle);
  f->seek(pos); file->iPos = f->position(); return file->iPos;
}
static void drawGifLine(GIFDRAW *line) {
  if (!matrix) return;
  int y = line->iY + line->y;
  if (y < 0 || y >= 64) return;
  if (line->ucDisposalMethod == 2) {
    for (int i = 0; i < line->iWidth; ++i)
      if (line->pPixels[i] == line->ucTransparent) line->pPixels[i] = line->ucBackground;
    line->ucHasTransparency = 0;
  }
  for (int i = 0; i < line->iWidth; ++i) {
    int x = line->iX + i;
    if (x < 0 || x >= 64) continue;
    uint8_t pixel = line->pPixels[i];
    if (line->ucHasTransparency && pixel == line->ucTransparent) continue;
    matrix->drawPixel(x, y, line->pPalette[pixel]);
  }
}
static void stopGif() { if (gifOpen) decoder.close(); gifOpen = false; activeGif = ""; }
static void drawClockOrText() {
  if (!matrix) return;
  matrix->fillScreen(0);
  matrix->setTextWrap(false);
  matrix->setTextColor(matrix->color565(124, 250, 180));
  matrix->setTextSize(1);
  if (mode == "clock") {
    struct tm t;
    char clockText[12] = "--:--";
    if (getLocalTime(&t, 10)) strftime(clockText, sizeof(clockText), "%H:%M", &t);
    matrix->setCursor(17, 26); matrix->print(clockText);
    matrix->setTextColor(matrix->color565(255, 194, 112));
    matrix->setCursor(5, 47); matrix->print("MORTYMEL");
  } else {
    for (int i = 0; i < 4; ++i) {
      int start = i * 10;
      if (start >= int(message.length())) break;
      matrix->setCursor(2, 12 + i * 10);
      matrix->print(message.substring(start, start + 10));
    }
  }
}
static void maintainDisplay() {
  if (!matrix) return;
  if (mode != "gif" || selectedGif.isEmpty() || !LittleFS.exists("/" + selectedGif)) {
    stopGif();
    if (!lastDraw || millis() - lastDraw >= 1000) { lastDraw = millis(); drawClockOrText(); }
    return;
  }
  if (!gifOpen || activeGif != selectedGif) {
    stopGif(); matrix->fillScreen(0);
    activeGif = selectedGif;
    gifOpen = decoder.open(("/" + activeGif).c_str(), gifOpenFile, gifCloseFile,
                           gifReadFile, gifSeekFile, drawGifLine);
    nextFrame = millis();
  }
  if (gifOpen && int32_t(millis() - nextFrame) >= 0) {
    int delayMs = 50;
    if (!decoder.playFrame(false, &delayMs)) { decoder.reset(); matrix->fillScreen(0); }
    nextFrame = millis() + constrain(delayMs, 20, 1000);
  }
}

static void registerRoutes() {
  web.on("/", HTTP_GET, [] { if (authed()) web.send_P(200, "text/html; charset=utf-8", WEB_UI); });
  web.on("/api/status", HTTP_GET, [] {
    if (!authed()) return;
    JsonDocument d;
    d["device"] = deviceId; d["wifi"] = WiFi.status() == WL_CONNECTED;
    d["mqtt"] = mqtt.connected(); d["matrix"] = matrix != nullptr;
    d["mode"] = mode; d["message"] = message; d["gif"] = selectedGif;
    d["brightness"] = brightness; d["ssid"] = ssid; d["timezone"] = timezone;
    d["mqtt_host"] = mqttHost; d["mqtt_port"] = mqttPort; d["mqtt_user"] = mqttUser;
    d["e_pin"] = matrixEPin;
    jsonReply(200, d);
  });
  web.on("/api/hardware", HTTP_POST, [] {
    if (!authed()) return;
    JsonDocument d; if (!bodyJson(d)) return;
    if (!d["e_pin"].is<int>()) { errorReply(400, "GPIO E inválido"); return; }
    int pin = d["e_pin"].as<int>();
    if (pin != -1 && (pin < 0 || pin > 48 || (pin >= 26 && pin <= 32))) {
      errorReply(400, "GPIO E fuera de rango o reservado para flash/PSRAM"); return;
    }
    matrixEPin = pin;
    prefs.putChar("e_pin", matrixEPin);
    JsonDocument ok; ok["ok"] = true; ok["restart"] = true; jsonReply(200, ok);
    delay(150); ESP.restart();
  });
  web.on("/api/display", HTTP_POST, [] {
    if (!authed()) return;
    JsonDocument d; if (!bodyJson(d)) return;
    String m = d["mode"] | ""; String g = d["gif"] | ""; String text = d["message"] | "";
    int level = d["brightness"] | 0;
    if ((m != "clock" && m != "text" && m != "gif") || level < 1 || level > 255 ||
        text.length() > 80 || (g.length() && (!validGifName(g) || !LittleFS.exists("/" + g)))) {
      errorReply(400, "Escena inválida"); return;
    }
    mode = m; message = text; selectedGif = g; brightness = level;
    saveDisplay(); publishState(); JsonDocument ok; ok["ok"] = true; jsonReply(200, ok);
  });
  web.on("/api/media", HTTP_GET, [] {
    if (!authed()) return;
    JsonDocument d; JsonArray a = d["files"].to<JsonArray>();
    File root = LittleFS.open("/");
    for (File f = root.openNextFile(); f; f = root.openNextFile()) {
      String name = String(f.name()); name.replace("/", "");
      if (validGifName(name)) { JsonObject o = a.add<JsonObject>(); o["name"] = name; o["bytes"] = f.size(); }
      f.close();
    }
    root.close(); jsonReply(200, d);
  });
  web.on("/api/media", HTTP_POST, [] {
    if (!authed()) return;
    if (uploadFile) uploadFile.close();
    if (uploadError || !uploadName.length() || !LittleFS.exists("/.upload")) {
      LittleFS.remove("/.upload"); errorReply(400, "GIF inválido o espacio insuficiente"); return;
    }
    File f = LittleFS.open("/.upload", "r"); uint8_t h[10]; bool header = f.read(h, 10) == 10;
    f.close();
    int w = h[6] | (h[7] << 8), ht = h[8] | (h[9] << 8);
    if (!header || !(memcmp(h, "GIF87a", 6) == 0 || memcmp(h, "GIF89a", 6) == 0) ||
        w < 1 || ht < 1 || w > 64 || ht > 64) {
      LittleFS.remove("/.upload"); errorReply(400, "Se requiere GIF de hasta 64 × 64"); return;
    }
    String path = "/" + uploadName;
    stopGif();
    if (LittleFS.exists(path)) LittleFS.remove(path);
    if (!LittleFS.rename("/.upload", path)) { errorReply(500, "No se pudo guardar"); return; }
    JsonDocument ok; ok["ok"] = true; jsonReply(200, ok);
  }, [] {
    if (!web.authenticate("admin", adminPassword.c_str())) return;
    HTTPUpload &u = web.upload();
    if (u.status == UPLOAD_FILE_START) {
      uploadError = false; uploadBytes = 0; uploadName = u.filename; stopGif();
      if (!validGifName(uploadName)) { uploadError = true; return; }
      LittleFS.remove("/.upload"); uploadFile = LittleFS.open("/.upload", "w");
      if (!uploadFile) uploadError = true;
    } else if (u.status == UPLOAD_FILE_WRITE && !uploadError) {
      uploadBytes += u.currentSize;
      if (uploadBytes > 400000 || uploadFile.write(u.buf, u.currentSize) != u.currentSize) uploadError = true;
    } else if (u.status == UPLOAD_FILE_END || u.status == UPLOAD_FILE_ABORTED) {
      if (uploadFile) uploadFile.close();
      if (u.status == UPLOAD_FILE_ABORTED) uploadError = true;
    }
  });
  web.on("/api/network", HTTP_POST, [] {
    if (!authed()) return; JsonDocument d; if (!bodyJson(d)) return;
    String next = d["ssid"] | ""; String pass = d["password"] | "";
    if (!next.length() || next.length() > 32 || pass.length() > 64) { errorReply(400, "Red inválida"); return; }
    ssid = next; if (pass.length()) wifiPassword = pass;
    prefs.putString("ssid", ssid); prefs.putString("wifi_pass", wifiPassword);
    JsonDocument ok; ok["ok"] = true; jsonReply(200, ok);
    WiFi.disconnect(); WiFi.begin(ssid.c_str(), wifiPassword.c_str());
  });
  web.on("/api/time", HTTP_POST, [] {
    if (!authed()) return; JsonDocument d; if (!bodyJson(d)) return;
    String next = d["timezone"] | "";
    if (!next.length() || next.length() > 64 || next.indexOf('\n') >= 0) {
      errorReply(400, "Zona horaria inválida"); return;
    }
    timezone = next; prefs.putString("tz", timezone);
    setenv("TZ", timezone.c_str(), 1); tzset();
    JsonDocument ok; ok["ok"] = true; jsonReply(200, ok);
  });
  web.on("/api/mqtt", HTTP_POST, [] {
    if (!authed()) return; JsonDocument d; if (!bodyJson(d)) return;
    String host = d["host"] | ""; String user = d["user"] | "";
    String pass = d["password"] | ""; int port = d["port"] | 0;
    if (host.length() > 128 || user.length() > 80 || pass.length() > 128 || port < 1 || port > 65535) {
      errorReply(400, "Ajustes MQTT inválidos"); return;
    }
    mqtt.disconnect(); mqttHost = host; mqttPort = port; mqttUser = user;
    if (pass.length()) mqttPassword = pass;
    prefs.putString("mqtt_host", mqttHost); prefs.putUShort("mqtt_port", mqttPort);
    prefs.putString("mqtt_user", mqttUser); prefs.putString("mqtt_pass", mqttPassword);
    lastConnect = millis() - 5001;
    JsonDocument ok; ok["ok"] = true; jsonReply(200, ok);
  });
  web.on("/api/admin", HTTP_POST, [] {
    if (!authed()) return; JsonDocument d; if (!bodyJson(d)) return;
    String pass = d["password"] | "";
    if (pass.length() < 12 || pass.length() > 80) { errorReply(400, "La contraseña debe tener 12 a 80 caracteres"); return; }
    prefs.putString("admin", pass); adminPassword = pass;
    JsonDocument ok; ok["ok"] = true; jsonReply(200, ok);
  });
  web.on("/api/update", HTTP_POST, [] {
    if (!authed()) return;
    if (otaError || Update.hasError() || !Update.isFinished()) { errorReply(400, "Actualización fallida"); return; }
    JsonDocument ok; ok["ok"] = true; jsonReply(200, ok);
    delay(150); ESP.restart();
  }, [] {
    if (!web.authenticate("admin", adminPassword.c_str())) return;
    HTTPUpload &u = web.upload();
    if (u.status == UPLOAD_FILE_START) {
      otaError = !u.filename.endsWith(".bin") || !Update.begin(UPDATE_SIZE_UNKNOWN);
    } else if (u.status == UPLOAD_FILE_WRITE && !otaError) {
      if (Update.write(u.buf, u.currentSize) != u.currentSize) otaError = true;
    } else if (u.status == UPLOAD_FILE_END && !otaError) {
      if (!Update.end(true)) otaError = true;
    } else if (u.status == UPLOAD_FILE_ABORTED) { Update.abort(); otaError = true; }
  });
  web.onNotFound([] {
    if (!authed()) return;
    String uri = web.uri();
    if (web.method() == HTTP_GET && uri.startsWith("/api/media/")) {
      String name = uri.substring(11);
      if (validGifName(name) && LittleFS.exists("/" + name)) {
        File f = LittleFS.open("/" + name, "r"); web.streamFile(f, "image/gif"); f.close(); return;
      }
    }
    if (web.method() == HTTP_DELETE && uri.startsWith("/api/media/")) {
      String name = uri.substring(11);
      if (validGifName(name) && LittleFS.exists("/" + name)) {
        stopGif(); LittleFS.remove("/" + name);
        if (selectedGif == name) { selectedGif = ""; mode = "clock"; saveDisplay(); publishState(); }
        JsonDocument ok; ok["ok"] = true; jsonReply(200, ok); return;
      }
    }
    errorReply(404, "No encontrado");
  });
}

void setup() {
  Serial.begin(115200);
  Serial0.begin(115200);
  prefs.begin("matrix", false);
  uint64_t id = ESP.getEfuseMac();
  // Preserve the identifier format used by existing MQTT Discovery entries.
  char suffix[13]; snprintf(suffix, sizeof(suffix), "%06X", uint32_t(id));
  deviceId = "matrix_" + String(suffix);
  adminPassword = prefs.getString("admin", "");
  apPassword = prefs.getString("ap_pass", "");
  if (!adminPassword.length()) {
    adminPassword = "M-" + String(uint32_t(esp_random()), HEX) + String(uint32_t(esp_random()), HEX);
    prefs.putString("admin", adminPassword);
  }
  if (!apPassword.length()) {
    apPassword = "A-" + String(uint32_t(esp_random()), HEX) + String(uint32_t(esp_random()), HEX);
    prefs.putString("ap_pass", apPassword);
  }
  ssid = prefs.getString("ssid", ""); wifiPassword = prefs.getString("wifi_pass", "");
  mqttHost = prefs.getString("mqtt_host", ""); mqttPort = prefs.getUShort("mqtt_port", 1883);
  mqttUser = prefs.getString("mqtt_user", ""); mqttPassword = prefs.getString("mqtt_pass", "");
  mode = prefs.getString("mode", "clock"); message = prefs.getString("message", "HOLA");
  selectedGif = prefs.getString("gif", ""); brightness = prefs.getUChar("brightness", 96);
  timezone = prefs.getString("tz", "UTC0");
  matrixEPin = prefs.getChar("e_pin", MATRIX_E_PIN);
  // Format only on the first boot. An unexpected mount failure later must not
  // silently destroy the user's saved GIF library.
  bool freshFileSystem = !prefs.getBool("fs_init", false);
  bool fsReady = LittleFS.begin(freshFileSystem);
  if (fsReady && freshFileSystem) prefs.putBool("fs_init", true);
  if (!fsReady) { Serial.println("ERROR LittleFS: no se borraron los GIF"); Serial0.println("ERROR LittleFS: no se borraron los GIF"); }
  WiFi.mode(WIFI_AP_STA);
  if (ssid.length()) {
    WiFi.begin(ssid.c_str(), wifiPassword.c_str());
    for (int i = 0; i < 40 && WiFi.status() != WL_CONNECTED; ++i) delay(200);
  }
  if (WiFi.status() != WL_CONNECTED) {
    apActive = WiFi.softAP(("Mortymel-" + String(suffix)).c_str(), apPassword.c_str());
    Serial.printf("AP: Mortymel-%s  clave: %s  web: http://192.168.4.1/\n", suffix, apPassword.c_str());
    Serial0.printf("AP: Mortymel-%s  clave: %s  web: http://192.168.4.1/\n", suffix, apPassword.c_str());
  } else {
    Serial.printf("Web: http://%s/\n", WiFi.localIP().toString().c_str());
    Serial0.printf("Web: http://%s/\n", WiFi.localIP().toString().c_str());
  }
  Serial.printf("Usuario web: admin  clave inicial/actual: %s\n", adminPassword.c_str());
  Serial0.printf("Usuario web: admin  clave inicial/actual: %s\n", adminPassword.c_str());
  configTime(0, 0, "pool.ntp.org");
  setenv("TZ", timezone.c_str(), 1); tzset();
  mqtt.setCallback(onMqtt); mqtt.setBufferSize(1024);
  if (matrixEPin >= 0) {
    HUB75_I2S_CFG cfg(64, 64, 1);
    cfg.gpio.e = matrixEPin;
    matrix = new MatrixPanel_I2S_DMA(cfg);
    if (!matrix->begin()) { delete matrix; matrix = nullptr; Serial.println("Panel sin iniciar"); }
    else { matrix->setBrightness8(brightness); decoder.begin(GIF_PALETTE_RGB565_LE); }
  } else Serial.println("Panel desactivado: configura GPIO E desde Sistema.");
  registerRoutes(); web.begin();
}

void loop() {
  web.handleClient(); maintainMqtt(); maintainDisplay();
  if (apActive && WiFi.status() == WL_CONNECTED) {
    WiFi.softAPdisconnect(true); apActive = false;
  }
  if (ssid.length() && WiFi.status() != WL_CONNECTED && millis() - lastWifiRetry > 12000) {
    lastWifiRetry = millis();
    WiFi.mode(WIFI_AP_STA);
    WiFi.reconnect();
    apActive = WiFi.softAP(("Mortymel-" + deviceId.substring(7)).c_str(), apPassword.c_str());
  }
  delay(2);
}
