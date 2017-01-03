# qtox-updater-sign

Simple program for signing releases.

Requires a file named `qtox-updater-skey` in the working directory that
contains the secret key.

To sign a release, either supply the name of the file to be signed as an
argument, or pipe data via stdin.

```bash
./qtox-updater-sign sign-this-binary > signature_file
# or
./qtox-updater-sign < sign-this-binary > signature_file
```
