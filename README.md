# Keypop (Wayland)
A simple key display application for Wayland compositors.

![b1](./public/b1.png)

## Features
- Shows typed keys on screen in real-time
- Window auto-hides after 2 seconds of inactivity
- Supports special keys (Enter, Tab, Ctrl, Alt, etc.)
- Optimized for low memory usage

## Install

### Nix
Add keypop to your `inputs` in `flake.nix`:

```nix
inputs = {
  nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";

  keypop = {
    url = "github:justanoobcoder/keypop";
    inputs.nixpkgs.follows = "nixpkgs";
  };
};
```

Install keypop:
```nix
{
  pkgs,
  inputs,
  ...
}:{
  home.packages = with pkgs; [
    inputs.keypop.packages.${pkgs.stdenv.hostPlatform.system}.default
  ];
}
```

## Build
```bash
make
```

## Install
```bash
sudo make install
```

## Run
Requires access to input devices. Add your user to the `input` group:

```bash
sudo usermod -aG input $USER
# Log out and back in
```

Then run:

```bash
./keypop
```

Or run with custom options:
```bash
# E.g., Blue background, Red text, Size 80, 1000x200 window, 80% opacity
./keypop -b "#0000FF" -c "#FF0000" -s 80 -g 1000x200 -o 0.8
```

Options:
- `-b <color>`: Background color hex (e.g. `#000000` or `000000`)
- `-c <color>`: Text color hex (e.g. `#FFFFFF` or `FFFFFF`)
- `-s <size>`: Font size (default 65)
- `-g <WxH>`: Window geometry (default 840x130)
- `-o <opacity>`: Background opacity (0.0 - 1.0, default 0.6)
- `-t <time>`: Window hide time in milliseconds (0 = never hide, default: 2000)
- `-v`: Get version info
- `-h`: Show help

## Configuration
Keypop can be configured via config file. Create a file named `keypop.conf` in `$XDG_CONFIG_HOME/keypop/` and add the following:
```
[settings]
background=#000000
foreground=#FFFFFF
font_size=65
geometry=840x130
opacity=0.6
hide_timeout=2000
```

## Exit
- Press `Ctrl+C` in terminal
- Or kill the process: `pkill keypop`
- Or close via Hyprland: `hyprctl dispatch killactive` with window focused

## Dependencies
- wayland-client
- wayland-protocols
- cairo
- libinput
- libudev
- xkbcommon
