@echo off

set PATH=%PATH%;%HOMEPATH%\.platformio\penv\Scripts\

platformio run -t uploadfs %*
platformio run -t upload %*
