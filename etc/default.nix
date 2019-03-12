# This file is a Nix port of the binary blobs for JLink.
# It is messy, unfinished and should not necessarily be relied upon.
# Only what is necessary for the toolchain of Daredevil (JLinkGDBServerCLExe,
# RTTClientExe) are confirmed to work. Remaining binaries are unlikely to be linked correctly.

{ stdenv, dpkg, buildFHSUserEnv, udev, fetchurl, buildEnv }:

let
  arch = {
    "x86_64-linux" = "x86_64";
    "i686-linux"   = "x86";
  }.${stdenv.system};

  jlinkSrc = stdenv.mkDerivation rec {
    name = "jlink-v${version}";
    version = "6.42c";
    version' = stdenv.lib.replaceStrings ["."] [""] version;

    src = fetchurl {
      url = "https://www.segger.com/downloads/jlink/JLink_Linux_V${version'}_${arch}.deb";
      # TODO: fix  sha256 for i686
      sha256 = "1ladqgszyicjg01waasbn5b6fngv4ap3vcksxcv1smrgr9yv9bv4";
      curlOpts = "-d accept_license_agreement=accepted -d confirm=yes";
    };

    nativeBuildInputs = [ dpkg ];
    unpackCmd = "mkdir tmp && dpkg -x $curSrc $_";

    installPhase = ''
      cp -r . $out
    '';

    dontPatchELF = true;

  };

  wrapJlink = name: deps: buildFHSUserEnv rec {
    inherit name;
    runScript = "${jlinkSrc}/opt/SEGGER/JLink_V*/${name}";
    targetPkgs = pkgs: deps;
  };

  flashLite = wrapJlink "JFlashLiteExe" [];
  flashSPI = wrapJlink "JFlashSPICLExe" [];
  jlink  = wrapJlink "JLinkExe" [ udev ];
  gdbServer = wrapJlink "JLinkGDBServerExe" [ udev ];
  gdbServerCL = wrapJlink "JLinkGDBServerCLExe" [ udev ];
  licenseManager = wrapJlink "JLinkLicenseManagerExe" [];
  remoteServer = wrapJlink "JLinkRemoteServerExe" [];
  remoteServerCL = wrapJlink "JLinkRemoteServerCLExe" [];
  rttClient = wrapJlink "JLinkRTTClientExe" [];
  rttLogger = wrapJlink "JLinkRTTLoggerExe" [];
  stm32 = wrapJlink "JLinkSTM32Exe" [];
  swoViewer = wrapJlink "JLinkSWOViewerCLExe" [];
  jtagLoad = wrapJlink "JTAGLoadExe" [];

in
  buildEnv rec {
    name = jlinkSrc.name;
    paths = [
      flashLite
      flashSPI
      jlink
      gdbServer
      gdbServerCL
      licenseManager
      remoteServer
      remoteServerCL
      rttClient
      rttLogger
      stm32
      swoViewer
      jtagLoad
    ];

    meta = with stdenv.lib; {
      description = "TODO";
      homepage = "https://www.segger.com/downloads/jlink/";
      license = licenses.unfree;
      platforms = platforms.linux;
      maintainers = with maintainers; [ tmplt ];
    };
  }
