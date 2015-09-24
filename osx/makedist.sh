#!/usr/bin/env bash

PWD=`pwd`

echo "<?xml version=\"1.0\" encoding=\"utf-8\" standalone=\"no\"?>
<installer-gui-script minSpecVersion=\"1\">
    <pkg-ref id=\"chat.tox.qtox\"/>
    <options hostArchitectures=\"x86_64\"/>
    <title>qTox</title>
    <license file=\"$PWD/gplv3.rtf\"/>
    <welcome file=\"$PWD/welcome.txt\"/>
    <domains enable_currentUserHome=\"true\" enable_localSystem=\"false\" enable_anywhere=\"false\"/>
    <options customize=\"never\" require-scripts=\"true\"/>
    <choices-outline>
        <line choice=\"default\">
            <line choice=\"chat.tox.qtox\"/>
        </line>
    </choices-outline>
    <allowed-os-versions>
    <os-version min=\"10.7\"/>
</allowed-os-versions>
    <choice id=\"default\"/>
    <choice id=\"chat.tox.qtox\" visible=\"false\">
        <pkg-ref id=\"chat.tox.qtox\"/>
    </choice>
    <pkg-ref id=\"chat.tox.qtox\" version=\"1\" onConclusion=\"none\">qtox.pkg</pkg-ref>
</installer-gui-script>" > distribution.xml
