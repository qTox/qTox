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
- **use** [`merge-pr.sh`] script to merge PRs, e.g. `./merge-pr.sh 1234`.
  
  You don't have to use it, but then you're running into risk of breaking
  travis build of master & other PRs, since it verifies all commit messages,
  indlucing merge messages.
  
  Risk, that can be avoided when one doesn't type manually merge message :wink:
- **might want** to use [`test-pr.sh`].
- give a PR some "breathing space" right after it's created – i.e. merging
  something right away can lead to bugs & regressions suddenly popping up, thus
  it's preferable to wait at least a day or so, to let people test & comment on
  the PR before merging.
  - with trivial changes, like fixing typos or something along those lines,
    feel free to merge right away.
- if PR requires some changes, comment what parts need to be adjusted, and
  assign the `PR-needs-changes` label – after requested changes are done,
  remove the label.
- if PR doesn't apply properly on top of current master (when using
  [`merge-pr.sh`] script), request a rebase and tag PR with `PR-needs-rebase`.
- if a PR requires changes but there has been no activity from the PR submitter
  for more than 2 months, close the PR.


# Issues

- tag issues
  - `up for grabs` tag should be used whenever no one is currently working on
    the issue, and you're not going to work on it in foreseeable future (hours,
    day or two).
  - when you request more info to be provided in the issue, tag it with
    `I-need-info`. Remove tag once needed info has been provided.
    - sometimes there are issue with only one comment, and no reply to query
      for more info. Those issues usually can be closed after some period,
      preferably after a month or more with no reply. To search for them, you
      can specify time period when issue with a given tag was last updated,
      e.g.: `label:I-need-info updated:<2016-03-01`.
- if you're going to fix the issue, assign yourself to it.
- when closing an issue, preferably state the reason why it was closed, unless
  it was closed automatically by commit message.
- when issue is a duplicate, tag it with `duplicate`, and issue that it was a
  duplicate of, tag with higher `duplicates:#`


# Translations from Weblate

Weblate provides an easy way for people to translate qTox. On one hand, it does
require a bit more attention & regular checking whether there are new
translations, on the other, it lessened problems that were happening with
"manual" way of providing translations.

To get translations into qTox:

1. Go to `https://hosted.weblate.org/projects/tox/qtox/#repository` and lock
   the repository for translations.
2. Make sure you have git setup to automatically gpg sign commits
3. In the root of the qTox repository execute the script
   `tools/update-weblate.sh`
4. Check what has been changed compared to master: `git diff upstream/master`
5. Checkout a new branch with e.g. `git checkout -b update_weblate` and open
   a Pull Request for it on Github.
6. After the Pull Request has been merged, unlock Weblate and `reset` it to
   master

## Adding new translations

Files to edit when adding a new translation:

- `CMakeLists.txt`
- `src/widget/form/settings/generalform.cpp`
- `translations/README.md`
- `translations/i18n.pri`
- `translations/translations.qrc`

Follow steps for adding translations from Weblate up to step 5. Next:

1. Edit files to add translation to qTox.
2. Before committing, clean up translation from Weblate and commit the change:

   ```
   ./tools/update-translation-files.sh translations/$TRANSLATION.ts
   git commit --amend translations/$TRANSLATION.ts
   ```
3. Commit the changes to other files adding language to qTox.


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

- Update version number for windows/osx packages using the
  [`./tools/update-versions.sh`] script, e.g. `./tools/update-versions.sh
  1.11.0`
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

- Create and GPG-sign tarball using [`./tools/create-tarball.sh`] script, and
  upload the tarball to the github release that was created by a Travis OSX
  release job.
- Update download links on https://tox.chat to point to the new release.
- Write a short blog post for https://github.com/qTox/blog/ and advertise the post
  on Tox IRC channels, popular Tox groups, reddit, or whatever other platforms.
- Open a PR to update the Flatpak manifest of our [Flathub repository] with the 
  changes from [`./flatpak/io.github.qtox.qTox.json`].
- Comment to the PR with `bot, build` to execute a test build
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
[`./tools/deweblate-translation-file.sh`]: /tools/deweblate-translation-file.sh
[`./tools/create-tarball.sh`]: /tools/create-tarball.sh
[`./tools/update-versions.sh`]: /tools/update-versions.sh
[Flathub repository]: https://github.com/flathub/io.github.qtox.qTox
[`./flatpak/io.github.qtox.qTox.json`]: flatpak/io.github.qtox.qTox.json
[the Flathub build bot]: https://flathub.org/builds/#/
