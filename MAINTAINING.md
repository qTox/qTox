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
- **use** [`merge-pr.sh`] script to merge PRs, e.g. `./merge-pr.sh 1234`.
  
  You don't have to use it, but then you're running into risk of breaking
  travis build of master & other PRs, since it verifies all commit messages,
  indlucing merge messages.
  
  Risk, that can be avoided when one doesn't type manually merge message :wink:
- **might want** to use [`test-pr.sh`].
  
  Commits that are about to be merged don't have to be signed, but merge-commit
  **must** be signed. To simplify process, and ensure that things are done
  "right", it's preferable to use [`merge-pr.sh`] script, which does that for
  you automatically.
- give a PR some "breathing space" right after it's created – i.e. merging
  something right away can lead to bugs & regressions suddenly popping up, thus
  it's preferable to wait at least a day or so, to let people test & comment on
  the PR before merging.
  - with trivial changes, like fixing typos or something along those lines,
    feel free to merge right away.
- if you're about to merge PR, assign yourself to it.
- if you decide that PR actually isn't to be (yet) merged, de-assign yourself.
- if PR requires some changes, comment what parts need to be adjusted, and
  assign the `PR-needs-changes` label – after requested changes are done,
  remove the label.
- if PR doesn't apply properly on top of current master (when using
  [`merge-pr.sh`] script), request a rebase and tag PR with `PR-needs-rebase`.

## Reviews

Currently `reviewable.io` is being used to review changes that land in qTox.

How to review:

1. Click on the `Reviewable` button in pull request.
2. Once Reviewable opens, comment on the lines that need changes.
3. Mark as reviewed only those files that don't require any changes – this
   makes it easier to see which files need to be changed & reviewed again once
   change is made.
4. If pull request is good to be merged, press `LGTM` button in Reviewable.
5. Once you're done with evaluating PR, press `Publish` to make comments
   visible on GitHub.

Note:

* when no one is assigned to the PR, *anyone* can review it
* when there are assigned people, only they can mark review as passed


# Issues

- tag issues
  - `help wanted` tag should be used whenever no one is currently working on
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
