#!/usr/bin/python2.7

from github3 import login, GitHub
from getpass import getpass, getuser
import time
import datetime
import sys
try:
    import readline
except ImportError:
    pass

platform=''
if (len(sys.argv) >= 3):
	platform=sys.argv[2]

versionNumber = str(time.time())
if (platform != ''):
	version = 'qtox-'+platform+'-'+versionNumber
else:
	version = 'qtox-'+versionNumber
if (platform == 'windows'):
	title = 'qTox Windows '+datetime.datetime.fromtimestamp(time.time()).strftime('%Y-%m-%d %H:%M:%S')
elif (platform == 'linux'):
	title = 'qTox Linux '+datetime.datetime.fromtimestamp(time.time()).strftime('%Y-%m-%d %H:%M:%S')
else:
	title = 'qTox '+datetime.datetime.fromtimestamp(time.time()).strftime('%Y-%m-%d %H:%M:%S')
user = "tux3"
password = ""
if password == "":
	password = getpass('GitHub password for {0}: '.format(user))

# Obviously you could also prompt for an OAuth token
if not (user and password):
    print("Cowardly refusing to login without a username and password.")
    sys.exit(1)

g = login(user, password)
repo = g.repository('tux3', 'qTox')
release = repo.create_release(version,'master',title,'This is an automated release of qTox, published by qTox\'s continous integration server.',False,False)

if (len(sys.argv) >= 2):
	file = open(sys.argv[1], 'r')
	release.upload_asset('application/octet-stream',sys.argv[1],file)

