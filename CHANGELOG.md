<a name=""></a>
## v1.4.1 (2016-05-09)

This release fixes an issue with the updater not installing updates correctly.
This update also fixes some problems with portable mode,
which could affect where the updater downloaded files.

#### Bug Fixes

* **settings:**  Look for portable setting in module path, not CWD ([95634f1c](https://github.com/qTox/qTox/commit/95634f1cfebdec0aadd71077175d8258a4e89d67))
* **updater:**  Use module path, not working dir ([0f1c8a78](https://github.com/qTox/qTox/commit/0f1c8a783beb9d09f3af01b68a2054379e16b1dd))



<a name=""></a>
## v1.4.0 (2016-04-24)

Time flows, and with the flow come new features, new bugfixes, and hopefully no
new bugs.

With this release, a partial changelog of changes since 1.3.0 is added.

Next release will contain a changelog with all changes.

We are hoping that you'll enjoy the new stuff.

Cheers. :)

*PS.* If it's not clear from the changelog below, audio groupchats have been
~fixed.

#### Breaking Changes

*   disable building with filter_audio by default ([116cc936](https://github.com/qTox/qTox/commit/116cc9366cd45eaff7262b92a8eb4d0fdbede096))

#### Bug Fixes

*   close groupcall if alone ([98d51399](https://github.com/qTox/qTox/commit/98d513990eabb0038112aa2242d45b22e58b2e43))
*   disable netcamview if no peer left ([622b543d](https://github.com/qTox/qTox/commit/622b543d9a8d29fca004cf41d7701b3dbef4eebf))
*   audiocall button disabled in groupchats ([db4f02a0](https://github.com/qTox/qTox/commit/db4f02a0c4065014bdbadab4a622aae14a112c76))
*   Close logfile only after the disabling logging to file ([de487890](https://github.com/qTox/qTox/commit/de4878909d75d9d4312db025f8c0fc30f9f79f39))
*   Make logMessageHandler thread-safe ([a7ffc08c](https://github.com/qTox/qTox/commit/a7ffc08cdb9521e1291e768d29aa39ee53a30dbe))
*   Deadlock while rotating logs ([c1e2a3c5](https://github.com/qTox/qTox/commit/c1e2a3c5b670c246c1acc138a8698a1f54591173))
*   increase faux offline message timeout ([76d8e193](https://github.com/qTox/qTox/commit/76d8e19320d2f15aeb019f72ecaaac2d6a23feab))
*   remove unnecessary qDebug call ([66f96019](https://github.com/qTox/qTox/commit/66f96019cb7389eb847b880a2e76bbd68a0ac288))
* **Widget::updateIcons:**
  *  workaround QIcon fallback bug ([0b53c4fd](https://github.com/qTox/qTox/commit/0b53c4fd5c71f43a240f2d980bac4e0166fffd2a))
  *  fix the way systray icons are loaded ([90874a47](https://github.com/qTox/qTox/commit/90874a478fb3cc6d953a0e37aeb110b95066eb19))
* **addfriendform:**  Removed extra connect return press ([66bcfdae](https://github.com/qTox/qTox/commit/66bcfdae3c8ae1f9d0b4a4fb1b3bf8f53440d53d))
* **addfriendform, widget:**  Remove Accepted Request ([53071e95](https://github.com/qTox/qTox/commit/53071e952e6d04f97fc8b3e1ab82e0fff8dd5293))
* **chatform:**  regression in detecting `tox:` type IDs ([48f3fb7d](https://github.com/qTox/qTox/commit/48f3fb7dcbf5ce990229114158f38098320ede97))
* **core, widget:**  Added checks ([f28c3a16](https://github.com/qTox/qTox/commit/f28c3a16ae6d54ed66ce64fbe7c5605badd34e65))
* **file transfer widget:**  QPushButton allows image to overflow ([32d588a4](https://github.com/qTox/qTox/commit/32d588a499a76a37340e930e17cb096ec4c27f24), closes [#3042](https://github.com/qTox/qTox/issues/3042))
* **genericchatitemwidget, micfeedbackwidget:**  Added members init in the constructor ([27faec91](https://github.com/qTox/qTox/commit/27faec918a023c5bba4b6ee67854438507220b35))
* **groupaudio:**
  *  don't set button to green while call running ([6d355154](https://github.com/qTox/qTox/commit/6d3551548b2568d5587992be1ecb3b559dd827f5))
  *  don't play audio while call is inactive ([5339ad97](https://github.com/qTox/qTox/commit/5339ad978bd2f962e0799e48452ac2549edf8bc7))
  *  avoid deadlock when ending groupcall ([afcd146a](https://github.com/qTox/qTox/commit/afcd146a5bd78d10083a5bf3eb009face24f02b6))
* **groupinviteform:**
  *  make list of groups scrollable ([b74ecd92](https://github.com/qTox/qTox/commit/b74ecd92d2f925ff5defee1a4ca0cd50ecf6a2e6))
  *  translation invite message ([24efaf05](https://github.com/qTox/qTox/commit/24efaf0594576ba4e761653c6d612f98b440c28a))
  *  remove deleted buttons from set ([f137ba71](https://github.com/qTox/qTox/commit/f137ba710cc823e920adf2976ee1061f5a61f9aa))
* **l18n:**  make typing notification & groupchat name translatable ([43e61041](https://github.com/qTox/qTox/commit/43e610415a094a649160bdfab1c571a8fc6fee1f))
* **login screen:**  Change text on login tooltip ([4e065f13](https://github.com/qTox/qTox/commit/4e065f1395b8feb35431c7736e531b6a0a6fee45))
* **main:**  Added check sodium_init result ([64a19d34](https://github.com/qTox/qTox/commit/64a19d34192e2ca9f864b1ccc3a320fb90bc780b))
* **profile:**  Don't require .ini to load profile ([56a36e2e](https://github.com/qTox/qTox/commit/56a36e2e0a1023a77f0b047a7273295a35aa1833))
* **profileform:**
  *  Add toxme username limitation ([132f87c0](https://github.com/qTox/qTox/commit/132f87c05e89e21824efa2c4dfef83d807e0f5bf))
  *  Deleted extra check and extra url ([1f7e23d0](https://github.com/qTox/qTox/commit/1f7e23d007887a0e198b8643961f391a06faf36c))
  *  Fixed very quick relogin segfault ([88de3a0a](https://github.com/qTox/qTox/commit/88de3a0a7a09b89b0a621452e53b4d6d4ec3bfe8))
  *  Fixed segfault on logut ([2e9295f4](https://github.com/qTox/qTox/commit/2e9295f420fc6832091337813010319166450270))
  *  Fix tab order, fix loop ([65ab1f4e](https://github.com/qTox/qTox/commit/65ab1f4e14d3ebc9dd13d4308988d999c83a5a47))
* **screenshot:**  incorrect screenshot capture resolution under HiDPI ([a36248b5](https://github.com/qTox/qTox/commit/a36248b5013c41c332df3bc13cac428bb0d3b18e))
* **systemtrayicon:**  only delete the systray backend that was used ([1d6f32c9](https://github.com/qTox/qTox/commit/1d6f32c9f9e481dc2fed445bb96ea2666f6d69d8))
* **systemtrayicon, widget:**  Added deallocate memory ([cbb7eeca](https://github.com/qTox/qTox/commit/cbb7eeca62f9ec8ad2047b41a7cb914b27d6c618))
* **title:**  Change title on initial startup on "Add friend" ([47d94045](https://github.com/qTox/qTox/commit/47d940455d35e4b0f76081a8877a881ebf843c86), closes [#3100](https://github.com/qTox/qTox/issues/3100))
* **toxme:**
  *  Delete extra check ([d1b706a4](https://github.com/qTox/qTox/commit/d1b706a4b3926fc3bd1f7af115358cf171080021))
  *  Fixed potential memory leaks ([8f4b6869](https://github.com/qTox/qTox/commit/8f4b6869f178c12a43d7d37336f32f8ecf7b1427))
  *  Fix possible segfault ([11ec3947](https://github.com/qTox/qTox/commit/11ec3947f566e7083a6345ce2eea317f31219c5e))
  *  Use format strings ([fc2a5723](https://github.com/qTox/qTox/commit/fc2a5723092ed1c2fe6b0991d9bd9def6ec24a98))
  *  Translation fixes ([9565a817](https://github.com/qTox/qTox/commit/9565a8175558d64c9ffdc2ec6b64e69d9ecdc58d))
* **video:**
  *  uses explicit default screen from QGuiApplication ([d2189f38](https://github.com/qTox/qTox/commit/d2189f3891b01ca9c4fa55b080e4457887c00f28))
  *  usage of invalid file descriptors on error ([556a8750](https://github.com/qTox/qTox/commit/556a8750a1d4c57d02bfe3a4caaffceaf816c783))
  *  incorrect desktop video resolution when using HiDPI ([75b40d0a](https://github.com/qTox/qTox/commit/75b40d0a6f82110aac62c864149210f8b491df4b))
* **widget:**  Change focus after creating group ([b111c509](https://github.com/qTox/qTox/commit/b111c509a7dcf3a0c3d7a72d92c080ff7fd92731))
* **widget, contentdialog:**  Added reset icon after activate chat window ([4edc5996](https://github.com/qTox/qTox/commit/4edc5996c74d8679c270c50328642665ed6b3aed))

#### Performance

* **camerasource:**  Passed parameter by reference ([910c41f4](https://github.com/qTox/qTox/commit/910c41f4fab106e1c736685ffad33141c1528107))
* **contentdialog:**  Delete redundant conditions ([904a1d49](https://github.com/qTox/qTox/commit/904a1d490973ed57327bc093aeaae1e7317b52e8))

#### Features

*   install icons with make install on unix ([218228b6](https://github.com/qTox/qTox/commit/218228b696367e688537a04afeb62055037afdc3))
*   disable building with filter_audio by default ([116cc936](https://github.com/qTox/qTox/commit/116cc9366cd45eaff7262b92a8eb4d0fdbede096))
* **audio:**  add (repair) support for group audio calls ([356543ca](https://github.com/qTox/qTox/commit/356543ca3ba9d084e9739bcd7b37c4597a245d1d))
* **chatform:**  add support for non-local file and samba share links ([47764c03](https://github.com/qTox/qTox/commit/47764c0397a51da2662021a781cefe29af46bf25))
* **profileform:**  Added ability to change toxme server ([41c5d4bf](https://github.com/qTox/qTox/commit/41c5d4bf14986348f04f9e60f538e79c1edc4a04))
* **toxme:**
  *  Add save toxme info ([204fe1d3](https://github.com/qTox/qTox/commit/204fe1d3dec00c0f4408f8a98c100377759f1218))
  *  Add ToxMe registration ([cb8bf134](https://github.com/qTox/qTox/commit/cb8bf134d2e8d0aaf60e136c51f79201b62f67f9))
