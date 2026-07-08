<#
Compile the Vulkan GLSL compute kernels (src-core/effects/vulkan/shaders/*.comp)
to the SPIR-V C headers under shaders/compiled/ that the vulkan/ C++ #includes.
Bash-free MSBuild counterpart of build_scripts/compile_vulkan_shaders.sh, run as
the Xlights.vcxproj PreBuildEvent (x64) so the headers exist before the C++ TUs
compile.  glslc is a build tool here, exactly like ispc for the .ispc kernels.

The generated headers are gitignored, not committed.  Diff-guarded so an
untouched kernel produces no file write (an incremental rebuild then doesn't
needlessly recompile the TU that #includes it).

glslc comes from the Vulkan SDK (VULKAN_SDK env var, set by the SDK installer)
or from PATH.
#>
param(
    [string]$ShaderDir = (Join-Path $PSScriptRoot '..\..\src-core\effects\vulkan\shaders')
)
$ErrorActionPreference = 'Stop'

$glslc = $null
if ($env:VULKAN_SDK -and (Test-Path (Join-Path $env:VULKAN_SDK 'Bin\glslc.exe'))) {
    $glslc = Join-Path $env:VULKAN_SDK 'Bin\glslc.exe'
} else {
    $cmd = Get-Command glslc.exe -ErrorAction SilentlyContinue
    if ($cmd) { $glslc = $cmd.Source }
}
if (-not $glslc) {
    Write-Error "glslc.exe not found. Install the Vulkan SDK (sets VULKAN_SDK) or put glslc on PATH."
}

$ShaderDir = (Resolve-Path $ShaderDir).Path
$outDir = Join-Path $ShaderDir 'compiled'
New-Item -ItemType Directory -Force -Path $outDir | Out-Null

$tmp = [System.IO.Path]::GetTempFileName()
try {
    $changed = 0
    Get-ChildItem (Join-Path $ShaderDir '*.comp') | ForEach-Object {
        $name = $_.BaseName
        $out  = Join-Path $outDir "$name.spv.h"

        & $glslc -O --target-env=vulkan1.1 -mfmt=c -I "$ShaderDir" -o $tmp $_.FullName
        if ($LASTEXITCODE -ne 0) { Write-Error "glslc failed on $($_.Name)" }

        $body = [System.IO.File]::ReadAllText($tmp)
        $content = "// Generated from $name.comp by build_scripts/msw/compile_vulkan_shaders.ps1 - do not edit`nstatic const uint32_t ${name}_spv[] =`n$body;`n"

        $existing = if (Test-Path $out) { [System.IO.File]::ReadAllText($out) } else { $null }
        if ($existing -ne $content) {
            [System.IO.File]::WriteAllText($out, $content, (New-Object System.Text.UTF8Encoding($false)))
            Write-Host "updated: $name.spv.h"
            $changed++
        }
    }
    if ($changed -eq 0) { Write-Host "All Vulkan shader headers up to date." }
} finally {
    Remove-Item $tmp -Force -ErrorAction SilentlyContinue
}
