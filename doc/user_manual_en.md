# qTox User Manual
## Index

* [User Profile](#user-profile)
* [Settings](#settings)
* [Groupchats](#groupchats)
* [Message Styling](#message-styling)
* [Multi Window Mode](#multi-window-mode)
* [Keyboard Shortcuts](#keyboard-shortcuts)

## User Profile
Your User Profile contains everything you share with other people on Tox. You can open it by clicking the picture in the top left corner. It contains the following settings:

### Public Information
* _Name:_ This is your nickname which everyone who has your Tox ID can see.
* _Status:_ You can post a status message here, which again everyone on who has your Tox ID can see.

### Tox ID

The long code in hexadecimal format is your Tox ID, share this with everyone you want to talk to. Click it to copy it to your clipboard.
Your Tox ID is also shown as QR code to easily share it with friends over a smartphone.

The "Save image" button saves the QR code into a image file, while the "Copy image" button copies into your clipboard.

### Profile

qTox allows you to use multiple Tox IDs with different profiles, each of which can have different nicknames, status messages and friends.

+ _Current profile:_ Shows the filename which stores your information.
+ _Current profile location:_ Shows the path to the profile file.
+ _Rename:_ Allows you to rename your profile. Your nickname and profile name don't have to be the same.
+ _Delete:_ Deletes your profile and the corresponding chat history.
+ _Export:_ Allows you to export your profile in a format compatible with other Tox clients. You can also manually back up your \*.tox files.
+ _Logout:_ Close your current profile and show the login window.
+ _Remove password:_ Removes the existing password for your profile. If the profile already has no password, you will be notified.
+ _Change password:_ Allows you to either change an existing password, or create a new password if your profile does not have one.

## Settings
### General

#### General Settings
* _Language:_ Changes which language the qTox interface uses.
* _Autostart:_ If set, qTox will start when you login on your computer. qTox will also automatically open the profile which was active when you ticked the checkbox, but this only works if your profile isn't encrypted (has no password set).
* _Light icon:_ If set, qTox will use a different icon, which is easier to see on black backgrounds.
* _Show system tray icon:_ If set, qTox will show its icon in your system tray.
    * _Start in tray:_ On start, qTox will only show its tray icon and no window.
    * _Minimize to tray:_ The minimize button on the top right, will minimize qTox to its tray icon. There won't be a taskbar item.
* _Auto away after (0 to disable):_ After the specified amount of time, qTox will set your status to "Away". A setting of 0 will never change your status.
* _Default directory to save files:_ Allows you to specify the default destination for incoming file transfers.
* _Autoaccept files:_ If set, qTox will automatically accept file transfers and put them in the directory specified above.

#### Chat
* _Play sound:_ If checked, qTox will play a sound when you get a new message.
* _Open window:_ If checked, the qTox window will be opened when you receive a new message. If you use the multiple windows mode, see [Multi Window Mode](#multi-window-mode) for details.
    * _Focus window:_ If checked, the qTox window will additionally be focused when you receive a new message.
* _Show contacts' status changes:_ If set, qTox will show contact status changes in your chat window.
* _Group chats always notify:_ If set, qTox will notify you on every new message in a groupchat.
* _Place groupchats at top of friend list:_ If checked, your groupchats will be at the top of the contacts list instead of being sorted with your other contacts.
* _Faux offline messaging:_ If enabled, qTox will attempt to send messages when a currently offline contact comes online again.
* _Compact contact list:_ If set, qTox will use a contact list layout which takes up less screen space.
* _Multiple windows mode:_ If enabled, the qTox user interface will be split into multiple independent windows. For details see [Multi Window Mode](#multi-window-mode).
    * _Open each chat in an individual window:_ If checked, a new window will be opened for every chat you open. If you manually grouped the chat into another window, the window which hosts the chat will be focused.

#### Theme
* _Use emoticons:_ If enabled, qTox will replace smileys ( e.g. `:-)` )  with corresponding graphical emoticons.
* _Smiley Pack:_ Allows you to choose from different sets of shipped emoticon styles.
* _Emoticon size:_ Allows you to change the size of the emoticons.
* _Style:_ Changes the appearance of qTox.
* _Theme color:_ Changes the colors qTox uses.
* _Timestamp format:_ Change the format in which qTox displays message timestamps.
* _Date format:_ Same as above for the date.

#### Connection Settings
* _Enable IPv6 (recommended):_ If enabled, qTox will use IPv4 and IPv6 protocols, whichever is available. If disabled, qTox will only use IPv4.
* _Enable UDP (recommended):_ If enabled, qTox will use TCP and UDP protocols. If disabled, qTox will only use TCP, which lowers the amount of open connections and slightly decreases required bandwidth, but is also slower and puts more load on other network participants.

Most users will want both options enabled, but if qTox negatively impacts your router or connection, you can try to disable them.

* _Proxy type:_ If you want to use a proxy, set the type here. "None" disables the proxy.
* _Address:_ If you use a proxy, enter the address here.
* _Port:_ If you use a proxy, enter the port here.
* _Reconnect:_ Reconnect to the Tox network, e.g. if you changed the proxy settings.

### Privacy

* _Send typing notifications:_ If enabled, notify your chat partner when you are currently typing.
* _Keep chat history:_ If enabled, qTox will save your sent and received messages. Encrypt your profile, if you want to encrypt the chat history.

#### NoSpam

NoSpam is a feature of Tox that prevents a malicious user from spamming you with friend requests. If you get spammed, enter or generate a new NoSpam value. This will alter your Tox ID. You don't need to tell your existing contacts your new Tox ID, but you have to tell new contacts your new Tox ID. Your Tox ID can be found in your [User Profile](#user-profile).

### Audio/Video
#### Audio Settings
* _Playback device:_ Select the device qTox should use for all audio output (notifications, calls, etc).
* _Playback:_ Here you can adjust the playback volume to your needs.
* _Capture device:_ Select the device qTox should use for audio input in calls.
* _Microphone:_ Set the input volume of your microphone with this slider. When you are talking normally, the displayed volume indicator should be in the green range.
* _Filter audio:_ If enabled, qTox will try to remove noise and echo from your audio input.

#### Video Settings
* _Video device:_ Select the video device qTox should use for video calls. "None" will show a dummy picture to your chat partner. "Desktop" will stream the content of your screen.
* _Resolution:_ You can select from the available resolutions and frame rates here. Higher resolutions provide more quality, but if the bandwidth of your connection is low, the video may get choppy.

  If you set up everything correctly, you should see the preview of your video device in the box below.

* _Rescan devices:_ Use this button to search for newly attached devices, e.g. you plugged in a webcam.

### Advanced
* _Make Tox portable:_ If enabled, qTox will load/save user data from the working directory, instead of ` ~/.config/tox/ `.
* _Reset to default settings:_ Use this button to revert any changes you made to the qTox settings.

### About
* _Version:_ Shows the version of qTox and the libraries it depends on. Please append this information to every bug report.
* _License:_ Shows the license under which the code of qTox is available.
* _Authors:_ Lists the people who developed this shiny piece of software.
* _Known Issues:_ Links to our list of known issues and improvements.

## Groupchats

Groupchats are a way to talk with multiple friends at the same time, like when you are standing together in a group. To create a groupchat click the groupchat icon in the bottom left corner and set a name. Now you can invite your contacts by right-clicking on the contact and selecting "Invite to group". Currently, if the last person leaves the chat, it is closed and you have to create a new one. Videochats and file transfers are currently unsupported in groupchats.

## Message Styling

Similar to other messaging applications, qTox supports stylized text formatting. Formatting follows [Markdown syntax](), thus:

* For **Bold**, surround text in double asterisks or underscores: `**text**` or `__text__`
* For **Italics**, surround text in single asterisks or underscores: `*text*` or `_text_`
* For **Strikethrough**, surround text in single tilde's: `~text~`
* For **Underline**, surround text in single dashes: `-text-`

Additionally, qTox supports three modes of Markdown parsing:

* `Plaintext`: No text is stylized
* `Show Formatting Characters`: Stylize text while showing formatting characters (Default)
* `Don't Show Formatting Characters`: Stylize text without showing formatting characters

*Note that any change in Markdown preference will require a restart.*

## Multi Window Mode

In this mode, qTox will separate its main window into a single contact list and one or multiple chat windows, which allows you to have multiple conversations on your screen at the same time. Additionally you can manually group chats into a window by dragging and dropping them onto each other. This mode can be activated and configured in [settings](#settings).

## Keyboard Shortcuts

The following shortcuts are currently supported:

| Shortcut | Action |
|----------|--------|
| ``Arrow up`` | Paste last sent message |
| ``CTRL`` + ``SHIFT`` + ``L`` | Clear chat |
| ``CTRL`` + ``q`` | Quit qTox |
| `CTRL` + `Page Down` | Switch to the next contact |
| `CTRL` + `Page Up` | Switch to the previous contact|
| `CTRL` + `TAB` | Switch to the next contact |
| `CTRL` + `SHIFT` + `TAB` | Switch to the previous contact|
