#!/usr/bin/env python

import getpass
from time import gmtime, strftime

username = getpass.getuser()
date = strftime("%a, %d %b %Y at %X UTC", gmtime())

print("-DBUILD_INFO='\"Built by %(username)s on %(date)s\"'" % locals())