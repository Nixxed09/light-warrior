$ErrorActionPreference = "Stop"

$projectRoot = Split-Path -Parent $PSScriptRoot
$taskGraphPath = Join-Path $projectRoot "TASK_GRAPH.json"
$pinPath = Join-Path $projectRoot "PIN.md"

Write-Host "Light Warrior status" -ForegroundColor Cyan
Write-Host "Project: $projectRoot"

if (Test-Path $pinPath) {
  Write-Host ""
  Get-Content -LiteralPath $pinPath
}

if (!(Test-Path $taskGraphPath)) {
  throw "Missing TASK_GRAPH.json"
}

$taskGraph = Get-Content -LiteralPath $taskGraphPath -Raw | ConvertFrom-Json
$tasks = @($taskGraph.tasks)

Write-Host ""
Write-Host "Task counts:" -ForegroundColor Cyan
$tasks | Group-Object status | Sort-Object Name | ForEach-Object {
  Write-Host ("- {0}: {1}" -f $_.Name, $_.Count)
}

$next = $tasks | Where-Object { $_.status -in @("pending", "failed", "needs_review") } | Select-Object -First 1
Write-Host ""
if ($next) {
  Write-Host "Next task: $($next.id) - $($next.title)" -ForegroundColor Yellow
  if ($next.acceptance) {
    Write-Host "Acceptance:"
    foreach ($item in $next.acceptance) {
      Write-Host "- $item"
    }
  }
} else {
  Write-Host "No pending tasks." -ForegroundColor Green
}

