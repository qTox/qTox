The qTox OS X updater is a mix of objective C and Go compiled as static binaries used do effortless updates in the background.

It uses Objective C to access Apples own security framework and call some long dead APIs in order to give the statically linked go updater permissions to install the latest build without prompting the user for every file.

* Release commits: ``https://github.com/Tox/qTox_updater``
* Development commits: ``https://github.mit.edu/sean-2/updater``

Compiling: 

* ```clang qtox_sudo.m -framework corefoundation -framework security -framework cocoa -Os -o qtox_sudo```
* ```go build updater.go```

(Starting with this commit all commits will be signed with [this key](http://pgp.mit.edu/pks/lookup?op=get&search=0x13D2043169D25DF4).)
