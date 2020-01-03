# Ferries Over Winslow
This is the code for electromechanical "ferry tracker clock" devices. This repository includes both the server, written in Go, and the client, which runs on an ESP8266 microcontroller and is written in C++. There are currently three non-prototype hardware revisions (and many more prototypes). Here are some pictures of the different models we've produced:

<p align="center">
 <strong>Mainsail v1</strong> (retail model)
 <br>
 <img src="https://raw.githubusercontent.com/pietroglyph/fow/master/readme_images/Mainsail_v1.jpeg" alt="The Mainsail v1 ferry tracker clock hardware revision" width="500px"/>
 <br>
 <strong>Kitty v2</strong> (kit model)
 <br>
 <img src="https://raw.githubusercontent.com/pietroglyph/fow/master/readme_images/Kitty_v2.jpg" alt="The Kitty v2 ferry tracker clock hardware revision" width="500px"/>
 <br>
 <strong>Kitty v1*</strong>
 <br>
 <img src="https://raw.githubusercontent.com/pietroglyph/fow/master/readme_images/Kitty_v1.jpg" alt="The Kitty v1 ferry tracker clock hardware revision" width="500px"/>
</p>

*Kitty v1 is no longer being sold as a kit.*


## Communication and Processing
The code is based upon a client-server model. The client is a small microcontroller that controls actuators and sensors. It communicates over HTTP with a more powerful computer running the server.
## Server
The server can provide data for both the miniature and full-sized ferries because the server's behavior should be the same for both. The processing steps are as follows:
 1. The server periodically makes requests to the Washington Department of Transportation REST API, from which it collects the latitude and longitude of the boats it is tracking.
 2. These lat/long value are matched to the closest point on a set of connected line segments (known as the "reference path"). We use this information to determine our progress (as a percentage) along this reference path.
 3. We estimate the ferry's future location based current on speed and heading, and calculate a progress percentage for both the current and future location.
 4. We serve the client these two percentages, how long ago the first percentage was calculated, and how far ahead the second percentage is from the first. This is enough information for the client to interpolate between these two values over time.

The server is written in Go.
## FERRY TEMPO client
The client for the miniature ferries communicates with the server, serves a configuration webpage over a WiFi network it broadcasts, and stores user-provided WiFi credentials across reboots. The client linearly interpolates between two ferry "progress" percentages provided by the server (there two percentages for each boat; two boats are currently supported). This progress percentage is used to actuate the display to the correct position. A variety of hardware is supported from the same codebase. The FERRY TEMPO client software is written in C++. You can read more about the FERRY TEMPO and flashing your board [here](https://github.com/pietroglyph/fow/tree/master/ferry-tempo).
