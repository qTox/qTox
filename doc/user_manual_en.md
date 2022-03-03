# qTox User Manual
## Index

* [Profile corner](#profile-corner)
* [Contact list](#contact-list)
* [User Profile](#user-profile)
* [Settings](#settings)
* [Groupchats](#groupchats)
* [Message Styling](#message-styling)
* [Quotes](#quotes)
* [Multi Window Mode](#multi-window-mode)
* [Keyboard Shortcuts](#keyboard-shortcuts)
* [Commandline Options](#commandline-options)
* [Emoji Packs](#emoji-packs)
* [Bootstrap Nodes](#bootstrap-nodes)
* [Avoiding Censorship](#avoiding-censorship)


## Profile corner

Located in the top left corner of the main window.

* __Avatar__: picture that is shown to your contacts. Clicking on it will open
  [user profile] where you can change it.
* __Name__: your name shown to your contacts. Clicking on it will open [user
  profile] where you can change it.
* __Status message__: your status message shown to your contacts. Click on it
  to change it.
* __Status orb__: colored orb button that shows your current status. Click on
  it to change it.

### Status

Status can be one of:

* `Online` – green
* `Away` – yellow
* `Busy` – red
* `Offline` – gray, set automatically by qTox when there is no connection to
  the Tox network.


## Contact list

Located on the left, below the [profile corner]. Can be sorted e.g. `By
Activity`.

`By Activity` sorting in qTox is updated whenever client receives something that
is directly aimed at you, and not sent to everyone, that is:

* audio/video call
* file transfer
* groupchat invite
* message

**Not** updated on:

* avatar change
* groupchat message
* name change
* [status](#status) change
* status message change

### Contact menu

Can be accessed by right-clicking on a contact or [circle](#circles). When
right-clicking on a contact a menu appears that has the following options:

* __Open chat in a new window:__ opens a new window for the chosen contact.
* __Invite to group:__ offers an option to create a new groupchat and
  automatically invite the friend to it or to an already existing groupchat.
* __Move to circle:__ offers an option to move friend to a new
  [circle](#circles), or to an existing one.
* __Set alias:__ set alias that will be displayed instead of contact's name.
* __Auto accept group invites:__ if enabled, all group chat invites from this
  friend are automatically accepted.
* __Auto accept files from this friend:__ option to automatically save files
  from the selected contact in a chosen directory.
* __Remove friend:__ option to remove the contact. Confirmation is needed to
  remove the friend.
* __Show details:__ show [details](#contact-details) of a friend.

#### Circles

Circles allow you to group contacts together and give this circle a name.
Contacts can be in one or in no circle.

#### Contact details

Contact details can be accessed by right-clicking on a contact and picking the
`Show details` option.

Some of the informations listed are:

* avatar
* name
* status message
* Public Key (PK) – note that PK is only a part of Tox ID and alone can't be
  used to add the contact e.g. on a different profile. Part not known by qTox
  is the [NoSpam](#nospam).


## User Profile

To access it, click on __Avatar__/__Name__ in the [profile corner].

Your User Profile contains everything you share with other people on Tox. You
can open it by clicking the picture in the top left corner. It contains the
following settings:

### Public Information

* __Name:__ This is your nickname which everyone who is on your contact list can
  see.
* __Status:__ You can post a status message here, which again everyone on your
  contact list can see.

#### Avatar

Your profile picture that all your friends can see. To add or change, click on
the avatar. To remove, right-click.

### Tox ID

The long code in hexadecimal format is your Tox ID, share this with everyone you
want to talk to. Click it to copy it to your clipboard. Your Tox ID is also
shown as QR code to easily share it with friends over a smartphone.

The "Save image" button saves the QR code into a image file, while the "Copy
image" button copies into your clipboard.

### Register on ToxMe

An integration for ToxMe service providers that allows you to create a simple
alias for your Tox ID that will look like `user@example.com`. A default service
provider (toxme.io) is already listed, and you can add your own.

* __Username:__ This will be used as an alias for your Tox ID.
* __Biography:__ Optional. If you want, you can write something here.
* __Server:__ Service address where your alias will be registered.

Note that by default aliases are public, but you can check the option to make a
private one, but given that it would be stored on a server that you don't
control, it's not actually private. At best you have a promise of privacy from
the owner of the server. For 100% privacy use Tox IDs.

After registration, you can give your new alias, e.g. `user@example.com` to
your friends instead of the long Tox ID.

### Profile

qTox allows you to use multiple Tox IDs with different profiles, each of which
can have different nicknames, status messages and friends.

+ __Current profile:__ Shows the filename which stores your information.
+ __Current profile location:__ Shows the path to the profile file.
+ __Rename:__ Allows you to rename your profile. Your nickname and profile name
  don't have to be the same.
+ __Delete:__ Deletes your profile and the corresponding chat history.
+ __Export:__ Allows you to export your profile in a format compatible with
  other Tox clients. You can also manually back up your \*.tox files.
+ __Logout:__ Close your current profile and show the login window.
+ __Remove password:__ Removes the existing password for your profile. If the
  profile already has no password, you will be notified.
+ __Change password:__ Allows you to either change an existing password, or
  create a new password if your profile does not have one.

## Friends' options
In the friend's window you can customize some options for this friend specifically.
* _Auto answer:_ chooses the way to autoaccept audio and video calls.
	* _Manual:_ All calls must be manually accepted.
	* _Audio:_ Only audio calls will be automatically accepted. Video calls must be manually accepted.
	* _Audio + video:_ All calls will be automatically accepted.

## Settings
### General

* __Language:__ Changes which language the qTox interface uses.
* __Autostart:__ If set, qTox will start when you login on your computer. qTox
  will also automatically open the profile which was active when you ticked the
  checkbox, but this only works if your profile isn't encrypted (has no
  password set).
* __Light icon:__ If set, qTox will use a different icon, which is easier to
  see on black backgrounds.
* __Show system tray icon:__ If set, qTox will show its icon in your system
  tray.
    * __Start in tray:__ On start, qTox will only show its tray icon and no
      window.
    * __Minimize to tray:__ The minimize button on the top right, will minimize
      qTox to its tray icon. There won't be a taskbar item.
    * __Close to tray__: The close button on the top right will minimize
      qTox to its tray icon. There won't be a taskbar item.
* __Play sound:__ If checked, qTox will play a sound when you get a new
  message.
    * __Play sound while Busy__: If checked, qTox will play a sound even
      when your status is set to `Busy`.
* __Show contacts' status changes:__ If set, qTox will show contact status
  changes in your chat window.
* __Faux offline messaging:__ If enabled, qTox will attempt to send messages
  when a currently offline contact comes online again.
* __Auto away after (0 to disable):__ After the specified amount of time, qTox
  will set your status to "Away". A setting of 0 will never change your status.
* __Default directory to save files:__ Allows you to specify the default
  destination for incoming file transfers.
* __Autoaccept files:__ If set, qTox will automatically accept file transfers
  and put them in the directory specified above.

### User Interface
#### Chat

* __Base font:__ You can set a non-default font and its size for the chat. The
  new font setting will be used for new messages and all messages after qTox
  has been restarted.
* __Text Style format:__ see [Message styling](#message-styling) section.

#### New message

* __Open window:__ If checked, the qTox window will be opened when you receive a
  new message. If you use the multiple windows mode, see
  [Multi Window Mode](#multi-window-mode) for details.
    * __Focus window:__ If checked, the qTox window will additionally be focused
      when you receive a new message.

#### Contact list

* __Group chats always notify:__ If set, qTox will notify you on every new
  message in a groupchat.
* __Place groupchats at top of friend list:__ If checked, your groupchats will
  be at the top of the contacts list instead of being sorted with your other
  contacts.
* __Compact contact list:__ If set, qTox will use a contact list layout which
  takes up less screen space.
* __Multiple windows mode:__ If enabled, the qTox user interface will be split
  into multiple independent windows. For details see [Multi Window
  Mode](#multi-window-mode).
    * __Open each chat in an individual window:__ If checked, a new window will
      be opened for every chat you open. If you manually grouped the chat into
      another window, the window which hosts the chat will be focused.

#### Emoticons

* __Use emoticons:__ If enabled, qTox will replace smileys ( e.g. `:-)` )  with
  corresponding graphical emoticons.
* __Smiley Pack:__ Allows you to choose from different sets of shipped emoticon
  styles.
* __Emoticon size:__ Allows you to change the size of the emoticons.

#### Theme

* __Style:__ Changes the appearance of qTox.
* __Theme color:__ Changes the colors qTox uses.
* __Timestamp format:__ Changes the format in which qTox displays message
  timestamps.
* __Date format:__ Same as above for the date.

### Privacy

* __Send typing notifications:__ If enabled, notify your chat partner when you
  are currently typing.
* __Keep chat history:__ If enabled, qTox will save your sent and received
  messages. Encrypt your profile, if you want to encrypt the chat history.
  ***Note*** that disabling history disables `Faux offline messaging`. With
  disabled history qTox doesn't store messages, so it can't try to re-send
  them.

#### NoSpam

NoSpam is a feature of Tox that prevents a malicious user from spamming you with
friend requests. If you get spammed, enter or generate a new NoSpam value. This
will alter your Tox ID. You don't need to tell your existing contacts your new
Tox ID, but you have to tell new contacts your new Tox ID. Your Tox ID can be
found in your [User Profile](#user-profile).

#### BlackList

BlackList is a feature of qTox that locally blocks a group member's messages across all your joined groups, in case someone spams a group. You need to put a members public key into the BlackList text box one per line to activate it. Currently qTox doesn't have a method to get the public key from a group member, this will be added in the future.

### Audio/Video
#### Audio Settings

* __Playback device:__ Select the device qTox should use for all audio output
  (notifications, calls, etc).
* __Volume:__ Here you can adjust the playback volume to your needs.
* __Capture device:__ Select the device qTox should use for audio input in
  calls.
* __Gain:__ Set the input volume of your microphone with this slider. When you
  are talking normally, the displayed volume indicator should be in the green
  range.

#### Video Settings

* __Video device:__ Select the video device qTox should use for video calls.
  "None" will show a dummy picture to your chat partner. "Desktop" will stream
  the content of your screen.
* __Resolution:__ You can select from the available resolutions and frame rates
  here. Higher resolutions provide more quality, but if the bandwidth of your
  connection is low, the video may get choppy.

If you set up everything correctly, you should see the preview of your video
device in the box below.

* __Rescan devices:__ Use this button to search for newly attached devices, e.g.
  you plugged in a webcam.

### Advanced
#### Portable

* __Make Tox portable:__ If enabled, qTox will load/save user data from the
  working directory, instead of ` ~/.config/tox/ `.

#### Connection Settings

* __Enable IPv6 (recommended):__ If enabled, qTox will use IPv4 and IPv6
  protocols, whichever is available. If disabled, qTox will only use IPv4.
* __Enable UDP (recommended):__ If enabled, qTox will use TCP and UDP protocols.
  If disabled, qTox will only use TCP, which lowers the amount of open
  connections and slightly decreases required bandwidth, but is also slower and
  puts more load on other network participants.

Most users will want both options enabled, but if qTox negatively impacts your
router or connection, you can try to disable them.

* __Proxy type:__ If you want to use a proxy, set the type here. "None" disables
  the proxy.
* __Address:__ If you use a proxy, enter the address here.
* __Port:__ If you use a proxy, enter the port here.
* __Reconnect:__ Reconnect to the Tox network, e.g. if you changed the proxy
  settings.

---

* __Reset to default settings:__ Use this button to revert any changes you made
  to the qTox settings. *Note that current implentation [is
  buggy](https://github.com/qTox/qTox/issues/3664) and aside from settings also
  friend [aliases] will be removed!*

### About
* __Version:__ Shows the version of qTox and the libraries it depends on. Please
  append this information to every bug report.
* __License:__ Shows the license under which the code of qTox is available.
* __Authors:__ Lists the people who developed this shiny piece of software.
* __Known Issues:__ Links to our list of known issues and improvements.

## Groupchats

Groupchats are a way to talk with multiple friends at the same time, like when
you are standing together in a group. To create a groupchat click the groupchat
icon in the bottom left corner and set a name. Now you can invite your contacts
by right-clicking on the contact and selecting "Invite to group". Currently, if
the last person leaves the chat, it is closed and you have to create a new one.
Videochats and file transfers are currently unsupported in groupchats.

## Message Styling

Similar to other messaging applications, qTox supports stylized text formatting.

* For **Bold**, surround text in single or double asterisks: `*text*`
 or `**text**`
* For **Italics**, surround text in single or double forward slashes: `/text/`
 or `//text//`
* For **Strikethrough**, surround text in single or double tilde's: `~text~`
 or `~~text~~`
* For **Underline**, surround text in single or double underscores: `_text_`
 or `__text__`
* For **Code**, surround your code in in single backticks: `` `text` ``

Additionally, qTox supports three modes of Markdown parsing:

* `Plaintext`: No text is stylized
* `Show Formatting Characters`: Stylize text while showing formatting characters
  (Default)
* `Don't Show Formatting Characters`: Stylize text without showing formatting
  characters

*Note that any change in Markdown preference will require a restart.*

qTox also supports action messages by prefixing a message with `/me`, where
`/me` is replaced with your current username. For example `/me likes cats`
turns into *` * qTox User likes cats`*.

## Quotes

qTox has feature to quote selected text in chat window:

1. Select the text you want to quote.
2. Right-click on the selected text and choose "Quote selected text" in the
context menu. You also can use `ALT` + `q` shortcut.
3. Selected text will be automatically quoted into the message input area in a
pretty formatting.

## Friend- and Groupinvites

To invite a friend to a chat with you, you have to click the `+` button on the
bottom left of the qTox window. The "Add a friend" Tab allows you to enter the
Tox ID of your friend, or the username of a [ToxMe service] if your friend
registered there.

On the "Friend requests" tab you can see, friend requests you got from other
Tox users. You can then choose to either accept or decline these requests.

On the Groupinvites page, you can create a new groupchat and add users to it by
using the context menu in your contact list. Invites from your contacts are
also displayed here and you can accept and decline them.

## Multi Window Mode

In this mode, qTox will separate its main window into a single contact list and
one or multiple chat windows, which allows you to have multiple conversations on
your screen at the same time. Additionally you can manually group chats into a
window by dragging and dropping them onto each other. This mode can be activated
and configured in [settings](#settings).

## Keyboard Shortcuts

The following shortcuts are currently supported:

| Shortcut                     | Action                         |
|------------------------------|--------------------------------|
| `Arrow up`                   | Paste last sent message        |
| `CTRL` + `SHIFT` + `L`       | Clear chat                     |
| `CTRL` + `q`                 | Quit qTox                      |
| `CTRL` + `Page Down`         | Switch to the next contact     |
| `CTRL` + `Page Up`           | Switch to the previous contact |
| `CTRL` + `TAB`               | Switch to the next contact     |
| `CTRL` + `SHIFT` + `TAB`     | Switch to the previous contact |
| `CTRL` + `p`                 | [Push to talk](#push-to-talk)  |
| `ALT` + `q`                  | Quote selected text            |
| `F11`                        | Toggle fullscreen mode         |

## Push to talk

In audio group chat microphone mute state will be changed while `Ctrl` +
`p` pressed and reverted on release.

## Commandline Options

| Option                             | Action                                             |
|------------------------------------|----------------------------------------------------|
| `-p` `<profile>`                   | Use specified unencrypted profile                  |
| `-l`                               | Start with loginscreen                             |
| `-I` `<on/off>`                    | Sets IPv6 toggle [Default: ON]                     |
| `-U` `<on/off>`                    | Sets UDP toggle [Default: ON]                      |
| `-L` `<on/off>`                    | Sets LAN toggle [Default: ON]                      |
| `-P` `<protocol>:<address>:<port>` | Applies [proxy options](#commandline-proxy-options) [Default: NONE]|

### Commandline Proxy Options
Protocol: NONE, HTTP or SOCKS5 <br>
Address: Proxy address <br>
Port: Proxy port number (0-65535) <br>
Example input: <br>
`qtox -P SOCKS5:192.168.0.1:2121` <br>
`qtox -P none`

## Emoji Packs

qTox provides support for custom emoji packs. To install a new emoji pack
put it in `%LOCALAPPDATA%/emoticons` for Windows or `~/.local/share/emoticons`
for Linux. If these directories don't exist, you have to create them. The emoji
files have to be in a subfolder also containing `emoticon.xml`, see the
structure of https://github.com/qTox/qTox/tree/v1.5.2/smileys for further
information.

## Bootstrap Nodes

qTox uses bootstrap nodes to find its way in to the DHT. The list of nodes is
stored in `~/.config/tox/` on Linux, `%APPDATA%\Roaming\tox` on Windows, and
`~/Library/Application Support/Tox` on macOS. `bootstrapNodes.example.json`
stores the default list. If a new list is placed at `bootstrapNodes.json`, it
will be used instead.

## Avoiding Censorship

Although Tox is distributed, to initially connect to the network
[public bootstrap nodes](https://nodes.tox.chat) are used. After first run,
other nodes will also be saved and reused on next start. We have seen multiple
reports of Tox bootstrap nodes being blocked in China. We haven't seen reports
of Tox connections in general being blocked, though Tox makes no effort to
disguise its connections. There are multiple options available to help avoid
blocking of bootstrap nodes:

* Tox can be used with a VPN.
* Tox can be used with a proxy, including with Tor
  * This can be done at [startup](#commandline-proxy-options) or
  * By setting [connection settings](#connection-settings).
* [Custom bootstrap nodes](#bootstrap-nodes) can be set. Note that these
require the DHT key of the node, which is different from the longterm Tox
public key, and which changes on every start of a client, so it's best to use a
[bootstrap daemon](https://github.com/TokTok/c-toxcore/tree/master/other/bootstrap_daemon).

[ToxMe service]: #register-on-toxme
[user profile]: #user-profile
[profile corner]: #profile-corner

# Extensions

qTox supports extra features through the use of extensions to the tox protocol. Not all contacts are going to support these extensions.

For most cases you won't have to do anything, but you may wonder why behavior of chats is different for some friends. There is a puzzle piece icon to the left of your contact's name in the top of a chat. If it's green that means that they support all the features qTox cares about. If it's yellow it means some of the features are supported. If it's red it means that they don't support any extensions.

You can hover over the icon to see which extensions they support. qTox should dynamically enable/disable features based on the extension set of your friend.
