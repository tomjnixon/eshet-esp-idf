{
  inputs.nixpkgs.url = "nixpkgs/nixos-23.11";
  inputs.flake-utils.url = "github:numtide/flake-utils";

  outputs = { self, nixpkgs, flake-utils }:
    flake-utils.lib.eachDefaultSystem (system:
      let
        pkgs = nixpkgs.legacyPackages.${system};
      in
      rec {
        packages.eshet-esp-idf = pkgs.stdenv.mkDerivation {
          name = "eshet-esp-idf";
          src = ./.;
          nativeBuildInputs = [ pkgs.cmake pkgs.ninja ];

          meta.mainProgram = "eshet_ota";
        };

        packages.default = packages.eshet-esp-idf;
      });
}
