#!/usr/bin/python2.7

# This script deploys a qTox update to an FTP server.
# Pass the path to the root of the local update server as argument, no spaces allowed

import sys
import os

target = 'win32'
prefix = '/qtox/'+target+'/'
uploadcmd1 = "bash -c '"+'ftp -n tux3-dev.tox.chat 0<<<"`echo -ne "user qtox-win-update-upload INSERT-PASSWORD-HERE\ncd '+target+'\nsend '
uploadcmd2 = '\n"`"'+"'"

def upload(file, rfile):
        #print(uploadcmd1+file+' '+rfile+uploadcmd2)
        os.system(uploadcmd1+file+' '+rfile+uploadcmd2)

# Check our local folders
if (len(sys.argv) < 2):
        print("ERROR: Needs the path to the local update server in argument")
        sys.exit(1)

localpath = sys.argv[1];

# Upload files/
filenames = next(os.walk(localpath+prefix+'/files/'))[2]
for filename in filenames:
        print("Uploading files/"+filename+'...')
        upload(localpath+prefix+'/files/'+filename, 'files/'+filename)

# Upload version and flist
print("Uploading flist...")
upload(localpath+prefix+'flist', 'flist')
print("Uploading version...")
upload(localpath+prefix+'version', 'version')
