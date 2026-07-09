<#
Compile the Vulkan GLSL kernels to the SPIR-V C headers the C++ #includes:
  - compute kernels: src-core/effects/vulkan/shaders/*.comp
  - graphics shaders: src-core/graphics/vulkan/shaders/*.vert|*.frag
Bash-free MSBuild counterpart of build_scripts/compile_vulkan_shaders.sh, run as
the Xlights.vcxproj PreBuildEvent (x64) so the headers exist before the C++ TUs
compile.  glslc is a build tool here, exactly like ispc for the .ispc kernels.

The generated headers (shaders/compiled/) are gitignored, not committed.
Diff-guarded so an untouched kernel produces no file write (an incremental
rebuild then doesn't needlessly recompile the TU that #includes it).

glslc comes from the Vulkan SDK (VULKAN_SDK env var, set by the SDK installer)
or from PATH.
#>
param(
    [string]$ShaderDir = (Join-Path $PSScriptRoot '..\..\src-core\effects\vulkan\shaders'),
    [string]$GfxShaderDir = (Join-Path $PSScriptRoot '..\..\src-core\graphics\vulkan\shaders')
)
$ErrorActionPreference = 'Stop'

$glslc = $null
# Honor a GLSLC override (the CMake build passes its find_program result); then
# the Vulkan SDK; then PATH.
if ($env:GLSLC -and (Test-Path $env:GLSLC)) {
    $glslc = $env:GLSLC
} elseif ($env:VULKAN_SDK -and (Test-Path (Join-Path $env:VULKAN_SDK 'Bin\glslc.exe'))) {
    $glslc = Join-Path $env:VULKAN_SDK 'Bin\glslc.exe'
} else {
    $cmd = Get-Command glslc.exe -ErrorAction SilentlyContinue
    if ($cmd) { $glslc = $cmd.Source }
}
if (-not $glslc) {
    Write-Error "glslc.exe not found. Install the Vulkan SDK (sets VULKAN_SDK) or put glslc on PATH."
}

$tmp = [System.IO.Path]::GetTempFileName()
$script:changed = 0

function Compile-ShaderDir([string]$dir, [string[]]$patterns) {
    $dir = (Resolve-Path $dir).Path
    $outDir = Join-Path $dir 'compiled'
    New-Item -ItemType Directory -Force -Path $outDir | Out-Null

    foreach ($pattern in $patterns) {
        Get-ChildItem (Join-Path $dir $pattern) -ErrorAction SilentlyContinue | ForEach-Object {
            $base = $_.BaseName
            $ext = $_.Extension.TrimStart('.')
            # .comp keeps the historical Name.spv.h / Name_spv naming; stage
            # shaders encode the stage so name.vert and name.frag can coexist.
            if ($ext -eq 'comp') {
                $sym = "${base}_spv"
                $out = Join-Path $outDir "$base.spv.h"
                $hdrName = "$base.spv.h"
            } else {
                $sym = "${base}_${ext}_spv"
                $out = Join-Path $outDir "$base.$ext.spv.h"
                $hdrName = "$base.$ext.spv.h"
            }

            & $glslc -O --target-env=vulkan1.1 -mfmt=c -I "$dir" -o $tmp $_.FullName
            if ($LASTEXITCODE -ne 0) { Write-Error "glslc failed on $($_.Name)" }

            $body = [System.IO.File]::ReadAllText($tmp)
            $content = "// Generated from $($_.Name) by build_scripts/msw/compile_vulkan_shaders.ps1 - do not edit`nstatic const uint32_t ${sym}[] =`n$body;`n"

            $existing = if (Test-Path $out) { [System.IO.File]::ReadAllText($out) } else { $null }
            if ($existing -ne $content) {
                [System.IO.File]::WriteAllText($out, $content, (New-Object System.Text.UTF8Encoding($false)))
                Write-Host "updated: $hdrName"
                $script:changed++
            }
        }
    }
}

try {
    Compile-ShaderDir $ShaderDir @('*.comp', '*.vert', '*.frag')
    Compile-ShaderDir $GfxShaderDir @('*.vert', '*.frag')
    if ($script:changed -eq 0) { Write-Host "All Vulkan shader headers up to date." }
} finally {
    Remove-Item $tmp -Force -ErrorAction SilentlyContinue
}
