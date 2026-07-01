param(
  [int]$Port = 8000,
  [string]$UrlPath = "/mcp"
)

$ErrorActionPreference = "Stop"

$url = "http://127.0.0.1:$Port$UrlPath"

try {
  $initBody = @{
    jsonrpc = "2.0"
    id = 1
    method = "initialize"
    params = @{
      protocolVersion = "2025-11-25"
      capabilities = @{}
      clientInfo = @{
        name = "light-warrior-mcp-check"
        version = "1.0"
      }
    }
  } | ConvertTo-Json -Depth 8

  $headers = @{
    Accept = "application/json, text/event-stream"
  }

  $init = Invoke-WebRequest -Uri $url -Method Post -ContentType "application/json" -Headers $headers -Body $initBody -UseBasicParsing -TimeoutSec 10
  $sessionId = $init.Headers["Mcp-Session-Id"]
  if (!$sessionId) {
    $sessionId = $init.Headers["mcp-session-id"]
  }

  $listHeaders = @{
    Accept = "application/json, text/event-stream"
  }
  if ($sessionId) {
    $listHeaders["Mcp-Session-Id"] = $sessionId
  }

  $listBody = @{
    jsonrpc = "2.0"
    id = 2
    method = "tools/list"
    params = @{}
  } | ConvertTo-Json -Depth 4

  $tools = Invoke-WebRequest -Uri $url -Method Post -ContentType "application/json" -Headers $listHeaders -Body $listBody -UseBasicParsing -TimeoutSec 10
  $toolJson = $tools.Content | ConvertFrom-Json
  $toolNames = @($toolJson.result.tools | ForEach-Object { $_.name })

  [pscustomobject]@{
    url = $url
    reachable = $true
    status_code = [int]$tools.StatusCode
    session_id = $sessionId
    tools = $toolNames -join ", "
    note = "MCP initialize and tools/list succeeded."
  }
} catch {
  $statusCode = $null
  if ($_.Exception.Response) {
    $statusCode = [int]$_.Exception.Response.StatusCode
  }

  [pscustomobject]@{
    url = $url
    reachable = $false
    status_code = $statusCode
    note = $_.Exception.Message
  }
  exit 1
}
