with import <nixpkgs> {};
mkShell {
  buildInputs = [ (callPackage ./default.nix {}) ];
  LD_LIBRARY_PATH="${stdenv.cc.cc.lib}/lib64:$LD_LIBRARY_PATH";
}
