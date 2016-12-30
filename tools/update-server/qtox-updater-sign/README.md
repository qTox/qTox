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

# Compiling

Requires `libsodium` and Rust.

To build a debug version: `cargo build`

To build a release version: `cargo build --release`

This will produce `qtox-updater-sign` binary in `target/debug` or
`target/release` directory respectively.
