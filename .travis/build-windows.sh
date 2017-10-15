#!/bin/bash
#
#    Copyright © 2016 by The qTox Project Contributors
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
#

# Fail out on error
set -exuo pipefail

# Just make sure those exists, makes logic easier
mkdir -p $DEP_CACHE

ls -lbh $DEP_CACHE

if [ -f $DEP_CACHE/one ] && [ -f $DEP_CACHE/two ]
then
  echo "test"
  rm -rf $DEP_CACHE/*
elif [ -f $DEP_CACHE/one ]
then
  touch $DEP_CACHE/two
else
  touch $DEP_CACHE/one
fi

ls -lbh $DEP_CACHE
