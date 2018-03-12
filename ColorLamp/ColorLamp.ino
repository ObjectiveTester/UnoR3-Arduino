//
// RGB LED controller via web interface
//  Steve Mellor 12 Mar 2018
//
// based on Arduino.cc WebServer example
// created 18 Dec 2009
// by David A. Mellis
// modified 9 Apr 2012
// by Tom Igoe
// modified 02 Sept 2015
// by Arturo Guadalupi
//

#include <Ethernet.h>

//network config
byte mac[] = {0x90, 0xA2, 0xDA, 0x0F, 0x7D, 0x5E};
IPAddress ip(192, 168, 0, 200);
EthernetServer server(80);

//hardware config
const int redPin = 3;
const int greenPin = 5;
const int bluePin = 6;

//initial config
byte redValue = 128;
byte greenValue = 128;
byte blueValue = 128;

String dec2hex(int val)
{
  String hex = String(val, HEX);
  if (hex.length() < 2)
    hex = "0" + hex;
  return hex;
}
String rgb = dec2hex(redValue) + dec2hex(greenValue) + dec2hex(blueValue);

void setColor()
{
  //ifdef for common cathode
  analogWrite(redPin, redValue);
  analogWrite(greenPin, greenValue);
  analogWrite(bluePin, blueValue);
}

void setup()
{
  Serial.begin(9600);

  pinMode(greenPin, OUTPUT);
  pinMode(redPin, OUTPUT);
  pinMode(bluePin, OUTPUT);

  setColor();

  // start the Ethernet connection and the server:
  Ethernet.begin(mac, ip);
  server.begin();
  Serial.print("Listening on ");
  Serial.println(Ethernet.localIP());
}

void sendResponse(EthernetClient client)
{
  // send the lamp page
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html");
  client.println("Connection: close");
  client.println();
  client.println("<html>");
  client.println("<script>");
  client.println("var co, nc;");
  client.println("var def = \"#" + rgb + "\";");
  client.println("var req = new XMLHttpRequest();");
  client.println();
  client.println("window.addEventListener(\"load\", startup, false);");
  client.println("function startup() {");
  client.println("  co = document.querySelector(\"#color\");");
  client.println("  co.value = def;");
  client.println("  co.addEventListener(\"change\", chColor, false);");
  client.println("  co.select();");
  client.println("}");
  client.println("function chColor(event) {");
  client.println("  console.log(co.value);");
  client.println("  nc = co.value.replace(\"#\", \"\");");
  client.println("  req.open('GET', '/?color='+nc, true);");
  client.println("  req.send();");
  client.println("}");
  client.println("</script>");
  client.println("<input type=\"color\" id=\"color\">");
  client.println("</html>");
}

void updateColor(String buffer)
{
  if (buffer.startsWith("color"))
  {
    rgb = buffer.substring(1 + buffer.indexOf("="), 7 + buffer.indexOf("="));
    //Serial.println(rgb);
    redValue = strtol(rgb.substring(0, 2).c_str(), 0, 16);
    greenValue = strtol(rgb.substring(2, 4).c_str(), 0, 16);
    blueValue = strtol(rgb.substring(4, 6).c_str(), 0, 16);

    setColor();
  }
}

void loop()
{
  // listen for incoming clients
  EthernetClient client = server.available();
  if (client)
  {
    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
    String req_str = "";

    while (client.connected())
    {
      if (client.available())
      {
        char c = client.read();
        req_str += c;

        if (c == '\n' && currentLineIsBlank && req_str.startsWith("GET"))
        {
          sendResponse(client);
          if (req_str.indexOf("color") > 0)
          {
            String req = String(req_str.substring(req_str.indexOf("color"), req_str.indexOf(" HTTP")));
            updateColor(req);
          }
          break;
        }
        if (c == '\n')
        {
          // you're starting a new line
          currentLineIsBlank = true;
        }
        else if (c != '\r')
        {
          // you've gotten a character on the current line
          currentLineIsBlank = false;
        }
      }
    }

    // give the web browser time to receive the data
    delay(1);
    // close the connection:
    client.stop();
  }
}
