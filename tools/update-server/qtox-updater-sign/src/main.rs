/*
    Copyright © 2016 by The qTox Project Contributors

    This file is part of qTox, a Qt-based graphical interface for Tox.

    qTox is libre software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    qTox is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with qTox.  If not, see <http://www.gnu.org/licenses/>.
*/

extern crate sodiumoxide;

use std::io::prelude::*;
use std::io;
use std::fs::File;

use sodiumoxide::crypto::sign::*;

/// file that contains secret key
const SKEY_FNAME: &'static str = "qtox-updater-skey";

/// if there is more than just program name in args, treat "1st" arg as a
/// filename to sign
/// otherwise just read bytes from stdin
fn read_from_file_or_stdin(buf: &mut Vec<u8>) {
    if std::env::args().count() > 1 {
        let fname = std::env::args().nth(1).expect("Failed to get fname");
        let mut file = File::open(fname).expect("Failed to open fname");
        file.read_to_end(buf).expect("Failed to read fname");
    } else {
        io::stdin().read_to_end(buf).expect("Failed to read stdin");
    }
}

/// get SecretKey from `SKEY_FNAME` file
fn get_secret_key() -> SecretKey {
    let mut skey_file = File::open(SKEY_FNAME)
        .expect(&format!("ERROR: {} can't open the secret (private) key file\n",
                std::env::args().next().unwrap()));

    let mut skey_bytes = Vec::with_capacity(SECRETKEYBYTES);
    skey_file.read_to_end(&mut skey_bytes)
        .expect(&format!("Failed to read {}", SKEY_FNAME));

    SecretKey::from_slice(&skey_bytes[..SECRETKEYBYTES])
        .expect("Failed to get right amout of bytes for SecretKey")
}


fn main() {
    let mut plaintext = Vec::new();

    read_from_file_or_stdin(&mut plaintext);

    let sk = get_secret_key();
    let signed = sign(&plaintext, &sk);

    io::stdout().write_all(&signed).expect("Failed to write signature");
}
