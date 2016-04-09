# Filing an issue

### Must read
* If you aren't sure, you can ask on the
  [**IRC channel**](https://webchat.freenode.net/?channels=qtox) or read our
  [**wiki**](https://github.com/tux3/qTox/wiki) first.
* Do a quick **search**. Others might have already reported the issue.
* Write in **English**!
* Provide **version** information (you can find version numbers in menu
  `Settings → About`):
```
OS: 
qTox version: 
Commit hash: 
toxcore: 
Qt: 
```
* Provide **steps** to reproduce the problem, it will be easier to pinpoint the
  fault.
* **Screenshots**! A screenshot is worth a thousand words. Just upload it.
  [(How?)](https://help.github.com/articles/file-attachments-on-issues-and-pull-requests)

### Good to know
* **Patience**. The dev team is small and resource limited. Devs have to find
  time, analyze the problem and fix the issue, it all takes time. :clock3:
* If you can code, why not become a **contributor** by fixing the issue and
  opening a pull request? :wink:
* Harsh words or threats won't help your situation. What's worse, your complaint
  will (very likely) be **ignored**. :fearful:


# Opening a pull request

### Must read
* Use [**commit message format**](#commit-message-format).
* Read our [**coding guidelines**](https://github.com/tux3/qTox/wiki/Coding).
* Keep the title **short** and provide a **clear** description about what your
  pull request does.
* Provide **screenshots** for UI related changes.
* Keep your git commit history **clean** and **precise**. Commits like
  `xxx fixup` should not appear.
* If your commit fixes a reported issue (for example #4134), add the following
  message to the commit `Fixes #4134.`.
  [Here is an example](https://github.com/tux3/qTox/commit/87160526d5bafcee7869d6741a06045e13d731d5).

### Good to know
* **Search** the pull request history! Others might have already implemented
  your idea and it could be waiting to be merged (or have been rejected
  already). Save your precious time by doing a search first.
* When resolving merge conflicts, do `git rebase <target_branch_name>`, don't do
  `git pull`. Then you can start fixing the conflicts.
  [Here is a good explanation](https://www.atlassian.com/git/tutorials/merging-vs-rebasing).


## <a name="commit"></a> Git Commit Guidelines

We have very precise rules over how our git commit messages can be formatted.
This leads to **more readable messages** that are easy to follow when looking
through the **project history**.  But also, we use the git commit messages to
**generate the qTox change log** using
[clog-cli](https://github.com/clog-tool/clog-cli).


### Commit Message Format
Each commit message consists of a **header** and a **body**.  The header has a
special format that includes a **type**, a **scope** and a **subject**:

```
<type>(<scope>): <subject>
<BLANK LINE>
<body>
```

The **header** is mandatory and the **body** is optional. The **scope** of the
header is also optional.

### Header
The header must be a short (72 characters or less) summary of the changes made.

#### Type
Must be one of the following:

* **feat**: A new feature
* **fix**: A bug fix
* **docs**: Documentation only changes
* **style**: Changes that do not affect the meaning of the code (white-space,
  formatting, etc)
* **refactor**: A code change that neither fixes a bug nor adds a feature
* **perf**: A code change that improves performance
* **revert**: Reverts a previous commit
* **test**: Adding missing tests
* **chore**: Changes to the build process or auxiliary tools and libraries such
  as documentation generation

##### Revert
If the commit reverts a previous commit, it should begin with `revert: `,
followed by the header of the reverted commit. In the body it should say:
`Revert commit <hash>.`, where the hash is the SHA of the commit being reverted.

#### Scope
The scope could be anything specifying place of the commit change. For example
`$location`, `$browser`, `$compile`, `$rootScope`, `ngHref`, `ngClick`,
`ngView`, etc.

#### Subject
The subject contains succinct description of the change:

* use the imperative, present tense: "change" not "changed" nor "changes"
* don't capitalize first letter
* no dot (.) at the end

A properly formed git commit subject line should always be able to complete the
following sentence:

> If applied, this commit will ___your subject line here___

### Body
Wrap the body at 72 characters whenever possible (for example, don't modify long
links to follow this rule). Just as in the **subject**, use the imperative,
present tense: "change" not "changed" nor "changes". The body should include the
motivation for the change and contrast this with previous behavior.

The body contains (in order of appearance):

* A detailed **description** of the committed changes.
* References to GitHub issues that the commit **closes** (e.g., `Closes #000` or
  `Fixes #000`).
* Any **breaking changes**.

Include every section of the body that is relevant for your commit.

**Breaking changes** should start with the phrase `BREAKING CHANGE:` with a
space or two newlines. The rest of the commit message is then used for this.
