twmn
====
A notification system adapted to work with tiling window managers.

Notifications are shown in a one-line bar called the notification slide.

`twmnc` is a command line tool to send notification requests to `twmnd`. Even though twmnc is more powerful, you can still use notify-send to send your notifications. See twmnc --help for more information.


`twmnd` is the daemon listening to notification requests and showing them one after another.

Configuration is done in an INI config file usually located at ~/.config/twmn/twmn.conf. The file will be created at first launch.


Default twmn.conf explained
---------------------------
<pre>
[gui]
; Absolute position from the top-left corner of the slide. You may need it
; for a multi-screen setup. You still have to set position in order to choose
; the slide animation. If this value is empty, twmd will try to figure out where
; to display the slide according to your desktop size and the position value.
absolute_position=  ; Supported format: WxH. Width and Height being integers.

; Background color.
background_color=black  ; CSS-like keywords and RBG hex are supported (eg. lightgray).

; An icon for the layout. Useful only for a layout file.
icon=  ; Path to image file. Optional.

; Font family.
font=Sans

; Font size.
font_size=13  ; In pixel.

; Text color.
foreground_color=white  ; CSS-like keywords and RBG hex are supported (eg. lightgray).

; Height of the slide bar. Useful to match your tiling window manager's bar.
height=18  ; In pixel.

; Position of the notification slide.
position=top_right  ; Accepted values: top_right, top_left, bottom_right,
                    ; bottom_left, top_center, bottom_center, center.


[icons]
; An icon. You can add as many as you want.
NAME_icon=  ; Path to image file. NAME being the icon's custom name.


[main]
; Program/command executed on current notification activation.
activate_command=  ; Path to command.

; Time each notification remains visible.
duration=3000  ; In millisecond.

; Enable or disable shortcuts.
enable_shortcuts=  ; true or false.

; UDP port used for notifications.
port=9797

; Program/command to play sound with.
sound_command=  ;  Path to command. Leave empty for no sound.


[shortcuts]
; Modifiers shortcuts.
modifiers=Alt+  ; Up to three modifiers. Use with the following shortcuts.

; Shows the previous notification. Mouse wheel up does it too.
previous=K

; Shows the next notification.  If a modification is manually shown it will ne be displayed again
; when twmnd process the notification stack. Mouse wheel up does it too.
next=J

; Activates the notification. Runs the command defined at activate_command.
; Mouse left click activates it too.
activate=Return

; Hides all notifications. To hide the current notification, use the "next" key instead.
; Mouse Right click has the same effect.
hide=X
</pre>
