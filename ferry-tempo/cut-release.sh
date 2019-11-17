#!/usr/bin/env bash

if [[ $2 == "" ]]; then
	echo "Usage: cut-release.sh [environment name] [hardware-revision string]"
	exit 1
fi


if [[ "$3" != "--no-rebuild" ]]; then
	read -p "Disconnect your computer from any FERRY TEMPO devices and press enter to begin"
	echo "The next two commands should output a failure message" >&2
	platformio run -t upload -e "$1"
	platformio run -t uploadfs -e "$1"
else
	echo "We will not rebuild the binaries"
fi

VERSION=$(./print-version.sh --raw)
echo "Got version as $VERSION"

FILE_START="$VERSION@production:$2:"

mkdir -p build
cp ./.pio/build/"$1"/firmware.bin ./build/"$FILE_START""flash.bin"
cp ./.pio/build/"$1"/spiffs.bin ./build/"$FILE_START""spiffs.bin"

echo "Copied releases to ./build/"
