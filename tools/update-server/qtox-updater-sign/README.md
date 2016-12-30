# qtox-updater-sign

Simple program for signing releases.

Requires file named `qtox-updater-skey` in workign directory that contains
secret key.

To sign release, either supply name of the data to be signed as an argument,
or pipe data via stdin.

```bash
./qtox-updater-sign sign-this-binary > signature_file

# or
./qtox-updater-sign < sign-this-binary > signature_file
```

# Compiling

Requires `libsodium` and Rust.

To build a debug version: `cargo build`

To build a release version: `cargo build --release`

This will produce `qtox-updater-sign` binary in either `target/debug` or
`target/release` directory.
