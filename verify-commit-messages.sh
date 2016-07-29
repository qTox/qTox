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

# Script for verifying conformance to commit message format of commits in commit
# range supplied.
#
# Script fails (non-zero exit status) if commit messages don't conform.

# usage:
#   ./$script $commit_range
#
# $commit_range – in format `abdce..12345`

ARG="$1"

echo "" # ← formatting

# Conform, /OR ELSE/.
if git log --format=format:'%s' "$ARG" | \
    grep -v -E '^((feat|fix|docs|style|refactor|perf|revert|test|chore)(\(.+\))?:.{1,68})|(Merge pull request #[[:digit:]]{1,10})$'
then
    echo ""
    echo "Above ↑ commits don't conform to commit message format:"
    echo "https://github.com/tux3/qTox/blob/master/CONTRIBUTING.md#commit-message-format"
    echo ""
    echo "Pls fix."
    echo ""
    echo "If you're not sure how to rewrite history, here's a helpful tutorial:"
    echo "https://www.atlassian.com/git/tutorials/rewriting-history/git-commit--amend/"
    echo ""
    echo "If you're still not sure what to do, feel free to pop on IRC, or ask in PR comments for help :)"
    # fail the build
    exit 1
fi
