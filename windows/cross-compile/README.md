# Cross-compile from Linux to Windows

## Usage

From the windows_builder/windows_builder.i686 docker image, run build.sh. NOTE:
The arch argument must match the builder image

```
$ docker compose run windows_builder
# mkdir build-windows
# cd build-windows
# # Example is using --arch x86_64, --arch i686 should be used if you are in the i686 docker image
# /qtox/windows/cross-compile/build.sh --src-dir /qtox --arch x86_64 --build-type release
```
