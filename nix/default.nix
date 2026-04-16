{
  lib,
  stdenv,
  pkg-config,
  wayland,
  wayland-scanner,
  wayland-protocols,
  cairo,
  pango,
  libinput,
  libxkbcommon,
  gtk3,
  libappindicator-gtk3,
}:
stdenv.mkDerivation rec {
  name = "keypop";
  version = "2.0";

  src = ../.;

  nativeBuildInputs = [
    pkg-config
    wayland-scanner
  ];

  buildInputs = [
    wayland
    wayland-protocols
    cairo
    pango
    libinput
    libxkbcommon
    gtk3
    libappindicator-gtk3
  ];

  patchPhase = ''
    substituteInPlace Makefile \
      --replace /usr/share/wayland-protocols \
                ${wayland-protocols}/share/wayland-protocols
  '';

  buildPhase = ''
    make
  '';

  installPhase = ''
    install -Dm755 keypop $out/bin/keypop
  '';

  meta = with lib; {
    description = "A simple Wayland key display application overlay";
    homepage = "https://github.com/justanoobcoder/keypop";
    license = licenses.mit;
    platforms = platforms.linux;
    mainProgram = "keypop";
  };
}
