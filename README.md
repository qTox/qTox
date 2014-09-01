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

<h2>Downloads</h2>

This client runs on Windows, Linux and Mac natively.<br/>

<a href="https://jenkins.libtoxcore.so/job/tux3-toxgui-win32/lastSuccessfulBuild/artifact/toxgui-win32.zip">Windows download</a><br/>
<a href="https://jenkins.libtoxcore.so/job/ToxGUI%20OS%20X/lastSuccessfulBuild/artifact/qtox.dmg">Mac download </a><br/>
<a href="https://jenkins.libtoxcore.so/job/qTox-linux-amd64/">Linux download</a> (click "Last successful artifacts")<br/>

<h3>Screenshots</h3>
<h5>Note: The screenshots may not always be up to date, but they should give a good idea of the general look and features</h5>
<img src="http://i.imgur.com/mMUdr6u.png"/>
<img src="http://i.imgur.com/66ARBGC.png"/>

<h3>Compiling on GNU-Linux</h3>
<h4>Acquiring dependencies</h4>
Compiling qTox requires several dependencies, however these are easily installable
with your system's package manager. The step-by-step instructions assume Debian-style apt, but
it should be easy enough to get the packes with yum or pacman.

First, we need Qt 5.2 with a C++11 compatible compiler:
```bash
sudo apt-get install build-essential qt5-qmake qt5-default
```

toxcore and toxav, the client-agnostic network code for Tox, has several dependencies 
of its own (see <a href="https://github.com/irungentoo/toxcore/blob/master/INSTALL.md#unix">its installation guide for more details</a>):
```bash
sudo apt-get install libtool autotools-dev automake checkinstall check git yasm libopus-dev libvpx-dev
```

Finally, qTox itself requires OpenAL and OpenCV:
```bash
sudo apt-get install libopenal-dev libopencv-dev
```

<h4>Compilation</h4>

Having acquired all the dependencies, the following commands should get and compile qTox:

```bash
wget -O qtox.tgz https://github.com/tux3/qTox/archive/master.tar.gz
tar xvf qtox.tgz
cd qTox-master
./bootstrap.sh # This will automagically download and compile libsodium, toxcore, and toxav
qmake
make # Should compile to "qtox"
```

And that's it!

<h4>Building packages</h4>

qTox now has the experimental and probably-dodgy ability to package itself (in .deb
form natively, and .rpm form with <a href="http://joeyh.name/code/alien/">alien</a>).

After installing the required dependencies, run `bootstrap.sh` and then run the
`buildPackages.sh` script, found in the tools folder. It will automatically get the
necessary packages for building .debs, so be prepared to type your password for sudo.

<h3>OSX Easy Install</h3>

Since https://github.com/ReDetection/homebrew-qtox you can easily install qtox with homebrew 
```bash
brew install --HEAD ReDetection/qtox/qtox
```

<h3>OSX Full Install Guide</h3>

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
