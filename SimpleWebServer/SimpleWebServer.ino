//
// Simple Web Server
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
#include <SPI.h>
#include <SD.h>

//network config
byte mac[] = {0x90, 0xA2, 0xDA, 0x0F, 0x7D, 0x5E};
IPAddress ip(192, 168, 0, 200);
EthernetServer server(80);

void setup()
{
  Serial.begin(9600);
  Serial.println("Serial up");

  Serial.print("Initializing SD card... ");
  if (!SD.begin(4))
  {
    Serial.println("failed!");
    return;
  }
  Serial.println("OK.");

  Ethernet.begin(mac, ip);
  server.begin();
  Serial.print("Listening on ");
  Serial.println(Ethernet.localIP());
}

void sendFile(EthernetClient client, String getreq)
{
  if (getreq.endsWith("/"))
  {
    getreq += "index.htm";
  }
  File dataFile = SD.open(getreq);
  String mimeType;

  if (getreq.endsWith("jpg"))
  {
    mimeType = "image/jpeg";
  }
  else if (getreq.endsWith("js"))
  {
    mimeType = "application/javascript";
  }
  else if (getreq.endsWith("png"))
  {
    mimeType = "image/png";
  }
  else if (getreq.endsWith("txt"))
  {
    mimeType = "text/plain";
  }
  else if (getreq.endsWith("zip"))
  {
    mimeType = "application/zip";
  }
  else
  {
    mimeType = "text/html";
  }

  // if the file is available, send it
  if (dataFile)
  {
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: " + mimeType);
    client.println("Connection: close");
    client.println();
    while (dataFile.available())
    {
      client.write(dataFile.read());
    }
    dataFile.close();
  }
  // if the file isn't present, error
  else
  {
    client.println("HTTP/1.1 404 NOT FOUND");
    client.println("Content-Type: text/html");
    client.println("Connection: close");
    client.println();
    //client.println("<html>");
    client.println(getreq + " not found");
    //client.println("</html>");
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

        // GET request handler
        if (c == '\n' && req_str.startsWith("GET"))
        {
          Serial.println(req_str);
          String req = String(req_str.substring(4 + req_str.indexOf("GET"), req_str.indexOf("HTTP") - 1));
          //Serial.println(req);
          sendFile(client, req);
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
