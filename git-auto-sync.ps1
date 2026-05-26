$ErrorActionPreference = 'Continue'
$repo = 'D:\code'
$log = Join-Path $repo '.git-auto-sync.log'
$global:pending = $false
$global:lastEvent = Get-Date

function Write-Log($message) {
    $time = Get-Date -Format 'yyyy-MM-dd HH:mm:ss'
    Add-Content -LiteralPath $log -Value "[$time] $message" -Encoding UTF8
}

function Should-IgnorePath($path) {
    if ([string]::IsNullOrWhiteSpace($path)) { return $true }
    $relative = $path.Substring($repo.Length).TrimStart('\')
    return (
        $relative -eq '.git' -or
        $relative -like '.git\*' -or
        $relative -like '.vs\*' -or
        $relative -like 'build\*' -or
        $relative -like 'out\*' -or
        $relative -like '*\build\*' -or
        $relative -like '*\out\*' -or
        $relative -eq '.git-auto-sync.log'
    )
}

function Push-CurrentBranch {
    $env:GIT_TERMINAL_PROMPT = '0'
    $env:GCM_INTERACTIVE = 'never'
    Set-Location -LiteralPath $repo
    $branch = git branch --show-current
    if ([string]::IsNullOrWhiteSpace($branch)) { $branch = 'master' }

    git push -u origin $branch | Out-Null
    if ($LASTEXITCODE -eq 0) {
        Write-Log "已成功推送到 origin/$branch。"
    } else {
        Write-Log "推送到 origin/$branch 失败，可能需要先登录 GitHub。"
    }
}

function Invoke-GitSync {
    Set-Location -LiteralPath $repo
    $status = git status --porcelain
    if (-not $status) {
        Write-Log '没有检测到需要提交的文件，准备检查是否有未推送的提交。'
        Push-CurrentBranch
        return
    }

    git add -A | Out-Null
    $statusAfterAdd = git status --porcelain
    if (-not $statusAfterAdd) {
        Write-Log '只检测到被忽略的文件变化，无需提交。'
        Push-CurrentBranch
        return
    }

    $message = '自动同步 ' + (Get-Date -Format 'yyyy-MM-dd HH:mm:ss')
    git commit -m $message | Out-Null
    if ($LASTEXITCODE -ne 0) {
        Write-Log '提交失败。'
        return
    }

    Write-Log "已创建提交：$message"
    Push-CurrentBranch
}

Write-Log 'Git 自动同步监听脚本已启动。'
$watcher = New-Object System.IO.FileSystemWatcher
$watcher.Path = $repo
$watcher.IncludeSubdirectories = $true
$watcher.EnableRaisingEvents = $true

$action = {
    if (-not (Should-IgnorePath $Event.SourceEventArgs.FullPath)) {
        $global:pending = $true
        $global:lastEvent = Get-Date
        Write-Log "检测到文件变化：$($Event.SourceEventArgs.FullPath)"
    }
}

Register-ObjectEvent $watcher Created -Action $action | Out-Null
Register-ObjectEvent $watcher Changed -Action $action | Out-Null
Register-ObjectEvent $watcher Deleted -Action $action | Out-Null
Register-ObjectEvent $watcher Renamed -Action $action | Out-Null

Invoke-GitSync

while ($true) {
    Start-Sleep -Seconds 5
    if ($global:pending -and ((Get-Date) - $global:lastEvent).TotalSeconds -ge 10) {
        $global:pending = $false
        Write-Log '文件变化已稳定，开始自动同步。'
        Invoke-GitSync
    }
}




