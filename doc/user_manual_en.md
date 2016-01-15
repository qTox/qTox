# qTox User Manual
## Index

* [User Profile](#user-profile)
* [Settings](#settings)
* [Groupchats](#groupchats)
* [Multi Window Mode](#multi-window-mode)
* [Keyboard Shortcuts](#keyboard-shortcuts)

## User Profile
Your User Profile contains everything you share with other people on Tox. You can open it by clicking the picture on the top right corner. It contains the following settings:

### Public Information
* _Name:_ This is your nickname which everyone who has your Tox ID can see.
* _Status:_ You can post a status message here, which again everyone on who has your ToxID can see.

### Tox ID

The long code in hexadecimal format is your Tox ID, share this with everyone you want to talk to. Click it to copy it to your clipboard.
Your Tox ID is also shown as QR-Code to easily share it with friends over a smartphone.

The "Save image" button saves the QR-Code into a image file, while the "Copy image" button copies into your clipboard.

### Profile

qTox allows you to use multiple Tox IDs with different profiles, which each can have different nicknames, status messages and friends.

+ _Current profile:_ Shows the filename which stores your information.
+ _Current profile locatation:_ Shows the path to the profile file.
+ _Rename_ This button allows you to rename your profile. Your nickname and profile name don't have to be the same.
+ _Delete_ This button deletes your profile.
+ _Export_ Allows you to export your profile in a format compatible with other Tox clients. You can also manually backup your *.tox files.
+ _Logout_ Close your current profile and show the login window.
+ _Remove password_ If you set a password for your profile, you can remove it with this button.
+ _Change password_ If you set a password for your profile, you can change the password with this button. You can also set a password for unencrypted profiles here.

## Settings
### General

#### General Settings
* _Language:_ Changes which language the qTox interface uses.
* _Autostart:_ If set, qTox will start when you login on your computer. qTox will also autmatically open the profile which was active when you ticked the checkbox, but this only works if your profile isn't encrypted (has no password set).
* _Light icon:_ If set, qTox will use a different icon, which is easier to see on black backgrounds.
* _Show system tray icon:_ If set, qTox will show its icon in your system tray.
    * _Start in tray:_ On start, qTox will only show its tray icon and no window.
    * _Minimize to tray:_ The minimize button on the top right, will minimize qTox to its tray icon. There won't be a taskbar item.

* _Auto away after (0 to disable):_ After the specified amount of time, qTox will set your Status to "Away". A setting of 0 will never change your status.
* _Default directory to save files:_ You can set the directory where qTox puts files you recieved here.
* _Autoaccept files:_ If set, qTox will automatically accept filetransfers and put them in the directory specified above.

#### Chat
* _Play sound:_ If checked, qTox will play a sound when you get a new message.
* _Open window:_ If checked, the qTox window will be opened. If you use the multiple windows mode, see [Multi Window Mode](#multi-window-mode) for details.
    * _Focus window:_ If checked, the qTox window will additionally be focused.
* _Show contacts' status changes:_ If set, qTox will show contact status changes in your chat history.
* _Group chats always notify:_ If set, qTox will notify you on every new message in a groupchat.
* _Place groupchats at top of friend list:_ If checked, your groupchats will be at the top of the contacts list, else they will be sorted with your other contacts.
* _Faux offline messaging:_ If enabled, qTox will attempt to send messages when a currently offline contact comes online again.
* _Compact contact list:_ If set, qTox will use a contact list layout which takes up less screen space.
* _Multiple windows mode:_ If enabled, the qTox user interface will be split into multiple independent windows. For details see [Multi Window Mode](#multi-window-mode).
    * _Open each chat in an individual window:_ If checked, a new window will be opened for every chat you open. If you manually grouped the chat into another window, the window which hosts the chat will be focused.

#### Theme
* _Use emoticons:_ If enabled, qTox will replace simleys ( e.g. :-) )  with Unicode emoticons.
* _Smiley Pack:_ You can choose from different sets of shiped emoticon styles. You can also install your own, see ??? for details<!-- TODO: add instructions for adding own smiley packs-->
* _Emoticon size:_ You can change the size of the emoticons here.
* _Style:_ Changes the appearance of qTox.
* _Theme color:_ Changes the colors qTox uses.
* _Timestamp format:_ Change the format in which qTox displays message timestamps.
* _Date format:_ Same as above for the date.

#### Connection Settings
* _Enable IPv6 (recommended):_ If enabled, qTox will use IPv4 and IPv6 protocols, whichever is available. If disabled, qTox will only use IPv4.
* _Enable UDP (recommended):_ If enabled, qTox will use TCP and UDP protocols. If disabled, qTox will only use TCP, which is supposed to lower the amount of traffic, but is also slower and puts more load on other network participants.
Most users will want the two settings enabled, but if qTox crashes your router, you can try to disable them. 

* _Proxy type:_ If you want to use a proxy, set the type here. "None" disables the proxy.
* _Address:_ If you use a proxy, enter the address here.
* _Port:_ If you use a proxy, enter the port here.
* _Reconnect:_ Reconnect to the tox network, e.g. if you changed the proxy settings.

### Privacy

* _Send typing notifications:_ If enabled, notify your chat partner when you are currently typing.
* _Keep chat history:_ If enabled, qTox will save your sent and recieved messages. Encrypt your profile, if you want to encrypt the chat history.

#### NoSpam

NoSpam is a feature of Tox to prevent someone spamming you with friend requests. If you get spammed, enter or generate a new NoSpam value, this will alter your Tox ID. You don't need to tell your existing contacts your new Tox ID, but you have to tell new contacts your new Tox ID. Your Tox ID can be found ??? <!-- TODO: link to page describing your profile-->

### Audio/Video
#### Audio Settings
* _Playback device:_ Select the device qTox should use for all audio output (notifications, calls,..).
* _Playback:_ Here you can adjust the playback volume to your needs.
* _Capture device:_ Select the device qTox should use for audio input in calls.
* _Microphone:_ Set the input volume of your microphone with this slider. When you are talking normaly, the display should be in the green range.
* _Filter audio_ If enabled, qTox will try to remove noise and echo from your audio input.

#### Video Settings
* _Video device:_ Select the video device qTox should use for video calls. "None" will show a dummy picture to your chat partner. "Desktop" will stream the content of your screen.
* _Resolution:_ You can select from the available resolutions and framerates here. Higher resolutions provide more quality, but if the bandwidth of your connection is low, the video may get choppy.

  If you set up everything correctly, you should see the preview of your video device in the box.

* _Rescan devices_ Use this button to search for newly attached devices, e.g. you plugged in a webcam.

### Advanced
* _Make Tox portable:_ If enabled, qTox will load/save user data from the working directory, instead of ``` ~/.config/tox/ ```.
* _Reset to default settings_ Use this button to delete any changes you made to the qTox settings.

### About
* _Version_ The version of qTox and the libraries it depends on. Please append this information to every bugreport.
* _License_ The license under which the code of qTox is available.
* _Authors_ The people who developed this shiny piece of software.
* _Known Issues_ Links to our list of known issues and improvements.

## Groupchats

Groupchats are a way to talk with multiple friends at the same time, like when you are standing together in a group. To create a groupchat click the groupchat icon <!-- TODO: add icon here --> and set a name. Now you can invite people by right clicking on the contact and selecting "Invite to group". Currently, if the last person leaves the chat, it is closed and you have to create a new one. <!-- TODO: add doc about audio/video/filetransfer support-->

## Multi Window Mode

If you activated this mode in the [settings](#settings), qTox will seperata its main window into a contact list and chat windows, which allows you to have multiple conversations on your screen on the same time. Additionally you can manually group chats into a window by dragging and dropping them onto eachother.

## Keyboard Shortcuts

The following shortcuts are currently supported:

| Shortcut | Action |
|----------|--------|
| ``Arrow up`` | Paste last message |
| ``CTRL`` + ``SHIFT`` + ``L`` | Clear chat |
| ``CTRL`` + ``q`` | Quit qTox |
| `CTRL` + `Page Down` | Switch to the next contact |
| `CTRL` + `Page Up` | Switch to the previous contact|
| `CTRL` + `TAB` | Switch to the next contact |
| `CTRL` + `SHIFT` + `TAB` | Switch to the previous contact|
