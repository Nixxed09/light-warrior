param(
  [Parameter(Mandatory=$true)]
  [string]$LogPath,
  [string]$OutPath = ""
)

$ErrorActionPreference = "Stop"

if (!(Test-Path -LiteralPath $LogPath)) {
  throw "Log file not found: $LogPath"
}

if ([string]::IsNullOrWhiteSpace($OutPath)) {
  $directory = Split-Path -Parent $LogPath
  $fileName = [System.IO.Path]::GetFileNameWithoutExtension($LogPath)
  $OutPath = Join-Path $directory "$fileName-input-transcript.json"
}

$logText = Get-Content -LiteralPath $LogPath -Raw
$logLines = if ($logText) { $logText -split "`r?`n" } else { @() }
$inputTranscript = @()

foreach ($line in $logLines) {
  if ($line -notmatch "LW_(?:INPUT_ACTION|PLAYER_INPUT_ACTION)\s+([a-z_]+)\s*(.*)$") {
    continue
  }

  $action = $Matches[1]
  $detailText = $Matches[2]
  $details = [ordered]@{}

  foreach ($vectorMatch in [regex]::Matches($detailText, "(target|location)=(V\([^)]+\))")) {
    $details[$vectorMatch.Groups[1].Value] = $vectorMatch.Groups[2].Value
  }

  foreach ($pairMatch in [regex]::Matches($detailText, "([a-z_]+)=([^ ]+)")) {
    $key = $pairMatch.Groups[1].Value
    if ($key -eq "target" -or $key -eq "location") {
      continue
    }

    $valueText = $pairMatch.Groups[2].Value
    $numericValue = 0.0
    if ([double]::TryParse($valueText, [System.Globalization.NumberStyles]::Float, [System.Globalization.CultureInfo]::InvariantCulture, [ref]$numericValue)) {
      $details[$key] = $numericValue
    } else {
      $details[$key] = $valueText
    }
  }

  $inputTranscript += [pscustomobject]@{
    action = $action
    details = [pscustomobject]$details
    source = ($line -replace "^.*LogTemp:\s*Display:\s*", "")
  }
}

if ($inputTranscript.Count -gt 0) {
  $inputTranscript | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath $OutPath
}

[pscustomobject]@{
  log = $LogPath
  transcript = $OutPath
  transcript_exists = Test-Path -LiteralPath $OutPath
  action_count = $inputTranscript.Count
}
