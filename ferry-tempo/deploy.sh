#!/usr/bin/env sh

platformio run -t uploadfs $@ && platformio run -t upload $@
