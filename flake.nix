{
  description = "A simple Wayland key display application overlay";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
  };

  outputs = {
    self,
    nixpkgs,
  }: let
    systems = ["x86_64-linux"];
    forAllSystems = nixpkgs.lib.genAttrs systems;
  in {
    packages = forAllSystems (system: let
      pkgs = nixpkgs.legacyPackages.${system};
    in {
      stable = pkgs.callPackage ./nix/default.nix {isStable = true;};
      keypop = pkgs.callPackage ./nix/default.nix {};
      default = self.packages.${system}.keypop;
    });
  };
}
