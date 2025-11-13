# Fix caffeine plugin config
$configDir = Join-Path $env:APPDATA "albert\albert"
if (-not (Test-Path $configDir)) {
    New-Item -ItemType Directory -Path $configDir -Force | Out-Null
}

$configPath = Join-Path $configDir "config"

Write-Host "Config file location: $configPath"

if (Test-Path $configPath) {
    Write-Host "`nCurrent config file contents:"
    Get-Content $configPath
    Write-Host "`nAdding caffeine/enabled=false..."
    
    # Read existing content
    $content = Get-Content $configPath -Raw
    
    # Check if [caffeine] section exists
    if ($content -match '\[caffeine\]') {
        # Replace enabled=true with enabled=false in caffeine section
        $content = $content -replace '(\[caffeine\]\s*(?:[^\[]|$)*?)enabled\s*=\s*true', '$1enabled=false'
        # If enabled is not set, add it
        if ($content -notmatch '\[caffeine\].*?enabled\s*=') {
            $content = $content -replace '(\[caffeine\])', '$1`nenabled=false'
        }
    } else {
        # Add caffeine section at the end
        $content += "`n[caffeine]`nenabled=false`n"
    }
    
    $content | Set-Content $configPath -Encoding UTF8
    Write-Host "`nUpdated config file:"
    Get-Content $configPath
} else {
    Write-Host "Config file does not exist. Creating it with caffeine disabled..."
    "[caffeine]`nenabled=false" | Set-Content $configPath -Encoding UTF8
    Write-Host "Created config file:"
    Get-Content $configPath
}

Write-Host "`nDone! You can now start albert.exe again."

