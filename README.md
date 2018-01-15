# tplink-raw-wifi

Test using tp-links to do raw wifi.


This looks interesting:https://sandilands.info/sgordon/capturing-wifi-in-monitor-mode-with-iw

To build the other half of this, for the ESP8266, please look here: https://github.com/cnlohr/MinEspSDKLib

Making build environment:

On the host system:

```# sudo apt-get install build-essential subversion libncurses5-dev zlib1g-dev gawk gcc-multilib flex git-core gettext libssl-dev```

Then somewhere unrelated...

```git clone --depth=1 https://git.lede-project.org/source.git```

```make```

And select your processor and wait.

Original attempt (failed because my GCC kept trying to link against libc)

Some combination of this works.  Mipsel is needed for little endian stuff.
```# sudo apt-get install gcc-mips-linux-gnu linux-libc-dev-mips-cross libc6-dev-mipsel-cross libc6-dev-mips-cross gcc-mipsel-linux-gnu gcc-multilib-mips-linux-gnu```

