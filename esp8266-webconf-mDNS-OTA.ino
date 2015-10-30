/**
 * @file esp8266-webconf-mDNS-OTA.ino
 * 
 * @author Pascal Gollor (http://www.pgollor.de/cms/)
 * @data 2015-08-17
 * 
 */


#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <FS.h>
#include <ESP8266WebServer.h>
#include <ArduinoOTA.h>


/**
 * @brief mDNS and OTA Constants
 * @{
 */
#define HOSTNAME "ESP8266-" ///< Hostename
#define APORT 8266 ///< OTA Port
/// @}

/**
 * @brief Default WiFi connection information.
 * @{
 */
const char* ap_default_ssid = "esp8266"; ///< Default SSID.
const char* ap_default_psk = "esp8266esp8266"; ///< Default PSK.
/// @}

/// Uncomment the next line for verbose output over UART.
#define SERIAL_VERBOSE

/// HTML answer on restart request.
#define RESTART_HTML_ANSWER "<!DOCTYPE html><html><head><meta http-equiv=\"refresh\" content=\"15; URL=http://" HOSTNAME ".local/\"></head><body>Restarting in 15 seconds.<br/><img src=\"/loading.gif\"></body></html>"

/// OTA server handle.
#ifdef SERIAL_VERBOSE
  ArduinoOTA ota_server(HOSTNAME, APORT, true);
#else
ArduinoOTA ota_server(HOSTNAME, APORT, false);
#endif

/// Webserver handle on port 80.
ESP8266WebServer g_server(80);

/// global WiFi SSID.
String g_ssid = "";

/// global WiFi PSK.
String g_pass = "";

/// Restart will be triggert on this time
unsigned long g_restartTime = 0;


/**
 * @brief Read WiFi connection information from file system.
 * @param ssid String pointer for storing SSID.
 * @param pass String pointer for storing PSK.
 * @return True or False.
 * 
 * The config file have to containt the WiFi SSID in the first line
 * and the WiFi PSK in the second line.
 * Line seperator can be \r\n (CR LF) \r or \n.
 */
bool loadConfig(String *ssid, String *pass)
{
  // open file for reading.
  File configFile = SPIFFS.open("/cl_conf.txt", "r");
  if (!configFile)
  {
    Serial.println("Failed to open cl_conf.txt.");

    return false;
  }

  // Read content from config file.
  String content = configFile.readString();
  configFile.close();
  
  content.trim();

  // Check if ther is a second line available.
  int8_t pos = content.indexOf("\r\n");
  uint8_t le = 2;
  // check for linux and mac line ending.
  if (pos == -1)
  {
    le = 1;
    pos = content.indexOf("\n");
    if (pos == -1)
    {
      pos = content.indexOf("\r");
    }
  }

  // If there is no second line: Some information is missing.
  if (pos == -1)
  {
    Serial.println("Infvalid content.");
    Serial.println(content);

    return false;
  }

  // Store SSID and PSK into string vars.
  *ssid = content.substring(0, pos);
  *pass = content.substring(pos + le);

  ssid->trim();
  pass->trim();

#ifdef SERIAL_VERBOSE
  Serial.println("----- file content -----");
  Serial.println(content);
  Serial.println("----- file content -----");
  Serial.println("ssid: " + *ssid);
  Serial.println("psk:  " + *pass);
#endif

  return true;
} // loadConfig


/**
 * @brief Save WiFi SSID and PSK to configuration file.
 * @param ssid SSID as string pointer.
 * @param pass PSK as string pointer,
 * @return True or False.
 */
bool saveConfig(String *ssid, String *pass)
{
  // Open config file for writing.
  File configFile = SPIFFS.open("/cl_conf.txt", "w");
  if (!configFile)
  {
    Serial.println("Failed to open cl_conf.txt for writing");

    return false;
  }

  // Save SSID and PSK.
  configFile.println(*ssid);
  configFile.println(*pass);

  configFile.close();
  
  return true;
} // saveConfig


/**
 * @brief Handle http root request.
 */
void handleRoot()
{
  String indexHTML;
  char buff[10];
  uint16_t s = millis() / 1000;
  uint16_t m = s / 60;
  uint8_t h = m / 60;
  
  File indexFile = SPIFFS.open("/index.html", "r");
  if (indexFile)
  {
    indexHTML = indexFile.readString();
    indexFile.close();

    snprintf(buff, 10, "%02d:%02d:%02d", h, m % 60, s % 60);

    // replace placeholder
    indexHTML.replace("[esp8266]", String(ESP.getChipId(), HEX));
    indexHTML.replace("[rssi]", String(WiFi.RSSI()));
    indexHTML.replace("[ssid]", g_ssid);
    indexHTML.replace("[pass]", g_pass);
    indexHTML.replace("[uptime]", buff);

    //Serial.println(g_indexHTML);
  }
  else
  {
    indexHTML = "<!DOCTYPE html><html><head><title>File not found</title></head><body><h1>File not found.</h1></body></html>";
  }

  g_server.send (200, "text/html", indexHTML);
} // handleRoot


/**
 * @brief Handle set request from http server.
 * 
 * URI: /set?ssid=[WiFi SSID],pass=[WiFi Pass]
 */
void handleSet()
{
  String response = "<!DOCTYPE html><html><head><meta http-equiv=\"refresh\" content=\"2; URL=http://";
  response += HOSTNAME;
  response += ".local\"></head><body>";
  
  // Some debug output
  Serial.print("uri: ");
  Serial.println(g_server.uri());

  Serial.print("method: ");
  Serial.println(g_server.method());

  Serial.print("args: ");
  Serial.println(g_server.args());

  // Check arguments
  if (g_server.args() < 2)
  {
    g_server.send (200, "text/plain", "Arguments fail.");
    return;
  }

  String ssid = "";
  String pass = "";

  // read ssid and psk
  for (uint8_t i = 0; i < g_server.args(); i++)
  {
    if (g_server.argName(i) == "ssid")
    {
      ssid = g_server.arg(i);
    }
    else if (g_server.argName(i) == "pass")
    {
      pass = g_server.arg(i);
    }
  }

  // check ssid and psk
  if (ssid != "" && pass != "")
  {
    // save ssid and psk to file
    if (saveConfig(&ssid, &pass))
    {
      // store SSID and PSK into global variables.
      g_ssid = ssid;
      g_pass = pass;

      response += "Successfull.";
    }
    else
    {
      response += "<h1>Fail save to config file.</h1>";
    }
  }
  else
  {
    response += "<h1>Wrong arguments.</h1>";
  }

  response += "</body></html>";
  g_server.send (200, "text/html", response);
} // handleSet


/**
 * @brief Load loading.gif from filesytem and draw.
 */
void drawLoading()
{
  File gif = SPIFFS.open("/loading.gif", "r");
  if (gif)
  {
    g_server.send(200, "image/gif", gif.readString());
    gif.close();
  }
  else
  {
    g_server.send(404, "text/plain", "Gif not found.");
  }
} // drawLoading


/**
 * @brief Arduino setup function.
 */
void setup()
{
  g_ssid = "";
  g_pass = "";

  Serial.begin(115200);
  
  delay(100);

  Serial.println("\r\n");
  Serial.print("Chip ID: 0x");
  Serial.println(ESP.getChipId(), HEX);

  // Set Hostname.
  String hostname(HOSTNAME);
  hostname += String(ESP.getChipId(), HEX);
  WiFi.hostname(hostname);

  // Print hostname.
  Serial.println("Hostname: " + hostname);
  //Serial.println(WiFi.hostname());


  // Initialize file system.
  if (!SPIFFS.begin())
  {
    Serial.println("Failed to mount file system");
    return;
  }

  // Load wifi connection information.
  if (! loadConfig(&g_ssid, &g_pass))
  {
    g_ssid = "";
    g_pass = "";

    Serial.println("No WiFi connection information available.");
  }

  // Check WiFi connection
  // ... check mode
  if (WiFi.getMode() != WIFI_STA)
  {
    WiFi.mode(WIFI_STA);
    delay(10);
  }

  // ... Compare file config with sdk config.
  if (WiFi.SSID() != g_ssid || WiFi.psk() != g_pass)
  {
    Serial.println("WiFi config changed.");

    // ... Try to connect to WiFi station.
    WiFi.begin(g_ssid.c_str(), g_pass.c_str());

    // ... Pritn new SSID
    Serial.print("new SSID: ");
    Serial.println(WiFi.SSID());

    // ... Uncomment this for debugging output.
    //WiFi.printDiag(Serial);
  }
  else
  {
    // ... Begin with sdk config.
    WiFi.begin();
  }

  Serial.println("Wait for WiFi connection.");

  // ... Give ESP 10 seconds to connect to station.
  unsigned long startTime = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startTime < 10000)
  {
    Serial.write('.');
    //Serial.print(WiFi.status());
    delay(500);
  }
  Serial.println();

  // Check connection
  if(WiFi.status() == WL_CONNECTED)
  {
    // ... print IP Address
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
  }
  else
  {
    Serial.println("Can not connect to WiFi station. Go into AP mode.");
    
    // Go into AP mode.
    WiFi.mode(WIFI_AP);

    delay(10);

    WiFi.softAP(ap_default_ssid, ap_default_psk);

    Serial.print("IP address: ");
    Serial.println(WiFi.softAPIP());
  }

  // Start OTA server.
  ota_server.setup();

  // Initialize web server.
  // ... Add requests.
  g_server.on("/", handleRoot);
  g_server.on("/set", HTTP_GET, handleSet);
  g_server.on("/restart", []() {
    g_server.send(200, "text/html", RESTART_HTML_ANSWER);
    g_restartTime = millis() + 100;
  } );
  g_server.serveStatic("/loading.gif", SPIFFS, "/loading.gif");

  // ... Start server.
  g_server.begin();
}


/**
 * @brief Arduino loop function.
 */
void loop()
{
  // Handle OTA server.
  ota_server.handle();
  yield();

  // Handle Webserver requests.
  g_server.handleClient();

  // trigger restart?
  if (g_restartTime > 0 && millis() >= g_restartTime)
  {
    g_restartTime = 0;
    ESP.restart();
  }
}

