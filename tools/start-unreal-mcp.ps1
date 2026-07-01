param(
  [int]$Port = 8000,
  [string]$UrlPath = "/mcp"
)

$ErrorActionPreference = "Stop"

$projectRoot = Split-Path -Parent $PSScriptRoot
$uproject = Join-Path $projectRoot "engine\LightWarrior.uproject"
$logRoot = Join-Path $projectRoot "logs"
$logPath = Join-Path $logRoot "unreal-mcp.log"

New-Item -ItemType Directory -Force -Path $logRoot | Out-Null

$unrealEditor = & (Join-Path $PSScriptRoot "find-unreal.ps1")

$arguments = @(
  "`"$uproject`"",
  "-NoSplash",
  "-log",
  "-abslog=`"$logPath`"",
  "-ModelContextProtocolStartServer",
  "-ModelContextProtocolPort=$Port"
)

$process = Start-Process -FilePath $unrealEditor -ArgumentList $arguments -PassThru

[pscustomobject]@{
  process_id = $process.Id
  url = "http://127.0.0.1:$Port$UrlPath"
  log = $logPath
}
