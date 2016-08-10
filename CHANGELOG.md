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
