param(
  [string]$CodeRoot = "D:\TE-Code",
  [string]$FfmpegBin = "C:\Users\pberg\AppData\Local\Microsoft\WinGet\Packages\Gyan.FFmpeg_Microsoft.Winget.Source_8wekyb3d8bbwe\ffmpeg-8.1.1-full_build\bin"
)

$ErrorActionPreference = "Stop"

function Test-Port {
  param([int]$Port)
  $conn = Get-NetTCPConnection -LocalPort $Port -State Listen -ErrorAction SilentlyContinue | Select-Object -First 1
  return $null -ne $conn
}

function Start-Hidden {
  param(
    [string]$FilePath,
    [string[]]$ArgumentList,
    [string]$WorkingDirectory
  )
  Start-Process -FilePath $FilePath -ArgumentList $ArgumentList -WorkingDirectory $WorkingDirectory -WindowStyle Hidden | Out-Null
}

$imageRoot = Join-Path $CodeRoot "image-engine"
$videoRoot = Join-Path $CodeRoot "video-engine"
$audioRoot = Join-Path $CodeRoot "audio-engine"

if (-not (Test-Path $imageRoot)) { throw "Missing image-engine at $imageRoot" }
if (-not (Test-Path $videoRoot)) { throw "Missing video-engine at $videoRoot" }
if (-not (Test-Path $audioRoot)) { throw "Missing audio-engine at $audioRoot" }

if (-not (Test-Port 3100)) {
  Start-Hidden -FilePath "npm.cmd" -ArgumentList @("run", "dev") -WorkingDirectory $imageRoot
}

if (-not (Test-Port 8300)) {
  Start-Hidden -FilePath (Join-Path $audioRoot ".venv\Scripts\python.exe") -ArgumentList @("-m", "uvicorn", "src.api.server:app", "--host", "127.0.0.1", "--port", "8300") -WorkingDirectory $audioRoot
}

if (-not (Test-Port 8200)) {
  if (Test-Path $FfmpegBin) {
    $env:PATH = "$FfmpegBin;$env:PATH"
  }
  Start-Hidden -FilePath (Join-Path $videoRoot ".venv\Scripts\python.exe") -ArgumentList @("-m", "uvicorn", "src.api.server:app", "--host", "127.0.0.1", "--port", "8200") -WorkingDirectory $videoRoot
}

Start-Sleep -Seconds 5

$checks = [ordered]@{}
$checks["image-engine"] = Test-Port 3100
$checks["video-engine"] = Test-Port 8200
$checks["audio-engine"] = Test-Port 8300

$checks.GetEnumerator() | ForEach-Object {
  $status = if ($_.Value) { "listening" } else { "not listening" }
  Write-Output "$($_.Key): $status"
}
