#!/bin/sh

platformio run -t uploadfs $@
platformio run -t upload $@
