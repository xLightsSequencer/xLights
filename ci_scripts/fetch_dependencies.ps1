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

$Dependencies = @(
    @{
        Name     = 'KLightMapper'
        Optional = $false
        Sentinel = 'lib\windows64\klightmapper.lib'
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
    if ((Test-Path $sentinel) -and -not $Force) {
        Write-Host "fetch_dependencies: $($dep.Name) already staged - skipping (use -Force to re-download)."
        return $true
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
            Copy-Item (Join-Path $x $item.From) $destDir -Force
        }
        Write-Host "fetch_dependencies: staged $($dep.Name)."
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
