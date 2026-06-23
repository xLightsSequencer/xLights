$hookInput = [Console]::In.ReadToEnd()
try { $transcript = ($hookInput | ConvertFrom-Json).transcript_path } catch { $transcript = '' }
if ($transcript -and (Test-Path $transcript)) {
    try {
        # Track how many lines we already counted to avoid double-counting on resume
        $offsetFile = $transcript + ".stopoffset"
        $startLine = 0
        if (Test-Path $offsetFile) { try { $startLine = [int](Get-Content $offsetFile -Raw).Trim() } catch { $startLine = 0 } }
        $allLines = @(Get-Content $transcript)
        $inputTk = 0; $cacheCreate = 0; $cacheRead = 0; $outputTk = 0; $model = ''
        for ($i = $startLine; $i -lt $allLines.Count; $i++) {
            try {
                $msg = $allLines[$i] | ConvertFrom-Json -ErrorAction SilentlyContinue
                if (-not $msg -or $msg.type -ne 'assistant') { continue }
                $m = $msg.message
                if (-not $model -and $m.model) { $model = $m.model }
                $u = $m.usage
                if (-not $u) { continue }
                $inputTk   += [int]($u.input_tokens)
                $cacheCreate += [int]($u.cache_creation_input_tokens)
                $cacheRead += [int]($u.cache_read_input_tokens)
                $outputTk  += [int]($u.output_tokens)
            } catch { continue }
        }
        # Save current line count so next stop only counts new lines
        $allLines.Count.ToString() | Set-Content -Path $offsetFile -Encoding UTF8 -ErrorAction SilentlyContinue
        if ($inputTk -gt 0 -or $cacheCreate -gt 0 -or $cacheRead -gt 0 -or $outputTk -gt 0) {
            if (-not $model) { $model = 'claude-sonnet-4-6' }
            $body = @{
                input_tokens = $inputTk
                output_tokens = $outputTk
                cache_creation_input_tokens = $cacheCreate
                cache_read_input_tokens = $cacheRead
                model = $model
                description = "auto"
                project = "H:\XlightsSourceDir\xLights"
            } | ConvertTo-Json -Compress
            # POST to MCP graph server (always running, reliable)
            $mcpPortFile = Join-Path "C:\Users\daryl\.dual-graph" "mcp_port"
            $mcpPort = if (Test-Path $mcpPortFile) { (Get-Content $mcpPortFile -Raw).Trim() } else { "8080" }
            Invoke-RestMethod -Method Post -Uri "http://127.0.0.1:$mcpPort/log" -ContentType 'application/json' -Body $body -ErrorAction SilentlyContinue | Out-Null
            # Also POST to token-counter-mcp dashboard if available
            $portFile = Join-Path $env:USERPROFILE ".claude\token-counter\dashboard-port.txt"
            $dashPort = if (Test-Path $portFile) { (Get-Content $portFile -Raw).Trim() } else { "8899" }
            Invoke-RestMethod -Method Post -Uri "http://127.0.0.1:$dashPort/log" -ContentType 'application/json' -Body $body -ErrorAction SilentlyContinue | Out-Null
        }
    } catch {}
}
