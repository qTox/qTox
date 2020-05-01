<div dir=rtl>

# راهنمای نصب
- [پیشنیاز‌ها](#پیشنیازها)
- [لینوکس](#لینوکس)
  - [نصب آسان](#نصب-آسان)
    - [آرچ لینوکس](#آرچ-آسان)
    - [فدورا](#فدورا-آسان)
    - [جنتوو](#جنتو-آسان)
    - [اسلکوار](#اسلکوار-آسان)
    - [فری بی‌اس‌دی](#فری-بی-اس-دی-آسان)
  - [نصب Git](#نصب-Git)
    - [آرچ](#گیت-آرچ)
    - [دبیان](#گیت-دبیان)
    - [فدورا](#فدورا-گیت)
    - [اوپن‌سوزه](#اوپن‌سوزه-گیت)
    - [اوبونتو](#اوبونتو-گیت)
  - [کپی کردن qTox](#کپی-کیوتاکس)
  - [GCC, Qt, FFmpeg, OpenAL Soft و qrencode](#سایر-نیازمندیها)
    - [آرچ](#سایر-نیاز-آرچ)
    - [دبیان](#سایر-نیاز-دبیان)
    - [فدورا](#سایر-نیزا-فدورا)
    - [اوپن‌سوزه](#سایر-نیاز-اوپن‌سوزه)
    - [اسلک‌وار](#سایر-نیازها-اسلکوار)
    - [اوبونتو نسخه بالاتر یا برابر 15.04](#سایر-نیاز-اوبونتو)
    - [اوبونتو نسخه بالاتر یا برابر با 16.04](#سایر-نیاز-اوبونتو16)
  - [نیازمندی های toxcore](#نیاز-تاکس-کر)
    - [آرچ](#آرچ-تاکس-کر)
    - [دبیان](#دبیان-تاکس-کر)
    - [فدورا](#فدورا-تاکس-کر)
    - [اوپن‌سوزه](#اوپن‌سوزه-تاکس-کر)
    - [اسلک‌وار](#اسلک-وار-تاکس-کر)
    - [اوبونتو نسخه بالاتر یا برابر 15.04](#اوبونتو-تاکس-کر)
  - [sqlcipher](#اس-کیو-ال)
  - [کامپایل toxcore](#کامپایل-تاکس-کر)
  - [کامپایل qTox](#کامپایل-کیو-تاکس)
- [OS X](#او-اس-ایکس)
- [ویندوز](#ویندوز)
  - [کامپایل برای سایر سیستم عامل ها در لینوکس](#کامپایل-برای-سایر-سیتمها)
  - [کامپایل در ویندوز](#نیتیو)
- [سویچ های زمان کامپایل](#سویچهای-کامپایل)


<a name="پیشنیازها" />

## پیشنیاز‌ها

| نام           | نسخه     | ماژولها                                                  |
|---------------|-------------|----------------------------------------------------------|
| [Qt]          | >= 5.5.0    | concurrent, core, gui, network, opengl, svg, widget, xml |
| [GCC]/[MinGW] | >= 4.8      | C++11 enabled                                            |
| [toxcore]     | >= 0.2.10   | core, av                                                 |
| [FFmpeg]      | >= 2.6.0    | avformat, avdevice, avcodec, avutil, swscale             |
| [CMake]       | >= 2.8.11   |                                                          |
| [OpenAL Soft] | >= 1.16.0   |                                                          |
| [qrencode]    | >= 3.0.3    |                                                          |
| [sqlcipher]   | >= 3.2.0    |                                                          |
| [pkg-config]  | >= 0.28     |                                                          |
| [filteraudio] | >= 0.0.1    | optional dependency                                      |

## نیازمندیهای اختیاری

این نیازمندی‌ها اختیاری هستند و میتوان آنها را در زمان کامپایل کردن فعال یا غیر فعال کرد. فعال کردن یا غیر فعال کردن آنها را میتوان با دادن دستورات لازم به `cmake` در زمان ساخت qTox انجام داد.

اگر فایلهای این نیازمندی ها وجود نداشته باشند، qTox بدون قابلیت های مربوطه ساخته خواهد شد.

### نیازمندیهای توسعه

این فایلها و برنامه ها برای انجام تست و ویرایش کد و سایر کارهای مربوط به توسعه برنامه مورد نیاز هستند. اگر این فایل ها موجود نباشند این قابلیت ها وجود نخواهند داشت.

| نام    | نسخه |
|---------|---------|
| [Check] | >= 0.9  |

### لینوکس

#### پشتیبانی از Auto-way

| نام            | نسخه  |
|-----------------|----------|
| [libXScrnSaver] | >= 1.2   |
| [libX11]        | >= 1.6.0 |

اگر پیشنیازهای مطرح شده در جدول فوق موجود نباشند. این قابلیت ارایه نخواهد شد.

#### آیکون برنامه در KDE / آیکون کنار ساعت در GTK

| نام       | نسخه |
|-------------|---------|
| [Atk]       | >= 2.14 |
| [Cairo]     |         |
| [GdkPixbuf] | >= 2.31 |
| [GLib]      | >= 2.0  |
| [GTK+]      | >= 2.0  |
| [Pango]     | >= 1.18 |

برای غیرفعال کردن:

<div dir=ltr>

`-DENABLE_STATUSNOTIFIER=False -DENABLE_GTK_SYSTRAY=False`

<div dir=rtl>

####   آیکن کنار ساعت در محیط Unity

به شکل پیش فرض غیر فعال میباشد.

| نام              | نسخه   |
|-------------------|-----------|
| [Atk]             | >= 2.14   |
| [Cairo]           |           |
| [DBus Menu]       | >= 0.6    |
| [GdkPixbuf]       | >= 2.31   |
| [GLib]            | >= 2.0    |
| [GTK+]            | >= 2.0    |
| [libappindicator] | >= 0.4.92 |
| [Pango]           | >= 1.18   |

برای فعال سازی:

<div dir=ltr>

`-DENABLE_APPINDICATOR=True`

<div dir=rtl>

## لینوکس
### نصب آسان

نصب آسان qTox برای ویرایش های مختلفی از لینوکس ارایه شده است:


* [آرچ لینوکس](#آرچ-آسان)
* [فدورا](#فدورا-آسان)
* [جنتوو](#جنتو-آسان)
* [اسلکوار](#اسلکوار-آسان)
* [فری بی اس دی](#فری-بی-اس-دی-آسان)

---

<a name="آرچ-آسان" />

#### آرچ لینوکس

PKGBUILD در مخزن `community` موجود است، برای نصب کافی است از این دستور استفاده شود:

<div dir=ltr>

```bash
pacman -S qtox
```

<div dir=rtl>

<a name="فدورا-آسان" />

#### فدورا

qTox در مخزن [RPMFusion](https://rpmfusion.org/) آماده دانلود است، برای نصب کافی است:

<div dir=ltr>

```bash
dnf install qtox
```

<div dir=rtl>


<a name="جنتو-آسان" />

#### جنتوو

qTox در جنتوو آماده دانلود است.

برای نصب آن دستور زیر را اجرا کنید:

<div dir=ltr>

```bash
emerge qtox
```

<div dir=rtl>



<a name="اسلکوار-آسان" />

#### اسلکوار

ساخت SlackBuild برنامه qTox را میتوان در آدرس زیر یافت:

<div dir=ltr>

http://slackbuilds.org/repository/14.2/network/qTox/

<div dir=rtl>


<a name="فری-بی-اس-دی-آسان" />

#### فری بی اس دی

qTox به صورت بسته های باینری آماده قابل دانلود است. برای نصب میتوان از دستور زیر استفاده کرد:

<div dir=ltr>

```bash
pkg install qTox
```

<div dir=rtl>

پورت qTox همچنین در ``net-im/qTox`` قابل دسترسی است. برای ساخت و نصب qTox از روی سورس کد برنامه با استفاده از پورت میتوان از دستورات زیر استفاده کرد:


<div dir=ltr>

```bash
cd /usr/ports/net-im/qTox
make install clean
```

<div dir=rtl>


----

اگر توزیع مورد نظر شما لیست نشده است، یا میخواهید/نیاز دارید که qTox را کامپایل نمایید، در اینجا راهنمای لازم برای این کار ارایه میشود.

----

قسمت عمده پکیج ها و فایل های پیشنیاز در اکثر منابع مدیریت بسته سیستم های عامل موجود هستند. شما میتوانید یا راهنمای زیر را دنبال نمایید یا تنها فایل <div dir=ltr> `./simple_make.sh`<div dir=rtl> را بعد از دانلود و کپی کردن منبع اجرا نمایید. که این فایل به شکل خودکار پیشنیاز های مورد نظر را دانلود و به کامپایل کردن برنامه اقدام میکند.

<a name="نصب-Git" />

### نصب کردن Git

برای کپی کردن کد منبع برنامه به ابزار Git نیاز است.



<a name="گیت-آرچ" />

#### آرچ

<div dir=ltr>

```bash
sudo pacman -S --needed git
```

<div dir=rtl>


<a name="گیت-دبیان" />

#### دبیان

<div dir=ltr>

```bash
sudo apt-get install git
```

<div dir=rtl>

<a name="فدورا-گیت" />

#### فدورا


<div dir=ltr>

```bash
sudo dnf install git
```

<div dir=rtl>

<a name="اوپن‌سوزه-گیت" />

#### اوپن‌سوزه


<div dir=ltr>

```bash
sudo zypper install git
```

<div dir=rtl>

<a name="اوبونتو-گیت" />

#### اوبونتو


<div dir=ltr>

```bash
sudo apt-get install git
```

<div dir=rtl>



<a name="کپی-کیوتاکس" />

### کپی کردن qTox

در مرحله بعد از نصب Git یک ترمینال باز کرده و به پوشه دلخواه خود تغیر مسیر دهید. در این پوشه اقدام به بارگیری و کپی کردن کد منبع qTox نمایید:

<div dir=ltr>

```bash
cd /home/$USER/qTox
git clone https://github.com/qTox/qTox.git qTox
```

<div dir=rtl>

اقدامات بعدی چنین فرض میکنند که شما کد منبع را در مسیر <div dir=ltr>`/home/$USER/qTox`<div dir=rtl> کپی کرده اید. اگر کد را در مسیر دیگری کپی نموده اید، دستورات زیر را به شکل مناسب تغییر دهید.



<a name="سایر-نیازمندیها" />

### GCC, Qt, FFmpeg, OpenAL Soft و qrencode

<a name="سایر-نیاز-آرچ" />

#### آرچ

<div dir=ltr>

```bash
sudo pacman -S --needed base-devel qt5 openal libxss qrencode ffmpeg
```

<div dir=rtl>


<a name="سایر-نیاز-دبیان" />

#### دبیان

**نکته: تنها دبیان نسخه های بالاتر از 9 پایدار (توزیع stretch) پشتیبانی میشوند**


<div dir=ltr>

```bash
sudo apt-get install \
    build-essential \
    cmake \
    ffmpeg \
    libavcodec-dev \
    libexif-dev \
    libgdk-pixbuf2.0-dev \
    libgtk2.0-dev \
    libopenal-dev \
    libqrencode-dev \
    libqt5opengl5-dev \
    libqt5svg5-dev \
    libsqlcipher-dev \
    libxss-dev \
    pkg-config \
    qrencode \
    qt5-default \
    qttools5-dev \
    qttools5-dev-tools \
    yasm
```

<div dir=rtl>

<a name="سایر-نیزا-فدورا" />

#### فدورا

**توجه داشته باشید که sqlcipher هنوز در همه ویرایش های فدورا موجود نیست**
در زمان نگارش این قسمت (نوامبر 2016)، فدورا 25 با sqlcipher ارایه میشود، اما فدورا 24 و ویرایش های قدیمی تر هنوز این کتابخانه را ندارند.
**این بدان معنی است که اگر نمیتوان sqlcipher را از منابع فدورا دانلود کرد و نصب نمود، میبایست به شکل جداگانه این کتابخانه دانلود و نصب شود.**

<div dir=ltr>

```bash
sudo dnf groupinstall "Development Tools" "C Development Tools and Libraries"
# (can also use sudo dnf install @"Development Tools")
sudo dnf install \
    ffmpeg-devel \
    gtk2-devel \
    libexif-devel \
    libXScrnSaver-devel \
    libtool \
    openal-soft-devel \
    openssl-devel \
    qrencode-devel \
    qt-creator \
    qt-devel \
    qt-doc \
    qt5-linguist \
    qt5-qtsvg \
    qt5-qtsvg-devel \
    qtsingleapplication \
    sqlcipher \
    sqlcipher-devel
```

<div dir=rtl>

**در صورت نیاز برای کامپایل به قسمت [sqlcipher](#اس-کیو-ال) مراجعه شود.**

<a name="سایر-نیاز-اوپن‌سوزه" />

#### اوپن‌سوزه

<div dir=ltr>

```bash
sudo zypper install \
    libexif-devel \
    libQt5Concurrent-devel \
    libQt5Network-devel \
    libQt5OpenGL-devel \
    libQt5Xml-devel \
    libXScrnSaver-devel \
    libffmpeg-devel \
    libqt5-linguist \
    libqt5-qtbase-common-devel \
    libqt5-qtsvg-devel \
    openal-soft-devel \
    patterns-openSUSE-devel_basis \
    qrencode-devel \
    sqlcipher-devel
```
<div dir=rtl>

<a name="سایر-نیازها-اسلکوار" />

#### اسلک وار

لیست تمام پیش نیاز های qTox و SlackBuilds مربوط به آنها را میتوان در آدرس زیر مشاهده نمود:
<div dir=ltr>

http://slackbuilds.org/repository/14.2/network/qTox/

<div dir=rtl>


<a name="سایر-نیاز-اوبونتو" />

#### اوبونتو نسخه بالاتر یا برابر 15.04

<div dir=ltr>

```bash
sudo apt-get install \
    build-essential cmake \
    libavcodec-ffmpeg-dev \
    libavdevice-ffmpeg-dev \
    libavfilter-ffmpeg-dev \
    libavutil-ffmpeg-dev \
    libexif-dev \
    libgdk-pixbuf2.0-dev \
    libglib2.0-dev \
    libgtk2.0-dev \
    libopenal-dev \
    libqrencode-dev \
    libqt5opengl5-dev \
    libqt5svg5-dev \
    libsqlcipher-dev \
    libswresample-ffmpeg-dev \
    libswscale-ffmpeg-dev \
    libxss-dev \
    qrencode \
    qt5-default \
    qttools5-dev-tools
```
<div dir=rtl>


<a name="سایر-نیاز-اوبونتو16" />

#### اوبونتو نسخه بالاتر یا برابر با 16.04

<div dir=ltr>

```bash
sudo apt-get install \
    build-essential \
    cmake \
    libavcodec-dev \
    libavdevice-dev \
    libavfilter-dev \
    libavutil-dev \
    libexif-dev \
    libgdk-pixbuf2.0-dev \
    libglib2.0-dev \
    libgtk2.0-dev \
    libopenal-dev \
    libqrencode-dev \
    libqt5opengl5-dev \
    libqt5svg5-dev \
    libsqlcipher-dev \
    libswresample-dev \
    libswscale-dev \
    libxss-dev \
    qrencode \
    qt5-default \
    qttools5-dev-tools \
    qttools5-dev
```

<div dir=rtl>

<a name="نیاز-تاکس-کر" />

### نیازمندی های toxcore

نصب تمامی پیش نیاز های toxcore.


<a name="آرچ-تاکس-کر" />

#### آرچ

<div dir=ltr>

```bash
sudo pacman -S --needed opus libvpx libsodium
```

<div dir=rtl>

<a name="دبیان-تاکس-کر" />

#### دبیان

<div dir=ltr>

```bash
sudo apt-get install libtool autotools-dev automake checkinstall check \
libopus-dev libvpx-dev libsodium-dev libavdevice-dev
```

<div dir=rtl>


<a name="فدورا-تاکس-کر" />

#### فدورا

<div dir=ltr>

```bash
sudo dnf install libtool autoconf automake check check-devel libsodium-devel \
opus-devel libvpx-devel
```

<div dir=rtl>

<a name="اوپن‌سوزه-تاکس-کر" />

#### اوپن‌سوزه

<div dir=ltr>

```bash
sudo zypper install libsodium-devel libvpx-devel libopus-devel \
patterns-openSUSE-devel_basis
```

<div dir=rtl>

<a name="اسلک-وار-تاکس-کر" />

#### اسلک وار

لیست تمامی پیش نیازهای toxcore و SlackBuilds آنها را میتوان در آدرس زیر مشاهده نمود:

<div dir=ltr>
http://slackbuilds.org/repository/14.2/network/toxcore/
<div dir=rtl>

<a name="اوبونتو-تاکس-کر" />

#### اوبونتو نسخه بالاتر یا برابر 15.04

<div dir=ltr>

```bash
sudo apt-get install libtool autotools-dev automake checkinstall check \
libopus-dev libvpx-dev libsodium-dev
```

<div dir=rtl>

<a name="اس-کیو-ال" />

### sqlcipher

اگر از یک نسخه قدیمی فدورا استفاده نمیکنید، این قسمت را رها کرده، و مستقیما به
[**toxcore**](#کامپایل-تاکس-کر) مراجعه کنید.

<div dir=ltr>

```bash
git clone https://github.com/sqlcipher/sqlcipher
cd sqlcipher
./configure --enable-tempstore=yes CFLAGS="-DSQLITE_HAS_CODEC" \
    LDFLAGS="-lcrypto"
make
sudo make install
cd ..
```

<div dir=rtl>

<a name="کامپایل-تاکس-کر" />

### کامپایل toxcore

اگر تمامی پیش نیاز های مورد نیاز را نصب داشته باشید، میتوانید به راحتی این دستورات را اجرا کنید و toxcore را کامپایل کنید:

<div dir=ltr>

```bash
git clone https://github.com/toktok/c-toxcore.git toxcore
cd toxcore
git checkout v0.2.12
autoreconf -if
./configure
make -j$(nproc)
sudo make install
echo '/usr/local/lib/' | sudo tee -a /etc/ld.so.conf.d/locallib.conf
sudo ldconfig
```

<div dir=rtl>


<a name="کامپایل-کیو-تاکس" />

### کامپایل qTox

**مطمئن شوید که تمامی پیش نیاز ها را نصب کرده اید**

اگر در حین کامپایل با مشکلی مواجه شوید بدون تردید پیش نیازی را نصب ندارید. بنابراین مطمئن شوید که *همه آنها را نصب کرده اید*.

اگر دارید qTox را در فدورا 25 کامپایل میکنید، باید `PKG_CONFIG_PATH` را به متغیر environment به شکل دستی اضفاه کنید:

<div dir=ltr>

```
export PKG_CONFIG_PATH="$PKG_CONFIG_PATH:/usr/local/lib/pkgconfig"
```

<div dir=rtl>

سپس این دستورات را در پوشه qTox اجرا کنید تا برنامه کامپایل شود:

<div dir=ltr>

```bash
cmake .
make
```

<div dir=rtl>

بعد از اتمام کامپایل میتوان qTox را با استفاده از فرمان <div dir=ltr>`./qtox`<div dir=rtl> اجرا نمود.

ضمن عرض تبریک، شما موفق به کامپایل qTox شده اید `:)`



#### دبیان / ابونتو / مینت

اگر پروسه کامپایل به خاطر عدم وجود یک پیش نیاز متوقف میشود، مانند `... libswscale/swscale.h missing` این دستور را امتحان کنید:

<div dir=ltr>

```bash
apt-file search libswscale/swscale.h
```
<div dir=rtl>

سپس بسته هایی را نصب کنید که فایل های مورد نیاز را فراهم میکنند. دوباره فرمان make را اجرا کنید، در صورت نیاز دوباره این مراحل را تکرار کنید. اگر برایتان امکان پذیر است لیستی از فایلهای مورد نیاز تهیه کنید و برای ما ارسال نمایید تا بتوانیم مشخص کنیم که چه فایلهایی به شکل معمول مورد نیاز کاربران است `;)`

---

### ساخت بسته ها

در یک روش دیگر، qTox به شکل آزمایشی و شاید غیر قابل اطمینان میتواند خود را بسته بندی کند.
(منظور بسته بندی به فرمت `.deb` به شکل خودکار، و بسته بندی به فرمت `.rpm` با استفاده از [alien](http://joeyh.name/code/alien/) است).

بعد از نصب پیش نیاز های مورد نیاز، `bootstrap.sh` را اجرا کنید، و سپس اسکریپت `buildPackages.sh` را اجرا نمایید، که میتوان آن را در پوشه tools پیدا نمود. اجرای این دستور به شکل خودکار بسته های مورد نیاز برای ساخت `.deb` را دانلود و نصب میکند، پس برای تایپ پسورد sudo آماده باشد.



<a name="او-اس-ایکس" />

## OS X

OS X ویرایشهای بالاتر از نسخه 10.8 پشتیبانی میشوند.

کامپایل qTox روی OS X  برای توسعه به سه ابزار زیر نیاز دارد:

[Xcode](https://developer.apple.com/xcode/),
[Qt 5.4+](https://www.qt.io/qt5-4/) and [homebrew](https://brew.sh).

### اسکریپت خودکار

حالا میتوانید به شکل خودکار سیستم OS X خود را برای کامپایل qTox با استفاده از اسکریپت <div dir=ltr>`./osx/qTox-Mac-Deployer-ULTIMATE.sh`<div dir=rtl> آماده کنید.

میتوان این اسکریپت را به شکل مجزای از منبع qTox اجرا نمود، و این اسکریپت همه چیزی است که برای ساخت و کامپایل روی OS X مورد نیاز است.

برای استفاده از این اسکریپت ابتدا ترمینال را از مسیر زیر اجرا کنید:
<div dir=ltr>`Applications > Utilities > Terminal.app`<div dir=rtl>

در صورتی که میخواهید بیشتر یاد بگیرید میتوانید این دستور را اجرا کنید:
<div dir=ltr>`./qTox-Mac-Deployer-ULTIMATE.sh -h`<div dir=rtl>

توجه داشته باشید که این اسکریپت همه تغیراتی که ذخیره نشده باشند را در منبع qTox به شکل اولیه بر مگرداند و این اتفاق در مرحله `update` رخ میدهد.


#### اجرای اولیه / نصب

اگر برای بار اول است که اسکریپت را اجرا میکنید میبایست مطمئن شوید که سیستم شما آماده است. برای این کار میتوانید به سادگی دستور <div dir=ltr>`./qTox-Mac-Deployer-ULTIMATE.sh -i`<div dir=rtl> را اجرا کنید تا شما را در مسیر آماده سازی راهنمایی کند.

بعد از نصب اولیه حالا شما میتوانید qTox را از روی کد برنامه بسازید. که این کار با اجرای : <div dir=ltr>`./qTox-Mac-Deployer-ULTIMATE.sh -b`<div dir=rtl> قابل انجام است.

اگر خطایی رخ ندهد، آنوقت شما یک نسخه قابل اجرا از برنامه qTox در پوشه خانه خود (home) در زیرشاخه <div dir=ltr>`~/qTox-Mac_Build`<div dir=rtl> دارید.

#### به روز رسانی

اگر به منظور آزمایش یا اجرای آخرین ویرایش برنامه نیاز به به روز رسانی برنامه دارید میتوانید <div dir=ltr>`./qTox-Mac-Deployer-ULTIMATE.sh -u`<div dir=rtl> را اجرا کنید و به فرامین ارایه شده عمل کنید.
(توجه داشته باشید که اگر میدانید که قبل از اجرای این دستور منابع را به روز رسانی کرده اید Y را وارد کنید).
سپس <div dir=ltr>`./qTox-Mac-Deployer-ULTIMATE.sh -b`<div dir=rtl> را اجرا کنید تا بار دیگر برنامه ساخته شود. (توجه داشته باشید که این کار ساخت های قبلی را پاک کرده و جایگزین میکند)


#### گسترش و توزیع

سیستم عامل OS X به منظور اضافه کردن قابلیت به اشتراک گذاری در فایل <div dir=ltr>`qTox.app`<div dir=rtl> با سیستم هایی که کتابخانه های لازم را ندارند به یک مرحله دیگر نیاز دارد.

اگر میخواهید برنامه ای را که کامپایل کرده اید با سایر دوستانتان که OS X دارند به اشتراک بگذارید قبل از این کار، دستور <div dir=ltr>`./qTox-Mac-Deployer-ULTIMATE.sh -d`<div dir=rtl> را اجرا کنید.



### کامپایل دستی
#### کتابخانه های مورد نیاز

homebrew را نصب کنید اگر آن را ندارید:

<div dir=ltr>

```bash
ruby -e "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install)"
```

<div dir=rtl>

در مرحله اول، تمامی پیش نیاز های موجود در `brew` را نصب کنید.

<div dir=ltr>

```bash
brew install git ffmpeg qrencode libtool automake autoconf check qt5 libvpx \
opus sqlcipher libsodium
```
<div dir=rtl>

در مرحله بعد [toxcore](https://github.com/toktok/c-toxcore/blob/master/INSTALL.md#osx)را نصب کنید

سپس qTox را کپی کنید:

<div dir=ltr>

```bash
git clone https://github.com/qTox/qTox
```
<div dir=rtl>

از این به بعد،
در مرحله آخر، همه فایلهای مورد نیاز را کپی کنید. هر بار که بسته های brew را آپدیت میکنید، میتوانید تمامی گام های بالا را انجام ندهید، و تنها دستورات زیر را اجرا کنید:

<div dir=ltr>

```bash
cd ./git/qTox
sudo bash bootstrap-osx.sh
```

<div dir=rtl>

#### کامپایل کردن

شما میتوانید qTox را با استفاده از Qt Creator بسازید.
[seperate download](http://www.qt.io/download-open-source/#section-6)
یا آن را به شکل دستی با استفاده از cmake ایجاد نمایید.

اگر بخواهید از cmake استفاده کنید، میتوانید qTox را در پوشه git کامپایل کنید:

<div dir=ltr>

```bash
cmake .
make
```

<div dir=rtl>

یا یک راه تمیز تر میتواند این باشد:

<div dir=ltr>

```bash
cd ./git/dir/qTox
mkdir ./build
cd build
cmake ..
```

<div dir=rtl>

#### گسترش و توزیع


اگر qTox را به شکل صحیح کامپایل کرده باشید، حالا میتوانید `qTox.app` ایجاد شده را با دیگران در اشتراک بگذارید.

با استفاده از qt5 homebrew نصب شده در پوشه build:

<div dir=ltr>

```bash
/usr/local/Cellar/qt5/5.5.1_2/bin/macdeployqt ./qTox.app
```

<div dir=rtl>


#### اجرای qTox

شما دو انتخاب دارید، یا روی qTox کلیک کنید که سریعا خارج میشود، یا دستور زیر را اجرا نمایید:

<div dir=ltr>

```bash
qtox.app/Contents/MacOS/qtox
```

<div dir=rtl>

میتوانید از محیط CLI ای که ایجاد کرده اید لذت ببرید و احتمال زیاد دوستان و خانواده شما فکر میکنند که شما یک هکر شده اید.

<a name="ویندوز" />

## ویندوز

<a name="کامپایل-برای-سایر-سیتمها" />

### کامپایل برای سایر سیستم عامل ها در لینوکس

مراجعه کنید به [`windows/cross-compile`](windows/cross-compile)

<a name="نیتیو" />

### کامپایل در ویندوز

#### Qt

در ابتدا فایل نصب Qt را از [qt.io](https://www.qt.io/download-open-source/) دانلود کنید. در طول نصب میباید toolchian مربوط به Qt را اسمبل کنید. میتوانید آخرین نسخه Qt را با استفاده از MinGW کامپایل کنید. هرچند که خود فایل نصب یک کامپایلر MinGW ارایه میکند، اما توصیه میشود که آن را به شکل جداگانه نصب کرد، چرا که Qt فاقد MSYS است که برای کامپایل و نصب OpenAL مورد نیاز است. به همین جهت میتوانید در صورت نیاز سربرگ `Tools` را غیر فعال کنید. گام های بعدی این چنین فرض میکنند که Qt در `C:\Qt` نصب شده است. اگر Qt را در مسیر دیگری نصب میکنید، دستورات زیر را به شکل مناسب تغیر دهید.

#### MinGW

فایل نصب MinGW را برای ویندوز از [sourceforge.net](http://sourceforge.net/projects/mingw/files/Installer/) دانلود و نصب کنید. مطمئن شوید که MSYS را انتخاب و نصب میکنید. MSYS مجموعه ای از ابزار های Unix برای ویندوز است. گام های بعدی در نظر میگیرند که MinGW در `C:\MinGW` نصب شده است. اگر مسیر نصب متفاوت است دستورات زیر را به شکل مناسب تغییر دهید. در برنامه نصب MinGW (mingw-get.exe) بسته های `mingw-developer-toolkit`, `mingw32-base`, `mingw32-gcc-g++`, `msys-base` و `mingw32-pthreads-w32` را انتخاب کنید. دقت داشته باشید که نسخه MinGW با ورژن و نسخه Qt سازگاری داشته باشد.

#### Wget

فایل نصب Wget برای ویندوز را از http://gnuwin32.sourceforge.net/packages/wget.htm دانلود و نصب کنید. این فایل ها را نصب کنید. گام های بعدی چنین فرض میکنند که Wget در مسیر <div dir=ltr>`C:\Program Files (x86)\GnuWin32\`<div dir=rtl> نصب شده است. اگر تصمیم دارید این فایل را در مسیر دیگری نصب کنید، دستورات زیر را به شکل مناسب تغییر دهید.


#### UnZip

فایل نصب UnZip برای ویندوز را از http://gnuwin32.sourceforge.net/packages/unzip.htm دانلود و آن را نصب کنید. گام هایی که در ادامه میآیند چنین فرض میکنند که UnZip در <div dir=ltr>`C:\Program Files (x86)\GnuWin32\`<div dir=rtl> نصب شده است. در صورتی که مسیر دیگری را انتخاب کرده اید دستورات را به شکل مناسب ویرایش نمایید.

#### تغییر PATH سیستم

فایل های اجرایی <div dir=ltr>MinGW/MSYS/CMake<div dir=rtl> را به متغیر PATH سیستم اضافه کنید که بتوان به شکل سراسری به آنها دسترسی داشت. به مسیر زیر بروید:

<div dir=ltr>

`Control Panel` -> `System and Security` -> `System` -> `Advanced system settings` -> `Environment Variables...`

<div dir=rtl>

یا دستور `sysdm.cpl` را اجرا کنید، سر برگ `Advanced system settings` را انتخاب کنید و روی دکمه `Environment Variables` کلیک کنید. در جعبه دوم (پایینی) به دنبال متغیر `PATH` بگردید و روی دکمه `Edit...` کلیک کنید. جعبه ورودی `Variable value:` به احتمال زیاد دارای چندین پوشه از قبل میباشد. هر پوشه با استفاده از نقطه ویرگول (;) جدا شده است. این جعبه ورودی را طوری تغییر دهید که پوشه های زیر را نیز شامل شود: <div dir=ltr>`;C:\MinGW\bin;C:\MinGW\msys\1.0\bin;C:\Program Files (x86)\CMake 2.8\bin;C:\Program Files (x86)\GnuWin32\bin`<div dir=rtl>. به احتمال زیاد پوشه CMake به شکل خودکار به مسیر اضافه شده است. توجه داشته باشید که مسیر هایی که دارای برنامه های `sh` و `bash` هستند، مانند مسیر <div dir=ltr>`C:\Program Files\OpenSSH\bin`<div dir=rtl> در آخر مسیر متغیر `PATH` قرار داشته باشند، در غیر اینصورت کامپایل انجام نخواهد شد.

#### کپی کردن منبع

کد منبع (https://github.com/qTox/qTox.git) را با استفاده از برنامه Git مورد نظر خود کپی کنید. برنامه های [SmartGit](http://www.syntevo.com/smartgit/) و [TorteiseGit](https://tortoisegit.org) هردو برای انجام این کار برنامه های خوبی هستند (توجه داشته باشید که شاید لازم باشد `git.exe` را به `PATH` اضافه کنید). گام های بعدی چنین فرض میکنند که کد ها را در مسیر `C:\qTox` کپی کرده اید. اگر تصمیم گرفته اید که مسیر دیگری را انتخاب کنید، دستورات را اصلاح نمایید.

#### دریافت پیش نیاز ها

اسکریپت `bootstrap.bat` که در مسیر `C:\qTox` قرار دارد را اجرا کنید. این اسکریپت به شکل خودکار پیش نیاز های لازم را دانلود خواهد کرد، آنها را کامپایل و در مسیر های لازم کپی خواهد نمود.

توجه داشته باشید که برخی از ابزار های ویروسیابی برخی از کتابخانه ها را به اشتباه ویروس شناسایی میکنند. اگر با این مشکل مواجه میشوید، به منظور کسب اطلاعات بیشتر به صفحه [problematic antiviruses](https://github.com/qTox/qTox/wiki/Problematic-antiviruses) مراجعه کنید.

<a name="سویچهای-کامپایل" />

## سویچ های زمان کامپایل

این سویچ ها به عنوان مولفه به دستور `cmake` اضافه میشوند. به عنوان مثال یک سویچ `SWITCH` که مقدار `YES` به آن اختصاص داده میشود، این مقدار به `cmake` منتقل میشود. و این اتفاق به شکل زیر انجام میشود:

<div dir=ltr>

```bash
cmake -DSWITCH=yes
```

<div dir=rtl>

سویچها:

- `SMILEYS`, مقادیر معنی دار:
  - اگر تعریف نشود یا میزان معنی داری به آن اختصاص پیدا نکند، همه شکلک ها اضافه خواهند شد
  - `DISABLED` – هیچ بسته شکلکی را اضافه نکن، تنها بسته های کاربر اضافه میشوند
  - `MIN` – تنها یک بسته شکلک ها اضافه شود


[Atk]: https://wiki.gnome.org/Accessibility
[Cairo]: https://www.cairographics.org/
[Check]: https://libcheck.github.io/check/
[CMake]: https://cmake.org/
[DBus Menu]: https://launchpad.net/libdbusmenu
[FFmpeg]: https://www.ffmpeg.org/
[GCC]: https://gcc.gnu.org/
[GdkPixbuf]: https://developer.gnome.org/gdk-pixbuf/
[GLib]: https://wiki.gnome.org/Projects/GLib
[GTK+]: https://www.gtk.org/
[libappindicator]: https://launchpad.net/libappindicator
[libX11]: https://www.x.org/wiki/
[libXScrnSaver]: https://www.x.org/wiki/Releases/ModuleVersions/
[MinGW]: http://www.mingw.org/
[OpenAL Soft]: http://kcat.strangesoft.net/openal.html
[Pango]: http://www.pango.org/
[pkg-config]: https://www.freedesktop.org/wiki/Software/pkg-config/
[qrencode]: https://fukuchi.org/works/qrencode/
[Qt]: https://www.qt.io/
[sqlcipher]: https://www.zetetic.net/sqlcipher/
[toxcore]: https://github.com/TokTok/c-toxcore/
[filteraudio]: https://github.com/irungentoo/filter_audio
