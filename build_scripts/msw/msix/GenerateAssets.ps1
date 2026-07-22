<#
.SYNOPSIS
    Generate the MSIX visual asset PNG set for xLights from a single source image.

.DESCRIPTION
    Produces the tile / logo / splash / file-association icons referenced by
    AppxManifest.template.xml. Uses only System.Drawing (ships with the .NET
    Framework on Windows) so there are no external image-tool dependencies.

    Tiles (71/150/310/wide/splash) center the logo with padding on a transparent
    canvas (the manifest BackgroundColor shows behind). Small icons (44 and the
    targetsize variants used for the taskbar, Start list, and file associations)
    fill the square so they stay legible at tiny sizes.

    Source is include/xlights.png (256x256). Sizes above 256 are mildly upscaled;
    regenerate from a larger master if a crisper 310/400% asset is ever needed.

.EXAMPLE
    pwsh -File GenerateAssets.ps1
    pwsh -File GenerateAssets.ps1 -Source ..\..\..\include\xlights.png -OutDir .\Assets
#>
[CmdletBinding()]
param(
    [string]$Source,
    [string]$OutDir
)

$ErrorActionPreference = 'Stop'
Add-Type -AssemblyName System.Drawing

$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
if (-not $Source) { $Source = Join-Path $scriptDir '..\..\..\include\xlights.png' }
if (-not $OutDir) { $OutDir = Join-Path $scriptDir 'Assets' }

$Source = (Resolve-Path $Source).Path
New-Item -ItemType Directory -Force -Path $OutDir | Out-Null

Write-Host "Source : $Source"
Write-Host "Output : $OutDir"

$src = [System.Drawing.Image]::FromFile($Source)

function New-Asset {
    param(
        [string]$Name,
        [int]$Width,
        [int]$Height,
        [ValidateSet('fill', 'fit')] [string]$Mode = 'fit',
        # Fraction of the shorter dimension the logo occupies in 'fit' mode.
        [double]$InnerScale = 0.75
    )
    $bmp = New-Object System.Drawing.Bitmap($Width, $Height, [System.Drawing.Imaging.PixelFormat]::Format32bppArgb)
    $g = [System.Drawing.Graphics]::FromImage($bmp)
    try {
        $g.InterpolationMode = [System.Drawing.Drawing2D.InterpolationMode]::HighQualityBicubic
        $g.SmoothingMode     = [System.Drawing.Drawing2D.SmoothingMode]::HighQuality
        $g.PixelOffsetMode   = [System.Drawing.Drawing2D.PixelOffsetMode]::HighQuality
        $g.Clear([System.Drawing.Color]::Transparent)

        if ($Mode -eq 'fill') {
            $dw = $Width; $dh = $Height; $dx = 0; $dy = 0
        } else {
            $box = [Math]::Min($Width, $Height) * $InnerScale
            $ratio = [Math]::Min($box / $src.Width, $box / $src.Height)
            $dw = [int]([Math]::Round($src.Width * $ratio))
            $dh = [int]([Math]::Round($src.Height * $ratio))
            $dx = [int](($Width - $dw) / 2)
            $dy = [int](($Height - $dh) / 2)
        }
        $g.DrawImage($src, $dx, $dy, $dw, $dh)

        $path = Join-Path $OutDir $Name
        $bmp.Save($path, [System.Drawing.Imaging.ImageFormat]::Png)
        Write-Host ("  {0,-40} {1}x{2}" -f $Name, $Width, $Height)
    } finally {
        $g.Dispose(); $bmp.Dispose()
    }
}

# --- Tiles & store / splash (logo centered with padding) ---
New-Asset 'Square71x71Logo.png'   71  71  fit  0.70
New-Asset 'Square150x150Logo.png' 150 150 fit  0.66
New-Asset 'Square310x310Logo.png' 310 310 fit  0.62
New-Asset 'Wide310x150Logo.png'   310 150 fit  0.80
New-Asset 'SplashScreen.png'      620 300 fit  0.45
New-Asset 'StoreLogo.png'         50  50  fill

# --- App-list / taskbar / file-association icon (fills the square) ---
New-Asset 'Square44x44Logo.png'   44  44  fill

# targetsize variants Windows picks for taskbar/Start/Explorer associations.
foreach ($s in 16, 24, 32, 48, 256) {
    New-Asset ("Square44x44Logo.targetsize-{0}.png" -f $s)            $s $s fill
    # unplated = no system background plate (used on the taskbar)
    New-Asset ("Square44x44Logo.targetsize-{0}_altform-unplated.png" -f $s) $s $s fill
}

$src.Dispose()
Write-Host "Done. Generated MSIX assets in $OutDir"
