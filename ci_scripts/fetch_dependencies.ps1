<#
.SYNOPSIS
  Fetch prebuilt binary dependencies for the Windows build and stage them where
  the Visual Studio / CMake builds expect them.

.DESCRIPTION
  Single entry point for pulling large prebuilt binaries from their upstream
  release artifacts instead of committing them into the repo (big binary blobs
  bloat the git history). The Windows build drivers
  (build_scripts\msw\build_VS_x64*.cmd) run this before msbuild.

  Add a dependency by appending an entry to the $Dependencies catalogue below:
    Name     friendly name (also the value for -Only)
    Optional $true  -> a failed/unavailable fetch just disables that feature and
                       the build continues (the VS project only defines the
                       feature macro when the staged lib is present).
             $false -> a failed fetch stops the script with a clear message
                       instead of a confusing downstream link error. Use this
                       for libraries the build genuinely cannot do without.
    Sentinel one or more files (relative to the repo root) that exist once the
             dependency is staged; used to skip work on repeat runs. List one
             per Stage destination, not just one per archive: a file proving the
             zip was unpacked says nothing about whether a later copy rule still
             has its output, and a tree missing only that output would be
             skipped and then build something that cannot run.
    Url      the .zip release asset to download.
    Archive  omit for a .zip. $false means the Url is a bare file with no
             archive to expand; Stage then takes a single @{ To; As } naming
             where to put it and what to call it.
    MinVersion
             for a Url that always points at the latest build rather than a
             pinned one. The staged file's own version is recorded, a fetch is
             skipped while that is at or above this floor, and a download that
             comes back older than the floor is a failure rather than staged.
    Stage    list of @{ From = '<path within the zip>'; To = '<dir under repo root>' }
             copy rules (From may contain a wildcard, e.g. 'klm\*.h').

  Today this fetches KLightMapper (Map-from-Lights camera scan). As the libs
  committed under lib\windows64 / bin64 are retired, move them here so the repo
  carries the fetch recipe instead of the binaries.

.PARAMETER Force
  Re-download every dependency even if its sentinel files already exist.

.PARAMETER Only
  Fetch only the dependency whose Name matches (e.g. -Only KLightMapper).
#>
param([switch]$Force, [string]$Only)

$ErrorActionPreference = 'Stop'
# Invoke-WebRequest's progress stream is huge noise in CI logs (and over SSH);
# silence it. Speeds the download up substantially too.
$ProgressPreference = 'SilentlyContinue'
$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$rootDir   = (Resolve-Path (Join-Path $scriptDir '..')).Path

function Read-VersionFile([string]$name) {
    (Get-Content (Join-Path $scriptDir $name) -Raw).Trim()
}

# -- Dependency catalogue ----------------------------------------------------
# Keep KLightMapper's version pin in sync with the macOS Swift-package pin
# (macOS/.../Package.resolved) so all three desktops use the same engine.
$klmVersion = Read-VersionFile 'klightmapper_version.txt'

# The shared dependency bundle - wxWidgets, FFmpeg, OpenSSL, curl and the rest -
# built from xLights-dependencies so all three desktops use identical versions.
$depsVersion = Read-VersionFile 'windows_deps_version.txt'

# The installer runs the VC++ redistributable, so it has to be at least as new
# as the CRT the toolset links against. An older one installs a runtime missing
# symbols the binaries import, and xLights then fails to start with a missing
# entry point rather than anything that names the cause.
$vcRedistMin = Read-VersionFile 'vcredist_min_version.txt'

# ONNX Runtime with the DirectML execution provider, used by stem separation.
# Windows only - macOS does the same work through CoreML - so it is fetched here
# rather than added to the cross-platform bundle, which exists to keep library
# versions identical across platforms.
$ortVersion = Read-VersionFile 'onnxruntime_version.txt'

$Dependencies = @(
    @{
        Name     = 'Dependencies'
        Optional = $false
        # Staged into its own directory rather than into include/ and
        # lib/windows64: putting the bundle's headers on the shared include/
        # path would shadow the system and bundle headers the other platforms
        # rely on, which is what moving include/windows-vendored fixed.
        # One per Stage destination below. bin64 is listed separately because
        # deleting a DLL from there leaves the bundle directory intact, which
        # used to skip the fetch and produce an executable missing SDL2.
        Sentinel = @('dependencies-bundle\lib\avcodec.lib', 'bin64\SDL2.dll')
        Stamp    = 'dependencies-bundle\.deps_version'
        Version  = $depsVersion
        Url      = "https://github.com/xLightsSequencer/xLights-dependencies/releases/download/$depsVersion/xLights-windows-dependencies-x64.zip"
        Stage    = @(
            @{ From = 'xLights-windows-dependencies\*'; To = 'dependencies-bundle' }
            # The runtime DLLs also have to sit beside the executable, which is
            # where the installer and the debugger both look for them.
            @{ From = 'xLights-windows-dependencies\bin\*.dll'; To = 'bin64' }
        )
    }
    @{
        Name     = 'KLightMapper'
        Optional = $false
        Sentinel = @('lib\windows64\klightmapper.lib', 'bin64\klightmapper.dll')
        # Records the staged version so a bump to klightmapper_version.txt
        # actually re-downloads instead of leaving the old lib and headers.
        Stamp    = 'lib\windows64\.klightmapper_version'
        Version  = $klmVersion
        Url      = "https://github.com/KulpLights/KLightMapper/releases/download/$klmVersion/klightmapper-windows-x64.zip"
        Stage    = @(
            @{ From = 'klightmapper.lib'; To = 'lib\windows64' }
            @{ From = 'klightmapper.dll'; To = 'bin64' }
            @{ From = 'klm\*.h';          To = 'include\klightmapper\klm' }
        )
    }
    @{
        Name     = 'OnnxRuntime'
        Optional = $false
        # HAVE_ORT is defined unconditionally for x64, so a missing runtime is a
        # compile error rather than a feature that quietly switches itself off.
        Sentinel = @('include\onnxruntime\onnxruntime_c_api.h',
                     'lib\windows64\onnxruntime.lib',
                     'bin64\onnxruntime.dll')
        Stamp    = 'include\onnxruntime\.ort_version'
        Version  = $ortVersion
        # A .nupkg is a zip, so the normal archive path handles it unchanged.
        Url      = "https://api.nuget.org/v3-flatcontainer/microsoft.ml.onnxruntime.directml/$ortVersion/microsoft.ml.onnxruntime.directml.$ortVersion.nupkg"
        Stage    = @(
            # The package also carries win-arm64 and win-x86 runtimes, ~14 MB
            # each. Nothing builds against them, so only x64 is staged.
            @{ From = 'build\native\include\*';                 To = 'include\onnxruntime' }
            @{ From = 'runtimes\win-x64\native\onnxruntime.lib'; To = 'lib\windows64' }
            @{ From = 'runtimes\win-x64\native\onnxruntime.dll'; To = 'bin64' }
        )
    }
    @{
        Name       = 'VCRedist'
        Optional   = $false
        # Microsoft serves the redistributable as a bare .exe, and the /18/ path
        # always points at the current build for that Visual Studio line - hence
        # a floor rather than an exact pin.
        Archive    = $false
        MinVersion = $vcRedistMin
        Sentinel   = 'build_scripts\msw\vcredist\VC_redist.x64.exe'
        Stamp      = 'build_scripts\msw\vcredist\.vcredist_version'
        Url        = 'https://aka.ms/vs/18/release/vc_redist.x64.exe'
        Stage      = @(
            @{ To = 'build_scripts\msw\vcredist'; As = 'VC_redist.x64.exe' }
        )
    }
)

function Fetch-Dependency($dep) {
    # Every sentinel must be present; one missing means something was cleaned
    # out from under us and the dependency has to be staged again.
    $sentinels = @($dep.Sentinel) | ForEach-Object { Join-Path $rootDir $_ }
    $missing   = @($sentinels | Where-Object { -not (Test-Path $_) })
    $stagedOk  = ($missing.Count -eq 0)
    $stamp    = if ($dep.Stamp) { Join-Path $rootDir $dep.Stamp } else { $null }
    # Skip only when what is staged IS the pinned version. Testing presence
    # alone would make a version bump a no-op on every existing tree, and the
    # stale headers - not the skipped download - are what the build then fails
    # on, with nothing pointing back at the fetch as the cause. Cast before
    # .Trim(): Get-Content -Raw yields $null on an empty stamp, and
    # $ErrorActionPreference = 'Stop' would turn that into a build abort.
    $staged = if ($stamp -and (Test-Path $stamp)) { ([string](Get-Content $stamp -Raw)).Trim() } else { '' }
    if (-not $stagedOk -and $missing.Count -lt $sentinels.Count) {
        Write-Host ("fetch_dependencies: {0} is missing {1} - re-staging." -f $dep.Name,
            (($missing | ForEach-Object { Split-Path $_ -Leaf }) -join ', '))
    }
    if ($stagedOk -and -not $Force) {
        if ($dep.MinVersion) {
            # A floating "latest" Url: anything at or above the floor is fine,
            # so do not re-download every time upstream publishes a new build.
            $current = $false
            if ($staged) {
                try { $current = ([version]$staged) -ge ([version]$dep.MinVersion) } catch { $current = $false }
            }
            if ($current) {
                Write-Host "fetch_dependencies: $($dep.Name) $staged already staged (floor $($dep.MinVersion)) - skipping (use -Force to re-download)."
                return $true
            }
            $was = if ($staged) { $staged } else { 'unversioned' }
            Write-Host "fetch_dependencies: $($dep.Name) staged copy is $was, need $($dep.MinVersion) or newer - re-fetching."
        }
        elseif (-not $stamp) {
            Write-Host "fetch_dependencies: $($dep.Name) already staged - skipping (use -Force to re-download)."
            return $true
        }
        elseif ($staged -eq $dep.Version) {
            Write-Host "fetch_dependencies: $($dep.Name) $($dep.Version) already staged - skipping (use -Force to re-download)."
            return $true
        }
        else {
            $was = if ($staged) { $staged } else { 'unversioned' }
            Write-Host "fetch_dependencies: $($dep.Name) staged copy is $was, want $($dep.Version) - re-fetching."
        }
    }

    Write-Host "fetch_dependencies: $($dep.Name) -> $($dep.Url)"
    $tmp = Join-Path ([System.IO.Path]::GetTempPath()) ("dep_" + [System.Guid]::NewGuid().ToString('N'))
    New-Item -ItemType Directory -Path $tmp -Force | Out-Null
    try {
        # Expand-Archive insists on a .zip extension, so only name it that when
        # the download actually is one.
        $dl = if ($dep.Archive -eq $false) { Join-Path $tmp 'dep.bin' } else { Join-Path $tmp 'dep.zip' }
        try {
            Invoke-WebRequest -Uri $dep.Url -OutFile $dl -UseBasicParsing
        } catch {
            Write-Warning "fetch_dependencies: download failed for $($dep.Name) ($($dep.Url)): $($_.Exception.Message)"
            return $false
        }
        $stagedFile = $null
        if ($dep.Archive -eq $false) {
            $item    = $dep.Stage[0]
            $destDir = Join-Path $rootDir $item.To
            New-Item -ItemType Directory -Path $destDir -Force | Out-Null
            $stagedFile = Join-Path $destDir $item.As
            Copy-Item $dl $stagedFile -Force
        }
        else {
            $x = Join-Path $tmp 'x'
            Expand-Archive -Path $dl -DestinationPath $x -Force
            foreach ($item in $dep.Stage) {
                $destDir = Join-Path $rootDir $item.To
                New-Item -ItemType Directory -Path $destDir -Force | Out-Null
                # -Recurse so a Stage entry can name a directory tree; harmless for files.
                Copy-Item (Join-Path $x $item.From) $destDir -Recurse -Force
            }
        }
        $stagedVersion = $dep.Version
        if ($dep.MinVersion) {
            # Record what actually arrived, and refuse anything below the floor
            # rather than staging a runtime that cannot load these binaries.
            $stagedVersion = (Get-Item $stagedFile).VersionInfo.ProductVersion
            $tooOld = $true
            if ($stagedVersion) {
                try { $tooOld = ([version]$stagedVersion) -lt ([version]$dep.MinVersion) } catch { $tooOld = $true }
            }
            if ($tooOld) {
                Write-Warning "fetch_dependencies: $($dep.Name) came back as '$stagedVersion', older than the required $($dep.MinVersion) - not staging."
                Remove-Item $stagedFile -Force -ErrorAction SilentlyContinue
                return $false
            }
        }
        # Written last, so an interrupted run leaves no stamp and the next one refetches.
        if ($stamp) {
            New-Item -ItemType Directory -Path (Split-Path -Parent $stamp) -Force | Out-Null
            Set-Content -Path $stamp -Value $stagedVersion -NoNewline
        }
        Write-Host "fetch_dependencies: staged $($dep.Name) $stagedVersion."
        return $true
    } finally {
        Remove-Item $tmp -Recurse -Force -ErrorAction SilentlyContinue
    }
}

$failed = @()
foreach ($dep in $Dependencies) {
    if ($Only -and $dep.Name -ne $Only) { continue }
    $ok = Fetch-Dependency $dep
    if (-not $ok) {
        if ($dep.Optional) {
            Write-Host "fetch_dependencies: $($dep.Name) is optional - continuing without it."
        } else {
            $failed += $dep.Name
        }
    }
}

if ($failed.Count -gt 0) {
    Write-Error "fetch_dependencies: required dependencies failed: $($failed -join ', ')"
    exit 1
}
exit 0
