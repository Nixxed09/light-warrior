param(
  [Parameter(Mandatory = $true)]
  [string]$AssetId,
  [string]$ProductOSRoot = "D:\TE-Code\ProductOS",
  [string]$Aspect = "16x9"
)

$ErrorActionPreference = "Stop"

$projectRoot = Split-Path -Parent $PSScriptRoot
$safeAssetId = $AssetId -replace "\.", "_"
$packageRoot = Join-Path $projectRoot "assets\generated\concepts\$safeAssetId"
$requestPath = Join-Path $packageRoot "request.json"

if (!(Test-Path $requestPath)) {
  throw "Concept request not found: $requestPath"
}

$request = Get-Content -LiteralPath $requestPath -Raw | ConvertFrom-Json
$prompt = $request.positive_prompt
$negative = if ($request.negative_prompt) { $request.negative_prompt } else { "text, words, letters, watermark, signature, logo, ui, frame, border" }
$outPath = Join-Path $packageRoot "concept_01.png"
$generator = Join-Path $ProductOSRoot "GamesOS\tools\gen_asset.py"

if (!(Test-Path $generator)) {
  throw "GamesOS generator not found: $generator"
}

$teneoPath = if (Test-Path "D:\TE-Code\teneo-production") { "D:\TE-Code\teneo-production" } else { "E:\TE-Code\teneo-production" }
if (!(Test-Path $teneoPath)) {
  throw "teneo-production not found. Set TENEO_PRODUCTION or clone it to D:\TE-Code\teneo-production."
}

$dependencyCheck = @'
import importlib.util
missing = [name for name in ("boto3", "requests") if importlib.util.find_spec(name) is None]
if missing:
    raise SystemExit("missing python modules: " + ", ".join(missing))
'@ | python -
if ($LASTEXITCODE -ne 0) {
  throw "GamesOS generator prerequisites missing. Install Python modules with: python -m pip install boto3 requests"
}

if (!(Get-Command aws -ErrorAction SilentlyContinue)) {
  throw "AWS CLI is not installed or not on PATH. Install/configure AWS credentials before running the Ideogram generator."
}

$env:TENEO_PRODUCTION = $teneoPath
python $generator --prompt $prompt --negative $negative --out $outPath --aspect $Aspect

if (!(Test-Path $outPath)) {
  throw "Generator completed without output: $outPath"
}

[pscustomobject]@{
  asset_id = $AssetId
  request = $requestPath
  output = $outPath
  generator = $generator
  teneo_production = $teneoPath
}
