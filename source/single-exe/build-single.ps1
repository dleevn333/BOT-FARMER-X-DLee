param(
    [Parameter(Mandatory=$true)][string]$ReleaseDirectory,
    [Parameter(Mandatory=$true)][string]$OutputExe,
    [string]$ReleaseVersion = 'v0.1'
)
$ErrorActionPreference = 'Stop'
if (-not (Get-Command cl.exe -ErrorAction SilentlyContinue)) { throw 'Use x64 Developer PowerShell for Visual Studio with C++ and Windows SDK.' }
$singleRoot = $PSScriptRoot
$generated = Join-Path $singleRoot 'obj'
New-Item -ItemType Directory -Path $generated -Force | Out-Null
& python (Join-Path $singleRoot 'generate_payload.py') $ReleaseDirectory $generated $ReleaseVersion
if ($LASTEXITCODE -ne 0) { throw 'Payload generation failed.' }
& rc.exe /nologo "/fo$(Join-Path $generated 'payload.res')" (Join-Path $generated 'payload.rc')
if ($LASTEXITCODE -ne 0) { throw 'Resource compile failed.' }
Push-Location $generated
try {
    & cl.exe /nologo /std:c++20 /utf-8 /EHsc /MT /O2 /DUNICODE /D_UNICODE "/I$generated" (Join-Path $singleRoot 'launcher.cpp') (Join-Path $generated 'payload.res') "/Fe:$OutputExe" /link /SUBSYSTEM:WINDOWS shell32.lib ole32.lib user32.lib
    if ($LASTEXITCODE -ne 0) { throw 'Single EXE build failed.' }
} finally { Pop-Location }
