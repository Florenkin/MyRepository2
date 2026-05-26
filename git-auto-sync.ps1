$ErrorActionPreference = 'Continue'
$repo = 'D:\code'
$log = Join-Path $repo '.git-auto-sync.log'
$pending = $false
$lastEvent = Get-Date

function Write-Log($message) {
    $time = Get-Date -Format 'yyyy-MM-dd HH:mm:ss'
    Add-Content -LiteralPath $log -Value "[$time] $message" -Encoding UTF8
}

function Should-IgnorePath($path) {
    if ([string]::IsNullOrWhiteSpace($path)) { return $true }
    $relative = $path.Substring($repo.Length).TrimStart('\')
    return (
        $relative -like '.git\*' -or
        $relative -like '.vs\*' -or
        $relative -like 'build\*' -or
        $relative -like 'out\*' -or
        $relative -like '*\build\*' -or
        $relative -like '*\out\*' -or
        $relative -eq '.git-auto-sync.log'
    )
}

function Invoke-GitSync {
    Set-Location -LiteralPath $repo
    $status = git status --porcelain
    if (-not $status) {
        Write-Log 'No changes to sync.'
        return
    }

    git add -A | Out-Null
    $statusAfterAdd = git status --porcelain
    if (-not $statusAfterAdd) {
        Write-Log 'Only ignored changes detected.'
        return
    }

    $message = 'Auto sync ' + (Get-Date -Format 'yyyy-MM-dd HH:mm:ss')
    git commit -m $message | Out-Null
    if ($LASTEXITCODE -ne 0) {
        Write-Log 'Commit failed.'
        return
    }

    $branch = git branch --show-current
    if ([string]::IsNullOrWhiteSpace($branch)) { $branch = 'master' }

    git push -u origin $branch | Out-Null
    if ($LASTEXITCODE -eq 0) {
        Write-Log "Pushed changes to origin/$branch."
    } else {
        Write-Log "Push failed for origin/$branch."
    }
}

Write-Log 'Git auto sync watcher started.'
$watcher = New-Object System.IO.FileSystemWatcher
$watcher.Path = $repo
$watcher.IncludeSubdirectories = $true
$watcher.EnableRaisingEvents = $true

$action = {
    if (-not (Should-IgnorePath $Event.SourceEventArgs.FullPath)) {
        $script:pending = $true
        $script:lastEvent = Get-Date
    }
}

Register-ObjectEvent $watcher Created -Action $action | Out-Null
Register-ObjectEvent $watcher Changed -Action $action | Out-Null
Register-ObjectEvent $watcher Deleted -Action $action | Out-Null
Register-ObjectEvent $watcher Renamed -Action $action | Out-Null

while ($true) {
    Start-Sleep -Seconds 5
    if ($pending -and ((Get-Date) - $lastEvent).TotalSeconds -ge 10) {
        $pending = $false
        Invoke-GitSync
    }
}
