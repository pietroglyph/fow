# Ferries Over Winslow
This is the code for robotic ferries running on a wire over a street, tracking the progress of the actual boats. There is also code here for miniature Arduino-powered ferries that have the same functionality as the large one.
## Communication and Processing
The code is based upon a client-server model. The client is a small microcontroller that controls actuators and sensors. It communicates over HTTP with a more powerful computer running the server.
## Server
The server can provide data for both the miniature and full-sized ferries because the server's behavior should be the same for both. The processing steps are as follows:
 1. The server periodically makes requests to the Washington Department of Transportation REST API, from which it collects the latitude and longitute of the boats it is tracking.
 2. These lat/long value are matched to the closest point on a set of connected line segments (known as the "reference path"). We use this information to determine our progress (as a percentage) along this reference path.
 3. We estimate the ferry's future location based current on speed and heading, and calculate a progress percentage for both the current and future location.
 4. We serve the client these two percentages, how long ago the first percentage was calculated, and how far ahead the second percentage is from the first. This is enough information for the client interpolate between these two values over time.

The server is written in Go.
## Mini-Client
The client for the miniature ferries communicates with the server, serves a configuration webpage over a WiFi network it broadcasts, and stores user-provided WiFi credentials across reboots. The client linearly interpolates between two ferry "progress" percentages provided by the server (there two percentages for each boat; two boats are currently supported). This progress percentage is used to actuate the display to the correct position. A variety of hardware is supported from the same codebase. The mini-client is written in C++. You can read more about it [here](https://github.com/pietroglyph/fow/tree/master/fow-mini).
