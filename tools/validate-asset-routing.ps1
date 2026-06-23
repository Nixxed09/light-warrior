$ErrorActionPreference = "Stop"

$repoRoot = Resolve-Path (Join-Path $PSScriptRoot "..")
$manifestPath = Join-Path $repoRoot "ASSET_MANIFEST.json"
$routingPath = Join-Path $repoRoot "assets/generated/asset-routing.json"

$manifest = Get-Content -Raw $manifestPath | ConvertFrom-Json
$routing = Get-Content -Raw $routingPath | ConvertFrom-Json

$manifestIds = @($manifest.assets | ForEach-Object { $_.id })
$routingIds = @($routing.assets | ForEach-Object { $_.asset_id })

$missingRoutes = @($manifestIds | Where-Object { $_ -notin $routingIds })
$unknownRoutes = @($routingIds | Where-Object { $_ -notin $manifestIds })

if ($missingRoutes.Count -gt 0) {
  throw "Missing asset-routing entries for: $($missingRoutes -join ', ')"
}

if ($unknownRoutes.Count -gt 0) {
  throw "asset-routing has entries not present in ASSET_MANIFEST.json: $($unknownRoutes -join ', ')"
}

foreach ($asset in $routing.assets) {
  if ($asset.concept_package) {
    $conceptPath = Join-Path $repoRoot $asset.concept_package
    if (!(Test-Path $conceptPath)) {
      throw "Concept package missing for $($asset.asset_id): $($asset.concept_package)"
    }
  }

  if ($asset.route_type -match "blender" -and $asset.blender_package) {
    $modelPath = Join-Path $repoRoot $asset.blender_package
    if (!(Test-Path $modelPath)) {
      Write-Warning "Blender/model package not generated yet for $($asset.asset_id): $($asset.blender_package)"
    }
  }
}

Write-Output "Asset routing valid for $($routingIds.Count) assets."
