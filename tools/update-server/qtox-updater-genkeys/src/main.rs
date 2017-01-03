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

fn main() {
    let mut skey_file = File::create(SECRET_KEY_FNAME)
        .expect(&format!("Failed to open {}", SECRET_KEY_FNAME));
    let mut pkey_file = File::create(PUBLIC_KEY_FNAME)
        .expect(&format!("Failed to open {}", PUBLIC_KEY_FNAME));

    // make sure to init sodiumoxide for proper entropy
    sodiumoxide::init();

    let (pk, sk): (PublicKey, SecretKey) = gen_keypair();

    skey_file.write_all(&sk.0).expect("Failed to write SecretKey");
    pkey_file.write_all(&pk.0).expect("Failed to write PublicKey");

    // SecretKey data should be read-only by owner
    let mut perms = skey_file.metadata()
        .expect("Failed to get SecretKey metadata")
        .permissions();
    perms.set_mode(0o400);

    drop(fs::set_permissions(SECRET_KEY_FNAME, perms)
        .expect("Failed to set read-only permissions on SecretKey"));

    println!("Wrote new keys to disk");
}
