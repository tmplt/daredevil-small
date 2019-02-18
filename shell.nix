{ pkgs ? import <nixpkgs> {} }:
with pkgs;

let
  jlink = stdenv.mkDerivation rec {
    name = "jlink-v${version}";
    version = "6.42c";

    src = fetchurl {
      url = "https://www.segger.com/downloads/jlink/JLink_Linux_V${stdenv.lib.replaceStrings ["."] [""] version}_x86_64.deb";
      sha256 = "1ladqgszyicjg01waasbn5b6fngv4ap3vcksxcv1smrgr9yv9bv4";
      curlOpts = "-d accept_license_agreement=accepted -d confirm=yes";
    };

    nativeBuildInputs = [ dpkg ];
    unpackCmd = "mkdir tmp && dpkg -x $curSrc $_";

    installPhase = ''
      cp -r . $out
    '';

    dontPatchELF = true;

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
    runScript = "${jlink}/opt/SEGGER/JLink_V6*/JLinkGDBServerCLExe";
  };

  rttClient = buildFHSUserEnv rec {
    name = "JLinkRTTClientExe";
    runScript = "${jlink}/opt/SEGGER/JLink_V6*/JLinkRTTClientExe";
  };

  JLinkExe = buildFHSUserEnv rec {
    name = "JLinkExe";
    runScript = "${jlink}/opt/SEGGER/JLink_V64*/JLinkExe";

    targetPkgs = pkgs: with pkgs; [
      udev
    ];
  };

in
  mkShell {
    buildInputs = [ gdbServer rttClient JLinkExe ];

    LD_LIBRARY_PATH="${stdenv.cc.cc.lib}/lib64:$LD_LIBRARY_PATH";
  }
