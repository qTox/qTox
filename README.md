qTox
======

Powerful Tox client that tries to follow the Tox UI mockup while running on all major systems. <br/>
This GUI uses code from @nurupo'tos ProjectTox-Qt-GUI, in particular the "Core" Toxcore wrapper. <br/>
However, it is not a fork.

<h2>Features</h2>

- One to one chat with friends
- Group chats
- File transfers, with previewing of images
- Audio calls
- Video calls (alpha)
- Tox DNS
- Translations in various languages

<h2>Requirements</h2>

This client runs on Windows, Linux and Mac natively, but is not build regularly for Linux <br/>
Linux users will have to compile the source code themselves if they want the latest updates.

<a href="https://jenkins.libtoxcore.so/job/tux3-toxgui-win32/lastSuccessfulBuild/artifact/toxgui-win32.zip">Windows download</a><br/>
<a href="https://jenkins.libtoxcore.so/job/ToxGUI%20OS%20X/lastSuccessfulBuild/artifact/toxgui.dmg">Mac download </a><br/>
<a href="https://mega.co.nz/#!9l5B0QqZ!O2glB8XE_Tcf4zTub2WEk-_9Ra43RoeiFV-AQBKDZJU">Linux download (12st July 2014 20:30 GMT)</a><br/>
Note that the Linux download has not been tested and may not be up to date.<br/>

<h3>Screenshots</h3>
<h5>Note: The screenshots may not always be up to date, but they should give a good idea of the general look and features</h5>
<img src="http://i.imgur.com/mMUdr6u.png"/>
<img src="http://i.imgur.com/66ARBGC.png"/>

<h3>Compiling</h3>
Compiling toxgui requires Qt 5.2 with the Qt Multimedia module and a C++11 compatible compiler. 
It also requires the toxcore and toxav libraries.

To compile, first clone or download the qTox repository and open a terminal in the qTox folder.
Then run the script bootstrap.sh (for Linux and Mac) or bootsrap.bat (for Windows) to download an up-to-date toxcore.
And finally run the commands "qmake" and "make" to start building qTox.


<h3>OSX Install Guide</h3>

<strong>This guide is intended for people who wish to use an existing or new ProjectTox-Core installation separate to the bundled installation with qTox, if you do not wish to use a separate installation you can skip to the section titled 'Final Steps'.</strong>

Installation on OSX, isn't quite straight forward, here is a quick guide on how to install;

The first thing you need to do is install ProjectTox-Core with a/v support. Refer to the INSTALL guide in the ProjectTox-Core github repo.

Next you need to download QtTools (http://qt-project.org/downloads), at the time of writing this is at version 5.3.0.
Make sure you deselect all the unnecessary components from the 5.3 checkbox (iOS/Android libs) otherwise you will end up with a very large download.

Once that is installed you will most likely need to set the path for qmake. To do this, open up terminal and paste in the following;

```bash
export PATH=/location/to/qmake/binary:$PATH
```

For myself, the qmake binary was located in /Users/mouseym/Qt/5.3/clang_64/bin/.

This is not a permanent change, it will revert when you close the terminal window, to add it permanently you will need to add echo the above line to your .profile/.bash_profile.

Once this is installed, do the following;

```bash
git clone https://github.com/tux3/qTox
cd toxgui
qmake
```

Now, we need to create a symlink to /usr/local/lib/ and /usr/local/include/
```
mkdir -p $HOME/qTox/libs
sudo ln -s /usr/local/lib $HOME/qTox/libs/lib
sudo ln -s /usr/local/include  $HOME/qTox/libs/include
```
<h5>Final Steps</h5>

The final step is to run 
```bash
make
``` 
in the qTox directory, or if you are using the bundled tox core installation, you can use 
```bash
./bootstrap.sh
```
Assuming all went well you should now have a qTox.app file within the directory. Double click and it should open!
