{pkgs, ...}: {
  packages = with pkgs; [
    pkg-config
    wayland-scanner
    wayland
    wayland-protocols
    cairo
    pango
    libinput
    libxkbcommon
    gtk3
    libappindicator-gtk3
    gnumake
    gcc
    gdb
    clang-tools # clangd LSP
  ];

  env = {
    PKG_CONFIG_PATH = with pkgs;
      lib.makeSearchPathOutput "dev" "lib/pkgconfig" [
        wayland
        wayland-protocols
        cairo
        pango
        libinput
        libxkbcommon
        gtk3
        libappindicator-gtk3
      ];

    WAYLAND_PROTOCOLS_DIR = "${pkgs.wayland-protocols}/share/wayland-protocols";
  };

  scripts.build.exec = ''
    sed -i "s|/usr/share/wayland-protocols|$WAYLAND_PROTOCOLS_DIR|g" Makefile
    make "$@"
  '';

  scripts.clean.exec = "make clean";

  enterShell = ''
    echo "keypop dev environment"
    echo "  build  → runs make (with Makefile patch)"
    echo "  clean  → runs make clean"
    gcc --version | head -1
  '';
}
