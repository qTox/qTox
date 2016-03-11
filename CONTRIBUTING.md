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
* Read our [**coding guidelines**](https://github.com/tux3/qTox/wiki/Coding).
* Keep the title **short** and provide a **clear** description about what your pull request does.
* Provide **screenshots** for UI related changes.
* Keep your git commit history **clean** and **precise**. Commits like `xxx fixup` should not appear.
* If your commit fixes a reported issue (for example #4134), add the following message to the commit `Fixes #4134.`. [Here is an example](https://github.com/tux3/qTox/commit/87160526d5bafcee7869d6741a06045e13d731d5).

### Good to know
* **Search** the pull request history! Others might have already implemented your idea and it could be waiting to be merged (or have been rejected already). Save your precious time by doing a search first.
* When resolving merge conflicts, do `git rebase <target_branch_name>`, don't do `git pull`. Then you can start fixing the conflicts. [Here is a good explanation](https://www.atlassian.com/git/tutorials/merging-vs-rebasing).

