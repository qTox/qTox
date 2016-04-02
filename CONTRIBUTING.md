# Filing an issue

### Must read
* If you aren't sure, you can ask on the [**IRC channel**](https://webchat.freenode.net/?channels=qtox) or read our [**wiki**](https://github.com/tux3/qTox/wiki) first.
* Do a quick **search**. Others might have already reported the issue.
* Write in **English**!
* Provide **version** information: (You can find version numbers in menu `Settings → About`)
  ```
qTox: 
Commit hash: 
toxcore: 
Qt: 
OS version: 

  ```
* Provide **steps** to reproduce the problem, it will be easier to pinpoint the fault.
* **Screenshots**! A screenshot is worth a thousand words. Just upload it. [(How?)](https://help.github.com/articles/file-attachments-on-issues-and-pull-requests)

### Good to know
* **Patience**. The dev team is small and resource limited. Devs have to find time, analyze the problem and fix the issue, it all takes time. :clock3:
* If you can code, why not become a **contributor** by fixing the issue and opening a pull request? :wink:
* Harsh words or threats won't help your situation. What's worse, your complaint will (very likely) be **ignored**. :fearful:


# Opening a pull request

### Must read
* Use **[commit message format](#commit-message-format)**.
* Read our [**coding guidelines**](https://github.com/tux3/qTox/wiki/Coding).
* Keep the title **short** and provide a **clear** description about what your pull request does.
* Provide **screenshots** for UI related changes.
* Keep your git commit history **clean** and **precise**. Commits like `xxx fixup` should not appear.
* If your commit fixes a reported issue (for example #4134), add the following message to the commit `Fixes #4134.`. [Here is an example](https://github.com/tux3/qTox/commit/87160526d5bafcee7869d6741a06045e13d731d5).

### Good to know
* **Search** the pull request history! Others might have already implemented your idea and it could be waiting to be merged (or have been rejected already). Save your precious time by doing a search first.
* When resolving merge conflicts, do `git rebase <target_branch_name>`, don't do `git pull`. Then you can start fixing the conflicts. [Here is a good explanation](https://www.atlassian.com/git/tutorials/merging-vs-rebasing).


## <a name="commit"></a> Git Commit Guidelines

We have very precise rules over how our git commit messages can be formatted.
This leads to **more readable messages** that are easy to follow when looking
through the **project history**.  But also, we use the git commit messages to
**generate the qTox change log** using [clog-cli]
(https://github.com/clog-tool/clog-cli).


### Commit Message Format
Each commit message consists of a **header**, a **body** and a **footer**.  The header has a special
format that includes a **type**, a **scope** and a **subject**:

```
<type>(<scope>): <subject>
<BLANK LINE>
<body>
<BLANK LINE>
<footer>
```

The **header** is mandatory and the **scope** of the header is optional.

Any line of the commit message cannot be longer 100 characters! This allows the message to be easier
to read on GitHub as well as in various git tools.

Note that in the future `gitcop` will be used to check if commits in pull
request conform to commit message format, but since it can't be configured to
have an optional `(<scope>)`, it will claim that messages without it are wrong,
while they're perfectly fine.

### Revert
If the commit reverts a previous commit, it should begin with `revert: `, followed by the header of the reverted commit. In the body it should say: `This reverts commit <hash>.`, where the hash is the SHA of the commit being reverted.

### Type
Must be one of the following:

* **feat**: A new feature
* **fix**: A bug fix
* **docs**: Documentation only changes
* **style**: Changes that do not affect the meaning of the code (white-space, formatting, etc)
* **refactor**: A code change that neither fixes a bug nor adds a feature
* **perf**: A code change that improves performance
* **test**: Adding missing tests
* **chore**: Changes to the build process or auxiliary tools and libraries such as documentation
  generation

### Scope
The scope could be anything specifying place of the commit change. For example `$location`,
`$browser`, `$compile`, `$rootScope`, `ngHref`, `ngClick`, `ngView`, etc...

### Subject
The subject contains succinct description of the change:

* use the imperative, present tense: "change" not "changed" nor "changes"
* don't capitalize first letter
* no dot (.) at the end

### Body
Just as in the **subject**, use the imperative, present tense: "change" not "changed" nor "changes".
The body should include the motivation for the change and contrast this with previous behavior.

### Footer
The footer should contain any information about **Breaking Changes** and is also the place to
reference GitHub issues that this commit **Closes**.

**Breaking Changes** should start with the word `BREAKING CHANGE:` with a space or two newlines. The rest of the commit message is then used for this.
