param([Parameter(Mandatory=$true)][string]$OpenCvBuild, [switch]$SettingsTest, [switch]$PlantTest, [switch]$HarvestTest, [switch]$SellTest)
$ErrorActionPreference = 'Stop'
if (-not (Get-Command cl.exe -ErrorAction SilentlyContinue)) {
    throw 'Run this script from an x64 Developer PowerShell for Visual Studio 2022 with C++ and Windows SDK installed.'
}
$sourceRoot = $PSScriptRoot
$releaseRoot = Split-Path $sourceRoot -Parent
$objectRoot = Join-Path $sourceRoot 'obj'
New-Item -ItemType Directory -Path $objectRoot -Force | Out-Null
$opencvRoot = (Resolve-Path -LiteralPath $OpenCvBuild).Path
$mainSource = if ($SellTest) {'sell_test.cpp'} elseif ($HarvestTest) {'harvest_test.cpp'} elseif ($PlantTest) {'plant_test.cpp'} elseif ($SettingsTest) {'settings_test.cpp'} else {'main.cpp'}
$exeName = if ($SellTest) {'sell_test.exe'} elseif ($HarvestTest) {'harvest_test.exe'} elseif ($PlantTest) {'plant_test.exe'} elseif ($SettingsTest) {'settings_test.exe'} else {'autofarmnongtrai-update.exe'}
$sources = if ($PlantTest -or $HarvestTest -or $SellTest) {@((Join-Path $sourceRoot $mainSource))} else {@($mainSource,'imgui.cpp','imgui_draw.cpp','imgui_impl_dx11.cpp','imgui_impl_win32.cpp','imgui_tables.cpp','imgui_widgets.cpp') | ForEach-Object {Join-Path $sourceRoot $_}}
Push-Location $objectRoot
try {
    & cl.exe /nologo /std:c++20 /EHsc /utf-8 /MT /O2 /DNDEBUG /DUNICODE /D_UNICODE /DNOMINMAX /DWIN32_LEAN_AND_MEAN "/I$(Join-Path $opencvRoot 'include')" "/I$sourceRoot" $sources "/Fe:$(Join-Path $releaseRoot $exeName)" /link "/LIBPATH:$(Join-Path $opencvRoot 'x64\vc16\lib')" opencv_world4120.lib user32.lib gdi32.lib imm32.lib d3d11.lib d3dcompiler.lib dxgi.lib windowsapp.lib normaliz.lib ole32.lib
    if ($LASTEXITCODE -ne 0) {throw "Build failed: $LASTEXITCODE"}
    Copy-Item -LiteralPath (Join-Path $opencvRoot 'x64\vc16\bin\opencv_world4120.dll') -Destination $releaseRoot -Force
} finally {Pop-Location}
