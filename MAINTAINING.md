**Guidelines, overview of maintenance process, etc.**

*“Thou shall GPG-sign.”*

# Git config

## GPG signing

While contributors are suggested to use GPG, as a maintainer you **are
required** to use GPG to sign commits & merges.

If you don't have GPG signing set up yet, now is the moment to do it.

[Config, etc.](/CONTRIBUTING.md#git-config)


## SSH

Preferably use SSH.

There are quite a few articles about that:
https://help.github.com/categories/ssh/

## Useful aliases

Check whether commits are GPG-signed with `git logs`

```
git config --global alias.logs 'log --show-signature'
```

# Commits

- **always** use [commit message format]
- **always** GPG-sign your commits.
- it's preferable to make a PR with changes that you're about to commit.

  Yes, there might be a situation where something has to be fixed "right now"
  on master..

  Perhaps a security fix, who knows what future holds. If it's not *that*
  important, you're still better off making a PR. Even when you'll just
  fast-forward commits from PR onto the `master` branch.

  Reasoning for it is that it's always hard to catch bugs/mistakes that you
  create, while someone else who just briefly looked at the changeset possibly
  can see a problem `:)`

# Pull requests

- **do not** push any `Merge`, `Squash & Merge`, etc. buttons on the website!
  The only allowed way of merging is locally, since otherwise merge will not
  be signed, and websites can fairly well mess things up.
- **always** test PR that is being merged.
- **always** GPG-sign PR that you're merging.

  Commits that are about to be merged don't have to be signed, but the
  merge-commit **must** be signed. To simplify the process, and ensure that
  things are done "right", it's preferable to use the [`merge-pr.sh`] script,
  which does that for you automatically.
- **use** [`merge-pr.sh`] script to merge PRs. First checkout the target
  branch, usually either `master` or a release dev branch e.g. `v1.17-dev`,
  make sure it's up to date with qTox/qTox, then e.g. `./merge-pr.sh 1234`.

  You don't have to use it, but then you're running into risk of breaking
  CI build of master & other PRs, since it verifies all commit messages,
  indlucing merge messages.

  Risk, that can be avoided when one doesn't type manually merge message :wink:
- **might want** to use [`test-pr.sh`].
- give a PR some "breathing space" right after it's created – i.e. merging
  something right away can lead to bugs & regressions suddenly popping up, thus
  it's preferable to wait at least a day or so, to let people test & comment on
  the PR before merging.
  - with trivial changes, like fixing typos or something along those lines,
    feel free to merge right away.
- if a PR requires some changes, comment what parts need to be adjusted,
  preferably by using the `Reviewable` button on the first comment.
- if PR doesn't apply properly on top of current master (when using
  [`merge-pr.sh`] script), request a rebase
- if a PR requires changes but there has been no activity from the PR submitter
  for more than 2 months, close the PR.

# Continous Integration

All CI is done through GitHub actions. Nightly builds are published to
qTox/qTox releases.

# Issues

## Tagging Issues

- When you request more info to be provided in the issue, tag it with
    `O-need-info`. Remove tag once the needed info has been provided.
    - If the needed information is not provided after 30 days, add the `O-stale`
      tag and a comment requesting the information again.
      - If the `O-stale` tag is present for more than 30 day, the issue should
        be closed.
- If you're going to fix the issue, assign yourself to it.
- when closing an issue, preferably state the reason why it was closed, unless
  it was closed automatically by commit message.
- When issue is a duplicate, close the issue with less useful information and
  comment the link to the other issue.

## Determining Priority

The priority of an issue should be determined by taking into account the user
impact and how hard it is to fix the issue.

### User impact

We have two labels to rate user impact `U-high` and `U-low`.

Use `U-high` if
- Many users have reported this issue
- The problem is triggered often during typical use of qTox
- The problem causes data loss
- There is no workaround

Use `U-low` if
- Few users reported this problem
- The problem occurs very sporadically
- The problem needs a very specific set of conditions to appear
- There is a workaround
- The problem appears only after multiple days of usage

### Difficulty to fix

We have two labels to estimate the difficulty of a fix, `D-easy` and `D-hard`.

Use `D-easy` if you think that:
- The issue is well described
- The issue can be consistently reproduced
- The issue needs no specific equipment to fix, e.g. specific OS, webcam,...
- The code that causes the problem is known

Use `D-hard` if you think that:
- The issue is described only vaguely
- The exact way to reproduce the issue is not known
- The issue happens only on a specifc OS
- The issue is not only caused by code from qTox

### Determining initial priority

After assesing the user impact and the difficulty to fix the issue you look up
the initial priority for the issue in the following table:

|          |  `U-high`  | `U-low`    |
|----------|------------|------------|
| `E-easy` | `P-high`   | `P-medium` |
| `E-hard` | `P-medium` | `P-low`    |

Possible security issues should be tagged with `P-high` initally. If they are
confirmed security issues, the tag should be changed to `P-very-high`, else
apply the normal rating process.

# Translations from Weblate

Weblate provides an easy way for people to translate qTox.

New translable strings need to be generated into a form Weblate can consume
using `./tools/update-translation-files.sh ALL` and commiting the result. This
should be done as soon as strings are available since weblate follows our
branch, so is checked for in CI.

To get translations into qTox, fast-forward merge from
https://hosted.weblate.org/git/tox/qtox/.

If a new translation language has been added, update the following files:
  - `translations/CMakeLists.txt`
  - `src/widget/form/settings/generalform.cpp`
  - `translations/README.md`
  - `translations/i18n.pri`
  - `translations/translations.qrc`

# Releases

## Tagging scheme

- tag versions that are to be released, make sure that they are GPG-signed,
  i.e. `git tag -s v1.8.0`
- use semantic versions for tags: `vMAJOR.MINOR.PATCH`
  - `MAJOR` – bump version when there are breaking changes to video, audio,
    text chats, groupchats, file transfers, and any other basic functionality.
    For other things, `MINOR` and `PATCH` are to be bumped.
  - `MINOR` – bump version when there are:
    - new features added
    - UI/feature breaking changes
    - other non-breaking changes
  - `PATCH` – bump when there have been only fixes added. If changes include
    something more than just bugfixes, bump `MAJOR` or `MINOR` version
    accordingly.
- bumping a higher-level version "resets" lower-version numbers, e.g.
  `v1.7.1 → v2.0.0`

## Steps for release

### Before tagging

- Format all code using the [`./tools/format-code.sh`] script
- Update the Flatpak manifest of our [Flathub repository] with the script in flatpak/update_flathub_descriptor_dependencies.py
  - Make sure to check if new dependencies need to be added, add them if necessary
- Update version number for windows/osx packages using the
  [`./tools/update-versions.sh`] script, e.g. `./tools/update-versions.sh
  1.11.0`
- Update toxcore version number to the latest tag in [`./buildscripts/download/download_toxcore.sh]
- Pull in latest translations from Weblate.
- Update the bootstrap nodelist at `./res/nodes.json` from https://nodes.tox.chat/json.
  This can be done by running [`./tools/update-nodes.sh`]
- Generate changelog with `clog`.
  - In a `MAJOR`/`MINOR` release tag should include information that changelog
    is located in the `CHANGELOG.md` file, e.g. `For details see CHANGELOG.md`
- To release a `PATCH` version after non-fix changes have landed on `master`
  branch, checkout latest `MAJOR`/`MINOR` version and `git cherry-pick -x`
  commits from `master` that you want `PATCH` release to include. Once
  cherry-picking has been done, tag HEAD of the branch.
  - When making a `PATCH` tag, include in tag message short summary of what the
    tag release fixes, and to whom it's interesting (often only some
    OSes/distributions would find given `PATCH` release interesting).

### After tagging

- Create and GPG-sign the tar.lz and tar.gz archives using
  [`./tools/create-tarballs.sh`] script, and upload both archives plus both
  signature files to the github draft release that was created by CI.
- Download the binaries that are part of the draft release, sign them in
  in detached and ascii armored mode, e.g. `gpg -a -b <artifact>`, and upload
  the signatures to the draft release.
- Add a title and description to the draft release, then publish the release.
- Update download links on https://tox.chat to point to the new release.
- Write a short blog post for https://github.com/qTox/blog/ and advertise the
  post on Tox IRC channels, popular Tox groups, reddit, or whatever other platforms.
- Merge [`./flatpak/io.github.qtox.qTox.json`] into the manifest of our
  [Flathub repository]. Keep the [Flathub repository]'s version of "sources" for
  qTox.
- After the build passed for qTox on all architectures on
  [the Flathub build bot], merge the PR into the master branch of our
  [Flathub repository].

# How to become a maintainer?

Contribute, review & test pull requests, be active, oh and don't forget to
mention that you would want to become a maintainer :P

Aside from contents of [`CONTRIBUTING.md`] you should also know the contents of
this file.

Once you're confident about your knowledge and you've been around the project
helping for a while, ask to be added to the `qTox` organization on GitHub.


[commit message format]: /CONTRIBUTING.md#commit-message-format
[`CONTRIBUTING.md`]: /CONTRIBUTING.md
[`merge-pr.sh`]: /merge-pr.sh
[`test-pr.sh`]: /test-pr.sh
[`./tools/create-tarball.sh`]: /tools/create-tarball.sh
[`./tools/update-nodes.sh`]: /tools/update-nodes.sh
[`./tools/update-versions.sh`]: /tools/update-versions.sh
[`./tools/format-code.sh`]: /tools/format-code.sh
[Flathub repository]: https://github.com/flathub/io.github.qtox.qTox
[`./flatpak/io.github.qtox.qTox.json`]: flatpak/io.github.qtox.qTox.json
[the Flathub build bot]: https://flathub.org/builds/#/
[qTox-nightly-release]: https://github.com/qTox/qTox-nightly-releases
