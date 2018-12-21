{ pkgs ? import <nixpkgs> {} }:
with pkgs;

let
  jlink = stdenv.mkDerivation rec {
    name = "jlink-v${version}";
    version = "6.40";

    src = fetchurl {
      url = "https://www.segger.com/downloads/jlink/JLink_Linux_x86_64.deb";
      sha256 = "eadf03c6a046efa69a10a9a30bd0c9a106f80034cd6906b51a2dddb3c54e5485";
      curlOpts = "-d accept_license_agreement=accepted -d confirm=yes";
    };

    nativeBuildInputs = [ dpkg ];
    unpackCmd = "mkdir tmp && dpkg -x $curSrc $_";

    installPhase = ''
      cp -r . $out
    '';

    meta = with stdenv.lib; {
      description = "TODO";
      homepage = "";
      license = licenses.unfree;
      platforms = platforms.linux;
      maintainers = with maintainers; [ tmplt ];
    };
  };

  # TODO: package all binaries in jlink
  gdbServer = buildFHSUserEnv rec {
    name = "JLinkGDBServerCLExe";
    runScript = "${jlink}/opt/SEGGER/JLink_V640/JLinkGDBServerCLExe";

    targetPkgs = pkgs: with pkgs; [
      udev
    ];
  };
in
  mkShell {
    buildInputs = [ gdbServer ];

    LD_LIBRARY_PATH="${stdenv.cc.cc.lib}/lib64:$LD_LIBRARY_PATH";
  }
