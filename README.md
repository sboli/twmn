twmn
====
A notification system for tiling window managers. `twmn` is two things:

`twmnc`: command line tool to send notifications to `twmnd`. You can also use `notify-send` for a similar purpose, but `twmnc` is more powerful. See `twmnc --help` for more information.

`twmnd`: daemon listening to notification requests and showing them one after another. Configure it at `~/.config/twmn/twmn.conf`. The file is generated the first time `twmnd` is launched.

Notifications are shown in a one-line bar called the notification slide. They can be navigated through and activated with shortcuts.

![](https://github.com/sboli/twmn/blob/master/screencast.gif)

See `twmn.conf` for more information.


About twmn.conf
---------------
<pre>
[gui]
; Screen number to display notifications on when using a multi-head desktop.
screen=  ; 0 indexed screen number

; WARNING: Deprecated by "screen"
; Absolute position from the top-left corner of the slide. You may need it for a multi-screen setup.
; You still have to set position in order to choose the slide animation. If empty, twmnd will try
; to figure out where to display the slide according to your desktop size and the slide position.
absolute_position=  ; Supported format: WxH. Width and Height being integers.

; Background color.
background_color=black  ; RBG hex and keywords (eg. lightgray) are supported.

; An icon for the layout. Useful only for a layout file.
icon=  ; Path to image file. Optional.

; Font family.
font=Sans

; Font size.
font_size=13  ; In pixel.

; Font variation.
; accepted values are:
; oblique, italic, ultra-light, light, medium, semi-bold, bold, ultra-bold, heavy, ultra-condensed,
; extra-condensed, condensed, semi-condensed, semi-expanded, expanded, extra-expanded, ultra-expanded.
font_variant=medium

; Text color.
foreground_color=white  ; RBG hex and keywords (eg. lightgray) are supported.

; Height of the slide bar. Useful to match your tiling window manager's bar.
height=18  ; In pixel.

; Position of the notification slide.
position=top_right  ; Accepted values: top_right (tr), top_left (tl), bottom_right (br),
                    ; bottom_left (bl), top_center (tc), bottom_center (bc), center (c).

; moves the position of the slide in +/- pixels on the x or y axis (e.g. "+50" or "-100")
offset_x=+0 ; default is 0
offset_y=+0 ; default is 0

; The animation to use when the slide appear
in_animation=38 ; see https://doc.qt.io/qt-5/qeasingcurve.html#Type-enum for types

; The in animation's duration
in_animation_duration=1000 ; in milliseconds

; The animation to use whe the slide is closing
out_animation=13

; The out animation's duration
out_animation_duration=1000 ; in milliseconds

; Enable or disable notification bounce when changing notification
bounce=true  ; true or false

; Change bounce duration
bounce_duration=500 ; in milliseconds

; If the character length is more then max_length the text is cut off and "..." is appended
max_length = -1 ; default is -1 (which means: don't cut off)

[icons]
; An icon. You can add as many as you want.
NAME=  ; Path to image file. NAME being the icon's custom name.


[main]
; Program/command to be executed on notification activation.
activate_command=  ; Path to command.

; Time each notification remains visible.
duration=3000  ; In millisecond.

; Host address to listen on for notifications.
host=127.0.0.1

; UDP port used for notifications.
port=9797

; Program/command to play sound with.
sound_command=  ;  Path to command. Leave empty for no sound.

</pre>


Installation
------------

For [Arch Linux](http://www.archlinux.org/) users, `twmn-git` is [on the AUR](https://aur.archlinux.org/packages/twmn-git/).

Otherwise you can install `twmnd` and `twmnc` manually:

1. install `boost` and `qt` if they weren't before, including the `widgets` and `x11extras` qt libraries
2. `git clone https://github.com/sboli/twmn.git` to get `twmn`
3. `cd twmn/`
4. `qmake` to generate a Makefile
5. `make` to compile
6. `sudo make install` to install `twmnd` and `twmnc` in `/usr/local/bin`. Make sure this folder is in your `$PATH` environment variable. (`export PATH=$PATH:/usr/local/bin`)
7. for `twmnd` to be always running, add it to your `.xinitrc`, `rc.conf` or else

The `storage_notifier` example requires `dbus-python` to be installed. The `mpd_notifier` example requires `python-mpd` to be installed and running.
