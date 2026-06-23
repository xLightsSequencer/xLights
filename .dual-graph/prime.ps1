$port = if (Test-Path 'H:\XlightsSourceDir\xLights\.dual-graph\mcp_port') { Get-Content 'H:\XlightsSourceDir\xLights\.dual-graph\mcp_port' } else { '8080' }
try {
    $out = (Invoke-WebRequest "http://127.0.0.1:$port/prime" -UseBasicParsing -TimeoutSec 3).Content
    if ($out) { Write-Output $out; Write-Error "[dual-graph] Context loaded (port $port)" }
} catch {
    Write-Error "[dual-graph] MCP server not reachable on port $port -- run dgc to restart"
}
$ctxFile = 'H:\XlightsSourceDir\xLights\CONTEXT.md'
if (Test-Path $ctxFile) { Write-Output ""; Write-Output "=== CONTEXT.md ==="; Get-Content $ctxFile -Raw; Write-Output "=== end CONTEXT.md ===" }
$storeFile = 'H:\XlightsSourceDir\xLights\.dual-graph\context-store.json'
if (Test-Path $storeFile) {
    $cutoff = (Get-Date).AddDays(-7).ToString('yyyy-MM-dd')
    try {
        $entries = (Get-Content $storeFile -Raw | ConvertFrom-Json) | Where-Object { $_.date -ge $cutoff } | Select-Object -First 15
        if ($entries) { Write-Output ""; Write-Output "=== Stored Context ==="; $entries | ForEach-Object { Write-Output ("[" + $_.type + "] " + $_.content) }; Write-Output "=== end Stored Context ===" }
    } catch {}
}
