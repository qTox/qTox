toxgui
======

For-fun Tox client that tries to follow the Tox UI mockup while running on all major systems. <br/>
This GUI uses code from @nurupo's ProjectTox-Qt-GUI, in particular the "Core" Toxcore wrapper. <br/>
However, it is not a fork.

<h2>Features</h2>

- One to one chat with friends
- Group chats
- File transfers, with previewing of images
- Audio calls
- Video calls
- Tox DNS
- Translations in various languages

<h2>Requirements</h2>

This client will run on Windows, Linux and Mac natively, but binairies are only be provided for Windows at the moment. <br/>
Linux and Mac users will have to compile the source code themselves.

<a href="https://jenkins.libtoxcore.so/job/tux3-toxgui-win32/lastSuccessfulBuild/artifact/toxgui-win32.zip">Windows download</a><br/>
<a href="http://speedy.sh/XXtHa/toxgui">Linux download (1st July 2014 01:30 GMT)</a><br/>
Note that the Linux download has not been tested and is not kept up to date.

<h3>Screenshots</h3>
<h5>Note: The screenshots may not always be up to date, but they should give a good idea of the general look and features</h5>
<img src="http://i.imgur.com/mMUdr6u.png"/>
<img src="http://i.imgur.com/66ARBGC.png"/>

<h3>Compiling</h3>
Compiling toxgui requires Qt 5.2 with the Qt Multimedia module and a C++11 compatible compiler. 
It also requires the toxcore and toxav libraries.

To compile, first clone or download the toxgui repository and open a terminal in the toxgui folder.
Then run the script bootstrap.sh (for Linux and Mac) or bootsrap.bat (for Windows) to download an up-to-date toxcore.
And finally run the commands "qmake" and "make" to start building toxgui.
