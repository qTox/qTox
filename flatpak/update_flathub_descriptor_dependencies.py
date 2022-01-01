#!/usr/bin/env python3

#    Copyright © 2021 by The qTox Project Contributors
#
#    This program is libre software: you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation, either version 3 of the License, or
#    (at your option) any later version.
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with this program.  If not, see <http://www.gnu.org/licenses/>.

from argparse import ArgumentParser
from pathlib import Path

import re
import tempfile
import json
import unittest
import subprocess

QTOX_ROOT = Path(__file__).parent.parent
DOWNLOAD_FILE_PATHS = QTOX_ROOT / 'buildscripts' / 'download'

def parse_args():
    parser = ArgumentParser(description="""
    Update dependencies of a flathub manifest to match the versions used by
    qtox. This script will iterate over all known dependencies in the manifest
    and replace their tags with the ones specified by our download_xxx.sh
    scripts. The commit hash for the tag will be replaced with whatever is
    currently in the git remote
    """)
    parser.add_argument(
        "--flathub-manifest",
        help="Path to flathub manifest",
        required=True,
        dest="flathub_manifest_path")
    parser.add_argument(
        "--output",
        help="Output manifest path",
        required=True,
        dest="output_manifest_path")
    return parser.parse_args()

VERSION_EXTRACT_REGEX=re.compile(".*_VERSION=(.*)")
def find_version(download_script_path):
    """
    Find the version specified for a given dependency by parsing its download script
    """
    # Unintelligent regex parsing, but it will have to do.
    # Hope there is no shell expansion in our version info, otherwise we'll have
    # to do something a little more intelligent
    with open(download_script_path) as f:
        script_content = f.read()
        matches = VERSION_EXTRACT_REGEX.search(script_content, re.MULTILINE)

    return matches.group(1)

class FindVersionTest(unittest.TestCase):
    def test_version_parsing(self):
        # Create a dummy download script and check that we can extract the version from it
        with tempfile.TemporaryDirectory() as d:
            sample_download_script = """
            #!/bin/bash

            source "$(dirname "$0")"/common.sh

            TEST_VERSION=1.2.3
            TEST_HASH=:)

            download_verify_extrat_tarball \
                https://test_site.com/${TEST_VERSION} \
                ${TEST_HASH}
            """

            sample_download_script_path = d + '/test_script.sh'
            with open(sample_download_script_path, 'w') as f:
                f.write(sample_download_script)

            self.assertEqual(find_version(sample_download_script_path), "1.2.3")

def load_flathub_manifest(flathub_manifest_path):
    with open(flathub_manifest_path) as f:
        return json.load(f)

def commit_from_tag(url, tag):
    git_output = subprocess.run(
        ['git', 'ls-remote', url, f"{tag}^{{}}"], check=True, stdout=subprocess.PIPE)
    commit = git_output.stdout.split(b'\t')[0]
    return commit.decode()

class CommitFromTagTest(unittest.TestCase):
    def test_commit_from_tag(self):
        self.assertEqual(commit_from_tag(str(QTOX_ROOT), "v1.17.3"), "c0e9a3b79609681e5b9f6bbf8f9a36cb1993dc5f")

def update_source(module, tag):
    module_source = module["sources"][0]
    module_source["tag"]= tag
    module_source["commit"] = commit_from_tag(module_source["url"], module_source["tag"])

def main(flathub_manifest_path, output_manifest_path):
    flathub_manifest = load_flathub_manifest(flathub_manifest_path)

    sqlcipher_version = find_version(DOWNLOAD_FILE_PATHS / 'download_sqlcipher.sh')
    sodium_version = find_version(DOWNLOAD_FILE_PATHS / 'download_sodium.sh')
    toxcore_version = find_version(DOWNLOAD_FILE_PATHS / 'download_toxcore.sh')
    toxext_version = find_version(DOWNLOAD_FILE_PATHS / 'download_toxext.sh')
    toxext_messages_version = find_version(DOWNLOAD_FILE_PATHS / 'download_toxext_messages.sh')

    for module in flathub_manifest["modules"]:
        if module["name"] == "sqlcipher":
            update_source(module, f"v{sqlcipher_version}")
        elif module["name"] == "libsodium":
            update_source(module, sodium_version)
        elif module["name"] == "c-toxcore":
            update_source(module, f"v{toxcore_version}")
        elif module["name"] == "toxext":
            update_source(module, f"v{toxext_version}")
        elif module["name"] == "tox_extension_messages":
            update_source(module, f"v{toxext_messages_version}")

    with open(output_manifest_path, 'w') as f:
        json.dump(flathub_manifest, f, indent=4)

if __name__ == '__main__':
    main(**vars(parse_args()))
