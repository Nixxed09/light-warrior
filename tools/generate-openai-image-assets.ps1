param(
  [string]$BatchPath = "assets/generated/internal-image-gen-batch-20260623.json",
  [string]$AssetId = "",
  [string]$Model = "",
  [switch]$DryRun
)

$ErrorActionPreference = "Stop"

$repoRoot = Resolve-Path (Join-Path $PSScriptRoot "..")
$batchFullPath = if ([System.IO.Path]::IsPathRooted($BatchPath)) {
  $BatchPath
} else {
  Join-Path $repoRoot $BatchPath
}

if (!(Test-Path $batchFullPath)) {
  throw "Batch file not found: $batchFullPath"
}

$batch = Get-Content -Raw $batchFullPath | ConvertFrom-Json
$utf8NoBom = New-Object System.Text.UTF8Encoding($false)
$modelToUse = if ($Model) { $Model } else { $batch.default_model }
if (!$modelToUse) {
  $modelToUse = "gpt-image-2"
}

$assets = @($batch.assets)
if ($AssetId) {
  $assets = @($assets | Where-Object { $_.asset_id -eq $AssetId })
  if ($assets.Count -eq 0) {
    throw "No asset found in batch for AssetId '$AssetId'"
  }
}

foreach ($asset in $assets) {
  $outputPath = Join-Path $repoRoot $asset.output_path
  $outputDir = Split-Path -Parent $outputPath
  New-Item -ItemType Directory -Force -Path $outputDir | Out-Null

  $promptParts = @()
  $promptParts += "Game: Light Warrior."
  $promptParts += "Asset ID: $($asset.asset_id)."
  $promptParts += $asset.positive_prompt
  if ($batch.global_rules) {
    $promptParts += "Global style rules: " + (($batch.global_rules | ForEach-Object { [string]$_ }) -join " ")
  }
  if ($asset.negative_prompt) {
    $promptParts += "Avoid: $($asset.negative_prompt)."
  }
  $prompt = ($promptParts -join "`n")

  $metadataPath = Join-Path $outputDir "internal-image-gen-request.json"
  $requestMetadata = [ordered]@{
    schema_version = "1.0"
    prepared_from_batch_date = $batch.created_at
    provider_system = "openai.images.generations"
    model = $modelToUse
    asset_id = $asset.asset_id
    kind = $asset.kind
    size = $asset.size
    output_path = $asset.output_path
    prompt = $prompt
    review_gate = $asset.review_gate
  }
  [System.IO.File]::WriteAllText($metadataPath, ($requestMetadata | ConvertTo-Json -Depth 8), $utf8NoBom)

  if ($DryRun) {
    Write-Host "[dry-run] $($asset.asset_id) -> $($asset.output_path)"
    continue
  }

  $apiKey = $env:OPENAI_API_KEY
  if ([string]::IsNullOrWhiteSpace($apiKey)) {
    throw "OPENAI_API_KEY is not set. Set it, then rerun this command."
  }

  $body = [ordered]@{
    model = $modelToUse
    prompt = $prompt
    size = $asset.size
    n = 1
  } | ConvertTo-Json -Depth 6

  $headers = @{
    Authorization = "Bearer $apiKey"
    "Content-Type" = "application/json"
  }

  Write-Host "Generating $($asset.asset_id) -> $($asset.output_path)"
  $response = Invoke-RestMethod `
    -Method Post `
    -Uri "https://api.openai.com/v1/images/generations" `
    -Headers $headers `
    -Body $body

  $first = @($response.data)[0]
  if ($first.b64_json) {
    [System.IO.File]::WriteAllBytes($outputPath, [System.Convert]::FromBase64String($first.b64_json))
  } elseif ($first.url) {
    Invoke-WebRequest -Uri $first.url -OutFile $outputPath
  } else {
    throw "Image response for $($asset.asset_id) did not include b64_json or url."
  }

  $resultPath = Join-Path $outputDir "internal-image-gen-result.json"
  $resultMetadata = [ordered]@{
    schema_version = "1.0"
    generated_at = (Get-Date).ToUniversalTime().ToString("o")
    provider_system = "openai.images.generations"
    model = $modelToUse
    asset_id = $asset.asset_id
    output_path = $asset.output_path
    source_batch = $BatchPath
    status = "generated_candidate"
    review_gate = $asset.review_gate
  }
  [System.IO.File]::WriteAllText($resultPath, ($resultMetadata | ConvertTo-Json -Depth 8), $utf8NoBom)
}
