#!/bin/bash

#   Copyright © 2016 Zetok Zalbavar <zetok@openmailbox.org>
#   Copyright © 2019 by The qTox Project Contributors
#
#   This file is part of qTox, a Qt-based graphical interface for Tox.
#   qTox is libre software: you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation, either version 3 of the License, or
#   (at your option) any later version.
#
#   qTox is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.
#
#   You should have received a copy of the GNU General Public License
#   along with qTox.  If not, see <http://www.gnu.org/licenses/>

# Script for verifying conformance to commit message format of commits in
# commit range supplied.
#
# Script fails (non-zero exit status) if commit messages don't conform.

# usage:
#   ./$script $commit_range
#
# $commit_range – in format `abdce..12345`


# Fail as soon as an error appears
set -eu -o pipefail

ARG="$1"

echo "" # ← formatting

grep_for_invalid() {
    # we can't rely on differentiating merge and normal commits since short clones that travis does may not be able
    # to tell if the oldest commit is a merge commit or not
    git log --format=format:'%s' "$ARG" \
        | grep -v -E '^((feat|fix|docs|style|refactor|perf|revert|test|chore)(\(.{,12}\))?:.{1,68})|(Merge .{1,70})$'
}

# Conform, /OR ELSE/.
if grep_for_invalid
then
    echo ""
    echo "Above ↑ commits don't conform to commit message format:"
    echo "https://github.com/qTox/qTox/blob/master/CONTRIBUTING.md#commit-message-format"
    echo ""
    echo "Please fix."
    echo ""
    echo "If you're not sure how to rewrite history, here's a helpful tutorial:"
    echo "https://www.atlassian.com/git/tutorials/rewriting-history/git-commit--amend/"
    echo ""
    echo "If you're still not sure what to do, feel free to pop on IRC, or ask in PR comments for help :)"
    # fail the build
    exit 1
fi
