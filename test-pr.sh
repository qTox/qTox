#!/bin/bash
#
#    Copyright © 2016 Zetok Zalbavar <zetok@openmailbox.org>
#
#    This program is free software: you can redistribute it and/or modify
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

# Script for testing pull requests. Works only when there are no merge
# conflicts. Assumes that working dir is a qTox git repo.
#

# usage:
#   ./$script $pr_number $optional_message
#
#
# $pr_number – number of the PR as shown on GH
# $optional_message – message that is going to be put in merge commit,
#       before the appended shortlog.
#

set -e -o pipefail

readonly PR=$1

# make sure to add newlines to the message, otherwise merge message
# will not look well
if [[ ! -z $2 ]]
then
    readonly OPT_MSG="
$2
"
fi

source_functions() {
    local fns_file="tools/lib/PR_bash.source"
    source $fns_file
}

main() {
    local remote_name="upstream"
    local merge_branch="test"
    source_functions
    exit_if_not_pr $PR
    add_remote "https"
    get_sources

    merge "--no-gpg-sign" \
        && after_merge_msg $merge_branch \
        || after_merge_failure_msg $merge_branch
}
main
