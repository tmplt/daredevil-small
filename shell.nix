{ pkgs ? import <nixpkgs> {} }:
with pkgs;

let
  jlink = stdenv.mkDerivation rec {
    name = "jlink-v${version}";
    version = "6.40b";

    src = fetchurl {
      url = "https://www.segger.com/downloads/jlink/JLink_Linux_x86_64.deb";
      sha256 = "0d0hs5bcff2il0i44hg2d7cs33p1kr8zqclpmi2a5p7cwbnaqmc8";
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
    runScript = "${jlink}/opt/SEGGER/JLink_V640b/JLinkGDBServerCLExe";

    targetPkgs = pkgs: with pkgs; [
      udev
    ];
  };

  rttClient = buildFHSUserEnv rec {
    name = "JLinkRTTClientExe";
    runScript = "${jlink}/opt/SEGGER/JLink_V640b/JLinkRTTClientExe";

    targetPkgs = pkgs: with pkgs; [
      udev
    ];
  };

  JLinkExe = buildFHSUserEnv rec {
    name = "JLinkExe";
    runScript = "${jlink}/opt/SEGGER/JLink_V640b/JLinkExe";

    targetPkgs = pkgs: with pkgs; [
      udev
    ];
  };

in
  mkShell {
    buildInputs = [ gdbServer rttClient JLinkExe ];

    LD_LIBRARY_PATH="${stdenv.cc.cc.lib}/lib64:$LD_LIBRARY_PATH";
  }
