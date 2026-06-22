$ErrorActionPreference = "Stop"

$candidates = @(
  "D:\Games\UE_5.8\Engine\Binaries\Win64\UnrealEditor.exe",
  "D:\Games\UE_5.7\Engine\Binaries\Win64\UnrealEditor.exe",
  "D:\Games\UE_5.6\Engine\Binaries\Win64\UnrealEditor.exe",
  "C:\Program Files\Epic Games\UE_5.8\Engine\Binaries\Win64\UnrealEditor.exe",
  "C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\Win64\UnrealEditor.exe",
  "C:\Program Files\Epic Games\UE_5.6\Engine\Binaries\Win64\UnrealEditor.exe",
  "C:\Program Files\Epic Games\UE_5.5\Engine\Binaries\Win64\UnrealEditor.exe",
  "C:\Program Files\Epic Games\UE_5.4\Engine\Binaries\Win64\UnrealEditor.exe",
  "C:\Program Files\Epic Games\UE_5.3\Engine\Binaries\Win64\UnrealEditor.exe"
)

foreach ($candidate in $candidates) {
  if (Test-Path $candidate) {
    Write-Output $candidate
    exit 0
  }
}

$command = Get-Command UnrealEditor -ErrorAction SilentlyContinue
if ($command) {
  Write-Output $command.Source
  exit 0
}

$roots = @(
  "D:\Games",
  "C:\Program Files\Epic Games",
  "D:\Epic Games",
  "D:\",
  "E:\"
)

foreach ($root in $roots) {
  if (Test-Path $root) {
    $found = Get-ChildItem -Path $root -Recurse -Filter UnrealEditor.exe -ErrorAction SilentlyContinue |
      Select-Object -First 1 -ExpandProperty FullName
    if ($found) {
      Write-Output $found
      exit 0
    }
  }
}

Write-Error "UnrealEditor.exe was not found. Install UE5 through Epic Games Launcher, then rerun this script."
