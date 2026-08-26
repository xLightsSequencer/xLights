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
    Sentinel a file (relative to the repo root) that exists once the dependency
             is staged; used to skip work on repeat runs.
    Url      the .zip release asset to download.
    Stage    list of @{ From = '<path within the zip>'; To = '<dir under repo root>' }
             copy rules (From may contain a wildcard, e.g. 'klm\*.h').

  Today this fetches KLightMapper (Map-from-Lights camera scan). As the libs
  committed under lib\windows64 / bin64 are retired, move them here so the repo
  carries the fetch recipe instead of the binaries.

.PARAMETER Force
  Re-download every dependency even if its sentinel file already exists.

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

$Dependencies = @(
    @{
        Name     = 'Dependencies'
        Optional = $false
        # Staged into its own directory rather than into include/ and
        # lib/windows64: putting the bundle's headers on the shared include/
        # path would shadow the system and bundle headers the other platforms
        # rely on, which is what moving include/windows-vendored fixed.
        Sentinel = 'dependencies-bundle\lib\avcodec.lib'
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
        Sentinel = 'lib\windows64\klightmapper.lib'
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
)

function Fetch-Dependency($dep) {
    $sentinel = Join-Path $rootDir $dep.Sentinel
    $stamp    = if ($dep.Stamp) { Join-Path $rootDir $dep.Stamp } else { $null }
    # Skip only when what is staged IS the pinned version. Testing presence
    # alone would make a version bump a no-op on every existing tree, and the
    # stale headers - not the skipped download - are what the build then fails
    # on, with nothing pointing back at the fetch as the cause. Cast before
    # .Trim(): Get-Content -Raw yields $null on an empty stamp, and
    # $ErrorActionPreference = 'Stop' would turn that into a build abort.
    $staged = if ($stamp -and (Test-Path $stamp)) { ([string](Get-Content $stamp -Raw)).Trim() } else { '' }
    if ((Test-Path $sentinel) -and -not $Force) {
        if (-not $stamp) {
            Write-Host "fetch_dependencies: $($dep.Name) already staged - skipping (use -Force to re-download)."
            return $true
        }
        if ($staged -eq $dep.Version) {
            Write-Host "fetch_dependencies: $($dep.Name) $($dep.Version) already staged - skipping (use -Force to re-download)."
            return $true
        }
        $was = if ($staged) { $staged } else { 'unversioned' }
        Write-Host "fetch_dependencies: $($dep.Name) staged copy is $was, want $($dep.Version) - re-fetching."
    }

    Write-Host "fetch_dependencies: $($dep.Name) -> $($dep.Url)"
    $tmp = Join-Path ([System.IO.Path]::GetTempPath()) ("dep_" + [System.Guid]::NewGuid().ToString('N'))
    New-Item -ItemType Directory -Path $tmp -Force | Out-Null
    try {
        $zip = Join-Path $tmp 'dep.zip'
        try {
            Invoke-WebRequest -Uri $dep.Url -OutFile $zip -UseBasicParsing
        } catch {
            Write-Warning "fetch_dependencies: download failed for $($dep.Name) ($($dep.Url)): $($_.Exception.Message)"
            return $false
        }
        $x = Join-Path $tmp 'x'
        Expand-Archive -Path $zip -DestinationPath $x -Force
        foreach ($item in $dep.Stage) {
            $destDir = Join-Path $rootDir $item.To
            New-Item -ItemType Directory -Path $destDir -Force | Out-Null
            # -Recurse so a Stage entry can name a directory tree; harmless for files.
            Copy-Item (Join-Path $x $item.From) $destDir -Recurse -Force
        }
        # Written last, so an interrupted run leaves no stamp and the next one refetches.
        if ($stamp) {
            New-Item -ItemType Directory -Path (Split-Path -Parent $stamp) -Force | Out-Null
            Set-Content -Path $stamp -Value $dep.Version -NoNewline
        }
        Write-Host "fetch_dependencies: staged $($dep.Name) $($dep.Version)."
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
