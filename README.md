# Ferries Over WInslow
This is the code for robotic ferries running on a wire over a street, tracking the progress of the actual boats. There is also code here for miniature Arduino-powered ferries that have the same functionality as the large one.
## Communication and Processing
The code is based upon a client-server model. The client is a small microcontroller that controls actuators and sensors. It communicates over TCP/IP with a more powerful server running on an internet connected computer.
## Server
The server provides data for both the miniature and full-sized ferries because its behaviro is the same for both. It communicates with the Washington Department of Transportation REST API, and uses a point on a latitude and longitude coordinate plane to calculate the "progress" along a set of connected line segments that represent the normal route of the boat on the coordinate plane. The server then estimates the ferry's future location based on speed and heading, and calculates a percentage for both. The server then provides the client with these two percentages, and a time that the first percentage was calculated for (the second percentage is assumed to be a constant amount of time ahead). The server is written in Go.
## Mini-Client
The client for the miniature ferries communicates with the internet connected server, and is proveded with two percentages that it linearly interpolates with to find position it should make the stepper motor go to. The mini-client is written in C++.
