<#
.SYNOPSIS
    Opt-in live multi-client PlayFab Multiplayer lobby coverage.

.DESCRIPTION
    Starts three headless Godot worker processes, signs each into PlayFab with
    a generated custom id, then exercises lobby create/search/join/update/leave
    and owner-migration flows. Matchmaking smoke is attempted only when
    PLAYFAB_MULTIPLAYER_MATCH_QUEUE is set.
#>
[CmdletBinding()]
param(
    [string]$RepoRoot = '.',
    [Parameter(Mandatory = $true)][string]$GodotExe,
    [string]$OutDir = 'build\test-results',
    [int]$TimeoutSec = 180,
    [switch]$VerboseOutput
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

$RepoRoot = [System.IO.Path]::GetFullPath($RepoRoot)
$OutDirAbsolute = if ([System.IO.Path]::IsPathRooted($OutDir)) {
    $OutDir
} else {
    Join-Path $RepoRoot $OutDir
}
$WorkerProject = Join-Path $RepoRoot 'tests\godot\playfab_multiplayer_worker'
$WorkerScript = Join-Path $WorkerProject 'worker.gd'
$TitleId = [Environment]::GetEnvironmentVariable('PLAYFAB_TITLE_ID')
if ($null -eq $TitleId) { $TitleId = '' }
$TitleId = $TitleId.Trim()
$Endpoint = [Environment]::GetEnvironmentVariable('PLAYFAB_ENDPOINT')
if ($null -eq $Endpoint) { $Endpoint = '' }
$Endpoint = $Endpoint.Trim()
$MatchmakingQueue = [Environment]::GetEnvironmentVariable('PLAYFAB_MULTIPLAYER_MATCH_QUEUE')
if ($null -eq $MatchmakingQueue) { $MatchmakingQueue = '' }
$MatchmakingQueue = $MatchmakingQueue.Trim()
$ConfiguredCustomId = [Environment]::GetEnvironmentVariable('PLAYFAB_CUSTOM_ID')
if ($null -eq $ConfiguredCustomId) { $ConfiguredCustomId = '' }
$ConfiguredCustomId = $ConfiguredCustomId.Trim()
$WorkerCustomIdPrefix = [Environment]::GetEnvironmentVariable('PLAYFAB_MULTIPLAYER_CUSTOM_ID_PREFIX')
if ($null -eq $WorkerCustomIdPrefix) { $WorkerCustomIdPrefix = '' }
$WorkerCustomIdPrefix = $WorkerCustomIdPrefix.Trim()

if ([string]::IsNullOrWhiteSpace($TitleId)) {
    Write-Host 'SKIP: PLAYFAB_TITLE_ID is not set; live PlayFab Multiplayer orchestration skipped.'
    exit 0
}
if (-not (Test-Path $WorkerScript)) {
    throw "Worker script not found: $WorkerScript"
}
if (-not (Test-Path (Join-Path $WorkerProject 'addons\godot_playfab\godot_playfab.gdextension'))) {
    throw "godot_playfab addon is not mirrored into $WorkerProject. Run the CMake build first."
}

$RunId = ([guid]::NewGuid().ToString('N')).Substring(0, 12)
$RunDir = Join-Path $OutDirAbsolute "playfab-multiplayer-live\$RunId"
$PidFile = Join-Path $OutDirAbsolute 'playfab-multiplayer-pids.txt'
New-Item -ItemType Directory -Force -Path $RunDir | Out-Null
New-Item -ItemType Directory -Force -Path (Split-Path -Parent $PidFile) | Out-Null

$script:SeqByWorker = @{}
$script:Workers = @()
$script:PassedScenarios = [System.Collections.Generic.List[string]]::new()

function ConvertTo-JsonFile {
    param(
        [Parameter(Mandatory = $true)][string]$Path,
        [Parameter(Mandatory = $true)][hashtable]$Payload
    )
    $temp = "$Path.tmp"
    $Payload | ConvertTo-Json -Depth 20 | Set-Content -Path $temp -Encoding UTF8
    Move-Item -Path $temp -Destination $Path -Force
}

function Read-JsonFile {
    param([Parameter(Mandatory = $true)][string]$Path)
    return (Get-Content -Path $Path -Raw | ConvertFrom-Json)
}

function Start-PlayFabWorker {
    param([Parameter(Mandatory = $true)][string]$WorkerId)

    $psi = [System.Diagnostics.ProcessStartInfo]::new()
    $psi.FileName = $GodotExe
    $psi.WorkingDirectory = $WorkerProject
    $psi.UseShellExecute = $false
    $psi.RedirectStandardOutput = $true
    $psi.RedirectStandardError = $true
    $psi.CreateNoWindow = $true
    foreach ($arg in @('--headless', '--script', 'res://worker.gd', '--', '--worker-id', $WorkerId, '--run-dir', $RunDir)) {
        [void]$psi.ArgumentList.Add($arg)
    }
    foreach ($entry in [Environment]::GetEnvironmentVariables().GetEnumerator()) {
        $psi.EnvironmentVariables[[string]$entry.Key] = [string]$entry.Value
    }
    $psi.EnvironmentVariables.Remove('PLAYFAB_DEVELOPER_SECRET_KEY')
    $psi.EnvironmentVariables['PLAYFAB_TITLE_ID'] = $TitleId
    if (-not [string]::IsNullOrWhiteSpace($Endpoint)) {
        $psi.EnvironmentVariables['PLAYFAB_ENDPOINT'] = $Endpoint
    }

    $proc = [System.Diagnostics.Process]::new()
    $proc.StartInfo = $psi
    [void]$proc.Start()
    $stdoutTask = $proc.StandardOutput.ReadToEndAsync()
    $stderrTask = $proc.StandardError.ReadToEndAsync()

    $record = [ordered]@{
        pid            = $proc.Id
        worker_id      = $WorkerId
        worker_project = $WorkerProject
        run_dir        = $RunDir
        started_at     = (Get-Date).ToUniversalTime().ToString('o')
    }
    Add-Content -Path $PidFile -Value ($record | ConvertTo-Json -Compress) -Encoding UTF8

    $worker = [pscustomobject]@{
        Id         = $WorkerId
        Process    = $proc
        StdoutTask = $stdoutTask
        StderrTask = $stderrTask
        Record     = $record
    }
    $script:Workers += $worker
    $script:SeqByWorker[$WorkerId] = 0
    return $worker
}

function Wait-WorkerReady {
    param([Parameter(Mandatory = $true)]$Worker)

    $readyPath = Join-Path $RunDir "$($Worker.Id).ready.json"
    $deadline = (Get-Date).AddSeconds([Math]::Min($TimeoutSec, 60))
    while ((Get-Date) -lt $deadline) {
        if (Test-Path $readyPath) {
            return Read-JsonFile -Path $readyPath
        }
        if ($Worker.Process.HasExited) {
            throw "Worker '$($Worker.Id)' exited before ready. stdout=$($Worker.StdoutTask.Result) stderr=$($Worker.StderrTask.Result)"
        }
        Start-Sleep -Milliseconds 100
    }
    throw "Timed out waiting for worker '$($Worker.Id)' to become ready."
}

function Invoke-WorkerCommand {
    param(
        [Parameter(Mandatory = $true)]$Worker,
        [Parameter(Mandatory = $true)][string]$Op,
        [hashtable]$Payload = @{},
        [int]$CommandTimeoutSec = $TimeoutSec,
        [switch]$AllowFailure
    )

    $seq = [int]$script:SeqByWorker[$Worker.Id] + 1
    $script:SeqByWorker[$Worker.Id] = $seq
    $commandPath = Join-Path $RunDir "$($Worker.Id).command.json"
    $responsePath = Join-Path $RunDir "$($Worker.Id).response.$seq.json"
    if (Test-Path $responsePath) {
        Remove-Item -Path $responsePath -Force
    }

    $command = @{} + $Payload
    $command['op'] = $Op
    $command['seq'] = $seq
    if ($AllowFailure) {
        $command['allow_error'] = $true
    }
    ConvertTo-JsonFile -Path $commandPath -Payload $command

    $deadline = (Get-Date).AddSeconds($CommandTimeoutSec)
    while ((Get-Date) -lt $deadline) {
        if (Test-Path $responsePath) {
            $response = Read-JsonFile -Path $responsePath
            Remove-Item -Path $responsePath -Force -ErrorAction SilentlyContinue
            if (-not [bool]$response.ok -and -not $AllowFailure) {
                $details = $response | ConvertTo-Json -Depth 20
                throw "Worker '$($Worker.Id)' op '$Op' failed: $details"
            }
            return $response
        }
        if ($Worker.Process.HasExited) {
            throw "Worker '$($Worker.Id)' exited during op '$Op'. stdout=$($Worker.StdoutTask.Result) stderr=$($Worker.StderrTask.Result)"
        }
        Start-Sleep -Milliseconds 100
    }
    throw "Timed out waiting for worker '$($Worker.Id)' op '$Op'."
}

function Stop-WorkerSafely {
    param([Parameter(Mandatory = $true)]$Worker)

    try {
        if (-not $Worker.Process.HasExited) {
            try {
                Invoke-WorkerCommand -Worker $Worker -Op 'exit' -CommandTimeoutSec 20 | Out-Null
            } catch {
                Write-Warning "Graceful exit failed for worker '$($Worker.Id)': $($_.Exception.Message)"
            }
        }
        if (-not $Worker.Process.WaitForExit(5000)) {
            $running = Get-Process -Id $Worker.Process.Id -ErrorAction SilentlyContinue
            if ($null -ne $running -and [int]$Worker.Record.pid -eq [int]$Worker.Process.Id -and [string]$Worker.Record.worker_project -eq $WorkerProject -and [string]$Worker.Record.run_dir -eq $RunDir) {
                Stop-Process -Id $Worker.Process.Id -Force
            }
        }
    } finally {
        if ($VerboseOutput) {
            Write-Host "[$($Worker.Id) stdout]"
            Write-Host $Worker.StdoutTask.Result
            Write-Host "[$($Worker.Id) stderr]"
            Write-Host $Worker.StderrTask.Result
        }
    }
}

function Wait-LobbySearchResult {
    param(
        [Parameter(Mandatory = $true)]$Worker,
        [Parameter(Mandatory = $true)][string]$Filter,
        [Parameter(Mandatory = $true)][string]$LobbyId
    )

    $deadline = (Get-Date).AddSeconds(45)
    do {
        $search = Invoke-WorkerCommand -Worker $Worker -Op 'search_lobbies' -Payload @{
            filter = $Filter
            max_results = 10
        }
        foreach ($summary in @($search.data.lobbies)) {
            if ([string]$summary.lobby_id -eq $LobbyId) {
                return $summary
            }
        }
        Start-Sleep -Seconds 2
    } while ((Get-Date) -lt $deadline)
    throw "Lobby '$LobbyId' did not appear in search results for filter '$Filter'."
}

function Complete-Scenario {
    param([Parameter(Mandatory = $true)][string]$Name)
    [void]$script:PassedScenarios.Add($Name)
}

function Assert-Condition {
    param(
        [Parameter(Mandatory = $true)][bool]$Condition,
        [Parameter(Mandatory = $true)][string]$Message
    )
    if (-not $Condition) {
        throw $Message
    }
}

function Get-JsonPropertyValue {
    param(
        $Object,
        [Parameter(Mandatory = $true)][string]$Name
    )
    if ($null -eq $Object) {
        return $null
    }
    $property = $Object.PSObject.Properties[$Name]
    if ($null -eq $property) {
        return $null
    }
    return $property.Value
}

function Invoke-WorkerSignIn {
    param(
        [Parameter(Mandatory = $true)]$Worker,
        [Parameter(Mandatory = $true)][string]$CustomId,
        [Parameter(Mandatory = $true)][bool]$CreateAccount
    )

    $maxAttempts = 6
    for ($attempt = 1; $attempt -le $maxAttempts; $attempt++) {
        $response = Invoke-WorkerCommand -Worker $Worker -Op 'sign_in' -Payload @{
            custom_id      = $CustomId
            create_account = $CreateAccount
        } -AllowFailure

        if ([bool]$response.ok) {
            return $response
        }

        $message = [string](Get-JsonPropertyValue -Object $response.data -Name 'message')
        if ($message -notmatch '0x892354DD' -or $attempt -eq $maxAttempts) {
            $details = $response | ConvertTo-Json -Depth 20
            throw "Worker '$($Worker.Id)' op 'sign_in' failed: $details"
        }

        $delaySec = [Math]::Min(30, 3 * $attempt)
        Write-Host "PlayFab sign-in for worker '$($Worker.Id)' hit client request rate limiting; retrying in $delaySec second(s)."
        Start-Sleep -Seconds $delaySec
    }

    throw "Worker '$($Worker.Id)' op 'sign_in' exhausted retry attempts."
}

function Assert-LobbySearchDoesNotFind {
    param(
        [Parameter(Mandatory = $true)]$Worker,
        [Parameter(Mandatory = $true)][string]$Filter,
        [Parameter(Mandatory = $true)][string]$LobbyId,
        [int]$Attempts = 3,
        [int]$DelaySec = 2
    )

    for ($attempt = 0; $attempt -lt $Attempts; ++$attempt) {
        $search = Invoke-WorkerCommand -Worker $Worker -Op 'search_lobbies' -Payload @{
            filter = $Filter
            max_results = 10
        }
        foreach ($summary in @($search.data.lobbies)) {
            if ([string]$summary.lobby_id -eq $LobbyId) {
                throw "Lobby '$LobbyId' unexpectedly appeared in search results for filter '$Filter'."
            }
        }
        if ($attempt -lt ($Attempts - 1)) {
            Start-Sleep -Seconds $DelaySec
        }
    }
}

function Get-LobbySnapshot {
    param(
        [Parameter(Mandatory = $true)]$Worker,
        [string]$LobbyId = ''
    )
    $payload = @{}
    if (-not [string]::IsNullOrWhiteSpace($LobbyId)) {
        $payload['lobby_id'] = $LobbyId
    }
    $inspect = Invoke-WorkerCommand -Worker $Worker -Op 'inspect_lobby' -Payload $payload
    return $inspect.data.lobby
}

function Wait-LobbyMemberCount {
    param(
        [Parameter(Mandatory = $true)]$Worker,
        [string]$LobbyId = '',
        [int]$ExpectedCount = 2,
        [switch]$Exact
    )

    $deadline = (Get-Date).AddSeconds(45)
    do {
        $lobby = Get-LobbySnapshot -Worker $Worker -LobbyId $LobbyId
        $count = [int]$lobby.member_count
        if (($Exact -and $count -eq $ExpectedCount) -or (-not $Exact -and $count -ge $ExpectedCount)) {
            return $lobby
        }
        Start-Sleep -Seconds 1
    } while ((Get-Date) -lt $deadline)
    $operator = if ($Exact) { '==' } else { '>=' }
    throw "Lobby did not reach member_count $operator $ExpectedCount."
}

function Wait-LobbyProperty {
    param(
        [Parameter(Mandatory = $true)]$Worker,
        [string]$LobbyId = '',
        [Parameter(Mandatory = $true)][string]$Key,
        [Parameter(Mandatory = $true)][string]$ExpectedValue
    )

    $deadline = (Get-Date).AddSeconds(45)
    do {
        $lobby = Get-LobbySnapshot -Worker $Worker -LobbyId $LobbyId
        $actual = Get-JsonPropertyValue -Object $lobby.properties -Name $Key
        if ([string]$actual -eq $ExpectedValue) {
            return $lobby
        }
        Start-Sleep -Seconds 1
    } while ((Get-Date) -lt $deadline)
    throw "Lobby property '$Key' did not become '$ExpectedValue'."
}

function Wait-LobbyMemberProperty {
    param(
        [Parameter(Mandatory = $true)]$Worker,
        [string]$LobbyId = '',
        [Parameter(Mandatory = $true)][string]$MatchKey,
        [Parameter(Mandatory = $true)][string]$MatchValue,
        [Parameter(Mandatory = $true)][string]$ExpectedKey,
        [Parameter(Mandatory = $true)][string]$ExpectedValue
    )

    $deadline = (Get-Date).AddSeconds(45)
    do {
        $lobby = Get-LobbySnapshot -Worker $Worker -LobbyId $LobbyId
        foreach ($member in @($lobby.members)) {
            $matched = Get-JsonPropertyValue -Object $member.properties -Name $MatchKey
            $actual = Get-JsonPropertyValue -Object $member.properties -Name $ExpectedKey
            if ([string]$matched -eq $MatchValue -and [string]$actual -eq $ExpectedValue) {
                return $lobby
            }
        }
        Start-Sleep -Seconds 1
    } while ((Get-Date) -lt $deadline)
    throw "No lobby member with '$MatchKey=$MatchValue' reached '$ExpectedKey=$ExpectedValue'."
}

function Wait-LobbyOwner {
    param(
        [Parameter(Mandatory = $true)]$Worker,
        [string]$LobbyId = '',
        [Parameter(Mandatory = $true)][string]$ExpectedEntityId
    )

    $deadline = (Get-Date).AddSeconds(45)
    do {
        $lobby = Get-LobbySnapshot -Worker $Worker -LobbyId $LobbyId
        $ownerId = Get-JsonPropertyValue -Object $lobby.owner_entity_key -Name 'id'
        if ([string]$ownerId -eq $ExpectedEntityId) {
            return $lobby
        }
        Start-Sleep -Seconds 1
    } while ((Get-Date) -lt $deadline)
    throw "Lobby owner did not migrate to expected entity id '$ExpectedEntityId'."
}

function Assert-HasLocalMember {
    param(
        [Parameter(Mandatory = $true)]$Lobby,
        [Parameter(Mandatory = $true)][string]$Label
    )
    Get-LocalMember -Lobby $Lobby -Label $Label | Out-Null
}

function Get-LocalMember {
    param(
        [Parameter(Mandatory = $true)]$Lobby,
        [Parameter(Mandatory = $true)][string]$Label
    )
    foreach ($member in @($Lobby.members)) {
        if ([bool]$member.is_local) {
            return $member
        }
    }
    throw "$Label did not report any local lobby member."
}

function Assert-LobbyListContains {
    param(
        [Parameter(Mandatory = $true)]$Worker,
        [Parameter(Mandatory = $true)][string]$LobbyId,
        [Parameter(Mandatory = $true)][bool]$Expected
    )
    $inspect = Invoke-WorkerCommand -Worker $Worker -Op 'inspect_lobbies'
    foreach ($lobby in @($inspect.data.lobbies)) {
        if ([string]$lobby.lobby_id -eq $LobbyId) {
            Assert-Condition -Condition $Expected -Message "Lobby '$LobbyId' unexpectedly remained tracked."
            return
        }
    }
    Assert-Condition -Condition (-not $Expected) -Message "Lobby '$LobbyId' was not tracked."
}

function Wait-MatchTicketStatus {
    param(
        [Parameter(Mandatory = $true)]$Worker,
        [Parameter(Mandatory = $true)][string]$StatusName,
        [int]$WaitTimeoutSec = 180
    )

    $wait = Invoke-WorkerCommand -Worker $Worker -Op 'wait_match_ticket' -Payload @{
        status_name  = $StatusName
        timeout_msec = $WaitTimeoutSec * 1000
    } -CommandTimeoutSec ($WaitTimeoutSec + 15)
    return $wait.data.ticket
}

function Assert-NoTrackedLobbies {
    param(
        [Parameter(Mandatory = $true)]$Worker,
        [Parameter(Mandatory = $true)][string]$Label
    )

    $inspect = Invoke-WorkerCommand -Worker $Worker -Op 'inspect_lobbies'
    $lobbiesValue = Get-JsonPropertyValue -Object $inspect.data -Name 'lobbies'
    [object[]]$lobbies = @()
    if ($null -ne $lobbiesValue) {
        $lobbies = @($lobbiesValue)
    }
    Assert-Condition -Condition ($lobbies.Count -eq 0) -Message "$Label unexpectedly had $($lobbies.Count) tracked lobby/lobbies."
}

try {
    Write-Host "PlayFab Multiplayer live run $RunId using title $TitleId"

    $hostWorker = Start-PlayFabWorker -WorkerId 'host'
    $clientWorker = Start-PlayFabWorker -WorkerId 'client'
    $observerWorker = Start-PlayFabWorker -WorkerId 'observer'
    Wait-WorkerReady -Worker $hostWorker | Out-Null
    Wait-WorkerReady -Worker $clientWorker | Out-Null
    Wait-WorkerReady -Worker $observerWorker | Out-Null

    $customPrefix = "gdk-pfmp-$RunId"
    $createWorkerAccounts = $true
    if (-not [string]::IsNullOrWhiteSpace($WorkerCustomIdPrefix)) {
        $customPrefix = $WorkerCustomIdPrefix
        $createWorkerAccounts = $false
    } elseif (-not [string]::IsNullOrWhiteSpace($ConfiguredCustomId)) {
        $customPrefix = "$ConfiguredCustomId-multiplayer"
        $createWorkerAccounts = $false
    }
    $hostSignIn = Invoke-WorkerSignIn -Worker $hostWorker -CustomId "$customPrefix-host" -CreateAccount $createWorkerAccounts
    $clientSignIn = Invoke-WorkerSignIn -Worker $clientWorker -CustomId "$customPrefix-client" -CreateAccount $createWorkerAccounts
    $observerSignIn = Invoke-WorkerSignIn -Worker $observerWorker -CustomId "$customPrefix-observer" -CreateAccount $createWorkerAccounts
    $hostEntityId = [string](Get-JsonPropertyValue -Object $hostSignIn.data.entity_key -Name 'id')
    $clientEntityId = [string](Get-JsonPropertyValue -Object $clientSignIn.data.entity_key -Name 'id')
    $observerEntityId = [string](Get-JsonPropertyValue -Object $observerSignIn.data.entity_key -Name 'id')

    $create = Invoke-WorkerCommand -Worker $hostWorker -Op 'create_lobby' -Payload @{
        config = @{
            max_players = 4
            access_policy = 0
            search_properties = @{
                string_key1 = $RunId
                string_key2 = 'live_lobby'
            }
            lobby_properties = @{
                run_id = $RunId
                phase = 'created'
            }
            member_properties = @{
                role = 'host'
            }
        }
    }
    $lobbyId = [string]$create.data.lobby.lobby_id
    $connectionString = [string]$create.data.lobby.connection_string
    if ([string]::IsNullOrWhiteSpace($lobbyId) -or [string]::IsNullOrWhiteSpace($connectionString)) {
        throw "Created lobby did not return both lobby_id and connection_string."
    }
    Assert-Condition -Condition ([string](Get-JsonPropertyValue -Object $create.data.lobby.owner_entity_key -Name 'id') -eq $hostEntityId) -Message 'Created public lobby owner did not match host entity.'
    Complete-Scenario 'public lobby create snapshot'

    $missingFilter = "string_key1 eq '$RunId-missing'"
    Assert-LobbySearchDoesNotFind -Worker $clientWorker -Filter $missingFilter -LobbyId $lobbyId -Attempts 1
    Complete-Scenario 'search no-results isolation'

    $privateRunId = "$RunId-private"
    $privateCreate = Invoke-WorkerCommand -Worker $hostWorker -Op 'create_lobby' -Payload @{
        make_primary = $false
        config = @{
            max_players = 2
            access_policy = 2
            search_properties = @{
                string_key1 = $privateRunId
                string_key2 = 'private_lobby'
            }
            lobby_properties = @{
                run_id = $RunId
                phase = 'private'
            }
            member_properties = @{
                role = 'host-private'
            }
        }
    }
    $privateLobbyId = [string]$privateCreate.data.lobby.lobby_id
    Assert-Condition -Condition (-not [string]::IsNullOrWhiteSpace($privateLobbyId) -and $privateLobbyId -ne $lobbyId) -Message 'Private lobby did not return a distinct lobby id.'
    Assert-LobbyListContains -Worker $hostWorker -LobbyId $lobbyId -Expected $true
    Assert-LobbyListContains -Worker $hostWorker -LobbyId $privateLobbyId -Expected $true
    Complete-Scenario 'multiple lobby tracking'

    $privateFilter = "string_key1 eq '$privateRunId'"
    Assert-LobbySearchDoesNotFind -Worker $clientWorker -Filter $privateFilter -LobbyId $privateLobbyId
    Invoke-WorkerCommand -Worker $hostWorker -Op 'leave_lobby' -Payload @{ lobby_id = $privateLobbyId } | Out-Null
    Assert-LobbyListContains -Worker $hostWorker -LobbyId $privateLobbyId -Expected $false
    Complete-Scenario 'private lobby not searchable'

    $invalidJoin = Invoke-WorkerCommand -Worker $clientWorker -Op 'join_lobby' -Payload @{
        connection_string = 'not-a-valid-playfab-lobby-connection-string'
    } -AllowFailure
    Assert-Condition -Condition (-not [bool]$invalidJoin.ok) -Message 'Invalid lobby connection string unexpectedly succeeded.'
    $clientTracked = Invoke-WorkerCommand -Worker $clientWorker -Op 'inspect_lobbies'
    Assert-Condition -Condition (@($clientTracked.data.lobbies).Count -eq 0) -Message 'Invalid join left a tracked lobby in the client worker.'
    Complete-Scenario 'invalid connection string typed failure'

    $filter = "string_key1 eq '$RunId'"
    Wait-LobbySearchResult -Worker $clientWorker -Filter $filter -LobbyId $lobbyId | Out-Null
    Complete-Scenario 'public lobby search by string key'

    Invoke-WorkerCommand -Worker $clientWorker -Op 'join_lobby' -Payload @{
        connection_string = $connectionString
        member_properties = @{ role = 'client' }
    } | Out-Null
    Complete-Scenario 'client join by connection string'

    Invoke-WorkerCommand -Worker $observerWorker -Op 'join_lobby' -Payload @{
        connection_string = $connectionString
        member_properties = @{ role = 'observer' }
    } | Out-Null
    $hostLobby = Wait-LobbyMemberCount -Worker $hostWorker -LobbyId $lobbyId -ExpectedCount 3
    $clientLobby = Wait-LobbyMemberCount -Worker $clientWorker -ExpectedCount 3
    $observerLobby = Wait-LobbyMemberCount -Worker $observerWorker -ExpectedCount 3
    Assert-HasLocalMember -Lobby $clientLobby -Label 'client lobby snapshot'
    Assert-HasLocalMember -Lobby $observerLobby -Label 'observer lobby snapshot'
    Complete-Scenario 'three-client membership snapshots'

    Invoke-WorkerCommand -Worker $clientWorker -Op 'set_member_properties' -Payload @{
        properties = @{ role = 'client'; ready = 'true' }
    } | Out-Null
    Wait-LobbyMemberProperty -Worker $hostWorker -LobbyId $lobbyId -MatchKey 'role' -MatchValue 'client' -ExpectedKey 'ready' -ExpectedValue 'true' | Out-Null
    Complete-Scenario 'member property propagation'

    Invoke-WorkerCommand -Worker $hostWorker -Op 'set_lobby_properties' -Payload @{
        lobby_id = $lobbyId
        properties = @{ run_id = $RunId; phase = 'joined'; mode = 'expanded' }
    } | Out-Null
    Wait-LobbyProperty -Worker $clientWorker -Key 'phase' -ExpectedValue 'joined' | Out-Null
    Wait-LobbyProperty -Worker $observerWorker -Key 'mode' -ExpectedValue 'expanded' | Out-Null
    Complete-Scenario 'lobby property propagation'

    Invoke-WorkerCommand -Worker $observerWorker -Op 'leave_lobby' | Out-Null
    $hostLobby = Wait-LobbyMemberCount -Worker $hostWorker -LobbyId $lobbyId -ExpectedCount 2 -Exact
    Wait-LobbyMemberCount -Worker $clientWorker -ExpectedCount 2 -Exact | Out-Null
    Complete-Scenario 'third member leave propagation'

    Invoke-WorkerCommand -Worker $clientWorker -Op 'leave_lobby' | Out-Null
    $hostLobby = Wait-LobbyMemberCount -Worker $hostWorker -LobbyId $lobbyId -ExpectedCount 1 -Exact
    Complete-Scenario 'client leave propagation'

    Invoke-WorkerCommand -Worker $clientWorker -Op 'join_lobby' -Payload @{
        connection_string = $connectionString
        member_properties = @{ role = 'client-returned'; state = 'returned' }
    } | Out-Null
    Wait-LobbyMemberCount -Worker $hostWorker -LobbyId $lobbyId -ExpectedCount 2 -Exact | Out-Null
    Wait-LobbyMemberProperty -Worker $hostWorker -LobbyId $lobbyId -MatchKey 'role' -MatchValue 'client-returned' -ExpectedKey 'state' -ExpectedValue 'returned' | Out-Null
    Complete-Scenario 'rejoin after leave'

    Invoke-WorkerCommand -Worker $hostWorker -Op 'leave_lobby' -Payload @{ lobby_id = $lobbyId } | Out-Null
    $clientLobby = Wait-LobbyMemberCount -Worker $clientWorker -ExpectedCount 1 -Exact
    Wait-LobbyOwner -Worker $clientWorker -ExpectedEntityId $clientEntityId | Out-Null
    Complete-Scenario 'owner migration after host leave'

    $matchmakingMessage = 'matchmaking skipped: PLAYFAB_MULTIPLAYER_MATCH_QUEUE is unset'
    if (-not [string]::IsNullOrWhiteSpace($MatchmakingQueue)) {
        Invoke-WorkerCommand -Worker $hostWorker -Op 'create_match_ticket' -Payload @{
            queue_name = $MatchmakingQueue
            timeout_seconds = 60
            attributes = @{ scenario = 'smoke'; run_id = $RunId }
        } | Out-Null
        Invoke-WorkerCommand -Worker $hostWorker -Op 'cancel_match_ticket' -CommandTimeoutSec 90 | Out-Null
        Complete-Scenario 'match ticket create and cancel'

        Assert-NoTrackedLobbies -Worker $hostWorker -Label 'host before arranged-lobby join'
        Assert-NoTrackedLobbies -Worker $observerWorker -Label 'observer before arranged-lobby join'

        Invoke-WorkerCommand -Worker $hostWorker -Op 'create_match_ticket' -Payload @{
            queue_name = $MatchmakingQueue
            timeout_seconds = 120
            attributes = @{ scenario = 'paired_match'; run_id = $RunId }
        } | Out-Null
        Invoke-WorkerCommand -Worker $observerWorker -Op 'create_match_ticket' -Payload @{
            queue_name = $MatchmakingQueue
            timeout_seconds = 120
            attributes = @{ scenario = 'paired_match'; run_id = $RunId }
        } | Out-Null

        $hostTicket = Wait-MatchTicketStatus -Worker $hostWorker -StatusName 'matched' -WaitTimeoutSec 180
        $observerTicket = Wait-MatchTicketStatus -Worker $observerWorker -StatusName 'matched' -WaitTimeoutSec 180
        Assert-Condition -Condition (-not [string]::IsNullOrWhiteSpace([string]$hostTicket.match_id)) -Message 'Host matched ticket did not include a match_id.'
        Assert-Condition -Condition ([string]$hostTicket.match_id -eq [string]$observerTicket.match_id) -Message "Matched tickets used different match_id values ('$($hostTicket.match_id)' vs '$($observerTicket.match_id)')."
        Assert-Condition -Condition (-not [string]::IsNullOrWhiteSpace([string]$hostTicket.arranged_lobby_connection_string)) -Message 'Host matched ticket did not include an arranged lobby connection string.'
        Assert-Condition -Condition (-not [string]::IsNullOrWhiteSpace([string]$observerTicket.arranged_lobby_connection_string)) -Message 'Observer matched ticket did not include an arranged lobby connection string.'
        Complete-Scenario 'two-player match completion'

        Assert-NoTrackedLobbies -Worker $hostWorker -Label 'host after match completion'
        Assert-NoTrackedLobbies -Worker $observerWorker -Label 'observer after match completion'

        $arrangedHost = Invoke-WorkerCommand -Worker $hostWorker -Op 'join_arranged_lobby' -Payload @{
            connection_string = [string]$hostTicket.arranged_lobby_connection_string
            member_properties = @{ role = 'match-host'; run_id = $RunId }
        } -CommandTimeoutSec 120
        $arrangedObserver = Invoke-WorkerCommand -Worker $observerWorker -Op 'join_arranged_lobby' -Payload @{
            connection_string = [string]$observerTicket.arranged_lobby_connection_string
            member_properties = @{ role = 'match-observer'; run_id = $RunId }
        } -CommandTimeoutSec 120

        $arrangedHostLobby = $arrangedHost.data.lobby
        $arrangedObserverLobby = $arrangedObserver.data.lobby
        Assert-Condition -Condition (-not [string]::IsNullOrWhiteSpace([string]$arrangedHostLobby.lobby_id)) -Message 'Host arranged-lobby join did not return a lobby_id.'
        Assert-Condition -Condition ([string]$arrangedHostLobby.lobby_id -eq [string]$arrangedObserverLobby.lobby_id) -Message "Arranged-lobby joins returned different lobby IDs ('$($arrangedHostLobby.lobby_id)' vs '$($arrangedObserverLobby.lobby_id)')."
        $arrangedHostLobby = Wait-LobbyMemberCount -Worker $hostWorker -LobbyId ([string]$arrangedHostLobby.lobby_id) -ExpectedCount 2 -Exact
        $arrangedObserverLobby = Wait-LobbyMemberCount -Worker $observerWorker -LobbyId ([string]$arrangedObserverLobby.lobby_id) -ExpectedCount 2 -Exact
        $arrangedHostLocalMember = Get-LocalMember -Lobby $arrangedHostLobby -Label 'host arranged-lobby snapshot'
        $arrangedObserverLocalMember = Get-LocalMember -Lobby $arrangedObserverLobby -Label 'observer arranged-lobby snapshot'
        $arrangedHostLocalEntity = Get-JsonPropertyValue -Object (Get-JsonPropertyValue -Object $arrangedHostLocalMember -Name 'entity_key') -Name 'id'
        $arrangedObserverLocalEntity = Get-JsonPropertyValue -Object (Get-JsonPropertyValue -Object $arrangedObserverLocalMember -Name 'entity_key') -Name 'id'
        Assert-Condition -Condition ([string]$arrangedHostLocalEntity -eq $hostEntityId) -Message 'Host arranged-lobby snapshot did not include the host local member.'
        Assert-Condition -Condition ([string]$arrangedObserverLocalEntity -eq $observerEntityId) -Message 'Observer arranged-lobby snapshot did not include the observer local member.'
        Complete-Scenario 'explicit arranged-lobby join'

        Invoke-WorkerCommand -Worker $hostWorker -Op 'leave_lobby' -Payload @{ lobby_id = [string]$arrangedHostLobby.lobby_id } -CommandTimeoutSec 120 | Out-Null
        Invoke-WorkerCommand -Worker $observerWorker -Op 'leave_lobby' -Payload @{ lobby_id = [string]$arrangedObserverLobby.lobby_id } -CommandTimeoutSec 120 | Out-Null
        Assert-NoTrackedLobbies -Worker $hostWorker -Label 'host after arranged-lobby cleanup'
        Assert-NoTrackedLobbies -Worker $observerWorker -Label 'observer after arranged-lobby cleanup'
        Complete-Scenario 'arranged-lobby cleanup'

        $matchmakingMessage = "matchmaking create/cancel, two-ticket match, explicit arranged-lobby join, and cleanup completed for queue '$MatchmakingQueue'"
    }

    Invoke-WorkerCommand -Worker $clientWorker -Op 'leave_all' | Out-Null
    Invoke-WorkerCommand -Worker $observerWorker -Op 'leave_all' | Out-Null
    Invoke-WorkerCommand -Worker $hostWorker -Op 'leave_all' | Out-Null

    Write-Host "OK: $($script:PassedScenarios.Count) multiplayer scenarios passed for lobby $lobbyId; final client member_count=$($clientLobby.member_count); $matchmakingMessage. Scenarios: $($script:PassedScenarios -join '; ')"
    exit 0
} catch {
    Write-Error $_.Exception.Message
    exit 1
} finally {
    foreach ($worker in @($script:Workers)) {
        Stop-WorkerSafely -Worker $worker
    }
}
