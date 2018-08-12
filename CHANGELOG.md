<a name=""></a>
## v1.16.3 (2018-07-18)

This point release fixes flatpak build. No feature changes.



<a name=""></a>
## v1.16.2 (2018-07-15)

This point release fixes dialog spam from receiving invalid filenames and logs
spam. No feature changes.

#### Bug Fixes

* **logging:**  only log toxcore messages above TRACE level ([4dc74201](https://github.com/qTox/qTox/commit/4dc7420162e69095942b392048c309e6246d6b21))
* **ui:**  don't emit filename change windows for every chat ([c1701345](https://github.com/qTox/qTox/commit/c1701345455ad5b253beeaa3d487daa01b8b1b21))



<a name=""></a>
## v1.16.1 (2018-07-04)

This point release fixes our deployment of Flapak and AppImage on Github. No
feature changes.

#### Features

* **deploy:**  upload Flatpak bundle to Github releases ([59b5578c](https://github.com/qTox/qTox/commit/59b5578c7bffc56f6227c60bfcb38f97d39ec8d9))

#### Bug Fixes

* **deploy:**  fix file path in AppImage deployment ([64602f38](https://github.com/qTox/qTox/commit/64602f38f154a3f3d2429146ae5d370b2202d1b8))



<a name=""></a>
## v1.16.0 (2018-07-02)

The most notable additions in this release are a new fullscreen mode for
video calls, a new call end sound and support for more camera resolutions. To
distribute qTox in a more user friendly manner we now publish Flatpak and
AppImage packages.

#### Bug Fixes

*   remove full screen btn from audio group chat ([0d3f061b](https://github.com/qTox/qTox/commit/0d3f061ba80d9f3f8a971d2b8e11a7d9b59d180a))
*   local toxcore install with bootstrap.sh ([9ca38750](https://github.com/qTox/qTox/commit/9ca3875079adf175f31f568e45aabc37e3409000), closes [#5199](https://github.com/qTox/qTox/issues/5199))
*   simple_make.sh script ([ead2152d](https://github.com/qTox/qTox/commit/ead2152d6f0d15f7e662975fb3ed8525109794c3))
*   Fix PR #5182. Eliminating the 'new' operator at ToxOptionsWrapper ([9b6cd1c0](https://github.com/qTox/qTox/commit/9b6cd1c0227006308d4fe556f2b721865c2d9b21))
*   Fix usage of unitialized functions ([06ae7ead](https://github.com/qTox/qTox/commit/06ae7ead0c7c23935c1c05c75d9cb11ed516224b))
*   two crashes, uncovered by the persistent groupchat patch ([48179b6a](https://github.com/qTox/qTox/commit/48179b6a19807383e298661a21f97db3b140eb44))
*   delete double initialization callDuration ([dc1f5ea0](https://github.com/qTox/qTox/commit/dc1f5ea0a319bf4cbf05989c414ccaea898b4826))
* **Core:**  fix use after free of proxyAddrData ([26b59d31](https://github.com/qTox/qTox/commit/26b59d312375ad6391228308aabe45f0a85a1194))
* **appimage:**  build sqlcipher form source ([64a7c24b](https://github.com/qTox/qTox/commit/64a7c24b2b5ad11a6df5dbb11da6e3aa7c0fd6f3))
* **audio:**
  *  fix error introduced in 67f2605971cf43093c72f811e4df90ab70544dd6 ([40d30153](https://github.com/qTox/qTox/commit/40d30153aed223b65b596dc7d3bf17573b04f3e9))
  *  connect the correct audio callbacks ([a00af087](https://github.com/qTox/qTox/commit/a00af087778c6315ef55ed77c4209cbb63a6323d))
  *  close the audio device after playing a sound ([a3370173](https://github.com/qTox/qTox/commit/a3370173df24cd6880e3e3845ddbbc7c090b7aed))
* **build:**
  *  Elimination the build warnings (Wunused-variable, Wreorder) ([2cd65610](https://github.com/qTox/qTox/commit/2cd65610fcce0c3dcf8a5e9cb9f313a76167c09a))
  *  correct install script nsis for win64 ([25e69572](https://github.com/qTox/qTox/commit/25e69572f89d816cfab5a8c0d1c261bae34d3cdd))
  *  make qTox compile with ffmpeg 4.0 and newer ([44193176](https://github.com/qTox/qTox/commit/441931765ffe3de349b28a28bf10a006edcc9949))
* **chatform:**
  *  name in window title and close detached chats ([39968a31](https://github.com/qTox/qTox/commit/39968a313d78c727046837901e6cc3d6c31d18e0))
  *  check for empty path when exporting profile ([757791ee](https://github.com/qTox/qTox/commit/757791eea4be390bb6d1cdc908d1cd3c4b18728d), closes [#5146](https://github.com/qTox/qTox/issues/5146))
* **core:**  Clean illegal chars from filenames ([ab85716f](https://github.com/qTox/qTox/commit/ab85716f00acfe00ff8035670919dd548d7f7f83))
* **docs:**  update toxcore build instructions ([b00cbc1d](https://github.com/qTox/qTox/commit/b00cbc1d6f3a7f8406e4a96e732c534068fde22c))
* **file:**  don't clean the filenames of avatar transfer ([2a8ab03e](https://github.com/qTox/qTox/commit/2a8ab03e46dd08efc4051a01bea56fe6a4c38a11))
* **history:**  don't save both action prefix and displayed name ([dfd2de83](https://github.com/qTox/qTox/commit/dfd2de836eae605e02a1afb270620dd9274f6385))
* **leak:**  Fix few memory leaks ([daaa5518](https://github.com/qTox/qTox/commit/daaa5518dd7c02c2de45690daa3f592206fc4023))
* **login:**  start login screen on profile select by -p option ([1af3ad69](https://github.com/qTox/qTox/commit/1af3ad69e884bc4e74a4fcdd452a6aff10bffd62))
* **settings:**
  *  automatically disable UDP when a proxy is set ([977b7fc9](https://github.com/qTox/qTox/commit/977b7fc9a02b2b44164ffb77ab35f4cdfae90542))
  *  prevent segfault on wrong proxy settings ([dfd5232e](https://github.com/qTox/qTox/commit/dfd5232e2fb727685a20804d7ca3b932ea239332))
* **simple_make:**  correct variable initialization ([1537f83e](https://github.com/qTox/qTox/commit/1537f83e85ff28dd73fb66161ae2cd5eeef692d1))
* **theme:**  clear stylesheet cache on theme colour change ([8ba8ce91](https://github.com/qTox/qTox/commit/8ba8ce91f3317794b72fb4937c459dac2856d367))
* **ui:**  increase number of low res camera options ([72931514](https://github.com/qTox/qTox/commit/72931514695a8691593d6a5abd2df1e340f95002))
* **video:**  unsubscribe the video device correctly ([e55f86c6](https://github.com/qTox/qTox/commit/e55f86c6a5b0344642fcb3d7a2550df6e899a6e5))
* **wayland:**  Fix desktop file name in Qt properties ([c1caeb58](https://github.com/qTox/qTox/commit/c1caeb585a8845eaa72c7db79fb334262eafdb8f))

#### Features

*   Add ability to remove dialog from content dialog with middle click ([aae567ed](https://github.com/qTox/qTox/commit/aae567ed8e299fc0cdd700e2e0020042ee1cba11))
*   Add ability to quit group with middle click ([228c431c](https://github.com/qTox/qTox/commit/228c431c890a7e68d078b441311892c691643926))
*   Add middle mouse clicked signal for GenericChatroom ([65fc1dc2](https://github.com/qTox/qTox/commit/65fc1dc266da29e0679f2b645c31bc428f0cf575))
* **appimage:**  build appimage on TravisCI ([f7345e4d](https://github.com/qTox/qTox/commit/f7345e4db264a5681490b9094981a65cac68d317))
* **call:**  add call end sound ([65896e45](https://github.com/qTox/qTox/commit/65896e45017f8f748bc5b9db10a4400d7fd418dc))
* **chat:**
  *  add UI option to mute group peers ([2fae2a30](https://github.com/qTox/qTox/commit/2fae2a30f76978ce722c5b24236384c8052ebfc4))
  *  full screen video chat ([d6df8883](https://github.com/qTox/qTox/commit/d6df8883e399b95a55c5a5870497c1dcd45a3917))
* **core:**  put c-toxcore log messages in the qTox log ([4faab075](https://github.com/qTox/qTox/commit/4faab0750d3841beeb08c7d17e85044b5013aea8))
* **history:**  load set number of messages from history ([ca32e77d](https://github.com/qTox/qTox/commit/ca32e77d7400e23a6a839f6a8d1f322bfe48bbf0))



<a name="v1.15.0"></a>
## v1.15.0 (2018-04-18)


#### Performance

* **ui:**  cache stylesheets to reduce memory usage ([6d9d26db](https://github.com/qTox/qTox/commit/6d9d26db654981dbd22bdb0a70dfbc48f89b2e60))

#### Bug Fixes

*   remove build date and time from main.cpp ([23f6ad70](https://github.com/qTox/qTox/commit/23f6ad7047a2391defd95b144cfcceac37994a51))
*   don't use invalid reference after erasing element ([1afed5f7](https://github.com/qTox/qTox/commit/1afed5f72e78b96222af4ab1d747cc73d6e8df35))
* **IPC:**  Update profileName to update IPC ID after login screen ([d2ad2107](https://github.com/qTox/qTox/commit/d2ad2107f2ea9238a0e56ff8dd883d308615ab57))
* **audio:**
  *  apply gain to both audio channels ([f6622e40](https://github.com/qTox/qTox/commit/f6622e40928196ae85724f340a760b4ea5f2fb2f))
  *  move audio timers to and start from audioThread ([5d0f9509](https://github.com/qTox/qTox/commit/5d0f9509a8d5fac940f402237efe4a8a6ab7e27b))
* **avform:**  Audio/Video settings tab #5011 issue bugfix ([ad6ddfc4](https://github.com/qTox/qTox/commit/ad6ddfc4f245f9d0e36a6ce0ef9646a50fde30f8))
* **build:**  Remove unused vars, add default case for switch ([bc18990f](https://github.com/qTox/qTox/commit/bc18990fd248c117916c88d640456e6b967a4802))
* **chatform:**  include pressed key(s) when changing focus ([a8fc6e5c](https://github.com/qTox/qTox/commit/a8fc6e5c6bb0284950eab8495c8bc8ad45fd6355))
* **chattextedit:**  ChatTextEdit bug fix for issue #5020 Corrected ([ebdc675c](https://github.com/qTox/qTox/commit/ebdc675cbc2dd0054306d461c382568c6148bf60))
* **core:**  split messages on utf8 multibyte character boundary properly ([869036f3](https://github.com/qTox/qTox/commit/869036f3c6fc567b56fca1e36b7b4bdcda03a089))
* **docs:**
  *  include master key fingerprint instead of subkey fingerprint ([3748ca6b](https://github.com/qTox/qTox/commit/3748ca6bc849046a44b83f16104490a685e65055))
  *  update dependencies for Debian ([7627d60a](https://github.com/qTox/qTox/commit/7627d60a89c89adea514f97b2f803617a22e6171))
* **history:**  don't save own messages when history is disabled ([b852809d](https://github.com/qTox/qTox/commit/b852809d0c554754496a54cadc7d14d3802bc183))
* **import:**  don't use java-style iterator before first element ([acea7c31](https://github.com/qTox/qTox/commit/acea7c315fd2b8cb89bac21956a16bca721f0712))
* **login:**  Don't dereference null pointer ([703876c3](https://github.com/qTox/qTox/commit/703876c3724386c540d7e62e4f5d19dad31477bb))
* **name:**  Clear alias on name change so that name changes are visible ([c2410e0a](https://github.com/qTox/qTox/commit/c2410e0a656f1708ed07bf8b2644300ae54af53c))
* **offlinemsg:**  make faux offline messages purely event based ([4951f909](https://github.com/qTox/qTox/commit/4951f90964317a8a409f5ab75ecde3073c72e491))
* **profile:**  don't error if no new avatar is selected ([cfdc1cdb](https://github.com/qTox/qTox/commit/cfdc1cdb2e74aeaece70c2798ec9b7fddbbbed39))
* **settings:**
  *  connect enable LAN discovery checkbox ([55d8922d](https://github.com/qTox/qTox/commit/55d8922d69f035580dbea92285648d75645bfcf0))
  *  add missing enableTestSound to settings ([65d59ba6](https://github.com/qTox/qTox/commit/65d59ba6b7d2f89bf1d4078df3a3647abc84858c))
* **ui:**  remove placeholder update UI ([cb0f2635](https://github.com/qTox/qTox/commit/cb0f26356d419b9be945f4f910c7415f14bf024d))
* **widget:**  Fix double free crash on group leave ([81989406](https://github.com/qTox/qTox/commit/81989406073a70f7e9b17f03dee78ed7be5e978a))

#### Features

* **Weblate:**  create script to automatically merge Weblate changes ([96ae4284](https://github.com/qTox/qTox/commit/96ae4284a09df48fda2cdd0868779ec32a9d18de))
* **camera:**  add higher resolution camera options ([04ecfe3f](https://github.com/qTox/qTox/commit/04ecfe3f344c29d9c598d38aaad46f2da8a17728))
* **ui:**
  *  add setting for disabling new message notification popup ([fcd88d65](https://github.com/qTox/qTox/commit/fcd88d65b2875c06312374186fd9a08a9637dac7))
  *  Add ability to disable LAN discovery ([9f8b0fed](https://github.com/qTox/qTox/commit/9f8b0fed07d5a1debb27252df6dc88e8941ae487))



<a name=""></a>
## v1.14.0 (2018-03-12)


#### Bug Fixes

*   Not quit on close if this setting is enabled ([e73dc10c](https://github.com/qTox/qTox/commit/e73dc10c7fd23b887cc5e2d5d4021bc02c8555ec))
*   add search symbol ' in history ([3e05279c](https://github.com/qTox/qTox/commit/3e05279c097b33b09cedcebae4150c839a23af35))
*   Use real channels number ([e74cc37a](https://github.com/qTox/qTox/commit/e74cc37a2d02e9d4cbd016bac9dbb7697e8445e7))
*   Allocate memory to input buffer ([900f2a1a](https://github.com/qTox/qTox/commit/900f2a1ad3b328359a0ae089e778b15280512a9d))
*   Call doAudio on timer timeout ([2353a66f](https://github.com/qTox/qTox/commit/2353a66fded32174421c9663ced5cfe4ceabe00b))
*   [un]subscribe output in avform ([8c05399e](https://github.com/qTox/qTox/commit/8c05399e418f2c0147ce2d9c7dd220a0cdc97765))
*   Correct display the call confirm window (CallConfirmWidget) ([f4fe343e](https://github.com/qTox/qTox/commit/f4fe343eca3eaf84f9ce300b59be9e83a70c204e))
*   elimination of warning '-Wreorder' ([0869d3d8](https://github.com/qTox/qTox/commit/0869d3d8fdc9e9de2f1df51c377ddba71a1ce523))
*   Use epsilon to compare float ([91dabf11](https://github.com/qTox/qTox/commit/91dabf11d31807f499d6e949373bf22762e80f5b))
* **UI:**  prevent deadlocks on logout and profile delete ([a49e3458](https://github.com/qTox/qTox/commit/a49e34589f40edfb3fc46d5700573f87d5dfe3d0))
* **build:**
  *  move Appdata file installation to /usr/share/metainfo ([5db0bdd3](https://github.com/qTox/qTox/commit/5db0bdd381f0f08c5685501702f2a2eb9d2f5674))
  *  add needed ffmpeg decoder to configuration ([8973a521](https://github.com/qTox/qTox/commit/8973a5216f49e65adc48d5fada8a574db598cced))
  *  Add missing dependency for openSUSE ([f7e089f7](https://github.com/qTox/qTox/commit/f7e089f7a71c41ff31d311fe7148e57b5c6fb60a))
* **chatform:**  Broaden URL matching to include unicode ([e564b85e](https://github.com/qTox/qTox/commit/e564b85e3c485b283855bfdf00dfc0ec5427fad4))
* **chatlog:**
  *  Match multi-character emoticons again ([9643e48e](https://github.com/qTox/qTox/commit/9643e48ef1d68948d52feec4e1be28c3ad61c0da))
  *  parse multi-length emoji properly ([5df63f9c](https://github.com/qTox/qTox/commit/5df63f9c2e6d78f4799447b0a22cdb9fb70c3fea))
* **chatwidget:**  fix send file button not working ([af1aebfd](https://github.com/qTox/qTox/commit/af1aebfd1a7409ea821be2a616067561b62751c0))
* **cmake:**
  *  fix platform extensions for windows ([7ad68e2f](https://github.com/qTox/qTox/commit/7ad68e2f43b458cd00ca27b9cfb20abf0b9ae46c))
  *  add missing dependency ([423f0956](https://github.com/qTox/qTox/commit/423f095622824a34d081fb69bddd83cddf83ca03))
* **core:**
  *  Adapt qtox to new conferences state change callback. ([1111949f](https://github.com/qTox/qTox/commit/1111949f450fb4fe63321386f7f452ee1663f07a))
  *  Use new callback API for bitrate set ([d2deec7c](https://github.com/qTox/qTox/commit/d2deec7c554b3df651fe789dfb7964748329eff4))
  *  Use new API for bitrate set ([2c8f03da](https://github.com/qTox/qTox/commit/2c8f03dada443e30d6189050c7cf6d42e01827c5))
* **cpu:**  Reduce CPU usage by avatar render ([8db61f96](https://github.com/qTox/qTox/commit/8db61f96ec78ac53479dd8db36eb192f6a1ddbcd))
* **friendwidget:**  Use queued connection to avoid removing 'this' ([9b4972e0](https://github.com/qTox/qTox/commit/9b4972e0459de2921370cda9de645eb64e37ecfc))
* **group:**  Show correct count of user on first creation ([0a590336](https://github.com/qTox/qTox/commit/0a590336b1467405a903464085dcdfc4474f93e6))
* **install:**  Fix gzip invalid usage ([266f63f6](https://github.com/qTox/qTox/commit/266f63f6dfb1869aa2339d48cdc9b52ece3597ce))
* **l10n:**
  *  Correction of the translation into Russian ([3fb42b75](https://github.com/qTox/qTox/commit/3fb42b75d75bf6c0240748ffff368b912b14a838))
  *  Correction of the translation into Russian ([9229fdd1](https://github.com/qTox/qTox/commit/9229fdd17e013a8bd60102648a200734890c2140))
* **smiley:**  change license of classic smileys to CC BY-SA 4.0 ([da7c12e2](https://github.com/qTox/qTox/commit/da7c12e20cac1ac7340b4bb4ec89f782e2e4a159))
* **travis:**
  *  try working around Travis + gitstats issue ([4c980945](https://github.com/qTox/qTox/commit/4c98094551ff4a1e7377a206b72fedd470b8be96))
  *  switch back to older Ubuntu Image ([378daeaa](https://github.com/qTox/qTox/commit/378daeaad4c5992a7acd2b650ff081d213556e10))
* **video:**
  *  improve debug message ([ff2fc18b](https://github.com/qTox/qTox/commit/ff2fc18be164fcbc89bfd46d64f4b0096a97aee5))
  *  choose first available resolution in preview automatically ([81522dea](https://github.com/qTox/qTox/commit/81522deabdc3fb11fd8d3e1feb59274a96583121))
  *  use float framerates also for V4L2 ([a2927de2](https://github.com/qTox/qTox/commit/a2927de27d4776b52303e07c07ce89e8dadf86c5))
  *  allow not integer framerates ([db7ee65d](https://github.com/qTox/qTox/commit/db7ee65d0efbe23a45e385a148b20701e521a5c5))
  *  Fix square form of a video ([8de8c14a](https://github.com/qTox/qTox/commit/8de8c14a76908cf84a322a0bfd9e2c7ad2b4fa16))
* **widget:**  Fix status pic alignment ([d9118cfc](https://github.com/qTox/qTox/commit/d9118cfc71e2b030914187df7fd9fb3d98378cf1))
* **windows:**  %APPDATA -> %APPDATA% in template ([f53b8282](https://github.com/qTox/qTox/commit/f53b82825bf76be5a6793d18f2d102ed7b222313))

#### Features

*   Add the cmake option USE_CCACHE ([aa9cff31](https://github.com/qTox/qTox/commit/aa9cff315d659a7ca2010fb4791893abc8c5abdb))
*   update to the new c-toxcore 0.2.0 conferences api ([d3d81bbd](https://github.com/qTox/qTox/commit/d3d81bbdf3c198a7c1258c6ad6405c6ab61cedd4))
*   add hot keys for search ([ffb51e8a](https://github.com/qTox/qTox/commit/ffb51e8a0ea7dc3fb01f1f7650edc80b779a9be2))
*   optimise search in history ([18fa8a74](https://github.com/qTox/qTox/commit/18fa8a745bdafddc00ba2f577c36451f40edfd61))
*   add search in text in group chats ([7718734c](https://github.com/qTox/qTox/commit/7718734c9ab9705c1a1274b2a447611c1a2e22b4))
*   remove search button and add line in context menu ([8bb80c77](https://github.com/qTox/qTox/commit/8bb80c770c1d21d1bdfc03c3d0569fabe6535e8f))
*   edit load history for search ([de9c9061](https://github.com/qTox/qTox/commit/de9c9061175c97a9ee203d18a39e73f77544d5e6))
*   add text search ([b881d32d](https://github.com/qTox/qTox/commit/b881d32d1bddb7352b8d24e2442ef6277ff0d583))
*   add form for search ([863c46c7](https://github.com/qTox/qTox/commit/863c46c73d1a2fc677f9142ba8d7a2e8dc659c2a))
*   add a button to search ([47d9da98](https://github.com/qTox/qTox/commit/47d9da98cf6811a30d35a1204e5342a4f7f4bf94))
*   Prefere new line as message break ([3b52402f](https://github.com/qTox/qTox/commit/3b52402fa20d2d5418e129e5f001b626401a9ae5))
* **UI:**  new status icons for message notifications ([4288785d](https://github.com/qTox/qTox/commit/4288785d31e215bc379223577f7d4dd65664ed86))
* **avatar:**  Add outline hightlight on mouse hover ([bb26485d](https://github.com/qTox/qTox/commit/bb26485db6fed706f4ebccaffe35740394210032))
* **groupchat:**  mark blocked users with different color ([a729f2f8](https://github.com/qTox/qTox/commit/a729f2f8c00d29d2837b6e380f5af1b95c344bad))
* **l10n:**
  *  add Macedonian translation ([1a06f85d](https://github.com/qTox/qTox/commit/1a06f85d3ccc91ff6f759a38534483fa40aaaa29))
  *  add Macedonian translation using Weblate ([41420331](https://github.com/qTox/qTox/commit/414203310a30720e02e06719bfcafbb8bcff9018))
  *  update French translation from Weblate ([a7e90969](https://github.com/qTox/qTox/commit/a7e9096919d4c0b89f061e8b77741d517f574838))
  *  update Portuguese translation from Weblate ([3bad087b](https://github.com/qTox/qTox/commit/3bad087bbff2fbff4c4d543df1f96931784c93df))
  *  update Portuguese translation from Weblate ([8c3be522](https://github.com/qTox/qTox/commit/8c3be5225f484469aed43dde04f03bc588ca2c15))

#### Performance

* **widget:**  don't save on setExpanded if categorywidget is unchanged Fix #4932 ([b9845e1d](https://github.com/qTox/qTox/commit/b9845e1d23eb23380f447692e3a813413e897c2d))



<a name=""></a>
## v1.13.0 (2017-11-25)

In this release we added microphone voice activation and the long-awaited
Github-like Identicon instead of default avatars. Of course also some bugs were
fixed and new ones added :P

#### Bug Fixes

*   use only well known categories in the desktop file ([4d36c23c](https://github.com/qTox/qTox/commit/4d36c23c9ba88a350a9b7ae6a7657fda3b8a2af2))
*   add default return in Widget::getStatusIconPath to fix warning ([8a9c34d8](https://github.com/qTox/qTox/commit/8a9c34d8886ac2db3def1d08e8c203839d68c37d))
*   URL patterns based on RFC 3986 ([6ffe4cd8](https://github.com/qTox/qTox/commit/6ffe4cd8d892183ecdfcba571ff8dd4d62d595fd))
*   using current setting in autoaccept dialog ([70b235f2](https://github.com/qTox/qTox/commit/70b235f271b2f3d1dfaf036d60bb7e8e63db2908))
*   remove unnecessary semicolon to compile with `-pedantic` flag ([37419825](https://github.com/qTox/qTox/commit/37419825cafcc0d4a239c2922abc074b753a4e29))
*   add missed <functional> header ([5e455699](https://github.com/qTox/qTox/commit/5e455699730284e71709326b30f897d103db17ea))
*   Use correct initialization order ([b41767d0](https://github.com/qTox/qTox/commit/b41767d0874eaf8887f908d2a9c5571f4342d5d5))
*   remove unused variables ([c089c80e](https://github.com/qTox/qTox/commit/c089c80e53be33a8d42ea28176763659c1eb2005))
* **build:**
  *  install libexif-dev on apt-based systems ([2066c2e0](https://github.com/qTox/qTox/commit/2066c2e002009a75708c61f2ec3c39ce588d0e56))
  *  do not build unix-specific test on Windows ([d69023c9](https://github.com/qTox/qTox/commit/d69023c9d78714f5e730e98bb251c8d0f723688a))
* **call:**  fix bugs introduced from ToxCall refactor ([1394dd1b](https://github.com/qTox/qTox/commit/1394dd1b7fc59da57f0bd7470f4caed22231670a))
* **cameradevice:**  Save string in local variable to avoid errors ([92def839](https://github.com/qTox/qTox/commit/92def839d084a97bed80e01e5afda49c593c34bd))
* **chatForm:**  Fix issue with mixing friend and group id ([5bc8ef4e](https://github.com/qTox/qTox/commit/5bc8ef4e74dee407447de52328c3ccf98307c46a))
* **chatform:**
  *  Subscribe on accept and reject buttons once ([d77fbb4b](https://github.com/qTox/qTox/commit/d77fbb4b1999da2e20d5ed0071b254eea99dce4b))
  *  Hide call confirm on call end ([f4a3bb28](https://github.com/qTox/qTox/commit/f4a3bb2812fbe3cab424a190a7f8114d26351cd7))
* **core:**  Check that result of get peer name is successful ([78262b41](https://github.com/qTox/qTox/commit/78262b41bd8d2bec9d50ac736302eaa389369404))
* **ffmpeg:**  don't use deprecated features ([2f13796a](https://github.com/qTox/qTox/commit/2f13796acc5e63a57a5af1e5a4ce102da506d5d5))
* **friend:**  Add save friend alias on change ([c0a7488c](https://github.com/qTox/qTox/commit/c0a7488c12923354d0ed25f405994327ce2eb31a))
* **group:**
  *  Send all parts of long message ([7c76bebe](https://github.com/qTox/qTox/commit/7c76bebebef9d20da38bd4b45c3fb4aed2cf5915))
  *  Add second signal for titile changed ([2f9ba3c4](https://github.com/qTox/qTox/commit/2f9ba3c4d793a9b50f932c82792a5f8afce5f2b8))
* **identicon:**  don't set identicon as an avatar ([676be5f6](https://github.com/qTox/qTox/commit/676be5f625dc485433ba9137a027cad2bef1b5f5))
* **init:**  register AV connects and call after AV is ready ([7170b485](https://github.com/qTox/qTox/commit/7170b48589c88eaa33263ffb0155dbf384b82d56), closes [#4651](https://github.com/qTox/qTox/issues/4651))
* **ipc:**  Add check if IPC init failed ([c274cec8](https://github.com/qTox/qTox/commit/c274cec87e957894c021bd7f27994a2f6ce5473c))
* **logout:**  Synchronously call showLogin to avoid multiple deletion ([5046fc90](https://github.com/qTox/qTox/commit/5046fc90103f8c33d8e5221a1e655414aad53f44), closes [#4201](https://github.com/qTox/qTox/issues/4201))
* **platform:**  use result to remove -Wunused-result warning ([729dc774](https://github.com/qTox/qTox/commit/729dc7740b04becae7c509c3aafbf553fd955b31))
* **preview:**  only downscale preview images, never upscale ([45b3575d](https://github.com/qTox/qTox/commit/45b3575d7902d99a3c8a31987b4ef92d4ba5d78f), closes [#4733](https://github.com/qTox/qTox/issues/4733))
* **profile:**  Fix status message and username signal connection typo ([d41031bd](https://github.com/qTox/qTox/commit/d41031bd15bfa6cce1a0f55b20952a98680c3f2f), closes [#4760](https://github.com/qTox/qTox/issues/4760))
* **settings:**
  *  prevent signed overflow and associated warning ([6d1b1f62](https://github.com/qTox/qTox/commit/6d1b1f62aba08b84e4dddf3436a936cddea3518f))
  *  Delete dynamically allocated members on destruction ([94cb6ce1](https://github.com/qTox/qTox/commit/94cb6ce1c562d7be0bc71e165f73c8ec5cd66353), closes [#4670](https://github.com/qTox/qTox/issues/4670))
* **test:**  If json string not quoted is server error ([7c744912](https://github.com/qTox/qTox/commit/7c744912268c53db3509dd7b5575c72df1e9b542))
* **toxme:**  Add #include <ctime> to toxmedata ([2f1bf107](https://github.com/qTox/qTox/commit/2f1bf107808953f5f080de3458f88abb33ad247a))
* **travis:**  make Windows cross-compilation caching work ([0966e91e](https://github.com/qTox/qTox/commit/0966e91e31684cbfdc8b5b339929db1f17907707))
* **ui:**  Set tooltip colour and background for all tooltips ([b1f40dec](https://github.com/qTox/qTox/commit/b1f40decd868f4df81ac8784bdcbb6a791d945ea), closes [#4667](https://github.com/qTox/qTox/issues/4667))
* **video:**
  *  don't use pointers to temporary objects ([5d6ae9ae](https://github.com/qTox/qTox/commit/5d6ae9ae372a95f8e6cd1950d87ba379eb1b8dfe))
  *  prevent segfault when ending videocall ([fed70602](https://github.com/qTox/qTox/commit/fed7060270932f3731d78c2f2ba30f5a10300f95))
  *  reduce default video bitrate to make it more usable ([ecea4104](https://github.com/qTox/qTox/commit/ecea41045c50611fb8f8b506c25c6f613073a4ce))
  *  Set toxav video bitrate to 0 if answering audio-only call ([1613044c](https://github.com/qTox/qTox/commit/1613044c68cee2ceee82b61e789e5225dda45f7d))

#### Performance

* **smileys:**
  *  Cleanup smileys icons by timer ([fa215949](https://github.com/qTox/qTox/commit/fa21594902beef73e70f69e80ae7acb016aa7ec6))
  *  Use shared_ptr to automaticaly count references ([c6400077](https://github.com/qTox/qTox/commit/c64000777510e8b6c899475bf49fba31af9fda8b))
  *  Use lazy smileys loading ([d83400bc](https://github.com/qTox/qTox/commit/d83400bcdc1bf177aabb6940ee3686f8e4f0e942))

#### Features

* **audio:**  microphone voice activation ([d24d4fb8](https://github.com/qTox/qTox/commit/d24d4fb8ea44999d037e7ca839c338140c5a59c7))
* **build:**  hash everything in Windows cross-compilation ([319d871b](https://github.com/qTox/qTox/commit/319d871be38d6f92386f626c7fcbd25252e680bd))
* **identicon:**
  *  use Identicons instead of empty avatars ([003fc6b0](https://github.com/qTox/qTox/commit/003fc6b0b0cbef99888a746d60566a1bcc3805f9))
  *  add algorithm to create identicons ([61b36d1b](https://github.com/qTox/qTox/commit/61b36d1bce87173fd290398db1045bd90a998dc9))
* **l10n:**
  *  update German translation from Weblate ([e0235dff](https://github.com/qTox/qTox/commit/e0235dffc5e3240696d5350bdbc1d2c5386e8cec))
  *  update German translation from Weblate ([af2addec](https://github.com/qTox/qTox/commit/af2addec860de6fb986250b2f7d22c8ea039ffd7))
  *  update German translation from Weblate ([13df29e5](https://github.com/qTox/qTox/commit/13df29e5d1f5e3c5365695b287778e024a9fc220))
  *  update Italian translation from Weblate ([45a053cc](https://github.com/qTox/qTox/commit/45a053ccb7dc414b4f464d32039b99707632e665))
  *  update Lithuanian translation from Weblate ([79f96c90](https://github.com/qTox/qTox/commit/79f96c902665e4c0d9f314a232201d2f16c137a9))
  *  update Slovak translation from Weblate ([5bd1b971](https://github.com/qTox/qTox/commit/5bd1b97170bc42561519448692d16c9f7fe3eee9))
  *  update Hungarian translation from Weblate ([d344e060](https://github.com/qTox/qTox/commit/d344e06016076265940dd1c293f946cd99df05c8))
  *  update Norwegian (old code) translation from Weblate ([64a83067](https://github.com/qTox/qTox/commit/64a8306731b1992cb49c79c3de8b921cb15ee92a))
  *  update Russian translation from Weblate ([a62535a5](https://github.com/qTox/qTox/commit/a62535a5e4acac41e95e2a186806d6f387fde9fd))
  *  update Russian translation from Weblate ([e9c9eeea](https://github.com/qTox/qTox/commit/e9c9eeea9f3bd60105b8eeeb15b3275102d3be7f))
  *  update Belarusian translation from Weblate ([652cdab8](https://github.com/qTox/qTox/commit/652cdab8d42101f6348dc183f5ad72a58082f8a2))
  *  update Romanian translation from Weblate ([fd7fe766](https://github.com/qTox/qTox/commit/fd7fe76697512ffc1188e0588b17c81f934207ba))
  *  update Estonian translation from Weblate ([d0394e88](https://github.com/qTox/qTox/commit/d0394e88e76752996bc24a8c179e17c0043d4160))
  *  update Spanish translation from Weblate ([00e33153](https://github.com/qTox/qTox/commit/00e3315360036b5db031139b2c4b05e6dca10eb5))
  *  update Serbian (latin) translation from Weblate ([6f4c675a](https://github.com/qTox/qTox/commit/6f4c675a46fe471fa00d9e63ae14306d0b88fe0d))
  *  update Serbian translation from Weblate ([1aec68fd](https://github.com/qTox/qTox/commit/1aec68fdf0c32a4a0a782c9ef256563818ebbf41))
  *  update Chinese (Simplified) translation from Weblate ([e828583b](https://github.com/qTox/qTox/commit/e828583b9af80c8aacbe208f2a6a831d8427e863))
  *  update Persian translation from Weblate ([b3ceda6e](https://github.com/qTox/qTox/commit/b3ceda6e47d485037ab3c1df9c2f433801d1b688))
  *  update Belarusian translation from Weblate ([07cf8838](https://github.com/qTox/qTox/commit/07cf8838b59059450b90a0c8cab7fb5285d02d59))
  *  update Estonian translation from Weblate ([235e6bfd](https://github.com/qTox/qTox/commit/235e6bfd34959d0f0d11da167e7b63339780963a))
  *  update Estonian translation from Weblate ([33d4cc4a](https://github.com/qTox/qTox/commit/33d4cc4a1dfc4c874eab8db916232e84000c7698))
  *  update Estonian translation from Weblate ([f82d3449](https://github.com/qTox/qTox/commit/f82d34492872148757dfddcc5663d9f7a21ffb71))
  *  update Estonian translation from Weblate ([53475ed2](https://github.com/qTox/qTox/commit/53475ed2dc334f4babebb14780dadc08b9b3e9a5))
  *  update Serbian translation from Weblate ([907c3e5c](https://github.com/qTox/qTox/commit/907c3e5c8b557f3051943d17ef6aa785f01b890f))
  *  update Serbian (latin) translation from Weblate ([f171c232](https://github.com/qTox/qTox/commit/f171c2327f19041b908ec4883d8d3426aa5f1e20))
  *  update Croatian translation from Weblate ([6f0b6925](https://github.com/qTox/qTox/commit/6f0b692562cbc303ac8f420f7a9aee82821ab8e8))
  *  add Serbian (latin) translation to UI ([641a8084](https://github.com/qTox/qTox/commit/641a80842e0d3fabb6b4872d2a342d167ff80958))
  *  add Serbian (latin) translation using Weblate ([66a680df](https://github.com/qTox/qTox/commit/66a680df8720880fda7254cedccb8ddd45f6ceed))
  *  add Serbian translation to UI ([3c4b1f11](https://github.com/qTox/qTox/commit/3c4b1f117d29293d6ebbb68457eb96452a9b557b))
  *  add Serbian translation using Weblate ([17c00487](https://github.com/qTox/qTox/commit/17c00487921d12c2f54ed25f08d943e6914231e1))
  *  add Persian translation to UI ([76df9a45](https://github.com/qTox/qTox/commit/76df9a453511e98693c7302b3f48598caff45d9e))
  *  add Persian translation using Weblate ([f2d3beb6](https://github.com/qTox/qTox/commit/f2d3beb67e734fc190fd331a1539a043590ffeee))
  *  update Lithuanian translation from Weblate ([7e80cbca](https://github.com/qTox/qTox/commit/7e80cbcaeee1c2187d3f31d83e0e31b615d5dac9))
  *  update Swedish translation from Weblate ([27a58b93](https://github.com/qTox/qTox/commit/27a58b938d381f6d9a39a78280f4bb5958b34193))
  *  update Portuguese translation from Weblate ([9b7e23c6](https://github.com/qTox/qTox/commit/9b7e23c68be190487cf131018e4a5a071aa23152))
  *  update Portuguese translation from Weblate ([227bba8f](https://github.com/qTox/qTox/commit/227bba8fa2624714d8a73e6cd4dc50aab5eaafea))
  *  update Portuguese translation from Weblate ([1425dff6](https://github.com/qTox/qTox/commit/1425dff6e7a711b5c053842620b8fdeaaed653ce))
  *  update Portuguese translation from Weblate ([8c07a2b2](https://github.com/qTox/qTox/commit/8c07a2b22503be0ededabb261a553db132a38109))
  *  update Portuguese translation from Weblate ([ccc5296f](https://github.com/qTox/qTox/commit/ccc5296fd30fe531de552abf76da3062c9e31c9f))
  *  update Estonian translation from Weblate ([8710563b](https://github.com/qTox/qTox/commit/8710563bd638d672271c62fab9a06ef4cb200313))
  *  update Estonian translation from Weblate ([d5e484b8](https://github.com/qTox/qTox/commit/d5e484b8d94469e866597d1794ec2d6eb0f6396f))
  *  update Norwegian (old code) translation from Weblate ([bcdbd2ca](https://github.com/qTox/qTox/commit/bcdbd2caef16a8f4bc11da287dba01db2ed1c640))
  *  update Romanian translation from Weblate ([2d2cdd01](https://github.com/qTox/qTox/commit/2d2cdd010a6cdd890cfea6bec2b7bd0a296a1ed6))
  *  update Belarusian translation from Weblate ([01b9bcd9](https://github.com/qTox/qTox/commit/01b9bcd9d214389cfe7f84fb1ac300bbb5bfb4bc))
  *  update Chinese (Simplified) translation from Weblate ([1514b800](https://github.com/qTox/qTox/commit/1514b80093c701318e187c8eb9ab93a4bd71822c))
  *  update Spanish translation from Weblate ([154002c0](https://github.com/qTox/qTox/commit/154002c0027eb7dea7f5b74e302ef29eb5a096fe))
  *  update Estonian translation from Weblate ([681661f6](https://github.com/qTox/qTox/commit/681661f61c55ba7b63e54b38594e9a85abb92454))
  *  update Romanian translation from Weblate ([47a83e3b](https://github.com/qTox/qTox/commit/47a83e3bf9d099dc084dd8098d43f73258c47b56))
  *  update Chinese (Simplified) translation from Weblate ([0145bb11](https://github.com/qTox/qTox/commit/0145bb11775d11dd2dc9b7025c2b1a0f8355fcc5))
  *  update Belarusian translation from Weblate ([753ea45b](https://github.com/qTox/qTox/commit/753ea45b6d1b0eb5744370f96eae0492af9fda79))
* **settings:**
  *  add an option to toggle identicons ([905ca770](https://github.com/qTox/qTox/commit/905ca7708662c6fe5520ad2707715075ad1aa41d))
  *  make audio quality setting persistent ([7ed2d97a](https://github.com/qTox/qTox/commit/7ed2d97aadab8aa0acb39e54aafaea06c0ebe57c))
  *  Add audio quality setting ([61eddc1f](https://github.com/qTox/qTox/commit/61eddc1f6b76f2da1a6d180426a2b7f53c757e2a), closes [#4693](https://github.com/qTox/qTox/issues/4693))
* **travis:**
  *  run tests on Windows ([21af6875](https://github.com/qTox/qTox/commit/21af6875cc7a1f5a311b8149ced9970570a4b2e5))
  *  Windows cross-compilation ([9358297a](https://github.com/qTox/qTox/commit/9358297af87ebb13dc4f74d2ced6033e38b25a5b))



<a name=""></a>
## v1.12.0 (2017-10-01)

This release provides many handy new features, most notable are mass import of
contacts from a file, improvements around images in the chatlog and an
experimental audio backend with echo cancelling.

#### Features

*   UI to import a list of contacts from a file ([0974da54](https://github.com/qTox/qTox/commit/0974da543a6b76851ffa86d24883c324651a8ff9), closes [#4181](https://github.com/qTox/qTox/issues/4181))
* **audio:**
  *  make the libfilteraudio dependency optional ([8652fe99](https://github.com/qTox/qTox/commit/8652fe99e3cb80fac22a601c89ba405962c5537d))
  *  add setting to switch between new and old audio backend ([9d0498e0](https://github.com/qTox/qTox/commit/9d0498e0cfdf654f4319675f019669dacde4dbcc))
  *  make echo cancellation work and improve some minor stuff ([5d60f09d](https://github.com/qTox/qTox/commit/5d60f09df4b59c3a42c7544fd92c78710dd9d5b0))
  *  add libfilteraudio ([9c603e86](https://github.com/qTox/qTox/commit/9c603e86546da199cd94f22f4e4f9349d526fad1))
  *  add alternative OpenAL backend ([c7157291](https://github.com/qTox/qTox/commit/c7157291679ccb0414789dd04acd43455ce6aafc))
* **build:**  Check sha256 of tarballs for Windows cross-compilation ([7f6f8a45](https://github.com/qTox/qTox/commit/7f6f8a4513bfa07356f7ac6a88da346d8c325e7a))
* **exif:**  Honour exif orientation tag ([414fa178](https://github.com/qTox/qTox/commit/414fa178b4908d8451ca5304ee9ca1ab7421b26a), closes [#1848](https://github.com/qTox/qTox/issues/1848))
* **l10n:**
  *  update Belarusian translation from Weblate ([d5c6d102](https://github.com/qTox/qTox/commit/d5c6d1027c4e9e892049cbd4fb4dff6829a99318))
  *  update Bulgarian translation from Weblate ([0e688da1](https://github.com/qTox/qTox/commit/0e688da1b0f6228478a5e18c5d2d63a369afcb43))
  *  update Chinese (Simplified) translation from Weblate ([78a0d339](https://github.com/qTox/qTox/commit/78a0d33942dcda153fac2fa30321227127cf60e1))
  *  update Estonian translation from Weblate ([035c158a](https://github.com/qTox/qTox/commit/035c158a9913389bd56faea4dcd5bd6b8f4a9de0))
  *  update Finnish translation from Weblate ([814a6ea0](https://github.com/qTox/qTox/commit/814a6ea0cf83c68b959cdf53b73e7e18fcf18231))
  *  update French translation from Weblate ([069feae3](https://github.com/qTox/qTox/commit/069feae311fef515eee948e90f4ed36a27160c87), [4abaf031](https://github.com/qTox/qTox/commit/4abaf03106d8d51b307d56d62f0ad7d1bfde8a4a), [53264f4c](https://github.com/qTox/qTox/commit/53264f4cf90357893f297d8a79580c9f7f697465), [5aa47be7](https://github.com/qTox/qTox/commit/5aa47be72a68d62b6bd8c3a6442bc43ad82b8800), [6917def2](https://github.com/qTox/qTox/commit/6917def238287a51458012983d27d70f0003d697), [8a94ad75](https://github.com/qTox/qTox/commit/8a94ad75efffe07da97725d9a1cbba9f87887b18), [8d3a6310](https://github.com/qTox/qTox/commit/8d3a631084f4281b07b910da63ebbf1a27275748), [a225eec5](https://github.com/qTox/qTox/commit/a225eec5fd36c9f154b90a9aaa148139d1961521), [d662866c](https://github.com/qTox/qTox/commit/d662866c413fccd0dc18fb1bdfba01e13023d1b5), [db4602ea](https://github.com/qTox/qTox/commit/db4602ead1cd7cbd031423429aca7747b96d5d03), [dbfc4794](https://github.com/qTox/qTox/commit/dbfc4794e53ed8523ba139031d03a512cd9a2dbb), [f2b648e4](https://github.com/qTox/qTox/commit/f2b648e4d3be5eba65c6444a544956133e9242db))
  *  update German translation from Weblate ([55425705](https://github.com/qTox/qTox/commit/5542570505bc4d7b3ca5e4ff33bbccf3b5e79f9a), [f0ca3bed](https://github.com/qTox/qTox/commit/f0ca3bed382cf4cc51118989cc39a87a12d37be2))
  *  update Hungarian translation from Weblate ([3b1a8ac7](https://github.com/qTox/qTox/commit/3b1a8ac704dabf14a60201a3fd114bf58db13cb7))
  *  update Lithuanian translation from Weblate ([c06abad8](https://github.com/qTox/qTox/commit/c06abad83ba2fc127502d272a4ee0c4e9c16a566))
  *  update Norwegian (old code) translation from Weblate ([868fd55d](https://github.com/qTox/qTox/commit/868fd55d8e30c9b17e1ae60a42c0640b4f3d6cad))
  *  update Polish translation from Weblate ([916f7aa2](https://github.com/qTox/qTox/commit/916f7aa250281c6beae8135df05aa627d18e6527))
  *  update Romanian translation from Weblate ([227516d2](https://github.com/qTox/qTox/commit/227516d206a8fea9f408861f5c63aa8c2a530f54), [e441db87](https://github.com/qTox/qTox/commit/e441db87d3b0f8c54f59848796bb76f56d47702b))
  *  update Russian translation from Weblate ([a6692e28](https://github.com/qTox/qTox/commit/a6692e28ee89fe56094d719d7235d281dfe07c6c))
  *  update Slovak translation from Weblate ([155812e4](https://github.com/qTox/qTox/commit/155812e444a4614c35e9c8ddeab619200c1d4643))
  *  update Swedish translation from Weblate ([57519ca7](https://github.com/qTox/qTox/commit/57519ca75de1f1426a3fe20ee78330733ce328aa))
  *  update Tamil translation from Weblate ([092cae62](https://github.com/qTox/qTox/commit/092cae62d05eada032f207ac3d13d55201905eee), [13c572c3](https://github.com/qTox/qTox/commit/13c572c393619f0da9777935b1c7bd8a5bcd437d))
* **login:**  Add command line argument to open login screen ([0906b8eb](https://github.com/qTox/qTox/commit/0906b8eb1f9a413e80320d002d15540736a6fabb), closes [#4673](https://github.com/qTox/qTox/issues/4673))
* **paste:**  Implement pasting images from clipboard ([5fed3736](https://github.com/qTox/qTox/commit/5fed37365f1d32f900814527bc8eb514986f2abd), closes [#1290](https://github.com/qTox/qTox/issues/1290))
* **settings:**  add group chat local member black list ([27ecace7](https://github.com/qTox/qTox/commit/27ecace752424b67924011e5838fd1f11857b3cf))
* **ui:**  output instructions when user tries to open a second instance ([48d65c26](https://github.com/qTox/qTox/commit/48d65c269a3dd3badd498b816f9d1cbfaa33bef4), closes [#3483](https://github.com/qTox/qTox/issues/3483))
* **video:**  Error message on call fail ([ac75f7b5](https://github.com/qTox/qTox/commit/ac75f7b5944dc6dfc0b7334b22f53a159d8e7bc7))

#### Bug Fixes

*   ATOMIC_FLAG_INIT can't be used inside braces initializer ([f790747f](https://github.com/qTox/qTox/commit/f790747f2adda60f8809bb0e1671f18c3f7c6168))
*   Return value ([f97c776a](https://github.com/qTox/qTox/commit/f97c776a8722c5f230cb709270559fdfcd77ba47))
*   Fix crash on auto accept click in AboutFriendForm ([bd6516ea](https://github.com/qTox/qTox/commit/bd6516eab707c3e4b208b937e91a7c0eef39fafa))
*   Fix crash on accept or reject friend request ([359a42f7](https://github.com/qTox/qTox/commit/359a42f78e8edf073d6132bca8954a55094385c5))
*   Keep open and share X11 connection ([ae5cb4bc](https://github.com/qTox/qTox/commit/ae5cb4bcc27754f46d30209bf1658e49ff559dc7))
* **IPC:**  don't double lock shared memory ([0bf27a00](https://github.com/qTox/qTox/commit/0bf27a000aa42559a5b04b7e03781a29885f6adf), closes [#4678](https://github.com/qTox/qTox/issues/4678))
* **audio:**
  *  Fix signed and unsigned comparation ([aa356bb7](https://github.com/qTox/qTox/commit/aa356bb73a7443720a0d73be1f5401940f014498))
  *  Echo cancelling supports only mono audio ([809c5e6b](https://github.com/qTox/qTox/commit/809c5e6b04c991960afcc66a308bad63ca473610))
* **autoaway:**  Improve autoaway algorithm. ([9fe503c7](https://github.com/qTox/qTox/commit/9fe503c7083b1e9f1600af0597455eb8237f8c6e))
* **avatar:**  Don't invert default avatar colour on connect ([be324b93](https://github.com/qTox/qTox/commit/be324b932b3e572b59f408faacd690110360dcbb), closes [#4629](https://github.com/qTox/qTox/issues/4629))
* **build:**
  *  Fix regression in how cmake finds libraries ([b7ef73d4](https://github.com/qTox/qTox/commit/b7ef73d401aabbd0f5ee6ccfcd1a9f0a962bdaab))
  *  Update toxcore and Qt versions for Windows cross-compilation ([80f1286e](https://github.com/qTox/qTox/commit/80f1286e7ca5025d3e9f93b4209f791e043ad367))
  *  Add libexif to the Windows cross-compilation ([95548c6a](https://github.com/qTox/qTox/commit/95548c6a5fcce7a31ffcaafcaec8650fd6f4c968))
  *  Fixed header file of libfilteraudio not being found ([8108bfdc](https://github.com/qTox/qTox/commit/8108bfdc358b5070d1dfbf57a02e25439ce09541))
  *  Windows cross-compilation didn't see libfilterudio ([b0b157e7](https://github.com/qTox/qTox/commit/b0b157e77bc87eae1f8235e7d15d76e82c992ce6))
  *  add install steps for libfilteraudio on OSX ([8f2401fe](https://github.com/qTox/qTox/commit/8f2401fee64d0b5535bd4d737a1caf2bffa30682))
  *  make the OpenAL backend build with OpenALSoft < 1.15 ([526701fa](https://github.com/qTox/qTox/commit/526701fa9f7a2a5c950e7b16b6578bf32bff4604))
  *  remove newline from timestamp ([0843b770](https://github.com/qTox/qTox/commit/0843b770c64204a078e0d73fe5f3b71b8ef17e52))
  *  fix sqlcipher header inclusion ([e7b5c93b](https://github.com/qTox/qTox/commit/e7b5c93bfe3d3fbcfe8659ab93c4d03492ddefb9))
* **chatform:**  Hide author on history like on new messages ([28979f57](https://github.com/qTox/qTox/commit/28979f577165b0ce2a8e67b8eea870218bdd0b21))
* **dependency:**  remove unused qt sql dependency ([5d159e02](https://github.com/qTox/qTox/commit/5d159e0203bdd0e72153cce9ca5c0fe8483ba0db))
* **desktop:**  Improved the comment in qtox.desktop ([38ef3ad6](https://github.com/qTox/qTox/commit/38ef3ad69de0dfa3b3126f6c8fbfcb6392aa46f2))
* **exit:**  delete heap-based QApplication before exit ([a601df31](https://github.com/qTox/qTox/commit/a601df31da0859255e1220d24f3c97eae9eb0cca), closes [#4648](https://github.com/qTox/qTox/issues/4648))
* **friends:**  Friend request button target location ([7f36ec92](https://github.com/qTox/qTox/commit/7f36ec9219e93fbbe2b790efc68215f5ec8bfb4b), closes [#4631](https://github.com/qTox/qTox/issues/4631))
* **friendwidget:**  Add correct index calculation ([ad58c176](https://github.com/qTox/qTox/commit/ad58c1767b2ab4ff8340076d5ede53aa6e889972))
* **group:**  Fix crash on group quit ([e606d3cb](https://github.com/qTox/qTox/commit/e606d3cb5573d956aa4df58fbe39a21b8c423860))
* **i18n:**  string `Call with %1 ended. %2` is no longer translatable ([56ac95da](https://github.com/qTox/qTox/commit/56ac95dad9a807b4799840d7b2c3b1c1c44869f2), closes [#4552](https://github.com/qTox/qTox/issues/4552))
* **ipc:**
  *  Reorder initialize list ([43c2308b](https://github.com/qTox/qTox/commit/43c2308b7bd845aa1db572642858f9dde762a28a))
  *  Call processEvent on timer timeout ([c2140e21](https://github.com/qTox/qTox/commit/c2140e21ad9a7edc893191f5e49307fc05933959))
* **l10n:**  use native language names ([817a9897](https://github.com/qTox/qTox/commit/817a9897c2e1289c7ceda19f0335582ec46ad343))
* **platform:**  handle terminating POSIX signals ([32b97cb9](https://github.com/qTox/qTox/commit/32b97cb9278e40c92337468fb61751cdf87a0542))
* **popup:**  Don't create two error popups for friend requests ([1f787cc4](https://github.com/qTox/qTox/commit/1f787cc475c804ef890dec7e24c51ba6c2a3f9c4), closes [#4633](https://github.com/qTox/qTox/issues/4633))
* **receipts:**  Prevent double message send for received receipt ([e9d63397](https://github.com/qTox/qTox/commit/e9d63397e1008e57a23c763aaa418fc65f57577b), closes [#2726](https://github.com/qTox/qTox/issues/2726))
* **threads:**  Always stop the camera device thread ([8522141b](https://github.com/qTox/qTox/commit/8522141b1e7db63dade973f1ba9048aecc7db41e))
* **tooltip:**  Set font colour to black for beige tooltip background ([fc1ed101](https://github.com/qTox/qTox/commit/fc1ed10199a4f34faf38081bf9b1a58ee5bdf033), closes [#4641](https://github.com/qTox/qTox/issues/4641))
* **toxme:**  Remove HTML tags from ID to un-break toxme integration ([5af1b4af](https://github.com/qTox/qTox/commit/5af1b4af73d05e8da4bbc174410f0854a810ecd6))
* **widget:**  Add removing friends and group on main window close ([638cdba5](https://github.com/qTox/qTox/commit/638cdba58d4ed506edb6eb5fadbeae8fa0c22995))
* **windows:**
  *  remove unneeded qt sql and fix problem when build dir exists ([68c5cdf0](https://github.com/qTox/qTox/commit/68c5cdf096fbdf5d18bad06b58241098f6873505))
  *  workaround docker problem when extracting with tar ([af15cd06](https://github.com/qTox/qTox/commit/af15cd06acf549df753abf9f8521e24de1586a07))



<a name=""></a>
## v1.11.0 (2017-07-16)

The most interesting new features that were added in this release are export of
history to plaintext file and outgoing call sound.

#### Performance

*   Optimize open/close device ([d704f5d2](https://github.com/qTox/qTox/commit/d704f5d21d7b33fbb52f89d9d2a5386015e6a3ce))
*   Pass std::function by reference ([365d703e](https://github.com/qTox/qTox/commit/365d703e8ab57fff4930062e2ecf65420dccebec))

#### Bug Fixes

*   Invoke device methods in deviceThread ([d86912ea](https://github.com/qTox/qTox/commit/d86912eacd10f9c6472e1ef920a258f0e1ef3b83))
*   Fix warning about stack protection ([efcad35f](https://github.com/qTox/qTox/commit/efcad35fd53279b32b7b169af256c0ac14d6e9d1))
*   Using foreach to iterate through CFLAGS_OTHER ([9a0632b0](https://github.com/qTox/qTox/commit/9a0632b0cf8bc78d1aae0f2e75b26b73cdb98e52))
*   Replace hardcored roaming path on QStandartPath ([f616ff36](https://github.com/qTox/qTox/commit/f616ff36b6db9142f6e49b2b8bc436b6652aa329))
*   Fix freez on circle removing ([4d15aed5](https://github.com/qTox/qTox/commit/4d15aed53fa8986747b450b42d23befc92600986))
*   gcc-7 build ([246e23ab](https://github.com/qTox/qTox/commit/246e23abf3aa64c494c6abf42c83944eb78f1b16))
* **IPC:**  Add update profileId in to IPC ([c2f82f78](https://github.com/qTox/qTox/commit/c2f82f7808ed075c9c31813151d2cf001c4c7d10))
* **bootstrap:**  qrencode url ([f90da3d0](https://github.com/qTox/qTox/commit/f90da3d07acdf25e7e871ee8447412bd315c5f02))
* **build:**
  *  disable linux specific stuff on windows ([a9d2b03c](https://github.com/qTox/qTox/commit/a9d2b03c7c8ea33e34c238e1079c66687a6e1ab0))
  *  when building without XSS extension ([ab622213](https://github.com/qTox/qTox/commit/ab62221375281ca1320bc7b437cbf33766827f94))
  *  add missing rcc options ([6b7c0c73](https://github.com/qTox/qTox/commit/6b7c0c738b8439df6d2966de4a1296d32805366f))
* **chatform:**  add space for current copy link ([41a781d3](https://github.com/qTox/qTox/commit/41a781d3f875a0af51aaa4c727ce94c6115c48ad))
* **cmake:**  fix icon for .exe file ([cf06cc7a](https://github.com/qTox/qTox/commit/cf06cc7a1d8c94965f6ed76ce3d499ce531d946e))
* **core:**
  *  use correct byte representation when bootstrapping ([4e5b1915](https://github.com/qTox/qTox/commit/4e5b191553d2dcd14e744aa4c9e81d8f4e5f66a7), closes [#4385](https://github.com/qTox/qTox/issues/4385))
  *  Add action message with friend request text ([b25f5b5e](https://github.com/qTox/qTox/commit/b25f5b5ed69681a0549a7ff3377704536b34c19d))
* **emojis:**  Add text variants for the blush emoji ([9a367ffb](https://github.com/qTox/qTox/commit/9a367ffbf010ad2698e32d2f1c24add9d6dbefb9))
* **macro:**  Fixed clang warning about USING_V4L ([dd59f5ee](https://github.com/qTox/qTox/commit/dd59f5ee90b6b05bd810596cfa3ad55c9b449776))
* **main:**  Use correct way to handle application quit ([0b5b3fcf](https://github.com/qTox/qTox/commit/0b5b3fcf1a91a8f163574a2650102fedead6431b))
* **message size:**  Replaced TOX_MAX_*_LENGTH with API calls. ([3963d3c1](https://github.com/qTox/qTox/commit/3963d3c150fa4afe2fe69f493da6452c783a5655))
* **profile:**  set maxLength on userName QLineEdit ([71a838b5](https://github.com/qTox/qTox/commit/71a838b53e75a239174a0de09988415f64325e36), closes [#4335](https://github.com/qTox/qTox/issues/4335))
* **qtox.pro:**  add openal.h and openal.cpp ([d78a9790](https://github.com/qTox/qTox/commit/d78a979021578d6837f4ccb17a352b1ebfaeb8ff))
* **settings:**
  *  set default values for some important settings ([da4f6222](https://github.com/qTox/qTox/commit/da4f6222d806ff74f799a7e380eca20ab9ff3342))
  *  compute toxcore version in runtime ([2d0a4e79](https://github.com/qTox/qTox/commit/2d0a4e794dd10818dc830c8384fc0f76f0184b7f))
* **ui:**
  *  Use native file picker dialog ([42a9534b](https://github.com/qTox/qTox/commit/42a9534b24fe789b23e2ab07e3c6963b503ea91a), closes [#3494](https://github.com/qTox/qTox/issues/3494))
  *  wrong size of svg images on buttons ([5b0bf9fc](https://github.com/qTox/qTox/commit/5b0bf9fc9b31cedac5c8dccc51c96852a9a1e1ce))
  *  add date message before info messages ([ed453598](https://github.com/qTox/qTox/commit/ed45359863cf617d94e8f39506971a515c495304), closes [#4388](https://github.com/qTox/qTox/issues/4388))
  *  do not save splitter state in multiple windows mode ([7e5387ca](https://github.com/qTox/qTox/commit/7e5387cad283877c422b2e6d4a8bd9404fff67cd))
  *  Set `Qt::Window` flag in multi windows mode ([6b767e47](https://github.com/qTox/qTox/commit/6b767e476551f9f7bb771803a3f356f3e45a8d9e))
  *  switch to settings after multiple windows mode ([0c98f6a5](https://github.com/qTox/qTox/commit/0c98f6a5488cb8d8b27471f3cfecd6dd7f582300))
  *  restore splitter state ([9d94cd0c](https://github.com/qTox/qTox/commit/9d94cd0c507afdbf4b69c8adcdd82546a6bc83b8), closes [#4387](https://github.com/qTox/qTox/issues/4387))
* **v4l:**  Fixed CMakeLists.txt for v4l on FreeBSD ([dfe696e2](https://github.com/qTox/qTox/commit/dfe696e25a26a163430aefff3ba7a75428050598))

#### Features

*   Add message count in tooltip to LoadHistoryDialog ([bb65a18d](https://github.com/qTox/qTox/commit/bb65a18de59677cbbe3f1568380c9149b4939466))
* **aboutform:**  Add OS name in issue template ([7b3bd45d](https://github.com/qTox/qTox/commit/7b3bd45d5089aacf3b72b4206dbff922cdef6dbb))
* **audio:**
  * OutgoingCallSound ([a06ad704](https://github.com/qTox/qTox/commit/a06ad7048e7f2a450da455e16b5d907566c3eedb))
  *  split the audio interface from the backend library ([28c2298a](https://github.com/qTox/qTox/commit/28c2298ad97e8aec6097d63ead55214bac8152ba))
* **chatform:**
  *  Highlight chat history ([3257770b](https://github.com/qTox/qTox/commit/3257770bfa61838ab398a31de28f623f7b28d77a), closes [#2296](https://github.com/qTox/qTox/issues/2296))
  *  Export chat history to file ([5e4ab769](https://github.com/qTox/qTox/commit/5e4ab76944ea5952b7123ca64102de4f50b4f4d3), closes [#4143](https://github.com/qTox/qTox/issues/4143))
* **l10n:**
  *  add Tamil translation ([37a93042](https://github.com/qTox/qTox/commit/37a93042601965e59c47adaad60598e3046808c3), [bf405e17](https://github.com/qTox/qTox/commit/bf405e17fb63036c23621b8abc96692fe813eb6b))
  *  update Arabic translation from Weblate ([3ab423da](https://github.com/qTox/qTox/commit/3ab423dad25b185d8d8fefd5dae14c4acee05c62))
  *  update Belarusian translation from Weblate ([0c95284e](https://github.com/qTox/qTox/commit/0c95284ecc5b05586915d1573184baa5a0b9872c))
  *  update Bulgarian translation from Weblate ([79e00832](https://github.com/qTox/qTox/commit/79e00832f6c8ee1e700449a02ab8781ba27e4c06))
  *  update Chinese (Simplified) translation from Weblate ([06519872](https://github.com/qTox/qTox/commit/065198724cc9cb6d9718b4f3ec2dfbd6f2a1eca0))
  *  update Esperanto translation from Weblate ([dfdf11d1](https://github.com/qTox/qTox/commit/dfdf11d17d4a7b3957097fb14dd2a7469c28b5fb))
  *  update Estonian translation from Weblate ([1d9e646b](https://github.com/qTox/qTox/commit/1d9e646b199fa615ebafdebdd2ba5b6014495bd5))
  *  update Finnish translation from Weblate ([e10c5247](https://github.com/qTox/qTox/commit/e10c5247deadb85aed06c673e9d671918192e7d4))
  *  update German translation from Weblate ([1196731f](https://github.com/qTox/qTox/commit/1196731fa1ef3ff79ad43faa5fe33ecbf5b6548f), [1ce73175](https://github.com/qTox/qTox/commit/1ce73175b3afa55cdd2d773bf18c29515bc0a6d0), [20d31c81](https://github.com/qTox/qTox/commit/20d31c81c9b9bf2f91f0e6a4cd0ec46fff6fea85), [32cd745a](https://github.com/qTox/qTox/commit/32cd745af62be64ab385a9aeb69d873431a5e14a), [d8cd871e](https://github.com/qTox/qTox/commit/d8cd871e7e8e57b8aab1d32d5502f7cdb5cc97ee))
  *  update Greek translation from Weblate ([b2501ed1](https://github.com/qTox/qTox/commit/b2501ed114fdad88dbe64d2f2c346eab053584ea))
  *  update Lithuanian translation from Weblate ([57f94139](https://github.com/qTox/qTox/commit/57f941398aac060c0fbb90ede1e80847905ff1c4))
  *  update Polish translation from Weblate ([5e432c5e](https://github.com/qTox/qTox/commit/5e432c5ecfb23d9fb78a76c1291f2d7f08c727e1))
  *  update Romanian translation from Weblate ([38374604](https://github.com/qTox/qTox/commit/383746043fa4c54a80769b2d57980e8785c5dda8))
  *  update Russian translation from Weblate ([317f250e](https://github.com/qTox/qTox/commit/317f250e68524f68c5f15a68bb722e0fe88fc76d), [3a252c9e](https://github.com/qTox/qTox/commit/3a252c9e96568a537baa24997a017efa8d4424d6), [d5831060](https://github.com/qTox/qTox/commit/d583106008bceb1e42624bd0dba007de7966d6f9))
  *  update Slovak translation from Weblate ([e8e1d41f](https://github.com/qTox/qTox/commit/e8e1d41ff56d45ae2e87961b19fe637aa561534f))
  *  update Spanish translation from Weblate ([0cd34485](https://github.com/qTox/qTox/commit/0cd3448508d0839919c07a677f5b643d111607c3))
  *  update Swedish translation from Weblate ([040f14b3](https://github.com/qTox/qTox/commit/040f14b38ee3b531b9e34ae0f50be1b93d3a247e))
  *  update Turkish translation from Weblate ([fd8decd5](https://github.com/qTox/qTox/commit/fd8decd52e10c8b128a757e2034ebd1aa1a37f78))
  *  update Ukrainian translation from Weblate ([bcc3c0ef](https://github.com/qTox/qTox/commit/bcc3c0ef92ef27c8dcd7284a6cfbf98cf6801c8f))
* **toxid:**  Show NoSpam and checksum parts of ToxID in colors. ([c0951a56](https://github.com/qTox/qTox/commit/c0951a5675bf6c8f6d2c4b05d18f48b88adae6a5))



<a name=""></a>
## v1.10.0 (2017-05-07)

The more prominent new features are an option to autojoin groupchats and
ability to select messages with triple click. More about new stuff below.

#### Features

*   stop using plaintext passwords in the code ([084f3b06](https://github.com/qTox/qTox/commit/084f3b06262b2ca8272238b2ec450d11840d7551))
* **audio:**  don't use dynamic memory allocation and remove resource leak ([b68ad4ce](https://github.com/qTox/qTox/commit/b68ad4ced2ce985c4e53b1469681ed3ec807d2b6))
* **chatform:**  mark message with triple click ([3acbc148](https://github.com/qTox/qTox/commit/3acbc148f51afc96bf5bd96e88da053bdd64f702))
* **groups:**  add option to automatically accept groupchat invites ([6a16a2bd](https://github.com/qTox/qTox/commit/6a16a2bdbc94f9eb20386794652f8a1766498a00))
* **l10n:**
  *  add Romanian translation to UI ([0192e927](https://github.com/qTox/qTox/commit/0192e9275f1ceb1cc4caa0b810f87da92d56b75a))
  *  add Romanian translation using Weblate ([8b22e962](https://github.com/qTox/qTox/commit/8b22e962edc1f3779525f678a2b1f1f99415c7f5))
  *  update Belarusian translation from Weblate ([57615230](https://github.com/qTox/qTox/commit/576152305aabb64b9b05e705f0eb0dfcb68c9329))
  *  update Chinese (Simplified) translation from Weblate ([9d6be415](https://github.com/qTox/qTox/commit/9d6be4151073762f323b497a9379cb67e5747898))
  *  update Esperanto translation from Weblate ([2c158eb5](https://github.com/qTox/qTox/commit/2c158eb57c0ed5dd50420589b8f706f99ea9b5af))
  *  update Estonian translation from Weblate ([e580edd1](https://github.com/qTox/qTox/commit/e580edd184c3426669014a036e7dd23db3c3dcae))
  *  update Finnish translation from Weblate ([2f8e8b84](https://github.com/qTox/qTox/commit/2f8e8b840b426c27539e1b2e40caa890343ad481))
  *  update French translation from Weblate ([0748b6c6](https://github.com/qTox/qTox/commit/0748b6c6c6abf16a1069c880554a1438ca268ea7))
  *  update French translation from Weblate ([38bb82e5](https://github.com/qTox/qTox/commit/38bb82e59bdde27c62b8d836dfae47283c3cfd22))
  *  update German translation from Weblate ([1dfb4a63](https://github.com/qTox/qTox/commit/1dfb4a630b865f7f0e17a1d948eb7e71eee98ddc), [87d66c01](https://github.com/qTox/qTox/commit/87d66c019c5607fdcc8d68167202967aa8f6f753))
  *  update Greek translation from Weblate ([6e6d32a7](https://github.com/qTox/qTox/commit/6e6d32a7405a28fb76f28a542d90e7318ad02a40))
  *  update Lithuanian translation from Weblate ([c970e065](https://github.com/qTox/qTox/commit/c970e065f681d8447c80c442df0f405f42ef1b97))
  *  update Russian translation from Weblate ([e6b76561](https://github.com/qTox/qTox/commit/e6b76561062e85633305d0653cdb01c3c50dba88))
  *  update Slovak translation from Weblate ([63d5caf3](https://github.com/qTox/qTox/commit/63d5caf37c03401398256a2443f10adc4b488850))
  *  update Spanish translation from Weblate ([615c3614](https://github.com/qTox/qTox/commit/615c3614974678b2e4a9fca7af015ec65bcde1b0))
  *  update Swedish translation from Weblate ([3145949c](https://github.com/qTox/qTox/commit/3145949cc27ed4445c6435726eb4e0f9b1eb655f))
  *  update Ukrainian translation from Weblate ([9dd40bec](https://github.com/qTox/qTox/commit/9dd40bec63b7470bdb455129a1d1e5a2ec60751c))
* **ui:**  grey out proxy settings if no proxy selected ([77aa2e92](https://github.com/qTox/qTox/commit/77aa2e92520de1323a3ac8ef9a154d0fdf041c0b))

#### Bug Fixes

*   fix segfault while trying to send messages ([b5f4628c](https://github.com/qTox/qTox/commit/b5f4628c8984d063cac385e5f2ae57e7371e58bd))
*   fix OpenAL name ([8a77723a](https://github.com/qTox/qTox/commit/8a77723a893e0211fd1f02e099408e8a92aa8f1c))
*   fixed segfault after accepting group invite ([532e05cb](https://github.com/qTox/qTox/commit/532e05cb0f135d763890f4146e14da036cc3b248))
*   set CSS font-weight of editbox  always to normal ([96b2977a](https://github.com/qTox/qTox/commit/96b2977a5f4ddad2be1d7d22b4628ff960f174aa))
*   remove useless line that caused a warning ([e9515f48](https://github.com/qTox/qTox/commit/e9515f4898aedfd9c70094d61a6f3b85debb9e3e))
*   fixed wrong formatting for multiple URL's in one message ([08208e9a](https://github.com/qTox/qTox/commit/08208e9aa515270a190ff85a1ba704c8a130c5cd))
*   silence various warnings ([f0cec44a](https://github.com/qTox/qTox/commit/f0cec44a8cb91a9867b5761fb3703a90691b892b))
*   Stop using deprecated avcodec_decode_video2 ([f07daaf1](https://github.com/qTox/qTox/commit/f07daaf1595e320a19408321fcb62fe76d48d118))
*   Remove unnecessary calls to av_frame_unref ([91414b4f](https://github.com/qTox/qTox/commit/91414b4fcef944765507112583787b887dd62966))
*   Stop using deprecated AVStream::codec ([9674bff0](https://github.com/qTox/qTox/commit/9674bff0cfe05fcd5b667afca6df5b214644939d))
*   silence warning by not using dynamic arrays on stack ([300ac12e](https://github.com/qTox/qTox/commit/300ac12e3f220265fdd4685efb8e75e9da0ef3cd))
* **UI:**
  *  rename `qTox.desktop` → `qtox.desktop` to fix displaying icon ([6de129fe](https://github.com/qTox/qTox/commit/6de129fef1122c20d356cfb5fe504cb2f0999128), closes [#4323](https://github.com/qTox/qTox/issues/4323))
  *  improve unclear message about text being resized in chatform ([c8ed535b](https://github.com/qTox/qTox/commit/c8ed535bbad3300c02871cbc718bee03accc6c26))
* **chatform:**  disable Tab in add friend message text  area ([195e891d](https://github.com/qTox/qTox/commit/195e891da156b87c4973b13e5decfb95445d885f))
* **core:**
  *  add missing nullptr check ([407413c6](https://github.com/qTox/qTox/commit/407413c65b1d7a43a1788e00935c02fa2d9ffe33))
  *  don't continue when the tox file is corrupted ([20db8b74](https://github.com/qTox/qTox/commit/20db8b740bcccd0889b4d19210f98c1f902b87bd))
* **groupinvite:**  prevent multiple groupinvites to the same group from showing up ([13029e30](https://github.com/qTox/qTox/commit/13029e3047cf021ff98ea816008e0f514b79fc0c))
* **includes:**  Added missing #include <ctime> ([e597d391](https://github.com/qTox/qTox/commit/e597d391fcd9b1630ae2b049b361549d4a34f9fc))
* **profile:**  toxsave wasn't correctly encrypted ([5b31cf6d](https://github.com/qTox/qTox/commit/5b31cf6d9d188d064b7151cdb21959b064c0cef6))
* **settings:**
  *  remove legacy code ([6a6e30d6](https://github.com/qTox/qTox/commit/6a6e30d6393f29b0e15b61d31267bcd60a9c8426))
  *  changing language sets title to "Add Friend" ([4886868e](https://github.com/qTox/qTox/commit/4886868eceec38bdb13031f36fb916828a399f0a))
* **ui:**  fixed typos of pseudo-states in stylesheets ([f35103e5](https://github.com/qTox/qTox/commit/f35103e547b517106cb5048b43dfe6dece5fe8d8))
* **widget:**  Add friendWidget deletation ([c7202c8b](https://github.com/qTox/qTox/commit/c7202c8b243453d9630a51d7de1f745b4b865abb))



<a name=""></a>
## v1.9.0 (2017-03-19)

The most noticeable change to UI are moved buttons in file transfer widget,
lessening chance of an accidental cancellation of a file transfer.

#### Bug Fixes

*   Fix incorrect headers order ([0fc39113](https://github.com/qTox/qTox/commit/0fc39113183afcf1bc0750b8727a48634d8a623a), closes [#4220](https://github.com/qTox/qTox/issues/4220))
*   emoji packs path in user manual ([a3e64618](https://github.com/qTox/qTox/commit/a3e646182a719edd135bcf7308e7b37a27ab09f9))
*   URL's are not affected by markdown ([18c3f3a2](https://github.com/qTox/qTox/commit/18c3f3a2170f32f1045a92290456a86737eb5254))
*   iterate all blocks ([7a5c5a86](https://github.com/qTox/qTox/commit/7a5c5a86fd82ab9812db019db2e98d700e08a900))
*   fixed documentation mistake ([e8ed7e1f](https://github.com/qTox/qTox/commit/e8ed7e1f530b380d0b676933e6298327aeeb4398))
*   Add splitter restorer ([a231532b](https://github.com/qTox/qTox/commit/a231532bd06a4a47432b5bc20d6d34028ca66941))
*   changed incorrect videocall button offline tooltip ([0effac2b](https://github.com/qTox/qTox/commit/0effac2bdd8834bd3b1edcc91f3c8ffed03abfe6))
*   Autoupdater trying to open files twice could fail ([20ff68c3](https://github.com/qTox/qTox/commit/20ff68c3a69ec8c92d06b8e36b8e8443d76e094f))
*   msleep in toxuri processEvents loops, to avoid 100% CPU ([2d0698af](https://github.com/qTox/qTox/commit/2d0698af95959a895e1cb86721d288d8ef5f3bae), closes [#1926](https://github.com/qTox/qTox/issues/1926))
*   Don't even try to add ourselves as a friend in the Tox URI handler ([bfda028a](https://github.com/qTox/qTox/commit/bfda028a2a3ef63181dbc8ffa42817e08e616f20))
*   Various IPC event handling and related bugs on startup ([c75ee8a6](https://github.com/qTox/qTox/commit/c75ee8a6619e6c546121462d28e221bb8df11f19), closes [#1926](https://github.com/qTox/qTox/issues/1926))
*   Friend list avatars not updating ([055c41bc](https://github.com/qTox/qTox/commit/055c41bc30de6901b63d14dbc50e384de5779d16))
*   Accept IDs as tox URIs, not just ToxDNS addresses ([1d307bcc](https://github.com/qTox/qTox/commit/1d307bcc0e3bde3282615f5f2db08578e7cf4984), closes [#1925](https://github.com/qTox/qTox/issues/1925))
* **UI:**  actually load bundled Unicode font as soon as possible ([bfd47733](https://github.com/qTox/qTox/commit/bfd47733c241677163e38b0bc28a9de87f4dc535))
* **audio:**  alternate audio fix implementation from #4139 ([62ac4801](https://github.com/qTox/qTox/commit/62ac480171e6b073689344ee8eb65430485166de))
* **cmake:**  Add ability to real disable optional dependency ([fb43e306](https://github.com/qTox/qTox/commit/fb43e306774db7b040fde550c9d4c479793d1bad))
* **core:**  Ignore online connection status ([ea50eaae](https://github.com/qTox/qTox/commit/ea50eaaef742141ec2023050d5d1ee8719583333))
* **db:**  Made RawDatabase::execLater executes statements asynchronously. ([54fb9f73](https://github.com/qTox/qTox/commit/54fb9f73ccb70735f0b551faaa2e713004a3864d))
* **font:**
  *  Made the font combobox not editable. ([ba28f163](https://github.com/qTox/qTox/commit/ba28f163fdd243998f0962cf4ad2ca5e5134d9f5))
  *  Made font changes in settings apply on screen instantly. ([742583bb](https://github.com/qTox/qTox/commit/742583bb9ed11eebea2686f985b7dd254f5a6d89))
* **icons:**  Removed unnecessary icon preparation. ([fac0021a](https://github.com/qTox/qTox/commit/fac0021a4752bca7d5a72fc2c1c464d59ba81754))
* **qmake:**  add forgotten files to qTox.pro ([db2e2d7a](https://github.com/qTox/qTox/commit/db2e2d7ab77836d65c27a57ece69220eeb17e0e9))
* **toxencrypt:**  return the plaintext after decryption ([14ee8af3](https://github.com/qTox/qTox/commit/14ee8af3cd156437b37ada4d06b07ccfbd2d3d4a))

#### Features

*   Switch windows portable links to stable versions ([b771f87e](https://github.com/qTox/qTox/commit/b771f87e0becbc649a5a3f0823efe153b6ffe2cb))
*   Add version numbers in windows installer download links ([b73770d7](https://github.com/qTox/qTox/commit/b73770d703e98d47e9635918ad635eadcbbe2852), closes [#4112](https://github.com/qTox/qTox/issues/4112))
* **UI:**  Move filetransferwidget buttons side-by-side. ([9322f29e](https://github.com/qTox/qTox/commit/9322f29ef4d6b45fb66623ef0655a496a3064b78), closes [#2597](https://github.com/qTox/qTox/issues/2597))
* **l10n:**
  *  add Chinese (Taiwan) translation to UI ([43d16d91](https://github.com/qTox/qTox/commit/43d16d91817f1224702b55aa2d6d53fb2aabd242))
  *  add Chinese (Taiwan) translation using Weblate ([b80e964d](https://github.com/qTox/qTox/commit/b80e964d18763fcb5b3f2699789d17e95607bd9c))
  *  update Belarusian translation from Weblate ([6fe20861](https://github.com/qTox/qTox/commit/6fe20861485e1261a4da67b15212825cd5fd5126), [a335e32a](https://github.com/qTox/qTox/commit/a335e32af50788e2e94cd4a1db65344f62e7323f), [f92bbbb9](https://github.com/qTox/qTox/commit/f92bbbb9dd4b37535e80e7687935958d43592299))
  *  update Bulgarian translation from Weblate ([6df92b73](https://github.com/qTox/qTox/commit/6df92b73629933f1d0a82da1b6ff4e7f8193670b))
  *  update Chinese (China) translation from Weblate ([95881c97](https://github.com/qTox/qTox/commit/95881c975dee764f1c545da99ddfcd9013ed54ac), [bb3ee7d0](https://github.com/qTox/qTox/commit/bb3ee7d040edfe0389461f677cff0d691f5e7071), [e0cb8e5f](https://github.com/qTox/qTox/commit/e0cb8e5f530cfaeb640ab3754b36f8e8a45f789a), [fbe0de9b](https://github.com/qTox/qTox/commit/fbe0de9bbbce117f81c6ce2f52ac29fcd9854543))
  *  update Chinese (Simplified) translation from Weblate ([ae676246](https://github.com/qTox/qTox/commit/ae676246106f4890f34099840a92536d4831b92d))
  *  update Chinese (Taiwan) translation from Weblate ([423fb83c](https://github.com/qTox/qTox/commit/423fb83c9f80a6b01f00ea980fc67e7fd1c0fbdb))
  *  update Chinese (Traditional) translation from Weblate ([9fadde67](https://github.com/qTox/qTox/commit/9fadde67a14fc4d125bab9e5f285ee42206f2bcc))
  *  update Czech translation from Weblate ([99d34b04](https://github.com/qTox/qTox/commit/99d34b049db7709c42cb0bf074aff9db1ab7d11e))
  *  update German translation from Weblate ([015c8b00](https://github.com/qTox/qTox/commit/015c8b009264cd978b2d50ddab2460ec5697d6e6), [036ea14b](https://github.com/qTox/qTox/commit/036ea14b2689d39d89e9cd35782536048227b806))
  *  update Greek translation from Weblate ([2551baab](https://github.com/qTox/qTox/commit/2551baab46415cea6f0f9bca48f65189cb201cc3), [cebfe4a6](https://github.com/qTox/qTox/commit/cebfe4a63bfcda211b366a9fe61f500fee898760))
  *  update Italian translation from Weblate ([c6ca1518](https://github.com/qTox/qTox/commit/c6ca15188384993863dce217e3de815407be7084))
  *  update Korean translation from Weblate ([9fca7365](https://github.com/qTox/qTox/commit/9fca736506454779892b447eeaee8fdaf2e42f5e))
  *  update Lithuanian translation from Weblate ([77ee1787](https://github.com/qTox/qTox/commit/77ee1787b11687bfc467880c720768c472c3a9f1))
  *  update Pirate translation from Weblate ([6d2cf522](https://github.com/qTox/qTox/commit/6d2cf5222b83ed58562a766fafd0dac546c2ead8))
  *  update Russian translation from Weblate ([17413c08](https://github.com/qTox/qTox/commit/17413c08641fddbc872176bc03f16807d6307531), [4d22b7cc](https://github.com/qTox/qTox/commit/4d22b7cc97401e447947e1150de8414e89e951eb), [c2b7d7a5](https://github.com/qTox/qTox/commit/c2b7d7a533838ae8382bc42f953e15470149f3e2))
  *  update Slovak translation from Weblate ([a6665fe0](https://github.com/qTox/qTox/commit/a6665fe028cabb1269f0143bc2ebfda189b63427), [cf5e7f3a](https://github.com/qTox/qTox/commit/cf5e7f3ad194f171ffbd835a01ac20861fffba65))
  *  update Swedish translation from Weblate ([8eeee452](https://github.com/qTox/qTox/commit/8eeee4526c83b1b043bd517c0e5152e6fcd3b480), [c5067f65](https://github.com/qTox/qTox/commit/c5067f65d1845cbc89b4b4910283183807c45c5b))
* **settings:**  create ui when needed instead of show/hide ([12bcc261](https://github.com/qTox/qTox/commit/12bcc26154722a1f5028ae55709830c89a3d3b69))
* **shortcut:**  Implemented F11 shortcut for toggling fullscreen. ([3a20a4ba](https://github.com/qTox/qTox/commit/3a20a4ba43e609c96385f25c1a9a20e55cbfa4fa))
* **toxencrypt:**  add function to get min length for isEncrypted() to succeed ([a5955c67](https://github.com/qTox/qTox/commit/a5955c67ede3de72d4b9b76730216bdcc8f62334))
* **toxpk:**  Add 'ToxPk::getPkSize()' method ([f5c0d61a](https://github.com/qTox/qTox/commit/f5c0d61ae6cda2106c9d88ef7050b4003e725489))



<a name=""></a>
## v1.8.0 (2017-02-05)

With this version build system has been migrated to `cmake` and `qmake`
support is now deprecated and will be removed in the `v1.9.0` release.


#### Bug Fixes

*   Now cannot send party invite to a friend which has "Offline" status ([034c507c](https://github.com/qTox/qTox/commit/034c507cc4fa83210a40b92c5aa458e9daf5ba5a))
*   Add workaround for FreeBSD and ICU ([22d3aa11](https://github.com/qTox/qTox/commit/22d3aa11debfc54ad32f3a4642ea81da058c0ee2))
*   Remove old ChatForm code from Widget ([52642578](https://github.com/qTox/qTox/commit/52642578a0d66c79c03b6dbfe0ab2af86a30302b))
*   Further Windows Jenkins link errors ([b6934927](https://github.com/qTox/qTox/commit/b693492702a5f65d278baf8d38b4f1a9ea537089))
*   Jenkins Windows builds ([debb6e9c](https://github.com/qTox/qTox/commit/debb6e9cc897de62687ad39880059472d0eb8e48))
*   fix warning about missing (removed) `cdata.h` ([4972ac1d](https://github.com/qTox/qTox/commit/4972ac1d537bab37aa73635c49067d75913f5662))
*   Fix problem with unicode symbols on FreeBSD ([56d65650](https://github.com/qTox/qTox/commit/56d656502c82d81e5339fad7eb7bc77545adea8b))
* **UI:**  fix own status message not being properly displayed in tooltip ([dccef4d4](https://github.com/qTox/qTox/commit/dccef4d49f62d10427f4a625a9595da881a1370d))
* **addfriend:**
  *  create ToxId from QString before emitting the signal ([a5d2c573](https://github.com/qTox/qTox/commit/a5d2c573ddca775567024ee9e64fe735146b1807))
  *  do not load own Tox ID from clipboard ([efcff576](https://github.com/qTox/qTox/commit/efcff57601a3dc04fc7376b6edbeff81487dc5b8))
* **avatar:**  wrong conversion of own ToxId ([0e4cff88](https://github.com/qTox/qTox/commit/0e4cff884db58e31e7a269721bc9e6138174b9b0))
* **build:**
  *  avoid version checking where unnecessary ([bf8b23e3](https://github.com/qTox/qTox/commit/bf8b23e373275f0d34d99ff79d0637f2fb558d18))
  *  do not invoke version update on travis ([07639011](https://github.com/qTox/qTox/commit/07639011daa5ff84b61e76b0ad0dc8e659c5a68b))
  *  attempt to make travis happy ([919a94b9](https://github.com/qTox/qTox/commit/919a94b978aa957ed3b7702600b90f72a952eb7e))
  *  set generic macOS Info.plist version by default ([3a88abdd](https://github.com/qTox/qTox/commit/3a88abddd7cb96bbff48d5c6601798173328590f))
  *  correct macOS deployment ([8912e9e0](https://github.com/qTox/qTox/commit/8912e9e016d7591eb1a1d1f782c90bd8da7b16c6))
  *  add an option to compile with autotools toxcore on Windows ([482757e6](https://github.com/qTox/qTox/commit/482757e6a2ab74410f33fca364af1b69f2695ae4))
  *  fix ASLR and DEP protection for windows builds ([ce0590d2](https://github.com/qTox/qTox/commit/ce0590d2d0788a3d0cb9318211e3c79761b49492))
* **chatform:**
  *  Add ability to cancel call ([320099fa](https://github.com/qTox/qTox/commit/320099faf80ca51e60c68953463a60b50b23e4c2))
  *  Fixed buttons ([aff0c6f3](https://github.com/qTox/qTox/commit/aff0c6f3a5a6f883329b4e20598ff72852fe2cca))
* **clipboard:**  Made clipboard operations work the same on all OSes. ([a18e0b6a](https://github.com/qTox/qTox/commit/a18e0b6a2cbea94229598c9e777ceea1aa46fff7))
* **cmake:**
  *  Add search for toxcore or libtoxcore ([a4537549](https://github.com/qTox/qTox/commit/a4537549c0c53a88a47ca7a06cf276f0430eb43e))
  *  Fix smileys resource adding ([fddda5de](https://github.com/qTox/qTox/commit/fddda5de61d302abe3f075dc35ef296c8a29a82f))
* **font:**  Fixed the editbox font reset problem ([c84837d6](https://github.com/qTox/qTox/commit/c84837d662b6838a55caae7b3d4a121abd2d0192))
* **icon:**
  *  add more size versions to qtox.ico ([ec95d7f5](https://github.com/qTox/qTox/commit/ec95d7f571431a8d46158266123cd0806fc4fdc3))
  *  correct the shape of the logo ([f258cf20](https://github.com/qTox/qTox/commit/f258cf20e243c7b426cb4096108924871b4958d0), closes [#4114](https://github.com/qTox/qTox/issues/4114))
* **icons:**  install icons for linux ([6b513819](https://github.com/qTox/qTox/commit/6b51381980e4d742efff0905a697040efbcce049))
* **jenkins:**  preserve the correct linking order ([5d4609bd](https://github.com/qTox/qTox/commit/5d4609bd0ee7854b27791c856ca7de93491232d1))
* **l10n:**  rename `zh` translation to `zh_CN` ([1df9f572](https://github.com/qTox/qTox/commit/1df9f5726dcd2605245f4164c7462922ef9464fb), closes [#4080](https://github.com/qTox/qTox/issues/4080))
* **nexus:**  Add qRegisterMetaType call for ToxPk ([328d0b6d](https://github.com/qTox/qTox/commit/328d0b6ddfe4f74ab701d926503860c6b1c77c1a))
* **osx:**  Remove useless '#ifdef Q_OS_MAC' ([f0467abc](https://github.com/qTox/qTox/commit/f0467abc34f3dc2308b9cd4473550e8f1d4ce787))
* **toxid:**  Tox ID construction from ToxMe ([a223510c](https://github.com/qTox/qTox/commit/a223510cf7b8ca72dce7b2f3d6ed9c6ca2284fab))
* **warnings:**  fix some warnings about the stack protector not working ([e7276e7b](https://github.com/qTox/qTox/commit/e7276e7b43dd7b8631074866c27dfd70e40ae9f5))

#### Features

* **UI:**  remove unnecessary spacings on the Profile page ([1fc412df](https://github.com/qTox/qTox/commit/1fc412df72156b264f77e62ba395fe25eeb39514))
* **chatform:**  Changed mic and vol buttons ([5381d55e](https://github.com/qTox/qTox/commit/5381d55e942bb08ce6ff870626a0e10bfaa2358c))
* **l10n:**
  *  update Belarusian translation from Weblate ([1494fbe4](https://github.com/qTox/qTox/commit/1494fbe4e1c3399b1b7cacde7dc4189c87c89763), ([f938fef2](https://github.com/qTox/qTox/commit/f938fef23366681114099a6518b0612262681bac))
  *  update Chinese (China) translation from Weblate ([65f29811](https://github.com/qTox/qTox/commit/65f29811e39de1cdf9402c0e68cda905f2e87821), [cf6e83ba](https://github.com/qTox/qTox/commit/cf6e83baf1bdd3a689ac9fd5c0bf652ce6cc736d))
  *  update Czech translation from Weblate ([9676e8c9](https://github.com/qTox/qTox/commit/9676e8c97a3ba7c2a12c9c4511d1db75c3006c04), [b7ec9016](https://github.com/qTox/qTox/commit/b7ec90167e89679552b4953ac3bcbb602c6dae4e))
  *  update Estonian translation from Weblate ([3d639e00](https://github.com/qTox/qTox/commit/3d639e007c235ca556aaed69bc88e6cbc5434dc3))
  *  update Finnish translation from Weblate ([48ebc821](https://github.com/qTox/qTox/commit/48ebc821d8568831a55322ee6b2988bcb12f8fff))
  *  update French translation from Weblate ([275fdc6a](https://github.com/qTox/qTox/commit/275fdc6aa08222e0441f042ccfa50cc1c1922c68), [c990dfd7](https://github.com/qTox/qTox/commit/c990dfd7cf6090d2e68ee29990853061cca12d83), [cd2b1b1a](https://github.com/qTox/qTox/commit/cd2b1b1ae62d6fc0ac27b860a78cced7acbacafb), [d82f0670](https://github.com/qTox/qTox/commit/d82f0670e301edee962d1ccb5f177da80a23d9a3), [edab0fe3](https://github.com/qTox/qTox/commit/edab0fe3fd87df673b8563ac115d34d42c92ed9c))
  *  update German translation from Weblate ([2c8c5c52](https://github.com/qTox/qTox/commit/2c8c5c5259387a57c4de5e80e08227523a419478), [6a492ea4](https://github.com/qTox/qTox/commit/6a492ea4277e07c468fbfce74252b0c384962502))
  *  update German translation from Weblate ([ac7352db](https://github.com/qTox/qTox/commit/ac7352db0574935b43116924c0bbda8e9757da01), [e5f1cdcb](https://github.com/qTox/qTox/commit/e5f1cdcbc42183d088fe8dfc3433a11b9497b825))
  *  update Hungarian translation from Weblate ([29715335](https://github.com/qTox/qTox/commit/29715335782137e985e408f412ccf3e5dfeba043))
  *  update Korean translation from Weblate ([c8660c8b](https://github.com/qTox/qTox/commit/c8660c8b80238a8698a9f6ccc74aeca7bfac90f4))
  *  update Lithuanian translation from Weblate ([7fab1632](https://github.com/qTox/qTox/commit/7fab1632d501693611c6db762f4454773f9ad336), [c6fa538e](https://github.com/qTox/qTox/commit/c6fa538ee547c27dd03dfcd37ae7a7d5f09131ed))
  *  update Pirate translation from Weblate ([d570c2b3](https://github.com/qTox/qTox/commit/d570c2b3cd213a6c4a8b0618e222e4e1d5683048))
  *  update Polish translation from Weblate ([a6f52704](https://github.com/qTox/qTox/commit/a6f5270461db2dbd25f8589fe4dcbdfb5478aef3))
  *  update Russian translation from Weblate ([5e04b72a](https://github.com/qTox/qTox/commit/5e04b72afd1261f787551930e0f3164b44e81891), [8bcaa221](https://github.com/qTox/qTox/commit/8bcaa221c8545301b462daa49b39af4b1d7e44df), [8c868e2c](https://github.com/qTox/qTox/commit/8c868e2cf587f7e465e550c29235ebb20ae2abf7), [ce69f2b9](https://github.com/qTox/qTox/commit/ce69f2b997ea6d8c19d8479964c8a33da949b8ce))
  *  update Slovak translation from Weblate ([3f0ee63e](https://github.com/qTox/qTox/commit/3f0ee63e02c842e0e10570f25f2146355ee841e8), [4278d165](https://github.com/qTox/qTox/commit/4278d16554f5e0efb1ae427582e39bbfffeec75b))
  *  update Spanish translation from Weblate ([09727af0](https://github.com/qTox/qTox/commit/09727af00b17ad3427ddbba1dc916765191a1725))
  *  update Swedish translation from Weblate ([bd965086](https://github.com/qTox/qTox/commit/bd9650862f9230c8eccef60d40a69fa0a15dcf71))
* **toxid:**
  *  reduce passing Tox IDs around ([e07d8d35](https://github.com/qTox/qTox/commit/e07d8d358f6fc890a77e029aa230b69bdecd325e))
  *  Improve validation of Tox IDs ([d196611f](https://github.com/qTox/qTox/commit/d196611ffeeccc200d42047c85af3f7fe93f0730))
  *  Improve the ToxId class ([94ec5614](https://github.com/qTox/qTox/commit/94ec5614f2bb50d7d487d5b933393ed0e3c05e03))
  *  Added correct checksum validation. ([b4c45692](https://github.com/qTox/qTox/commit/b4c4569299d734d8056e48a6d6dd62f4df2ae86f))



<a name=""></a>
## v1.7.0 (2016-12-25)


#### Breaking Changes

* **database:**  use own public key as salt to encrypt the database ([c4b9d302](https://github.com/qTox/qTox/commit/c4b9d302d0c3007102436f7f76498a7a0b8edcaf))

#### Features

*   Added Kiss smiley shortcut for Universe and emojione ([059bb9a3](https://github.com/qTox/qTox/commit/059bb9a3a2ebf4c4310723ab386babc96c77cc15))
* **UI:**
  *  make font in message input field use font settings ([c14cbdf4](https://github.com/qTox/qTox/commit/c14cbdf4fd99352d1a70ad0c9dfeb78e309b1228))
  *  add `My` to text labels on Profile page ([785e546e](https://github.com/qTox/qTox/commit/785e546e9e76144f2f39135cd425e0d924c960dd))
  *  make information text in settings selectable ([e9778e81](https://github.com/qTox/qTox/commit/e9778e81a238d2688ba5106b602d08208a9311a9))
  *  make it possible to select text of the received friend request ([06606958](https://github.com/qTox/qTox/commit/0660695873ef8ab1f5e31761c113065560d3a75b))
  *  Add 'Copy link' context menu item ([a7de2680](https://github.com/qTox/qTox/commit/a7de2680d971337c697357c052dc90ddb468883f), closes [#927](https://github.com/qTox/qTox/issues/927))
  *  make link to filled issue template on About page visible ([feaedeed](https://github.com/qTox/qTox/commit/feaedeed841f419a8eb5b7f40a14e27ef1e104cb))
  *  add accessibility labels to all remaining .ui files ([25e9d3c4](https://github.com/qTox/qTox/commit/25e9d3c46cf717bd623be1bda8dbc7a8541d84e1))
  *  add accessibility labels to the add friend page ([8854589b](https://github.com/qTox/qTox/commit/8854589b0d3ddc50bf899413c1ef970c43d127c4))
  *  add accessibility labels to the Profile page ([949e3cb8](https://github.com/qTox/qTox/commit/949e3cb830134a1412b93d751a1d16dd80877aba))
  *  add some accessibility labels to login pages ([953fe241](https://github.com/qTox/qTox/commit/953fe2416a97ebdeb6de45a801e66fc3b434d551))
  *  rename `Toxme register` → `Register on ToxMe` ([2433fa9d](https://github.com/qTox/qTox/commit/2433fa9dcee8c10a9d8ec050b8bf06c0facdce7e))
* **database:**
  *  make a backup before upgrading ([c1d471fa](https://github.com/qTox/qTox/commit/c1d471faa18ecb8f04001b97937e14d6c34a9a46))
  *  use own public key as salt to encrypt the database ([c4b9d302](https://github.com/qTox/qTox/commit/c4b9d302d0c3007102436f7f76498a7a0b8edcaf))
* **l10n:**
  *  add Korean translation ([6e040b8e](https://github.com/qTox/qTox/commit/6e040b8e7c50b53b13c8bd46a6b38df7b34a0e33), [5267b195](https://github.com/qTox/qTox/commit/5267b19503a815a269b4cc7bc4fb77b270157467))
  *  add Pirate translation ([93feae0c](https://github.com/qTox/qTox/commit/93feae0c95bb9a6b3a8bbe1022c403e8167bc7b3), [c75f0b74](https://github.com/qTox/qTox/commit/c75f0b74a29449dac1c3320e4ea2c1702b03c4c9))
  *  add Swahili translation ([f28e136e](https://github.com/qTox/qTox/commit/f28e136ec45b0f7ec26c4ebaa6e7eef7b39f9c0d), [8b671323](https://github.com/qTox/qTox/commit/8b67132337f3c90a9df8ed0ed06a7021386b6774))
  *  update Arabic translation from Weblate ([1969084b](https://github.com/qTox/qTox/commit/1969084baa892a2fab957f5c326973efd0b938d8), [b0d03622](https://github.com/qTox/qTox/commit/b0d03622b2a244cef4fbe8902aca53efcbebd624))
  *  update Belarusian translation from Weblate ([10b09036](https://github.com/qTox/qTox/commit/10b09036ca9dab7a9670b5b421897bbc23250d62), [899d74f1](https://github.com/qTox/qTox/commit/899d74f1d95136eb9e97d1ebdcbcd0fde46753f5), [a3c876c1](https://github.com/qTox/qTox/commit/a3c876c14af4a9fa581c7c627b81a7ec1fe821c5), [b8aef1dc](https://github.com/qTox/qTox/commit/b8aef1dcebae378eb08c245ffb6d83cb3876e3b0), [bb445a8e](https://github.com/qTox/qTox/commit/bb445a8e7155b18683ff77235ba64428b1b20069), [fa550a12](https://github.com/qTox/qTox/commit/fa550a127f2a4bc9eb3c4f2e4f466ba773845a5b))
  *  update Bulgarian translation from Weblate ([595657c2](https://github.com/qTox/qTox/commit/595657c232f0826170c61ae605b831c495b58d34), [88049cc2](https://github.com/qTox/qTox/commit/88049cc2277810f0fa5e30c65c4732f4aa25db04))
  *  update Chinese translation from Weblate ([0f5b2151](https://github.com/qTox/qTox/commit/0f5b2151ee334b397e1eaaefd49e47e2a71e2c90), [3a656bb3](https://github.com/qTox/qTox/commit/3a656bb3ca623b9ec33b6130eeab0c8212874cf0), [b4c740b1](https://github.com/qTox/qTox/commit/b4c740b1b70bcf7c6ab78f2c3b99c018da9bae70), [db990ac7](https://github.com/qTox/qTox/commit/db990ac71f21120bfeb7fe5307b3ab9c3a0ba212), [dd4673cf](https://github.com/qTox/qTox/commit/dd4673cf6ad5a59e7cfc39ba3dd6faa25f76b85e))
  *  update Czech translation from Weblate ([7c5eaeae](https://github.com/qTox/qTox/commit/7c5eaeae7b990aef63d82445a7b8f356c554f14d), [b12b1cdc](https://github.com/qTox/qTox/commit/b12b1cdc2ed37d14facea45f0f775c1539883a88))
  *  update Esperanto translation from Weblate ([1e39d944](https://github.com/qTox/qTox/commit/1e39d94425789702c1222f71ccb0524d608d1d47))
  *  update Estonian translation from Weblate ([e6bb215a](https://github.com/qTox/qTox/commit/e6bb215a23363b6feab5a3f590ef0c0ea91378c6))
  *  update French translation from Weblate ([2519594a](https://github.com/qTox/qTox/commit/2519594aa2ec156bb6af3a192bdefe7a1ffa50f9), [cca4038f](https://github.com/qTox/qTox/commit/cca4038f655029b753e42cee45b46121c93fe3e6))
  *  update German translation from Weblate ([03197e5c](https://github.com/qTox/qTox/commit/03197e5c97140030d5bda334ecea3e1a464e6e6e), [9125060d](https://github.com/qTox/qTox/commit/9125060d35d00c71cb23167625f3ca3d35306ac2))
  *  update Hungarian translation from Weblate ([8cb357fd](https://github.com/qTox/qTox/commit/8cb357fda788b2f71f4cdd17a427f1b50ab16fed), [b0a564da](https://github.com/qTox/qTox/commit/b0a564dacde79ac361d6a9ba53db1e43ac0cfa08), [cdb47dce](https://github.com/qTox/qTox/commit/cdb47dce74d8991a12f93f1e2acf4965ffe34f41), [ff69b61f](https://github.com/qTox/qTox/commit/ff69b61f36541276b4a8764184f1927713ee900a))
  *  update Italian translation ([6a520c9c](https://github.com/qTox/qTox/commit/6a520c9c90eed3cc9eaab28f7b64550edc39eada), [ed472001](https://github.com/qTox/qTox/commit/ed47200181b29ec7b37651f6d450273f4002ec01))
  *  update Lithuanian translation from Weblate ([7d7a1845](https://github.com/qTox/qTox/commit/7d7a18454cef4118bf272b5849f122b68a2f1f5f))
  *  update Polish translation ([5ed1048e](https://github.com/qTox/qTox/commit/5ed1048e7254bb657436b852a8c4b3561932971d))
  *  update Portuguese translation from Weblate ([26ff01ba](https://github.com/qTox/qTox/commit/26ff01ba023c75e9f93914f7e082f3c3adab0643))
  *  update Russian translation from Weblate ([15638d0c](https://github.com/qTox/qTox/commit/15638d0c31a008dc54fe63d1ac264c212d18c590), [16accc74](https://github.com/qTox/qTox/commit/16accc745ce13ef6ca2a35c67bae2714849b261d), [2fa4073d](https://github.com/qTox/qTox/commit/2fa4073d903b4bc648580bb50fed51cede06ad5e), [eb15b157](https://github.com/qTox/qTox/commit/eb15b157d014555d5b91d60bd4c7421034e2a315))
  *  update Slovak translation from Weblate ([051672af](https://github.com/qTox/qTox/commit/051672afb52f893bd24a1aacc431354a6a0e0258), [cc6efe93](https://github.com/qTox/qTox/commit/cc6efe93bcaba5c696616ccaa231b7e93efe272c))

#### Bug Fixes

*   add missing Q_OBJECT macro to the PasswordEdit class ([f5011cdb](https://github.com/qTox/qTox/commit/f5011cdb2feb2da042e373b4e4094e967bde8120))
*   Fixed compact layout ([f870bf19](https://github.com/qTox/qTox/commit/f870bf1953672d948907477415a5cd2143d14a03))
*   remove unnecessary variable when checking whether save is a TES ([231e1d74](https://github.com/qTox/qTox/commit/231e1d746a9b97bb5da952c7565c0d6f052f26de))
*   Status reflection for friend calls ([917812da](https://github.com/qTox/qTox/commit/917812daaabc8b8118526691e67a3f6e2dd8fcb2))
*   dataToString correctness regression ([a4bda265](https://github.com/qTox/qTox/commit/a4bda2652550f3b2c2c8275dc8f607498f1c06ff))
* **UI:**
  *  make link in `About` setting page point to all contributors ([326d5219](https://github.com/qTox/qTox/commit/326d5219bc5bdabc87028e1040b98cde1e7c8950))
  *  remove unnecessary space between a word and colon ([fba5f8cc](https://github.com/qTox/qTox/commit/fba5f8ccc44b3227dc7d064f6c4d2307b616b839))
  *  copy friend's non-truncated status message instead of truncated ([9d1275b3](https://github.com/qTox/qTox/commit/9d1275b3963bfa87a4c1321d2d590db994e87ff3))
  *  Changed tab order to be same with visual order ([f7899d7b](https://github.com/qTox/qTox/commit/f7899d7b5044706d339537ff9658087a7ad22e37))
* **core:**  don't emit idSet signal twice ([355fb2ae](https://github.com/qTox/qTox/commit/355fb2ae413cad8f680ce3e8b1615c9415715135))
* **l10n:**  fix wrongly positioned `%` in translations ([e831ba94](https://github.com/qTox/qTox/commit/e831ba9427f356513eb66e1c4c26f889cc1a129d))
* **videoButton:**  Added disabling button while call is active ([eff27f69](https://github.com/qTox/qTox/commit/eff27f690e8a8af2e8f1fe0cd80f3b21dea2bed7))



<a name=""></a>
## v1.6.0 (2016-11-13)


#### Bug Fixes

*   Fixed crash on profile import ([a130a60e](https://github.com/qTox/qTox/commit/a130a60e9a09096de5e562d6bb89edf3e9984cca))
*   hide option to `Load automatically` for encrypted profiles ([b5efc2bb](https://github.com/qTox/qTox/commit/b5efc2bb7124ccbbd256792c524ffffdd02fab3f))
*   use `qAbs()` instead of `abs()` for better platform compatibility ([79c249be](https://github.com/qTox/qTox/commit/79c249be55f084c1bf68dbe927c847a4ea45e045), closes [#3613](https://github.com/qTox/qTox/issues/3613))
* **UI:**
  *  bump default emoticon size, since otherwise it can be too small ([bbdd4f04](https://github.com/qTox/qTox/commit/bbdd4f044c2c6b70fcb8794c8e5f01f1380af168))
  *  string for option auto-accepting files ([1d4ea1ec](https://github.com/qTox/qTox/commit/1d4ea1ec408c7c974f0a408835f599864320a15f))
* **audio:**
  *  Clear audio buffer when ending audio loop ([244d1dc3](https://github.com/qTox/qTox/commit/244d1dc386f634a0a27e58e26a5f0fd22340081f))
  *  keep the data pointed to by tmpDevName in scope ([af37fa7b](https://github.com/qTox/qTox/commit/af37fa7b20faf22129fe8d949ba28ca01b0a462f), closes [#3786](https://github.com/qTox/qTox/issues/3786))
* **avform:**
  *  Fixed empty value in audio and video lists ([15181c43](https://github.com/qTox/qTox/commit/15181c431a800347bd7bf82fa2703ea05bfe0dd5))
  *  Added shift screen region ([eaccbf3d](https://github.com/qTox/qTox/commit/eaccbf3da9a9e90897142370ec76defc85e7fe02))
* **build:**
  *  enable backports repository for Debian Jessie ([fb5639b2](https://github.com/qTox/qTox/commit/fb5639b292c99c176fe91ca844f7360820ce68ac), closes [#3679](https://github.com/qTox/qTox/issues/3679))
  *  fix OpenAL build on Windows ([2f7076c1](https://github.com/qTox/qTox/commit/2f7076c1687dcf163ee4e63105e37bac9d9aebd3))
* **chatform:**  remove std::dynamic_pointer_cast in favor of static cast ([775b6a32](https://github.com/qTox/qTox/commit/775b6a325be9a19104e73a90c7836446bf429536), closes [#3801](https://github.com/qTox/qTox/issues/3801))
* **chatlog:**  silence warning about unused parameters ([8ff0d28a](https://github.com/qTox/qTox/commit/8ff0d28abf8b11940b13fa3ae861db0bc5dce9e1))
* **i18n:**
  *  Remove HTML tags from translation ([f5f7b5ab](https://github.com/qTox/qTox/commit/f5f7b5abba756860e3c2e3949b92f6abafc0c3aa))
  *  remove HTML from translations ([c8bde5bc](https://github.com/qTox/qTox/commit/c8bde5bc452aaf68e88424bc448f86a16b282ffc))
  *  Removed extra tag from translation ([fde9ad53](https://github.com/qTox/qTox/commit/fde9ad535667e25869879fcde2dd24d8c7d8ca51))
* **l10n:**  correct ru translation ([5c901d0e](https://github.com/qTox/qTox/commit/5c901d0ea5bd5fc32a05138f705023a316b0813b))
* **rawdatabase:**  Added anonymizing SQL query in logs ([85ee69f0](https://github.com/qTox/qTox/commit/85ee69f0082105a6df187d85eb831ea5e7ce8c3e))
* **settings:**
  *  Changed grid layout on form layout ([9fd0dd79](https://github.com/qTox/qTox/commit/9fd0dd790b766355377cb53f11cfb7d4a099b1b6))
  *  Properly update taskbar on disconnect ([6e165ceb](https://github.com/qTox/qTox/commit/6e165ceb475313655a80e166ea1d88577ee8afb0))
  *  Change test sound button to checkbox ([22b89bea](https://github.com/qTox/qTox/commit/22b89beaf14f82f623464967f1980e5d78040649), closes [#3800](https://github.com/qTox/qTox/issues/3800))
  *  Play test sound when user enables test sound ([9b46cf64](https://github.com/qTox/qTox/commit/9b46cf64041a4c65155d3db75ed2d08630a673f0), closes [#3735](https://github.com/qTox/qTox/issues/3735))
  *  correct empty listbox audio device ([84a95700](https://github.com/qTox/qTox/commit/84a95700398273129ff2e6d63ea884f64c7ee686))
  *  Use old settings instead of default values ([6f0431ae](https://github.com/qTox/qTox/commit/6f0431ae50499bc2a3a19ddc9e83f86f55e105d5))
  *  correct default value of dateformat ([e794acbc](https://github.com/qTox/qTox/commit/e794acbccc29ee7d1a262c283b838e8b297b5527))
  *  Fixed group to load makeToxPortable setting ([a3c201d3](https://github.com/qTox/qTox/commit/a3c201d3b2562c794450ba15117ff7a69408da57))
  *  Added loader for old settings ([740d91b6](https://github.com/qTox/qTox/commit/740d91b61a66f5a5929dafc5fa86095d02bcf35a))
  *  const getters ([e2f9d2cf](https://github.com/qTox/qTox/commit/e2f9d2cfe873a16841277f32ed581cbcdcb8bddc))
* **settingsform:**  Fix layout spacing ([f1975231](https://github.com/qTox/qTox/commit/f197523155000fe719947b73aa84bffdc9fff385))
* **text:**  Change idealSize calculation ([c8512fa9](https://github.com/qTox/qTox/commit/c8512fa98461a54f3febc076fa1d7f99eae1d593))
* **timeformat:**
  *  Remove duplicate of timeformat ([661388d7](https://github.com/qTox/qTox/commit/661388d73195a539285fd6dc7acad4e86ba0e57b))
  *  Make timestamp editable same as date format ([d6f323ce](https://github.com/qTox/qTox/commit/d6f323cefc15b477ec588a3e88a133188ecbac84))
  *  Fixed list of available time formats ([35e7da85](https://github.com/qTox/qTox/commit/35e7da85d34fa8f5bd0c9ec6da10894c932af568))
* **ui:**  prevent text style from being reset during retranslation ([28c29157](https://github.com/qTox/qTox/commit/28c29157f8ea7f956bd76a3979becb41d6968bb3), closes [#3805](https://github.com/qTox/qTox/issues/3805))
* **uiform:**  Added format constraints ([13baf932](https://github.com/qTox/qTox/commit/13baf9326d869d8b410a6a7f9c447766fcdea45e))
* **video:**  fix scaling issues under HiDPI displays with desktop video ([ef157ca8](https://github.com/qTox/qTox/commit/ef157ca8af14862c1d69d62ab609fe44754c4b3f))
* **videomode:**  Added ability to store negative coordinates ([26a27cf3](https://github.com/qTox/qTox/commit/26a27cf3f8de49b0dd49e56b55d1ad0303b93975))
* **widget:**
  *  do not block logout on Linux desktop environments v2 ([6b00779d](https://github.com/qTox/qTox/commit/6b00779ddecb4a28c20ef3d00eadd0ffd5490c48))
  *  do not block logout on Linux desktop environments ([7ee883bc](https://github.com/qTox/qTox/commit/7ee883bcc7cf09fe3b87ab7b4594165b7cc08ba9), closes [#1485](https://github.com/qTox/qTox/issues/1485))

#### Features

*   bump font size from undersized ([2d472004](https://github.com/qTox/qTox/commit/2d472004cd3a6afc73889f836e80c3c9fa6b29ac))
* **autoAnswer:**  add auto answer feature ([7fe8a86c](https://github.com/qTox/qTox/commit/7fe8a86c0329481cd34f5ed4e134d2db7a11d096))
* **avform:**
  *  Made warning and debug info more useful ([94783778](https://github.com/qTox/qTox/commit/9478377868506559f7b126c501e24c8f9c844806))
  *  Added automatic screen rescan ([08229735](https://github.com/qTox/qTox/commit/08229735b26cd849343810e1f84c2c3bf5807dc5))
* **dateformat:**  Edit the date format string ([b3083250](https://github.com/qTox/qTox/commit/b3083250ff8f71936db56993a9d7b1216d8323f9))
* **l10n:**
  *  add Slovak translation from Weblate ([ad176260](https://github.com/qTox/qTox/commit/ad176260d5d89412759a85755ef42b9b69d0fdaf), [52ae7093](https://github.com/qTox/qTox/commit/52ae7093f7af9d26fde1cdb37ec7051bb6c16516))
  *  update Arabic translation from Weblate ([7d704e8d](https://github.com/qTox/qTox/commit/7d704e8dc8e4830e85dba6711e58261ff5124f1d))
  *  update Belarusian translation from Weblate ([0842b50b](https://github.com/qTox/qTox/commit/0842b50b760882014bfbf85c27583f22cf34f3b3), [226f4eaf](https://github.com/qTox/qTox/commit/226f4eaf4f98c9c53edd370f68b91c79c2acd895), [4296d377](https://github.com/qTox/qTox/commit/4296d377eec0f8a136b3d62eb06a5fb122c766a8), [60af778f](https://github.com/qTox/qTox/commit/60af778fb8d0fbd8373c4d994d56d3b21281212a), [d898e7d4](https://github.com/qTox/qTox/commit/d898e7d40a4cac1b8e9c2cbbb8f7caa72c71d1a9))
  *  update Chinese translation from Weblate ([11b19e42](https://github.com/qTox/qTox/commit/11b19e42e8a45b53f8f0185a1073a36726de26a8), [3539e840](https://github.com/qTox/qTox/commit/3539e8405159ab4b0f15b23ae88ca11b34f32c5c), [4f20d032](https://github.com/qTox/qTox/commit/4f20d032d1cd67a7667f1403e816854d933507d4), [5f087194](https://github.com/qTox/qTox/commit/5f087194050bbb148cc3b9743e5fd3e0a4144e3a))
  *  update Czech translation from Weblate ([037f2da8](https://github.com/qTox/qTox/commit/037f2da82bd4b573fa29e69d1e050dcb2458eac6), [3c8b8d63](https://github.com/qTox/qTox/commit/3c8b8d63449968904934ea791010d14ede146c6a), [6b4fe28c](https://github.com/qTox/qTox/commit/6b4fe28c534fc12ce686240af682f5f31ef3f2f7), [a6d94e8f](https://github.com/qTox/qTox/commit/a6d94e8f839e34e73d7785ce2cf55f29d1713d7b))
  *  update Danish translation from Weblate ([ab312318](https://github.com/qTox/qTox/commit/ab3123187df057357356891c7d51db1823cd7c94))
  *  update Dutch translation from Weblate ([2b3eaa1b](https://github.com/qTox/qTox/commit/2b3eaa1bc54727ae2820883fc393593461020472), [2c5a38cc](https://github.com/qTox/qTox/commit/2c5a38cc586ff811b5b039e073c4a60f48cc2880))
  *  update Estonian translation from Weblate ([018a88df](https://github.com/qTox/qTox/commit/018a88dfe6e4e0ef57b5c476f8ef06c7c720230c), [bed51e88](https://github.com/qTox/qTox/commit/bed51e884c1971c2487b4119dcb14584a9dfedd0), [d8a525ac](https://github.com/qTox/qTox/commit/d8a525acb3ae68f98428a111ab366c08c97916ea), [e0c71216](https://github.com/qTox/qTox/commit/e0c7121666570743674f949baf663eae1cc2905d), [ee2b93ae](https://github.com/qTox/qTox/commit/ee2b93ae21d3af9298a68a34adb0aeeaae9ccc10), [f6aa3085](https://github.com/qTox/qTox/commit/f6aa3085ef5d47e1073bd5032d622e4244770dc9))
  *  update French translation from Weblate ([55d1fbeb](https://github.com/qTox/qTox/commit/55d1fbeb42e0e83fe6ef5af0441eac4eb3d82592), [a24a6df5](https://github.com/qTox/qTox/commit/a24a6df55076d4b0cf2f3accfcba8065f45e8edb), [f5371032](https://github.com/qTox/qTox/commit/f5371032a4d40e528628159db996b0fbd12908cd))
  *  update German translation from Weblate ([00fa4d3b](https://github.com/qTox/qTox/commit/00fa4d3b6203e0d0355b86fc1a39215672ba9077), [0a37f7ea](https://github.com/qTox/qTox/commit/0a37f7ea7151568e6bd02f5e57e40e3851310b61), [1cb65ce4](https://github.com/qTox/qTox/commit/1cb65ce42249e36bfc6a33a5fb3d31bbde73369f), [7942206b](https://github.com/qTox/qTox/commit/7942206ba988eda20dbdcfa3f79d81fa1a5ef6c8), [79a847f1](https://github.com/qTox/qTox/commit/79a847f14dd6d610ab88440f1ae200262e97de02), [a2e559d7](https://github.com/qTox/qTox/commit/a2e559d7597f63a2f519b90840f686834cd87a20))
  *  update Greek translation from Weblate ([ba2d62fe](https://github.com/qTox/qTox/commit/ba2d62feeddf7e054b9ec10f92ca997814d48b94))
  *  update Hungarian translation from Weblate ([0e47ecda](https://github.com/qTox/qTox/commit/0e47ecdad5b40856dc32c82de75ef8a8cb02044a), [16b32e3b](https://github.com/qTox/qTox/commit/16b32e3bbf55b6b4b486a1e51444183da0c9423c), [16e4198b](https://github.com/qTox/qTox/commit/16e4198b094d13a9be29ce9c806915e72a28ea15))
  *  update Japanese translation from Weblate ([f733f475](https://github.com/qTox/qTox/commit/f733f4755124dff89441c6e959dae888feb34c58))
  *  update Lithuanian translation from Weblate ([25e86e54](https://github.com/qTox/qTox/commit/25e86e5464fcf556bcd8d538cb07cc97acd83432), [55bb35ac](https://github.com/qTox/qTox/commit/55bb35acaa32c5b75189cb6b909f8eb4a016641b), [7e6e3970](https://github.com/qTox/qTox/commit/7e6e39703b0fc9810f7aa0c0a84a173b092d46ea))
  *  update Norwegian (old code) translation from Weblate ([548df204](https://github.com/qTox/qTox/commit/548df204979a3805795e5aed661767d38041f0f7))
  *  update Polish translation ([7fc98a9e](https://github.com/qTox/qTox/commit/7fc98a9ea7292dea74adb38d533fb3f74992a261))
  *  update Portuguese translation from Weblate ([9e39708e](https://github.com/qTox/qTox/commit/9e39708e76bc583025491be245879c1034e98c0e))
  *  update Russian translation from Weblate ([0865217d](https://github.com/qTox/qTox/commit/0865217d468bf9084c4e3755ba11a42046eec5b4), [628cfd23](https://github.com/qTox/qTox/commit/628cfd23222df3406ef0b39580565b75d94ae352), [dd2a5dad](https://github.com/qTox/qTox/commit/dd2a5dadc57b13eba98e2dbb21893c3ec7d3550b), [fb418c64](https://github.com/qTox/qTox/commit/fb418c64836e562a11f298319520add2cdc160ff))
  *  update Slovak translation from Weblate ([287e2bcc](https://github.com/qTox/qTox/commit/287e2bcc38ee08d59f7877ee9aaef397fe53abca), [dcfbdf8f](https://github.com/qTox/qTox/commit/dcfbdf8f95c865c8336ed5ffbc094b91fb2315b2))
  *  update Spanish translation from Weblate ([8a8859b7](https://github.com/qTox/qTox/commit/8a8859b7a93a980e010b3e1081553142143afaf3))
* **translation:**  add update language Spanish ([a7c46fff](https://github.com/qTox/qTox/commit/a7c46fff2de152c77b54cd54aa8f9a209077b05e))
* **settings:**
  *  Made `Reconnect` button wider ([4a5f89bb](https://github.com/qTox/qTox/commit/4a5f89bb98fc8657514e35c5022713358a453d49))
  *  Export and copy debug log ([3c6bd043](https://github.com/qTox/qTox/commit/3c6bd043a04edb0ae76444f5d54ce8eaf5fb0b14), closes [#2890](https://github.com/qTox/qTox/issues/2890))
  *  Moved proxy to personal settings ([4dfe3ec2](https://github.com/qTox/qTox/commit/4dfe3ec22636150074a0afc73644582e14f744ff))
  *  Added privacy block ([9499bdd4](https://github.com/qTox/qTox/commit/9499bdd45877f20bcb8f82606a5db7bc6e3a2db5))
  *  Deleted settings header ([a1041ed1](https://github.com/qTox/qTox/commit/a1041ed11a27e9b9de617a7e11c907d83ba55952))
  *  Added reset settings button ([9c9f1c11](https://github.com/qTox/qTox/commit/9c9f1c11d1fece24402384dbd6b41d1c05aec00b))
  *  Extracted user interface settings on new tab ([fb4aa4c8](https://github.com/qTox/qTox/commit/fb4aa4c8f628b47083402221380172951ab41532))
  *  add notification signals for changed settings values ([f00b9008](https://github.com/qTox/qTox/commit/f00b9008e669905e6178c84f4b2e81456ee9cb5e))
* **smileys:**
  *  try to load smileys from XDG_DATA_DIR directories ([29da2210](https://github.com/qTox/qTox/commit/29da2210c98a2f1bf2dcaf6af18ba818ceef5d2c))
  *  allow compiling qTox with minimal or no smileys ([ae769106](https://github.com/qTox/qTox/commit/ae76910631a9cc15dd9673d5dfc0392aeffeb459))
* **uiform:**  Separated time and date format and examples ([d909d99b](https://github.com/qTox/qTox/commit/d909d99bd22e2ba35459c7bb3be0fa8724f97346))



<a name=""></a>
## v1.5.0 (2016-08-09)

The most important change is video improvements. Bored by waiting minutes for
video call to start? Fixed.

Among other things, qTox has been translated into 5 new languages.

More information on features / fixes / changes below.

#### Breaking Changes

* **textstyle:**  Change markdown syntax to be more intuitive ([32e48a97](https://github.com/qTox/qTox/commit/32e48a979ca78717a212800547c95ca0f1e67b8f))
* **widget:**  Disable sound notification for `busy` status ([e7785ab4](https://github.com/qTox/qTox/commit/e7785ab4c2790b2c10b33c416bc78ad23a16cc63))

#### Features

* **audio:**
  *  add slider tickmarks, improving better visible orientation ([431a10f8](https://github.com/qTox/qTox/commit/431a10f82b6370153aa843856d109287078b0dc5))
  *  add real gain control of the input device ([f72baa61](https://github.com/qTox/qTox/commit/f72baa613f6f15de454783eeb4e709c691aef4ad))
* **avform, screenshotgrabber:**  Added custom screen region selection ([9cfd678c](https://github.com/qTox/qTox/commit/9cfd678c262b223413dab30d656aff50ae7ce470))
* **bootstrap.sh:**  add an option to install sqlcipher ([66f270ec](https://github.com/qTox/qTox/commit/66f270ecad591fc6e5547af12753358a63c4b171))
* **cameradevice, avform:**  Added ability of screen selection ([d781a4f7](https://github.com/qTox/qTox/commit/d781a4f762a75a5766d9ca534ef0f034bf332ea0))
* **camerasource:**  Change default video mode to preferred ([c3de6238](https://github.com/qTox/qTox/commit/c3de6238ca5efa9e42b484a755934c986d0d4b6e))
* **capslock:**  Added caps lock checker ([97f95e7e](https://github.com/qTox/qTox/commit/97f95e7e91a5dc8f5b1136238efc7c5cb10d55f4))
* **chat:**
  *  add the ui settings to alter font and size for chat messages ([41c96eb1](https://github.com/qTox/qTox/commit/41c96eb15962306e1da02c69a1f515b5223fd270))
  *  add settings to alter the chat view's base font ([8ba20541](https://github.com/qTox/qTox/commit/8ba205419048a039a8dbc84c87dfa4c6b2cb1252))
* **chatform:**  Disable call buttons if friend is offline ([bbefe011](https://github.com/qTox/qTox/commit/bbefe0119d405cf7bcc0e7a90571b1c18b993817))
* **doxygen:**  Created simple doxygen config file ([194c55a4](https://github.com/qTox/qTox/commit/194c55a4c5c217c060ca78e11eec8c3b349aeffe))
* **emoticons:**  add ASCII-less version of emojione emoticons ([c4b4155a](https://github.com/qTox/qTox/commit/c4b4155a53d72fbcad7475590d860c94bea336b3), closes [#3398](https://github.com/qTox/qTox/issues/3398))
* **emoticonswidget:** Keep emoticon option open ([d0ea5bb4](https://github.com/qTox/qTox/commit/d0ea5bb4fd6e5ffdb2bb11c4e63d1c518af36b65))
* **genericchatform:**  add "Quote selected text" feature to chat window ([40a805c2](https://github.com/qTox/qTox/commit/40a805c2fd66d7c5cd618fb4974dcd65bf7df650))
* **gui, setpassworddialog:**  Added buttons translation ([58e503bb](https://github.com/qTox/qTox/commit/58e503bb14e747b16a30c22dffc7397999098bac))
* **importProfile:**  Add way to import profile ([9ea25d1f](https://github.com/qTox/qTox/commit/9ea25d1fbd928657cfcc0e73e868783f866e5ea9))
* **i18n:**
  *  Make activity by time labels translated by locale ([f2aada8f](https://github.com/qTox/qTox/commit/f2aada8f4fd404947ca4fa2d34e00df45c25f76f))
  *  make Markdown settings translatable ([3e22593a](https://github.com/qTox/qTox/commit/3e22593ae71f388972a59079ccf6d719f980d035))
* **l10n:**
  *  Add Danish translations ([c8c7bda3](https://github.com/qTox/qTox/commit/c8c7bda38e7e9167c0eab084452df192b8485f59))
  *  Add Hebrew translation ([83b89f12](https://github.com/qTox/qTox/commit/83b89f1233c49e3c8d92a2368f4b0cb5df870937))
  *  add initial Belarusian translation ([684835de](https://github.com/qTox/qTox/commit/684835de1b3a26aac83d1fa99f2bb5a78eab411e))
  *  add initial Esperanto translation ([7971975c](https://github.com/qTox/qTox/commit/7971975cbe5f53ad328bb8af76b09170d341f869))
  *  add Japanese translation ([d06efd38](https://github.com/qTox/qTox/commit/d06efd387bdaa85143e6924352a0bcc038537a98), closes [#3223](https://github.com/qTox/qTox/issues/3223))
  *  add Lojban translation ([237351fd](https://github.com/qTox/qTox/commit/237351fdd28841a61fcc733d9f039471fcf05e40))
  *  add Uighur translation ([3ee8f72a](https://github.com/qTox/qTox/commit/3ee8f72a6c79e53d19d0083d1d0d2c3f0b01d97c))
  *  Update Arabic translation ([91af5c95](https://github.com/qTox/qTox/commit/91af5c951a51e6398e74dd4c85065a942a0a5da4))
  *  update Belarusian translation ([1b16466c](https://github.com/qTox/qTox/commit/1b16466cf924576e09ed92691eb82c7d6300b439)) ([44420953](https://github.com/qTox/qTox/commit/444209537828eb4aab20ed47f7ec304863c3e48d)) ([526f13aa](https://github.com/qTox/qTox/commit/526f13aa0df27eba245918b6b15f1f981e29a1e2)) ([7c6ba752](https://github.com/qTox/qTox/commit/7c6ba75200e8e3cd5461d99d85002ce1ccd1e488)) ([97d8c7a1](https://github.com/qTox/qTox/commit/97d8c7a108f3f6541eacd6c8dda3eb11c0f0813a)) ([daabda84](https://github.com/qTox/qTox/commit/daabda84c47c4ea5a29c45e045689b917fa9c750)) ([f2c19912](https://github.com/qTox/qTox/commit/f2c19912c46c89f212bb6c809d6a54619018252d))
  *  update Bulgarian translation ([10d913ee](https://github.com/qTox/qTox/commit/10d913ee4ac80c54f79060915e37f32eb14ec385)) ([b6b149a7](https://github.com/qTox/qTox/commit/b6b149a756841ca901dee893992239161d1d7399)) ([6052364b](https://github.com/qTox/qTox/commit/6052364bca754ac3e49638737b76c597fe158e8b)) ([e0b41d57](https://github.com/qTox/qTox/commit/e0b41d5764d76c35e627f3b23a24fffd265b2d49))
  *  Update Chinese translation ([fe432dea](https://github.com/qTox/qTox/commit/fe432dead428cbdd9d18b93fd60a9744a1ea210d)) ([f8ee4484](https://github.com/qTox/qTox/commit/f8ee448412b5bca7a97a106d9e2507fe63bce998))
  *  update Czech translation ([1e9efbfe](https://github.com/qTox/qTox/commit/1e9efbfe694f0eabdc42edb7901d4282c28c6498)) ([83f874e5](https://github.com/qTox/qTox/commit/83f874e5ceb1ac814974a927d415a18c6c98b95f)) ([8d94ca92](https://github.com/qTox/qTox/commit/8d94ca92275b552e39cc91f8c48cf96f0e0fc0e5)) ([d951cb75](https://github.com/qTox/qTox/commit/d951cb7589ce9e3908b36deeac151797453576b1))
  *  update Dutch translation ([8ac47bf0](https://github.com/qTox/qTox/commit/8ac47bf06b038cd7decf62a114347579488559a7))
  *  Update Estonian translation ([2cd35e17](https://github.com/qTox/qTox/commit/2cd35e17360dd1a9b66f0eb5d0ed163c1903895b)) ([4137a19f](https://github.com/qTox/qTox/commit/4137a19fbc1286f5c66e1814c3b0cde94fdd0617)) ([6d7d9c33](https://github.com/qTox/qTox/commit/6d7d9c33a5c3d2a2948832f2d9a5807865537c3d)) ([85a701f5](https://github.com/qTox/qTox/commit/85a701f5f820874d0f1c2993f8ffffc124bc030f)) ([9c8335fa](https://github.com/qTox/qTox/commit/9c8335fa16eb8ab7e0a74b8585561816f0cc6285)) ([ba0d7ec7](https://github.com/qTox/qTox/commit/ba0d7ec76814ceb4b272cefddf15e427a94af7e2)) ([c6fba9c5](https://github.com/qTox/qTox/commit/c6fba9c54872a35f708f9344647a4258d206dbb6))
  *  update French translation ([2a368436](https://github.com/qTox/qTox/commit/2a368436dbed906d928d44771aa96d93e64951e4)) ([402f9eb9](https://github.com/qTox/qTox/commit/402f9eb936ac3b1bad75ea35a6bdf9e8e9a0eac3)) ([4b42a6db](https://github.com/qTox/qTox/commit/4b42a6dba385da8e730f028291e06afae079e4d7)) ([525db227](https://github.com/qTox/qTox/commit/525db2276a49ec24a17b89966a626a1252ce2b13)) ([5a147646](https://github.com/qTox/qTox/commit/5a147646c59a21a27cc342e454a6c6b2078035ab)) ([774f3c16](https://github.com/qTox/qTox/commit/774f3c1641cdf0ece2e5a8193a1468bcc3229031)) ([d9fc36db](https://github.com/qTox/qTox/commit/d9fc36db4bf097b02e37fa8874432ed44309cd8e)) ([f6f336a7](https://github.com/qTox/qTox/commit/f6f336a7a7a40eac4fc7210d60a020d9cecac4b4))
  *  Update German translation ([beca3a9c](https://github.com/qTox/qTox/commit/beca3a9c45a65f5085bd6d8c1dff91a73215619f)) ([750d1b50](https://github.com/qTox/qTox/commit/750d1b50cc5f05fce2348bf352d2565201cd5d3c)) ([1107b642](https://github.com/qTox/qTox/commit/1107b6421be5a2ef3f945b2c78456fba42e31c27)) ([2b65fac3](https://github.com/qTox/qTox/commit/2b65fac36f5c04878dded3a456832ebe9f4cf1ca)) ([351c4166](https://github.com/qTox/qTox/commit/351c4166b26bd93877cea973212043f42513ad18)) ([65019117](https://github.com/qTox/qTox/commit/6501911730c1937f0b71116898e4023f4a6fc039)) ([8a0a8f1f](https://github.com/qTox/qTox/commit/8a0a8f1f75ef80019165f208485e6f567a30cbed)) ([962206db](https://github.com/qTox/qTox/commit/962206db769112c7e2ee35743d31081bfa3f42f8))
  *  update Hungarian translation ([0c3f3817](https://github.com/qTox/qTox/commit/0c3f3817c3e9e18abc168761f5d60b6de850aac0)) ([9bc642ee](https://github.com/qTox/qTox/commit/9bc642eee9def1a7c594216c1a6c23e09fc5c18f)) ([c6938d6c](https://github.com/qTox/qTox/commit/c6938d6c4e809b7d99357641914c9f08dea3ffb6))
  *  update Italian translation ([7d308f99](https://github.com/qTox/qTox/commit/7d308f99ce306333a1c285633070bfe8cab45bf7)) ([e7089a3d](https://github.com/qTox/qTox/commit/e7089a3d1a46c799c6c8e7b9f149306a8d59272d)) ([e6f870f4](https://github.com/qTox/qTox/commit/e6f870f4b408e4df3f95e0159adb1f5e3d0169e5))
  *  Update Japanese translation ([75d64dc6](https://github.com/qTox/qTox/commit/75d64dc68dc86fe8479e5c48dd54f375e4c63d34))
  *  update Lithuanian translation ([0bb416cd](https://github.com/qTox/qTox/commit/0bb416cd76b8191a57d8dcd24d135972f64c024c)) ([9d108840](https://github.com/qTox/qTox/commit/9d10884029f1dae69fdad3ba03f691acf2da5d18)) ([281d94ef](https://github.com/qTox/qTox/commit/281d94ef4e9db5edf1fe208a834f2a5648a8b4d9)) ([e19f4c70](https://github.com/qTox/qTox/commit/e19f4c70092b6e9e022da45a6b594014db837a10))
  *  Update Norwegian translation ([1466fbf5](https://github.com/qTox/qTox/commit/1466fbf554ba810b07305969a55a0f5425df488e))
  *  update Polish translation ([9a3ba021](https://github.com/qTox/qTox/commit/9a3ba02145a3357c1e4f3044372d03dfebcfb568)) ([e7c0159f](https://github.com/qTox/qTox/commit/e7c0159fce86df7b9c2f7627faea41425b432f55)) ([6f074061](https://github.com/qTox/qTox/commit/6f074061cb02c9013ab6ccb493755509485a73e6)) ([88b839c1](https://github.com/qTox/qTox/commit/88b839c1af1e0b5fabb7a6ac8034d208620cc638)) ([a49e7f27](https://github.com/qTox/qTox/commit/a49e7f276a9a528d34d29245c21a5886108ff2b8))
  *  Update Russian translation ([0856d4dd](https://github.com/qTox/qTox/commit/0856d4dd13dc61b7b4ff8c69b01eab4a6f56f430)) ([1826e2ae](https://github.com/qTox/qTox/commit/1826e2aebb2458f90c91bdd8168a78c1b5569ee5)) ([21b5cc3f](https://github.com/qTox/qTox/commit/21b5cc3f9db21068eef0b7138d362d7295b1e5ed)) ([29dbd030](https://github.com/qTox/qTox/commit/29dbd030765a5f382ceb0e386fe381cfc51ee5de)) ([31ecfd8b](https://github.com/qTox/qTox/commit/31ecfd8b12996cea6439e03dd97c54b952a6436f)) ([379aaa0f](https://github.com/qTox/qTox/commit/379aaa0fdb85a0fd384110a387e10aa7de49d17f)) ([6beea2bd](https://github.com/qTox/qTox/commit/6beea2bda594ac9d32144765e4ba3873e102684b)) ([861cf7d9](https://github.com/qTox/qTox/commit/861cf7d93de8800224c145f6806dc986cd237874)) ([d4ff03c8](https://github.com/qTox/qTox/commit/d4ff03c82b66a7efddcb453a53c5898f7625075b))
  *  update Spanish translation ([17f43668](https://github.com/qTox/qTox/commit/17f43668a417192155d9c41be5b6cb6550728d5e)) ([f81f20f0](https://github.com/qTox/qTox/commit/f81f20f0cfba9e0bdec7599e0ae9eca55bf14e5a)) ([090a715b](https://github.com/qTox/qTox/commit/090a715b4cf2fc731d864b7de7fdeeec47d33e05))
  *  Update Ukrainian translation ([2ab5af56](https://github.com/qTox/qTox/commit/2ab5af566f6030425b596b84af5274a5fa3d0e4d)) ([3a5e91a2](https://github.com/qTox/qTox/commit/3a5e91a20854f24a33e4be422054309f993f88b1))
* **loginform:**  Added caps lock indicator to newPass ([cbe8fb8e](https://github.com/qTox/qTox/commit/cbe8fb8ef989e6e178343f1d5d5e016a62ce1d70))
* **loginscreen:**  Created new CapsLockIndicator class ([fb7fcaaa](https://github.com/qTox/qTox/commit/fb7fcaaa8ca98fe32ebdc3d4cab2834ac951b50a))
* **main:**  Changed time in logs to UTC. ([4018c004](https://github.com/qTox/qTox/commit/4018c0041b179bbd03f152c43ab0e70dbf00a9f0))
* **notificationscrollarea:**  Add ability to delete widget from traced widgets list ([e3d74117](https://github.com/qTox/qTox/commit/e3d74117caa1e1289879388640797a1a9594c35a))
* **profile:**
  *  add a dialog to indicate profile deletion error ([78fd245e](https://github.com/qTox/qTox/commit/78fd245e4cb0a73570b5d6a24d612ac099dd16ef))
  *  show warning on failure to delete profile ([1dabbca9](https://github.com/qTox/qTox/commit/1dabbca94c172bf21f83e23400ef99e5be45c084))
* **profileform:**  Added log toxme errors ([d2d5b230](https://github.com/qTox/qTox/commit/d2d5b2306450032ae485909dc0c480d3c02b3596))
* **settings, generalform, widget:**  Added setting for sound notification with busy status ([e23eb1c5](https://github.com/qTox/qTox/commit/e23eb1c5f770b84a41ab061c2e6163ea8c9337a9))
* **smileys:**  add emojione emoji-pack and make it the default ([3f4a0abe](https://github.com/qTox/qTox/commit/3f4a0abe6b10ddf335c89a6220e079bb858c9fd6), closes [#3315](https://github.com/qTox/qTox/issues/3315))
* **status:**:  add ability to copy status messages ([57ce030f](https://github.com/qTox/qTox/commit/57ce030f1d19f4a16f309149addc80bed53b8b2d))
* **systemtray:**  add "Show" action to context menu ([a851a5b1](https://github.com/qTox/qTox/commit/a851a5b18da8c295fba441c817920cf65db2cdbb))
* **textstyle:**  Change markdown syntax to be more intuitive ([32e48a97](https://github.com/qTox/qTox/commit/32e48a979ca78717a212800547c95ca0f1e67b8f), closes [#3404](https://github.com/qTox/qTox/issues/3404))
* **video:**
  *  redesign and improve VideoFrame class ([38b1a9b6](https://github.com/qTox/qTox/commit/38b1a9b63dabdd9b71e7824c602f9ff77e99eb1e))
  *  add setting for 120p very-low-res video ([6045ced3](https://github.com/qTox/qTox/commit/6045ced3f8b538b0c3044370878d4feb76103480))
* **videomode:**  Added possible video shift ([fd701df1](https://github.com/qTox/qTox/commit/fd701df1012763cba98bbfbbf7bf9ccd898f1c03))
* **widget:**  Disable sound notification for `busy` status ([e7785ab4](https://github.com/qTox/qTox/commit/e7785ab4c2790b2c10b33c416bc78ad23a16cc63))


#### Bug Fixes

*   increase timer for checking offline messages timeout (again) ([a77afca1](https://github.com/qTox/qTox/commit/a77afca1ec8f86cda68fe7ba853f7e4d854c346f))
*   correctly tab-complete nicks starting with `$` ([dbd16ae6](https://github.com/qTox/qTox/commit/dbd16ae6a362809e8fc2936968cd7f0677b7e6f0))
* **.gitattributes:**  bootstrap.sh execution fails on MSYS ([ad828621](https://github.com/qTox/qTox/commit/ad828621359963e6eaadb07cf2b834eb9d53cf01))
* **about-qtox:**  fix QString "missing argument" warning ([f2f48a8f](https://github.com/qTox/qTox/commit/f2f48a8f07106dd4c3828e202b184062ab39798f))
* **addfriendform:**  Fixed problem with reading friend request ([7be8ad01](https://github.com/qTox/qTox/commit/7be8ad01da50c9f95312dea4a5e56ec2df8517a2))
* **audio:**  actually disable the audio in/out device in settings, when selected ([9694d6b6](https://github.com/qTox/qTox/commit/9694d6b6d434bb7557c9766e89dce5fb4cb89502))
* **avform:**
  *  display true video height in video mode selection ([192c1e8f](https://github.com/qTox/qTox/commit/192c1e8ff53e6c29c92d4e99107c70c9f9ae00cc))
  *  add missing "first" video mode back to video modes ([5324e768](https://github.com/qTox/qTox/commit/5324e768c31643fa1741bf94105c4604be482a7c), closes [#3588](https://github.com/qTox/qTox/issues/3588))
  * Add skipped camera open call ([1f9b7b13](https://github.com/qTox/qTox/commit/1f9b7b13de54668e0dbe185fbbc3b09c369b5f77), closes [#3476](https://github.com/qTox/qTox/issues/3476))
  *  Added rounding height in mode name. ([c2e3358d](https://github.com/qTox/qTox/commit/c2e3358dd2eca3839a5f9fc858d8f4546dc842a2))
  *  Changed "best modes" search algorithm. ([6e1ef706](https://github.com/qTox/qTox/commit/6e1ef70651647d01e55057d48bd5d482b8530777))
  *  initialize slider value from settings ([c9dbfa5e](https://github.com/qTox/qTox/commit/c9dbfa5eacd07c58d234c73dc58d91d1ad5a8f12))
  *  make "Screen" translatable ([24f0b11a](https://github.com/qTox/qTox/commit/24f0b11a4d9d9020f7146c0c94b0b65de89a9d0f))
  *  Added restoring selected region ([1c515821](https://github.com/qTox/qTox/commit/1c5158213dd6f38acdcf202e660b6e383764c436))
  *  Took default resolution from middle of list ([2d861ee2](https://github.com/qTox/qTox/commit/2d861ee25b2f6ac5607005ea89f6866d7321e66d))
* **bootstrap.sh:**  add instructions for missing unzip & adjust path ([fa5ee5b1](https://github.com/qTox/qTox/commit/fa5ee5b1adbfa2cfde3595cdee1a42ca4cac8a11), closes [#3153](https://github.com/qTox/qTox/issues/3153))
* **build:**
  *  Link qrencode statically on Jenkins ([0a976c7a](https://github.com/qTox/qTox/commit/0a976c7a50649ae0874bbf341a273b669901a183))
  *  Jenkins ffmpeg link order ([9de833ad](https://github.com/qTox/qTox/commit/9de833ad396b5842431fdbb3908f4d3ead2522aa))
  *  Fix jenkins static builds ([790f9ffc](https://github.com/qTox/qTox/commit/790f9ffc670997172fffad2e4a6c4eaf794aed71))
* **capslockindicator:**
  *  also update indicator when the app gets focus ([2fe41071](https://github.com/qTox/qTox/commit/2fe41071bedb689ba1776affe97e50138b9d6984))
  *  fix altering the line edit height ([653e0b5a](https://github.com/qTox/qTox/commit/653e0b5af2dc176e80eb617df89f32bf8a00427e), closes [#3379](https://github.com/qTox/qTox/issues/3379))
  *  Tooltip color was changed. Tooltip translation was added ([bbe158c7](https://github.com/qTox/qTox/commit/bbe158c7d99dff54f2f96cd1ed7f31b8cfc2816a))
* **chat:**  cleanup chat css base style ([989b15e6](https://github.com/qTox/qTox/commit/989b15e6560c2bd8fbdfafcb4b9736a49af72774))
* **chat window:**  prevent right click from opening chat window ([b9a392d5](https://github.com/qTox/qTox/commit/b9a392d59ee2760a68712b835e21375425bdaebe), closes [#3205](https://github.com/qTox/qTox/issues/3205))
* **chatform:**
  *  Fixed call buttons ([dbe0a159](https://github.com/qTox/qTox/commit/dbe0a1596376a3edc4c1c10d604c9aaa54c8af73))
  *  Markdown after emojis ([998f0915](https://github.com/qTox/qTox/commit/998f0915db515851d92e32873e6ac4c528cd9a16))
* **chatform, screenshotgrabber:**  Fixed memory leak ([bf7c62d6](https://github.com/qTox/qTox/commit/bf7c62d6fa57b3f8b5078d55bf20372fe6aa47b7))
* **chatlog:**  Don't delete active transfer widget ([abf7b423](https://github.com/qTox/qTox/commit/abf7b42324f85c00ca1105ac11a8c989e8488aa2))
* **chattextedit.cpp:**  fix drag-and-drop to be consistent across systems ([70fc247b](https://github.com/qTox/qTox/commit/70fc247b7092b0bd320b3a59bcaf57d896257da8))
* **contentdialog, widget:**  Remove "new message" bar after reading message ([b2c1f468](https://github.com/qTox/qTox/commit/b2c1f468946aff73b23fb9ac6539331f6c0c7fbe))
* **corevideosource:**  Partial revert of [ef641ce6d3398792c10b30bf24a81c5a6005fe06](https://github.com/qTox/qTox/commit/ef641ce6d3398792c10b30bf24a81c5a6005fe06) ([b1adef2f](https://github.com/qTox/qTox/commit/b1adef2fd0b9aeef58cea34c6637b8cd3f6e16a8), closes [#3527](https://github.com/qTox/qTox/issues/3527))
* **directshow:**  Fixed problem with crosses initialization ([504ad534](https://github.com/qTox/qTox/commit/504ad534e0e9d27077a49222f2c7f9e0d568b22d))
* **doc:**  CONTRIBUTING.md typos ([4eed2549](https://github.com/qTox/qTox/commit/4eed2549aaef7c72f2e0ddf92696f35f209ae1a4))
* **friendlistwidget:**  use nullptr instead of `0` ([f1543144](https://github.com/qTox/qTox/commit/f1543144be7726a9d2dbb6e04ca9b0a4c1000737))
* **friendwidget:**  the limitation of the group's  name in the shortcut menu ([d357fe1c](https://github.com/qTox/qTox/commit/d357fe1c650f57f4dad34294b544640f2d06eb88))
* **generalform:**  call UI retranslation when date or time format changes ([d601599d](https://github.com/qTox/qTox/commit/d601599de8c2d18b98f5fabd1b8ac78468fada8e))
* **genericchatform:**
  *  Fixed position of screenshot button ([86e44143](https://github.com/qTox/qTox/commit/86e44143ad7e473198ff6aa340be4136f2cca569))
  * separate messages from different days ([8ebad59a](https://github.com/qTox/qTox/commit/8ebad59a3e3aa42324a5821760b0752546a1e5ac))
* **groupinviteform:**
  *  escape HTML ([e4bc8570](https://github.com/qTox/qTox/commit/e4bc857037cf7cecd88929af8af453a800af2201))
  *  consider dateTime format in group invites ([6030b083](https://github.com/qTox/qTox/commit/6030b083b131cb367971eb205403d80764c2564e), closes [#3058](https://github.com/qTox/qTox/issues/3058))
* **i18n:**  Divide getting and translating Toxme error message ([98a1f23b](https://github.com/qTox/qTox/commit/98a1f23bfbbe341c50ed9ee2cc52c1f9dfc15110))
* **l10n:**
  *  remove unnecessary space in Czech translation ([47153b3d](https://github.com/qTox/qTox/commit/47153b3d77011c4a944bd597ad3a059e9668ed7c))
  *  missing argument in German translation ([e6e666fa](https://github.com/qTox/qTox/commit/e6e666fa8ce36cd3792d4536a59ab2b65fd5b546))
  *  incorrect/missing arguments in Arabic translation ([82bd897b](https://github.com/qTox/qTox/commit/82bd897ba46e388ecdb60c16484e056a607e45fa))
* **loginscreen.cpp:**  fix password input focus after mouse click ([6e8ea15a](https://github.com/qTox/qTox/commit/6e8ea15a15c82ce29a5d0d1267d699af75a5f3cd))
* **main:**  Closing file before removing ([29ab61ef](https://github.com/qTox/qTox/commit/29ab61efdf2e4c335c9f32d3552934bc59057830))
* **markdown:**  Remove spaces from markdown translation ([fca5f155](https://github.com/qTox/qTox/commit/fca5f15532ddbc21d70e6295d0742218cbc6fe1e))
* **passwordedit.cpp:**  Fix build issue with Qt 5.3 ([f18db4fd](https://github.com/qTox/qTox/commit/f18db4fd508c170fb93ae1e04fc7f2e0b5486bb7), closes [#3416](https://github.com/qTox/qTox/issues/3416))
* **passwordfields:**  use PasswordEdit widget for all password fields ([e3d0cc0e](https://github.com/qTox/qTox/commit/e3d0cc0e55ee91d85942b3beddac716435b78bee), closes [#3378](https://github.com/qTox/qTox/issues/3378))
* **platform:**  Added checkCapsLock OSX implementation ([35a0e1fb](https://github.com/qTox/qTox/commit/35a0e1fb6f0e0417ddc24408904a334d95e05c65))
* **profile:**
  *  Fix for opening file dialog using Nautilus file manager ([881409b9](https://github.com/qTox/qTox/commit/881409b91fe133ae53b2e0344a6d764fd0d0a6d7), closes [#3436](https://github.com/qTox/qTox/issues/3436))
  *  change password buttons behaviour ([f9edd39b](https://github.com/qTox/qTox/commit/f9edd39bba64a0fd92c6e9820f3265618d2057fc), closes [#3300](https://github.com/qTox/qTox/issues/3300))
* **profileform:**  set parent for validator ([93c6aa8a](https://github.com/qTox/qTox/commit/93c6aa8ac0519110e4de249346811e60dbb006c6))
* **qtox.pro:**  don't depend on GTK in order to build on Linux ([2d06b996](https://github.com/qTox/qTox/commit/2d06b9960c60f1fc886ae04c595bd9a921cff316))
* **screen-grabber:**  fix crash ([780a0179](https://github.com/qTox/qTox/commit/780a017928cfa5613fddada6e787bd6f3965baa9))
* **settings:**
  *  Look for portable setting in module path, not CWD ([17e57982](https://github.com/qTox/qTox/commit/17e57982dffa0dabffb7839cfd34fb34ef51ec88))
  *  correct ordering of languages ([7c63594a](https://github.com/qTox/qTox/commit/7c63594adf16d033c2ad40b369bb79fa98f8a95b))
  *  make it clear that `Markdown` is about text formatting ([67d01a73](https://github.com/qTox/qTox/commit/67d01a73c440f6b4ffeb9d5f0dbf560c832cf41f))
* **simple_make.sh:**
  *  add sqlite dependencies for Fedora ([5cb271b0](https://github.com/qTox/qTox/commit/5cb271b0c0ce2d3a1700d05559c7a0a3ce34af25))
  *  add missing dependencies for Fedora ([5b51f71f](https://github.com/qTox/qTox/commit/5b51f71ff86a0784aa0e74e13face02eda5d6c47), closes [#2998](https://github.com/qTox/qTox/issues/2998))
* **systemtray:**  don't activate qTox widget on tray icon click in Unity backend ([2f0ffdd2](https://github.com/qTox/qTox/commit/2f0ffdd27e8750f1554d0d10174ce3e831e2ac9a))
* **systemtrayicon:**
  *  don't set an invalid and useless icon on GTK ([a13c5667](https://github.com/qTox/qTox/commit/a13c566736b11880e43fe3af870b521175aeabe7), closes [#3154](https://github.com/qTox/qTox/issues/3154))
* **toxsave, profileimporter:**  Added `remove` function call before overwrite file ([58ea0afe](https://github.com/qTox/qTox/commit/58ea0afed1c699badecb6310786b6b018e23c0e5))
* **translator:**  Added layout direction reset on translation. ([927d512f](https://github.com/qTox/qTox/commit/927d512fa2a9925db59a22e0a46a7ae992995924))
* **ui:**  Prevent suicide crash on logout ([2bdd9824](https://github.com/qTox/qTox/commit/2bdd9824c758a7ce3ff0cb06fa59ed7d8c81455a), closes [#2480](https://github.com/qTox/qTox/issues/2480))
* **updater:**  Use module path, not working dir ([0a2e96ab](https://github.com/qTox/qTox/commit/0a2e96ab07a9c0427660124a2f1ff45bec0ba0b8))
* **video:**
  *  guard storeVideoFrame() against freeing in-use memory ([5b31b5db](https://github.com/qTox/qTox/commit/5b31b5db723e97daa7404d5c17ee9340d97198e8))
  *  force the use of non-deprecated pixel formats for YUV ([df3345dc](https://github.com/qTox/qTox/commit/df3345dce5827672a4106b11575728690fcd38f2))
  *  use a QReadWriteLock to manage camera access ([de6475f3](https://github.com/qTox/qTox/commit/de6475f3d362442122b574c6f5b3f84a60b90784))
  *  specify color ranges for pixel formats that are not YUV ([00270ee4](https://github.com/qTox/qTox/commit/00270ee4d21ea5cc28591d568f1b68bd2c35bf73))
  *  fix invalid VideoSource ID allocation ([707f7af2](https://github.com/qTox/qTox/commit/707f7af29ab55d75957cf62fc731797881c018ce))
  *  added declaration for missing biglock in CameraSource ([c4f88df7](https://github.com/qTox/qTox/commit/c4f88df7c986b1fb61caead9ce58d6c9b37cfbfe))
  *  fix a use-after-free with VideoFrame ([8487dcec](https://github.com/qTox/qTox/commit/8487dcecf846043733d1c77083169d9599bfc8a1))
  *  fix slanted video when video size is not divisible by 8 ([904495d2](https://github.com/qTox/qTox/commit/904495d2bf2b85da4c2f381778a0bb09b4ce1bbf))
  *  fix memory leak caused by unfreed buffers in CoreVideoSource ([3df6b990](https://github.com/qTox/qTox/commit/3df6b990ae4e137ae4069961131ebb39e1dbdf06))
  *  fix CoreAV and VideoSurface to conform to new VideoFrame ([277ddc3d](https://github.com/qTox/qTox/commit/277ddc3d2f31078fdb4320ce834785c2120d02a1))
  *  Changed minimum window size with video ([f8a45b40](https://github.com/qTox/qTox/commit/f8a45b40519de8f38cef80f5ffc41c19417e0139))
  *  do not list the same mode twice ([03c39236](https://github.com/qTox/qTox/commit/03c392369477870273cfc6cc69986d5ffad82bd3))
  *  fix video resolution setting ([b4df3c8b](https://github.com/qTox/qTox/commit/b4df3c8b4aabf62f2170586a6a9a49d648d60040), closes [#1033](https://github.com/qTox/qTox/issues/1033))
* **videoframe:**  Added correct image copy ([1ddc1371](https://github.com/qTox/qTox/commit/1ddc1371a0ea12de9621bd387b633ee8cdd575b3))
* **widget:**
  * change received files execution method ([def2e880](https://github.com/qTox/qTox/commit/def2e880c9a4190b697bc97edd6d775b15ea1978), closes [#3140](https://github.com/qTox/qTox/issues/3140))
  *  Added saving window state before closing ([bfb5dae6](https://github.com/qTox/qTox/commit/bfb5dae6faa12fdcc09952d27e9904314e01762a))
  *  properly open chat window ([c17c3405](https://github.com/qTox/qTox/commit/c17c3405bfd14acd494fb81dd624740ea0f0ccd4), closes [#3386](https://github.com/qTox/qTox/issues/3386))
  *  rename "Activate" to "Show" ([6173199a](https://github.com/qTox/qTox/commit/6173199a5b8eb81c9957044bc31edb2a77248a45))
  *  delete icon in destructor ([f82f49da](https://github.com/qTox/qTox/commit/f82f49da4d88fa89349146f5083366b3773187c3))
  *  open a chat window instead of contacts list in multi-window mode ([fdf0cbb1](https://github.com/qTox/qTox/commit/fdf0cbb1e1b82f4f4327bb0a1d6d64da340ffadd), closes [#3212](https://github.com/qTox/qTox/issues/3212))
  * show unread messages notification ([c81e6e2d](https://github.com/qTox/qTox/commit/c81e6e2dd1d499a0b14bf2a0f494f31cbfcc7329))
  *  properly show status messages ([dcb8c3f3](https://github.com/qTox/qTox/commit/dcb8c3f3232c5458d86e1595aa704ce3362cb41b), closes [#3123](https://github.com/qTox/qTox/issues/3123))
* **x11grab:**  try and use the current display ([294bdab7](https://github.com/qTox/qTox/commit/294bdab77f8fb6c2b29917f5318ffe4ec2bc2ab6))



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
