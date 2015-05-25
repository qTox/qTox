#!/usr/bin/python2.7

# This script deploys a qTox update to Amazon S3: It will upload ./qtox/win32/version, ./qtox/win32/flist, and ./qtox/win32/files/*
# Pass the path to the root of the local update server as argument, no spaces allowed

import sys
import os
import boto
from boto.s3.key import Key

# Check our local folders
if (len(sys.argv) < 2):
	print("ERROR: Needs the path to the local update server in argument")
	sys.exit(1)

localpath = sys.argv[1];
prefix = "/qtox/win32/" # We only support Windows for now

# Connect to S3
conn = boto.connect_s3()
bucket = conn.get_bucket('qtox-updater')
print("Connected to S3")
sys.stdout.flush()

# Delete the old version, so nobody downloads a half-uploaded update
print("Deleting version ...")
sys.stdout.flush()
oldversion = Key(bucket)
oldversion.key = prefix+'version'
bucket.delete_key(oldversion)

# Upload files/
filenames = next(os.walk(localpath+prefix+'/files/'))[2]
for filename in filenames:
	print("Uploading files/"+filename+'...')
	sys.stdout.flush()
	k = Key(bucket)
	k.key = prefix+'files/'+filename
	k.set_contents_from_filename(localpath+prefix+'/files/'+filename)
	k.make_public()

# Upload version and flist
print("Uploading flist...")
sys.stdout.flush()
flist = Key(bucket)
flist.key = prefix+'flist'
flist.set_contents_from_filename(localpath+prefix+'flist')
flist.make_public()

print("Uploading version...")
sys.stdout.flush()
version = Key(bucket)
version.key = prefix+'version'
version.set_contents_from_filename(localpath+prefix+'version')
version.make_public()

