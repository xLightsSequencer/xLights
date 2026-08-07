<#
.SYNOPSIS
    Generate an .appinstaller file for Windows App Installer auto-updates.

.DESCRIPTION
    Writes the App Installer manifest that gives sideloaded nightly MSIX testers a
    TestFlight-like experience: once a tester installs from the .appinstaller URL,
    Windows App Installer re-checks that URL on every launch and silently updates
    to the newest MSIX.

    Hosting model: the .appinstaller and the .msix are published as assets on the
    reused `nightly` GitHub release tag, so their download URLs stay constant
    across nightlies. The package Version increments each build (via the run
    number), which is what App Installer compares to decide to update.

    Uses the 2021 schema so ShowPrompt / UpdateBlocksActivation / on-launch checks
    are available. Identity (Name/Publisher) MUST match the signed MSIX exactly.

.EXAMPLE
    pwsh -File MakeAppInstaller.ps1 -Version 2026.11.0.123 `
        -Publisher "CN=Kulp Lights LLC, O=Kulp Lights LLC, L=..., S=..., C=US" `
        -BaseUrl "https://github.com/xLightsSequencer/xLights/releases/download/nightly" `
        -MsixFileName xLights-x64.msix -OutFile ..\output\xLights.appinstaller
#>
[CmdletBinding()]
param(
    [Parameter(Mandatory = $true)] [string]$Version,
    [Parameter(Mandatory = $true)] [string]$Publisher,
    [Parameter(Mandatory = $true)] [string]$BaseUrl,
    [string]$IdentityName     = 'KulpLightsLLC.xLights',
    [string]$MsixFileName     = 'xLights-x64.msix',
    [string]$AppInstallerName = 'xLights.appinstaller',
    [Parameter(Mandatory = $true)] [string]$OutFile
)

$ErrorActionPreference = 'Stop'

if ([string]::IsNullOrWhiteSpace($IdentityName)) { $IdentityName = 'KulpLightsLLC.xLights' }
if ($Version -notmatch '^\d+\.\d+\.\d+\.\d+$') {
    throw "Version must be 4-part numeric Major.Minor.Build.Revision (got '$Version')."
}

$base = $BaseUrl.TrimEnd('/')
$selfUri = "$base/$AppInstallerName"
$msixUri = "$base/$MsixFileName"

# XML-escape the publisher DN (may contain & or quotes in some locales).
$pub = [System.Security.SecurityElement]::Escape($Publisher)

$xml = @"
<?xml version="1.0" encoding="utf-8"?>
<AppInstaller
  xmlns="http://schemas.microsoft.com/appx/appinstaller/2021"
  Uri="$selfUri"
  Version="$Version">
  <MainPackage
    Name="$IdentityName"
    Publisher="$pub"
    Version="$Version"
    ProcessorArchitecture="x64"
    Uri="$msixUri" />
  <UpdateSettings>
    <OnLaunch HoursBetweenUpdateChecks="0" ShowPrompt="true" UpdateBlocksActivation="false" />
    <AutomaticBackgroundTask />
  </UpdateSettings>
</AppInstaller>
"@

[System.IO.File]::WriteAllText($OutFile, $xml, (New-Object System.Text.UTF8Encoding($false)))
Write-Host "Wrote $OutFile (version $Version)"
Write-Host "  self : $selfUri"
Write-Host "  msix : $msixUri"
