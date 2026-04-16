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
  fetchFromGitHub,
  isStable ? false,
}:
stdenv.mkDerivation rec {
  pname = "keypop";
  version = "2.0.1";

  src =
    if isStable
    then
      fetchFromGitHub {
        owner = "justanoobcoder";
        repo = "keypop";
        rev = "v${version}";
        hash = "sha256-z5EOGy3FTMBn8HrVy9M5qwRc5fcZuQyiiphIGPX0/jI=";
      }
    else ../.;

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
      --replace-fail /usr/share/wayland-protocols \
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
