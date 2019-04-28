#!/usr/bin/env python

import distutils.spawn
import subprocess
import os

if not distutils.spawn.find_executable("git.exe"):
    print("-DVERSION='\"unknown-nogit\"'")
    exit(0)
elif not os.name == "posix":
    print("-DVERSION='\"unknown-noposix\"'")
else:
    subprocess.call("./print-version.sh")