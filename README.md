<a name="qtox" />

<p align="center">
<img src="https://qtox.github.io/assets/imgs/logo_head.png" alt="qTox" />
</p>

---

<p align="center">
<a href="https://github.com/qTox/qTox/blob/master/LICENSE">
<img src="https://img.shields.io/badge/license-GPLv3%2B-blue.svg" alt="GPLv3+" />
</a>
<a href="https://travis-ci.org/qTox/qTox">
<img src="https://travis-ci.org/qTox/qTox.svg?branch=master" alt="Travis CI" />
</a>
<a href="https://hosted.weblate.org/engage/tox/?utm_source=widget">
<img src="https://hosted.weblate.org/widgets/tox/-/svg-badge.svg"
     alt="Translate on Weblate" />
</a>
<a href="https://gitter.im/qtox/qTox?utm_source=badge">
<img src="https://badges.gitter.im/Join Chat.svg" alt="Gitter">
</a>
<a href="https://github.com/qTox/release-schedule/blob/master/README.md">
<img src="https://qtox.github.io/release-schedule/status.svg"
title="Week of Merges: population of Merges increases!
Week of Testing: Your mana regenerates!" />
</a></p>

---

<p align="center"><b>
qTox is a chat, voice, video, and file transfer instant messaging client using
the encrypted peer-to-peer Tox protocol.
</b></p>

 **[User Manual] |**
 **[Install/Build] |**
 **[Roadmap] |**
 **[Report bugs] |**
 **[Jenkins builds] |**
 **[Mailing list] |**
 **IRC:** [#qtox@freenode]

---

Windows | Linux | OS X | FreeBSD
--------|-------|------|--------
**[64 bit release]**| **[Arch]**, **[Debian]**, **[Fedora]**, **[Gentoo]**, **[openSUSE]**, **[Ubuntu]** | **[Latest release]**  | **[Package & Port]**
[32 bit release]|**[AppImage]**, [Flatpak] | [Building instructions] |
[64 bit][64nightly], [32 bit][32nightly] nightly | [From Source], [Flatpak nightly], [AppImage nightly] | [Latest nightly] |

_**Bold** options are recommended._

Builds other than installer/packages don't receive updates automatically, so
make sure you get back to this site and regularly download the latest version of
qTox.

### Help us

If you're wondering how could you help, fear not, there are plenty of ways
:smile:

Some of them are:

* Spread the good word about qTox to make it more popular :smile:
* Have an opinion? Make sure to [voice it in the issues that need it] :wink:
* Fixing [easy issues] or [issues that need help]. Make sure to read
  [Contributing] first though :wink:
* [Testing] and [reporting bugs]
* [Translating, it's easy]
* [Reviewing and testing pull requests] – you don't need to be able to code to
  do that :wink:
* Take a task from our Roadmap below

### Roadmap

Currently qTox is under a feature freeze to clean up our codebase and tools.
During this time we want to prepare qTox for upcoming new features of toxcore.

The next steps are:


* move all toxcore abstractions into their own subproject
* write basic tests for this Core
* format the code base
* rework our TravisCI setup for faster PR checks
* rethink our Issue tracker

The current state is tracked in the [Code cleanup] project.



### Screenshots
Note: The screenshots may not always be up to date, but they should give a good
idea of the general look and features.


![Screenshot 01](https://i.imgur.com/olb89CN.png)
![Screenshot 02](https://i.imgur.com/tmX8z9s.png)


### Features

- One to one chat with friends
- Group chats
- File transfers, with previewing of images
- Audio calls, including group calls
- Video calls
- ToxMe and Tox URI support
- Translations in over 30 languages
- Avatars
- Faux offline messages
- History
- Screenshots
- Emoticons
- Auto-updates on Windows and packages on Linux
- And many more options!


### Organizational stuff

Happens in both IRC channel [#qtox@freenode] and on [qTox-dev mailing list].
If you are interested in participating, **join the channel** and **subscribe to
the mailing list**.

There are [IRC logs] available.

### GPG fingerprints

List of GPG fingerprints used by qTox developers to sign commits, merges, tags,
and possibly other stuff.

Active qTox maintainers:

```
7EB3 39FE 8817 47E7 01B7  D472 EBE3 6E66 A842 9B99      - Anthony Bilinski
3103 9166 FA90 2CA5 0D05  D608 5AF9 F2E2 9107 C727      – Diadlo
CA92 21C5 389B 7C50 AA5F  7793 52A5 0775 BE13 DF17      - noavarice
DA26 2CC9 3C0E 1E52 5AD2  1C85 9677 5D45 4B8E BF44      – sudden6
141C 880E 8BA2 5B19 8D0F  850F 7C13 2143 C1A3 A7D4      – tox-user
2880 C860 D95C 909D 3DA4  5C68 7E08 6DD6 6126 3264      – tux3
```

Past qTox maintainers:

```
C7A2 552D 0B25 0F98 3827  742C 1332 03A3 AC39 9151      – initramfs
BA78 83E2 2F9D 3594 5BA3  3760 5313 7C30 33F0 9008      – zetok
F365 8D0A 04A5 76A4 1072  FC0D 296F 0B76 4741 106C      – agilob
1157 616B BD86 0C53 9926  F813 9591 A163 FF9B E04C      – antis81
1D29 8BC7 25B7 BE82 65BA  EAB9 3DB8 E053 15C2 20AA      – Dubslow
```

Windows updates, managed by `tux3`:

```
AED3 1134 9C23 A123 E5C4  AA4B 139C A045 3DA2 D773
```

[#qtox@freenode]: https://webchat.freenode.net/?channels=qtox
[64 bit release]: https://github.com/qTox/qTox/releases/download/v1.17.2/setup-qtox-x86_64-release.exe
[32 bit release]: https://github.com/qTox/qTox/releases/download/v1.17.2/setup-qtox-i686-release.exe
[32nightly]: https://github.com/qTox/qTox-nightly-releases/releases/download/ci-master-latest/setup-qtox-i686-release.exe
[64nightly]: https://github.com/qTox/qTox-nightly-releases/releases/download/ci-master-latest/setup-qtox-x86_64-release.exe
[Flatpak]: https://github.com/qTox/qTox/releases/download/v1.17.2/qTox-v1.17.2.x86_64.flatpak
[Flatpak nightly]: https://github.com/qTox/qTox-nightly-releases/releases/download/ci-master-latest/qtox.flatpak
[AppImage]: https://github.com/qTox/qTox/releases/download/v1.17.2/qTox-v1.17.2.x86_64.AppImage
[AppImage nightly]: https://github.com/qTox/qTox-nightly-releases/releases/tag/ci-master-latest
[Arch]: /INSTALL.md#arch
[Building instructions]: /INSTALL.md#os-x
[Contributing]: /CONTRIBUTING.md#how-to-start-contributing
[Debian]: https://packages.debian.org/search?keywords=qtox
[easy issues]: https://github.com/qTox/qTox/labels/E-easy
[Latest release]: https://github.com/qTox/qTox/releases/download/v1.17.2/qTox.dmg
[Latest nightly]: https://github.com/qTox/qTox-nightly-releases/releases/download/ci-master-latest/qTox.dmg
[Fedora]: /INSTALL.md#fedora
[Gentoo]: /INSTALL.md#gentoo
[openSUSE]: /INSTALL.md#opensuse
[Install/Build]: /INSTALL.md
[IRC logs]: https://github.com/qTox/qtox-irc-logs
[issues that need help]: https://github.com/qTox/qTox/labels/help%20wanted
[Jenkins builds]: https://build.tox.chat/
[Mailing list]: https://lists.tox.chat
[From Source]: /INSTALL.md#linux
[qTox-dev mailing list]: https://lists.tox.chat/listinfo/qtox-dev
[Package & Port]: /INSTALL.md#freebsd-easy
[Report bugs]: https://github.com/qTox/qTox/wiki/Writing-Useful-Bug-Reports
[reporting bugs]: https://github.com/qTox/qTox/wiki/Writing-Useful-Bug-Reports
[Reviewing and testing pull requests]: /CONTRIBUTING.md#reviews
[Roadmap]: https://github.com/qTox/qTox/milestones
[Testing]: https://github.com/qTox/qTox/wiki/Testing
[Translating, it's easy]: /translations/README.md
[User Manual]: /doc/user_manual_en.md
[Ubuntu]: https://packages.ubuntu.com/search?keywords=qtox
[voice it in the issues that need it]: https://github.com/qTox/qTox/labels/I-feedback-wanted
[Code cleanup]: https://github.com/qTox/qTox/projects/3?fullscreen=true
