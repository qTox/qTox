/*
    Copyright © 2017 by The qTox Project Contributors

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

extern crate sodiumoxide;

use std::io::prelude::*;
use std::fs::{self, File};
use std::os::unix::fs::PermissionsExt;

use sodiumoxide::crypto::sign::*;

/// files that contains secret/public keys
const SECRET_KEY_FNAME: &'static str = "qtox-updater-skey";
const PUBLIC_KEY_FNAME: &'static str = "qtox-updater-pkey";


/// Supply `mode` as octet, e.g. `0o640`
fn set_permission(file: &File, mode: u32) {
    let mut perms = file.metadata()
        .expect("Failed to get file metadata")
        .permissions();
    perms.set_mode(mode);

    drop(fs::set_permissions(SECRET_KEY_FNAME, perms)
        .expect(&format!("Failed to set {:o} permissions on {}",
                mode, SECRET_KEY_FNAME)));
}


fn main() {
    let mut skey_file = File::create(SECRET_KEY_FNAME)
        .expect(&format!("Failed to open {}", SECRET_KEY_FNAME));
    let mut pkey_file = File::create(PUBLIC_KEY_FNAME)
        .expect(&format!("Failed to open {}", PUBLIC_KEY_FNAME));

    // set secret key file as rw only by the owner
    set_permission(&skey_file, 0o600);

    // make sure to init sodiumoxide for proper entropy
    if !sodiumoxide::init() {
        println!("Failed to initialize sodiumoxide, something is very wrong. \
                 Aborting");
        ::std::process::exit(1)
    }

    let (pk, sk): (PublicKey, SecretKey) = gen_keypair();

    skey_file.write_all(&sk.0).expect("Failed to write SecretKey");
    // secret key file should be read-only by owner
    set_permission(&skey_file, 0o400);

    pkey_file.write_all(&pk.0).expect("Failed to write PublicKey");

    println!("Wrote new keys to disk");
}
