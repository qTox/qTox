# Hardening qTox with AppArmor

qTox can be confined with AppArmor on Linux to reduce attack vectors in case
remote code execution exploit is being used. Please note that [MAC's] (of
course) does not guarantee perfect security, but it will:
- Deny access to your `~/.bashrc`, `~/.ssh/*`
  `~/.config/path/to/your/password/manager/file`, etc.
- Deny creating autostart entries (in `~/.config/autostart`, etc).
- Deny launching random executables (like `sudo`, `su`, etc...).
- And more.

Consider using additional security measures like [Firejail] to improve security
even more.

Please also note that not all distributions have full AppArmor feature set
available. For example, Debian (at least up to Debian 10 (buster)) does not have
network, DBus mediation available. Also, X Server, shared user configuration
files (like `~/.config/QtProject.conf`, caches, etc), opening web links via
unconfined browsers introduces additional attack vectors, too. So please be
cautious even with number of security measures applied.

**AppArmor profile attaches only to `/usr/bin/qtox` and `/usr/local/bin/qtox`
executables by default**. See [Tuning permissions](#tuning-permissions) for
custom setups.

## Installing profile

Select AppArmor profile from appropriate `security/apparmor/X` subdirectory
depending on what AppArmor version is available for your Linux distribution
release:

- 2.13.3
  - Debian 11 (bullseye) (or newer)
  - Ubuntu 19.10 (or newer)
  - openSUSE Tumbleweed
- 2.13.2
  - Debian 10 (buster)
  - Ubuntu 19.04 (Disco Dingo)
- 2.12.1
  - Debian 9 (stretch) or older
  - Ubuntu 18.10 or older

To enable AppArmor profile on your system, run prepared install script:

```
sudo security/apparmor/x.y.z/install.sh
```
Restart `qTox` if it was already running before enabling AppArmor profile.

## Checking if qTox is actually confined

Run `aa-status` command line utility and check if `qTox` is listed within `X
processes are in enforced mode.` list:
```
sudo aa-status
   ...
21 processes are in enforce mode.
   /usr/lib/ipsec/charon (2421)            
   /usr/sbin/cups-browsed (839)
   ...
   /usr/bin/qtox (16315) qtox
   ...
```

Alternatively, use `ps` and `grep`:

```
ps auxZ | fgrep qtox
qtox (enforce)                  vincas   16315  2.0  1.1 1502292 180220 ?      SLl  12:21   0:38 /usr/bin/qtox
```

If OK it's marked as `(enforce)`. `unconfined` means AppArmor profile is not
attached to the process, no confinement is being applied.

## Troubleshooting

If you believe that some feature is unavailable, or some files you need access
to are inaccessible due to enforced AppArmor profile, check system logs for the
hints.

On Debian/Ubuntu:

```
sudo fgrep DENIED /var/log/syslog
```

On openSUSE, OR if you have `auditd` daemon installed:
```
sudo fgrep DENIED /var/log/audit/audit.log
```

You will see messages like this:
```
type=AVC msg=audit(1549793273.269:149): apparmor="DENIED" operation="open" profile="qtox" name="/home/vincas/.config/klanguageove
rridesrc" pid=3037 comm="qtox" requested_mask="r" denied_mask="r" fsuid=1000 ouid=1000
```

This means that `r`ead access was denied to the file
`/home/vincas/.config/klanguageoverridesrc`, owned by you (ouid 1000), by
AppArmor profile `qtox` (available in `/etc/apparmor.d/usr.bin.qtox`).

Please create issue if you detect new AppArmor `DENIED` messages and you believe
that these denials are relevant for other users too. Meanwhile, workaround by
adding manual rule. DO NOT modify `/etc/apparmor.d/usr.bin.qtox` directly! See
[Tuning permissions](#tuning-permissions) for fixing access issues.

## Tuning permissions

If you need access to files (for file sharing) other than from your `$HOME` or
mounted media, create/modify `/etc/apparmor.d/tunables/usr.bin.qtox.d/local`
file and append writable path variable:

```
@{qtox_additional_rw_dirs} += /path/to/some/directory
```

Alternatively, if you need more custom/advanced rules (not only for file
access), create/modify `/etc/apparmor.d/local/usr.bin.qtox` file.

Rule example for reading only, recursively (note the comma!):

```
/path/to/directory/** r,
```

For reading and writing, recursively:
```
/path/to/directory/** rw,
```

Restart AppArmor to reload profiles after modifications:

```
sudo systemctl restart apparmor
```

If AppArmor restart fails, check syntax errors by invoking AppArmor parser
directly:

```
sudo apparmor_parser -r /etc/apparmor.d/usr.bin.qtox
```

For custom installations, when `qTox` executable is not `/usr/bin/qtox` or
`/usr/local/bin/qtox`:
1. create `/etc/apparmor.d/tunables/usr.bin.qtox.d/local`, adding
   `@{qtox_prefix} += /path/to/your/custom/install/prefix` line.
2. modify `/etc/apparmor.d/usr.bin.qtox` profile attachement path: `profile qtox
   /{usr{,local}/bin/qtox,path/to/your/qtox_executable} {`

Restart AppArmor and [check](#checking-if-qtox-is-actually-confined) if `qTox`
process under custom path is actually confined.

## Other resources

Check [Debian], [Ubuntu], [Upstream AppArmor] Wiki pages for more info.

[Debian]: https://wiki.debian.org/AppArmor
[Firejail]: https://firejail.wordpress.com
[MAC's]: https://en.wikipedia.org/wiki/Mandatory_access_control
[Ubuntu]: https://wiki.ubuntu.com/AppArmor
[Upstream AppArmor]: https://gitlab.com/apparmor/apparmor/wikis/home
