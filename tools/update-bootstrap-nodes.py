#!/usr/bin/env python3
"""
#
#    Copyright © 2016 sudden6 <sudden6@gmx.at>
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
"""

# Gets a list of nodes from https://nodes.tox.chat/json and prints them out
# in the format of qTox settings.ini config file.

import urllib.request
import json

response = urllib.request.urlopen('https://nodes.tox.chat/json')
raw_json = response.read().decode('utf8', 'ignore')
nodes = json.loads(raw_json)['nodes']

header = '[DHT%20Server]\n'
data = ""
node_nr = 1

for node in nodes:
	if len(node['ipv4']) > 4:
		prefix = 'dhtServerList\\' + str(node_nr) + '\\'
		data += prefix + 'name='    + node['maintainer'] + '\n'
		data += prefix + 'userId='  + node['public_key'] + '\n'
		data += prefix + 'address=' + node['ipv4']       + '\n'
		data += prefix + 'port='    + str(node['port'])  + '\n'
		node_nr = node_nr + 1

header +='dhtServerList\size=' + str(node_nr - 1) + '\n'

print(header + data)
