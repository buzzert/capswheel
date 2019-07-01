# Capswheel
Fakes scroll wheel events using mouse input, activated with a hotkey.

I wrote this for my [GPD Pocket](https://gpd.hk/gpdpocket2), which is a cool tiny computer that has a tiny touchpad but no ability to scroll (unless you use the touchscreen).

By default, you hold the (useless) caps lock key and move your finger along the touchpad to scroll. You might want to also disable the caps lock key in Xorg via:
```
setxkbmap -option caps:none
```

## Requirements
- `base-devel`
- `libpthread`
- [`interception-tools`](https://gitlab.com/interception/linux/tools)

## Installation
1. Run `make`
2. Configure which keyboard and mouse device to watch in `capswheel.service`. If you're running this on a GPD Pocket 2, the defaults should work for you. Check inside of `/dev/input/by-id` for the correct one for your machine.
3. `make install`
4. Start the service with `systemctl start capswheel`
5. Enable the service at boot with `systemctl enable capswheel`

