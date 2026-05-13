<#
.SYNOPSIS
    Configures a PlayFab sandbox title for this repo's live tests.

.DESCRIPTION
    Reads a PlayFab developer secret key from the process, user, or machine
    environment, then idempotently prepares the online resources that the
    PlayFab live tests need:

      - a custom-ID account used by tests that sign in with create_account=false
      - three Multiplayer worker custom-ID accounts for host/client/observer
      - a PlayFab Matchmaking queue for Multiplayer live smoke coverage
      - the wave4_settle_smoke leaderboard definition
      - API-service fixtures for accounts, friends, player data,
        title data, publisher data, statistics, and catalog draft-item tests
      - a small title-data marker describing the configured test resources

    The secret key is read from the process, user, or machine environment. It
    is never printed, written to disk, or forwarded to Godot. PlayFab Lobby
    search keys do not need title configuration; the live Multiplayer tests use
    PlayFab's reserved string_keyN/number_keyN properties.
#>
[CmdletBinding()]
param(
    [string]$TitleId = '10D176',
    [string]$SecretKeyEnvVar = 'PLAYFAB_DEVELOPER_SECRET_KEY',
    [string]$CustomId = 'godot-gdk-ext-live-smoke',
    [string]$MultiplayerCustomIdPrefix = '',
    [string]$MatchmakingQueueName = 'godot_gdk_ext_live_smoke_queue',
    [string]$ServiceFixturePrefix = '',
    [string]$LeaderboardName = 'wave4_settle_smoke',
    [string]$ServiceStatisticName = 'godot_services_smoke_stat',
    [string]$ServiceCatalogItemId = 'godot-services-smoke-item',
    [string]$ServiceTitleDataKey = 'godot_services_smoke_title_data',
    [string]$ServicePublisherDataKey = 'godot_services_smoke_publisher_data',
    [string]$ServicePlayerDataKey = 'godot_services_smoke_player_data',
    [int]$LeaderboardSizeLimit = 1000,
    [switch]$SkipCustomIdAccount,
    [switch]$SkipMultiplayerWorkerAccounts,
    [switch]$SkipMultiplayerMatchmakingQueue,
    [switch]$SkipApiServiceFixtures,
    [switch]$SkipServiceAccounts,
    [switch]$SkipServicePlayerData,
    [switch]$SkipServiceTitleData,
    [switch]$SkipServiceStatistic,
    [switch]$SkipServiceCatalogDraftItem,
    [switch]$SkipTitleDataMarker
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

$TitleId = $TitleId.Trim()
$SecretKeyEnvVar = $SecretKeyEnvVar.Trim()
$CustomId = $CustomId.Trim()
$MultiplayerCustomIdPrefix = $MultiplayerCustomIdPrefix.Trim()
$MatchmakingQueueName = $MatchmakingQueueName.Trim()
$ServiceFixturePrefix = $ServiceFixturePrefix.Trim()
$LeaderboardName = $LeaderboardName.Trim()
$ServiceStatisticName = $ServiceStatisticName.Trim()
$ServiceCatalogItemId = $ServiceCatalogItemId.Trim()
$ServiceTitleDataKey = $ServiceTitleDataKey.Trim()
$ServicePublisherDataKey = $ServicePublisherDataKey.Trim()
$ServicePlayerDataKey = $ServicePlayerDataKey.Trim()

if ([string]::IsNullOrWhiteSpace($TitleId)) {
    throw 'TitleId must not be empty.'
}
if ([string]::IsNullOrWhiteSpace($SecretKeyEnvVar)) {
    throw 'SecretKeyEnvVar must not be empty.'
}
if (-not $SkipCustomIdAccount -and [string]::IsNullOrWhiteSpace($CustomId)) {
    throw 'CustomId must not be empty unless -SkipCustomIdAccount is specified.'
}
if ([string]::IsNullOrWhiteSpace($MultiplayerCustomIdPrefix) -and -not [string]::IsNullOrWhiteSpace($CustomId)) {
    $MultiplayerCustomIdPrefix = "$CustomId-multiplayer"
}
if ([string]::IsNullOrWhiteSpace($ServiceFixturePrefix) -and -not [string]::IsNullOrWhiteSpace($CustomId)) {
    $ServiceFixturePrefix = "$CustomId-services"
}
if (-not $SkipMultiplayerWorkerAccounts -and [string]::IsNullOrWhiteSpace($MultiplayerCustomIdPrefix)) {
    throw 'MultiplayerCustomIdPrefix must not be empty unless -SkipMultiplayerWorkerAccounts is specified.'
}
if (-not $SkipMultiplayerMatchmakingQueue) {
    if ([string]::IsNullOrWhiteSpace($MatchmakingQueueName)) {
        throw 'MatchmakingQueueName must not be empty unless -SkipMultiplayerMatchmakingQueue is specified.'
    }
    if ($MatchmakingQueueName -notmatch '^[A-Za-z0-9_]{1,64}$') {
        throw 'MatchmakingQueueName must contain only ASCII letters, digits, and underscores, with a maximum length of 64 characters.'
    }
}
if (-not $SkipApiServiceFixtures -and -not $SkipServiceAccounts -and [string]::IsNullOrWhiteSpace($ServiceFixturePrefix)) {
    throw 'ServiceFixturePrefix must not be empty unless -SkipApiServiceFixtures or -SkipServiceAccounts is specified.'
}
if ([string]::IsNullOrWhiteSpace($LeaderboardName)) {
    throw 'LeaderboardName must not be empty.'
}
if (-not $SkipApiServiceFixtures -and -not $SkipServiceStatistic -and [string]::IsNullOrWhiteSpace($ServiceStatisticName)) {
    throw 'ServiceStatisticName must not be empty unless -SkipApiServiceFixtures or -SkipServiceStatistic is specified.'
}
if (-not $SkipApiServiceFixtures -and -not $SkipServiceCatalogDraftItem -and [string]::IsNullOrWhiteSpace($ServiceCatalogItemId)) {
    throw 'ServiceCatalogItemId must not be empty unless -SkipApiServiceFixtures or -SkipServiceCatalogDraftItem is specified.'
}
if (-not $SkipApiServiceFixtures -and -not $SkipServiceTitleData -and [string]::IsNullOrWhiteSpace($ServiceTitleDataKey)) {
    throw 'ServiceTitleDataKey must not be empty unless -SkipApiServiceFixtures or -SkipServiceTitleData is specified.'
}
if (-not $SkipApiServiceFixtures -and -not $SkipServiceTitleData -and [string]::IsNullOrWhiteSpace($ServicePublisherDataKey)) {
    throw 'ServicePublisherDataKey must not be empty unless -SkipApiServiceFixtures or -SkipServiceTitleData is specified.'
}
if (-not $SkipApiServiceFixtures -and -not $SkipServicePlayerData -and [string]::IsNullOrWhiteSpace($ServicePlayerDataKey)) {
    throw 'ServicePlayerDataKey must not be empty unless -SkipApiServiceFixtures or -SkipServicePlayerData is specified.'
}
if ($LeaderboardSizeLimit -lt 1) {
    throw 'LeaderboardSizeLimit must be greater than zero.'
}

function Get-SecretEnvironmentVariable {
    param([Parameter(Mandatory = $true)][string]$Name)

    foreach ($target in @(
            [System.EnvironmentVariableTarget]::Process,
            [System.EnvironmentVariableTarget]::User,
            [System.EnvironmentVariableTarget]::Machine)) {
        $value = [Environment]::GetEnvironmentVariable($Name, $target)
        if (-not [string]::IsNullOrWhiteSpace($value)) {
            return $value.Trim()
        }
    }
    return ''
}

$SecretKey = Get-SecretEnvironmentVariable -Name $SecretKeyEnvVar
if ([string]::IsNullOrWhiteSpace($SecretKey)) {
    throw "Set $SecretKeyEnvVar to the PlayFab title developer secret key before running this script. The value is read from the process, user, or machine environment only and is not printed."
}

$script:TitleIdForRequests = $TitleId
$script:RequestTimeoutSec = 60

function Get-ObjectProperty {
    param(
        [AllowNull()]$Object,
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

function Set-ObjectProperty {
    param(
        [Parameter(Mandatory = $true)]$Object,
        [Parameter(Mandatory = $true)][string]$Name,
        $Value
    )

    if ($Object -is [hashtable]) {
        $Object[$Name] = $Value
        return
    }

    $property = $Object.PSObject.Properties[$Name]
    if ($null -ne $property) {
        $property.Value = $Value
        return
    }

    $Object | Add-Member -NotePropertyName $Name -NotePropertyValue $Value
}

function Get-PlayFabResponseData {
    param($Response)

    $data = Get-ObjectProperty -Object $Response -Name 'data'
    if ($null -ne $data) {
        return $data
    }
    return $Response
}

function Get-PlayFabRestError {
    param([Parameter(Mandatory = $true)]$ErrorRecord)

    $bodyText = $null
    if ($null -ne $ErrorRecord.ErrorDetails -and -not [string]::IsNullOrWhiteSpace($ErrorRecord.ErrorDetails.Message)) {
        $bodyText = $ErrorRecord.ErrorDetails.Message
    }

    $response = Get-ObjectProperty -Object $ErrorRecord.Exception -Name 'Response'
    if ([string]::IsNullOrWhiteSpace($bodyText) -and $null -ne $response) {
        try {
            $content = Get-ObjectProperty -Object $response -Name 'Content'
            if ($null -ne $content) {
                $bodyText = $content.ReadAsStringAsync().GetAwaiter().GetResult()
            } elseif ($response -is [System.Net.HttpWebResponse]) {
                $stream = $response.GetResponseStream()
                if ($null -ne $stream) {
                    $reader = [System.IO.StreamReader]::new($stream)
                    try {
                        $bodyText = $reader.ReadToEnd()
                    } finally {
                        $reader.Dispose()
                    }
                }
            }
        } catch {
            $bodyText = $null
        }
    }

    $statusCode = $null
    if ($null -ne $response) {
        try {
            $statusCodeValue = Get-ObjectProperty -Object $response -Name 'StatusCode'
            if ($null -ne $statusCodeValue) {
                $statusCode = [int]$statusCodeValue
            }
        } catch {
            $statusCode = $null
        }
    }

    $errorName = $null
    $errorCode = $null
    $errorMessage = $null
    if (-not [string]::IsNullOrWhiteSpace($bodyText)) {
        try {
            $json = $bodyText | ConvertFrom-Json
            $errorName = Get-ObjectProperty -Object $json -Name 'error'
            $errorCode = Get-ObjectProperty -Object $json -Name 'errorCode'
            $errorMessage = Get-ObjectProperty -Object $json -Name 'errorMessage'
            if ($null -eq $statusCode) {
                $statusCode = Get-ObjectProperty -Object $json -Name 'code'
            }
        } catch {
            $errorMessage = $bodyText
        }
    }

    if ([string]::IsNullOrWhiteSpace([string]$errorMessage)) {
        $errorMessage = $ErrorRecord.Exception.Message
    }

    return [pscustomobject]@{
        Error        = $errorName
        ErrorCode    = $errorCode
        ErrorMessage = $errorMessage
        StatusCode   = $statusCode
    }
}

function Format-PlayFabRestError {
    param([Parameter(Mandatory = $true)]$ErrorInfo)

    $parts = [System.Collections.Generic.List[string]]::new()
    if (-not [string]::IsNullOrWhiteSpace([string]$ErrorInfo.Error)) {
        $parts.Add([string]$ErrorInfo.Error)
    }
    if ($null -ne $ErrorInfo.ErrorCode) {
        $parts.Add("errorCode=$($ErrorInfo.ErrorCode)")
    }
    if ($null -ne $ErrorInfo.StatusCode) {
        $parts.Add("http=$($ErrorInfo.StatusCode)")
    }
    if (-not [string]::IsNullOrWhiteSpace([string]$ErrorInfo.ErrorMessage)) {
        $parts.Add([string]$ErrorInfo.ErrorMessage)
    }

    if ($parts.Count -eq 0) {
        return 'unknown PlayFab REST error'
    }
    return ($parts -join '; ')
}

function Invoke-PlayFabRest {
    param(
        [Parameter(Mandatory = $true)][string]$Route,
        [Parameter(Mandatory = $true)][hashtable]$Headers,
        [Parameter(Mandatory = $true)][hashtable]$Body
    )

    $uri = "https://$script:TitleIdForRequests.playfabapi.com/$Route"
    $jsonBody = $Body | ConvertTo-Json -Depth 20 -Compress

    try {
        $response = Invoke-RestMethod `
            -Uri $uri `
            -Method Post `
            -Headers $Headers `
            -ContentType 'application/json' `
            -Body $jsonBody `
            -TimeoutSec $script:RequestTimeoutSec

        return [pscustomobject]@{
            Ok   = $true
            Data = Get-PlayFabResponseData -Response $response
        }
    } catch {
        return [pscustomobject]@{
            Ok    = $false
            Error = Get-PlayFabRestError -ErrorRecord $_
        }
    }
}

function Assert-PlayFabRestResponse {
    param(
        [Parameter(Mandatory = $true)]$Response,
        [Parameter(Mandatory = $true)][string]$Route
    )

    if (-not [bool]$Response.Ok) {
        throw "PlayFab API '$Route' failed for title '$script:TitleIdForRequests': $(Format-PlayFabRestError -ErrorInfo $Response.Error)"
    }
    return $Response.Data
}

function Get-TitleEntityToken {
    param([Parameter(Mandatory = $true)][hashtable]$SecretHeaders)

    $route = 'Authentication/GetEntityToken'
    $response = Invoke-PlayFabRest -Route $route -Headers $SecretHeaders -Body @{}
    $data = Assert-PlayFabRestResponse -Response $response -Route $route
    $token = [string](Get-ObjectProperty -Object $data -Name 'EntityToken')
    if ([string]::IsNullOrWhiteSpace($token)) {
        throw "PlayFab API '$route' did not return an EntityToken."
    }

    $entity = Get-ObjectProperty -Object $data -Name 'Entity'
    $entityId = Get-ObjectProperty -Object $entity -Name 'Id'
    $entityType = Get-ObjectProperty -Object $entity -Name 'Type'
    Write-Host "OK: acquired title entity token for entity type '$entityType' id '$entityId'."
    return $token
}

function Ensure-CustomIdAccount {
    param(
        [Parameter(Mandatory = $true)][hashtable]$SecretHeaders,
        [Parameter(Mandatory = $true)][string]$Id
    )

    $route = 'Server/LoginWithCustomID'
    $body = @{
        CustomId      = $Id
        CreateAccount = $true
        CustomTags    = @{
            source = 'godot-public-gdk-ext-live-tests'
        }
    }
    $response = Invoke-PlayFabRest -Route $route -Headers $SecretHeaders -Body $body
    $data = Assert-PlayFabRestResponse -Response $response -Route $route
    $playFabId = [string](Get-ObjectProperty -Object $data -Name 'PlayFabId')
    $newlyCreated = Get-ObjectProperty -Object $data -Name 'NewlyCreated'
    $state = if ($true -eq $newlyCreated) { 'created' } else { 'already exists' }

    if ([string]::IsNullOrWhiteSpace($playFabId)) {
        Write-Host "OK: custom-ID account '$Id' is available."
    } else {
        Write-Host "OK: custom-ID account '$Id' $state (PlayFabId $playFabId)."
    }

    return [pscustomobject]@{
        CustomId     = $Id
        PlayFabId    = $playFabId
        NewlyCreated = $newlyCreated
    }
}

function Ensure-MultiplayerWorkerAccounts {
    param(
        [Parameter(Mandatory = $true)][hashtable]$SecretHeaders,
        [Parameter(Mandatory = $true)][string]$CustomIdPrefix
    )

    foreach ($role in @('host', 'client', 'observer')) {
        [void](Ensure-CustomIdAccount -SecretHeaders $SecretHeaders -Id "$CustomIdPrefix-$role")
    }
}

function Get-MatchmakingQueue {
    param(
        [Parameter(Mandatory = $true)][hashtable]$EntityHeaders,
        [Parameter(Mandatory = $true)][string]$Name
    )

    $route = 'Match/GetMatchmakingQueue'
    $response = Invoke-PlayFabRest -Route $route -Headers $EntityHeaders -Body @{ QueueName = $Name }
    $data = Assert-PlayFabRestResponse -Response $response -Route $route
    return (Get-ObjectProperty -Object $data -Name 'MatchmakingQueue')
}

function Assert-MatchmakingQueue {
    param(
        [Parameter(Mandatory = $true)]$Queue,
        [Parameter(Mandatory = $true)][string]$Name
    )

    if ($null -eq $Queue) {
        throw "Matchmaking queue '$Name' was not returned by PlayFab after configuration."
    }

    $actualName = [string](Get-ObjectProperty -Object $Queue -Name 'Name')
    $minMatchSize = [int](Get-ObjectProperty -Object $Queue -Name 'MinMatchSize')
    $maxMatchSize = [int](Get-ObjectProperty -Object $Queue -Name 'MaxMatchSize')
    $serverAllocationEnabled = [bool](Get-ObjectProperty -Object $Queue -Name 'ServerAllocationEnabled')
    $errors = [System.Collections.Generic.List[string]]::new()

    if ($actualName -ne $Name) {
        [void]$errors.Add("Name='$actualName'")
    }
    if ($minMatchSize -ne 2) {
        [void]$errors.Add("MinMatchSize=$minMatchSize")
    }
    if ($maxMatchSize -ne 2) {
        [void]$errors.Add("MaxMatchSize=$maxMatchSize")
    }
    if ($serverAllocationEnabled) {
        [void]$errors.Add("ServerAllocationEnabled=$serverAllocationEnabled")
    }

    if ($errors.Count -gt 0) {
        throw "Matchmaking queue '$Name' exists but does not match live-test requirements: $($errors -join '; ')."
    }
}

function Ensure-MatchmakingQueue {
    param(
        [Parameter(Mandatory = $true)][hashtable]$EntityHeaders,
        [Parameter(Mandatory = $true)][string]$Name
    )

    $route = 'Match/SetMatchmakingQueue'
    $queue = [ordered]@{
        Name                    = $Name
        MinMatchSize            = 2
        MaxMatchSize            = 2
        ServerAllocationEnabled = $false
        Rules                   = @(
            [ordered]@{
                Type                          = 'StringEqualityRule'
                Attribute                     = @{
                    Path   = 'run_id'
                    Source = 'User'
                }
                AttributeNotSpecifiedBehavior = 'UseDefault'
                DefaultAttributeValue         = '__missing_run_id__'
                Weight                        = 1
                Name                          = 'RunIdRule'
            }
        )
    }
    $response = Invoke-PlayFabRest -Route $route -Headers $EntityHeaders -Body @{ MatchmakingQueue = $queue }
    [void](Assert-PlayFabRestResponse -Response $response -Route $route)

    $configuredQueue = $null
    for ($attempt = 1; $attempt -le 3; $attempt++) {
        try {
            $configuredQueue = Get-MatchmakingQueue -EntityHeaders $EntityHeaders -Name $Name
            break
        } catch {
            if ($attempt -eq 3) {
                throw
            }
            Start-Sleep -Seconds 2
        }
    }

    Assert-MatchmakingQueue -Queue $configuredQueue -Name $Name
    Write-Host "OK: matchmaking queue '$Name' is configured for 2-player live smoke tests."
    return [pscustomobject]@{
        name                      = $Name
        min_match_size            = 2
        max_match_size            = 2
        server_allocation_enabled = $false
        string_equality_rule      = 'run_id'
    }
}

function Get-LeaderboardDefinition {
    param(
        [Parameter(Mandatory = $true)][hashtable]$EntityHeaders,
        [Parameter(Mandatory = $true)][string]$Name
    )

    $route = 'Leaderboard/GetLeaderboardDefinition'
    $response = Invoke-PlayFabRest -Route $route -Headers $EntityHeaders -Body @{ Name = $Name }
    if (-not [bool]$response.Ok) {
        $errorName = [string](Get-ObjectProperty -Object $response.Error -Name 'Error')
        if ($errorName -eq 'LeaderboardNotFound') {
            return $null
        }
        [void](Assert-PlayFabRestResponse -Response $response -Route $route)
    }
    return $response.Data
}

function Assert-LeaderboardDefinition {
    param(
        [Parameter(Mandatory = $true)]$Definition,
        [Parameter(Mandatory = $true)][string]$Name
    )

    $errors = [System.Collections.Generic.List[string]]::new()
    $actualName = [string](Get-ObjectProperty -Object $Definition -Name 'Name')
    $entityType = [string](Get-ObjectProperty -Object $Definition -Name 'EntityType')
    $columnsValue = Get-ObjectProperty -Object $Definition -Name 'Columns'
    $columns = if ($null -eq $columnsValue) { @() } else { @($columnsValue) }

    if ($actualName -ne $Name) {
        $errors.Add("Name is '$actualName', expected '$Name'")
    }
    if ($entityType -ne 'title_player_account') {
        $errors.Add("EntityType is '$entityType', expected 'title_player_account'")
    }
    if ($columns.Count -ne 1) {
        $errors.Add("Columns count is $($columns.Count), expected 1")
    } elseif ($null -eq $columns[0]) {
        $errors.Add('Column 0 is null')
    } else {
        $columnName = [string](Get-ObjectProperty -Object $columns[0] -Name 'Name')
        $sortDirection = [string](Get-ObjectProperty -Object $columns[0] -Name 'SortDirection')
        if ($columnName -ne 'score') {
            $errors.Add("Column name is '$columnName', expected 'score'")
        }
        if ($sortDirection -ne 'Descending') {
            $errors.Add("Column sort is '$sortDirection', expected 'Descending'")
        }
    }

    if ($errors.Count -gt 0) {
        throw "Leaderboard '$Name' exists but does not match live-test requirements: $($errors -join '; '). Delete or rename the incompatible leaderboard in PlayFab Game Manager, then rerun this script."
    }
}

function Ensure-LeaderboardDefinition {
    param(
        [Parameter(Mandatory = $true)][hashtable]$EntityHeaders,
        [Parameter(Mandatory = $true)][string]$Name,
        [Parameter(Mandatory = $true)][int]$SizeLimit
    )

    $definition = Get-LeaderboardDefinition -EntityHeaders $EntityHeaders -Name $Name
    if ($null -ne $definition) {
        Assert-LeaderboardDefinition -Definition $definition -Name $Name
        Write-Host "OK: leaderboard '$Name' already exists with the expected title_player_account score column."
        return
    }

    $route = 'Leaderboard/CreateLeaderboardDefinition'
    $body = @{
        Name                 = $Name
        EntityType           = 'title_player_account'
        SizeLimit            = $SizeLimit
        Columns              = @(
            @{
                Name          = 'score'
                SortDirection = 'Descending'
            }
        )
        VersionConfiguration = @{
            ResetInterval        = 'Manual'
            MaxQueryableVersions = 1
        }
        CustomTags           = @{
            source = 'godot-public-gdk-ext-live-tests'
        }
    }

    $response = Invoke-PlayFabRest -Route $route -Headers $EntityHeaders -Body $body
    if (-not [bool]$response.Ok) {
        $errorName = [string](Get-ObjectProperty -Object $response.Error -Name 'Error')
        if ($errorName -ne 'LeaderboardNameConflict') {
            [void](Assert-PlayFabRestResponse -Response $response -Route $route)
        }
    } else {
        Write-Host "OK: created leaderboard '$Name'."
    }

    $definition = Get-LeaderboardDefinition -EntityHeaders $EntityHeaders -Name $Name
    if ($null -eq $definition) {
        throw "Leaderboard '$Name' was not found after creation."
    }
    Assert-LeaderboardDefinition -Definition $definition -Name $Name
    if (-not [bool]$response.Ok) {
        Write-Host "OK: leaderboard '$Name' already exists with the expected title_player_account score column."
    }
}

function Set-TitleDataValue {
    param(
        [Parameter(Mandatory = $true)][hashtable]$SecretHeaders,
        [Parameter(Mandatory = $true)][string]$Key,
        [Parameter(Mandatory = $true)][string]$Value
    )

    $route = 'Admin/SetTitleData'
    $body = @{
        Key   = $Key
        Value = $Value
    }

    $response = Invoke-PlayFabRest -Route $route -Headers $SecretHeaders -Body $body
    [void](Assert-PlayFabRestResponse -Response $response -Route $route)
    Write-Host "OK: wrote title data key '$Key'."
}

function Set-PublisherDataValue {
    param(
        [Parameter(Mandatory = $true)][hashtable]$SecretHeaders,
        [Parameter(Mandatory = $true)][string]$Key,
        [Parameter(Mandatory = $true)][string]$Value
    )

    $route = 'Admin/SetPublisherData'
    $body = @{
        Key   = $Key
        Value = $Value
    }

    $response = Invoke-PlayFabRest -Route $route -Headers $SecretHeaders -Body $body
    [void](Assert-PlayFabRestResponse -Response $response -Route $route)
    Write-Host "OK: wrote publisher data key '$Key'."
}

function Set-UserDataValue {
    param(
        [Parameter(Mandatory = $true)][hashtable]$SecretHeaders,
        [Parameter(Mandatory = $true)][string]$PlayFabId,
        [Parameter(Mandatory = $true)][string]$Key,
        [Parameter(Mandatory = $true)][string]$Value,
        [Parameter(Mandatory = $true)][string]$Route,
        [string]$Permission = ''
    )

    $data = @{}
    $data[$Key] = $Value

    $body = @{
        PlayFabId  = $PlayFabId
        Data       = $data
        CustomTags = @{
            source = 'godot-public-gdk-ext-live-tests'
        }
    }
    if (-not [string]::IsNullOrWhiteSpace($Permission)) {
        $body['Permission'] = $Permission
    }

    $response = Invoke-PlayFabRest -Route $Route -Headers $SecretHeaders -Body $body
    [void](Assert-PlayFabRestResponse -Response $response -Route $Route)
}

function Set-UserPublisherDataValue {
    param(
        [Parameter(Mandatory = $true)][hashtable]$SecretHeaders,
        [Parameter(Mandatory = $true)][string]$PlayFabId,
        [Parameter(Mandatory = $true)][string]$Key,
        [Parameter(Mandatory = $true)][string]$Value
    )

    $route = 'Server/UpdateUserPublisherData'
    $data = @{}
    $data[$Key] = $Value

    $body = @{
        PlayFabId  = $PlayFabId
        Data       = $data
        Permission = 'Public'
        CustomTags = @{
            source = 'godot-public-gdk-ext-live-tests'
        }
    }

    $response = Invoke-PlayFabRest -Route $route -Headers $SecretHeaders -Body $body
    [void](Assert-PlayFabRestResponse -Response $response -Route $route)
}

function Get-StatisticDefinition {
    param(
        [Parameter(Mandatory = $true)][hashtable]$EntityHeaders,
        [Parameter(Mandatory = $true)][string]$Name
    )

    $route = 'Statistic/GetStatisticDefinition'
    $response = Invoke-PlayFabRest -Route $route -Headers $EntityHeaders -Body @{ Name = $Name }
    if (-not [bool]$response.Ok) {
        $errorName = [string](Get-ObjectProperty -Object $response.Error -Name 'Error')
        if ($errorName -in @('StatisticNotFound', 'StatisticDefinitionNotFound')) {
            return $null
        }
        [void](Assert-PlayFabRestResponse -Response $response -Route $route)
    }
    return $response.Data
}

function Assert-StatisticDefinition {
    param(
        [Parameter(Mandatory = $true)]$Definition,
        [Parameter(Mandatory = $true)][string]$Name
    )

    $errors = [System.Collections.Generic.List[string]]::new()
    $actualName = [string](Get-ObjectProperty -Object $Definition -Name 'Name')
    $entityType = [string](Get-ObjectProperty -Object $Definition -Name 'EntityType')
    $columnsValue = Get-ObjectProperty -Object $Definition -Name 'Columns'
    $columns = if ($null -eq $columnsValue) { @() } else { @($columnsValue) }

    if ($actualName -ne $Name) {
        $errors.Add("Name is '$actualName', expected '$Name'")
    }
    if ($entityType -ne 'title_player_account') {
        $errors.Add("EntityType is '$entityType', expected 'title_player_account'")
    }
    if ($columns.Count -ne 1) {
        $errors.Add("Columns count is $($columns.Count), expected 1")
    } elseif ($null -eq $columns[0]) {
        $errors.Add('Column 0 is null')
    } else {
        $columnName = [string](Get-ObjectProperty -Object $columns[0] -Name 'Name')
        $aggregationMethod = [string](Get-ObjectProperty -Object $columns[0] -Name 'AggregationMethod')
        if ($columnName -ne 'value') {
            $errors.Add("Column name is '$columnName', expected 'value'")
        }
        if ($aggregationMethod -ne 'Last') {
            $errors.Add("Column aggregation method is '$aggregationMethod', expected 'Last'")
        }
    }

    if ($errors.Count -gt 0) {
        throw "Statistic '$Name' exists but does not match API-service test requirements: $($errors -join '; '). Delete or rename the incompatible statistic in PlayFab Game Manager, then rerun this script."
    }
}

function Ensure-StatisticDefinition {
    param(
        [Parameter(Mandatory = $true)][hashtable]$EntityHeaders,
        [Parameter(Mandatory = $true)][string]$Name
    )

    $definition = Get-StatisticDefinition -EntityHeaders $EntityHeaders -Name $Name
    if ($null -ne $definition) {
        Assert-StatisticDefinition -Definition $definition -Name $Name
        Write-Host "OK: statistic '$Name' already exists with the expected title_player_account value column."
        return
    }

    $route = 'Statistic/CreateStatisticDefinition'
    $body = @{
        Name                 = $Name
        EntityType           = 'title_player_account'
        Columns              = @(
            @{
                Name              = 'value'
                AggregationMethod = 'Last'
            }
        )
        VersionConfiguration = @{
            ResetInterval        = 'Manual'
            MaxQueryableVersions = 1
        }
        CustomTags           = @{
            source = 'godot-public-gdk-ext-live-tests'
        }
    }

    $response = Invoke-PlayFabRest -Route $route -Headers $EntityHeaders -Body $body
    if (-not [bool]$response.Ok) {
        $errorName = [string](Get-ObjectProperty -Object $response.Error -Name 'Error')
        if ($errorName -notin @('StatisticNameConflict', 'StatisticDefinitionNameConflict')) {
            [void](Assert-PlayFabRestResponse -Response $response -Route $route)
        }
    } else {
        Write-Host "OK: created statistic '$Name'."
    }

    $definition = Get-StatisticDefinition -EntityHeaders $EntityHeaders -Name $Name
    if ($null -eq $definition) {
        throw "Statistic '$Name' was not found after creation."
    }
    Assert-StatisticDefinition -Definition $definition -Name $Name
    if (-not [bool]$response.Ok) {
        Write-Host "OK: statistic '$Name' already exists with the expected title_player_account value column."
    }
}

function Get-CatalogConfig {
    param([Parameter(Mandatory = $true)][hashtable]$EntityHeaders)

    $route = 'Catalog/GetCatalogConfig'
    $response = Invoke-PlayFabRest -Route $route -Headers $EntityHeaders -Body @{}
    if (-not [bool]$response.Ok) {
        $errorName = [string](Get-ObjectProperty -Object $response.Error -Name 'Error')
        if ($errorName -in @('CatalogConfigNotFound', 'NotAuthorizedByTitle')) {
            return $null
        }
        [void](Assert-PlayFabRestResponse -Response $response -Route $route)
    }

    return Get-ObjectProperty -Object $response.Data -Name 'Config'
}

function Ensure-CatalogConfigForDraftItem {
    param([Parameter(Mandatory = $true)][hashtable]$EntityHeaders)

    $config = Get-CatalogConfig -EntityHeaders $EntityHeaders
    if ($null -eq $config) {
        $config = [ordered]@{}
    }

    $changed = $false
    if (-not [bool](Get-ObjectProperty -Object $config -Name 'IsCatalogEnabled')) {
        Set-ObjectProperty -Object $config -Name 'IsCatalogEnabled' -Value $true
        $changed = $true
    }

    if (-not $changed) {
        Write-Host "OK: catalog is enabled for API-service fixture items."
        return
    }

    $route = 'Catalog/UpdateCatalogConfig'
    $body = @{
        Config     = $config
        CustomTags = @{
            source = 'godot-public-gdk-ext-live-tests'
        }
    }
    $response = Invoke-PlayFabRest -Route $route -Headers $EntityHeaders -Body $body
    [void](Assert-PlayFabRestResponse -Response $response -Route $route)
    Write-Host "OK: enabled catalog for API-service fixture items."
}

function Get-CatalogDraftItem {
    param(
        [Parameter(Mandatory = $true)][hashtable]$EntityHeaders,
        [Parameter(Mandatory = $true)][string]$FriendlyId
    )

    $route = 'Catalog/GetDraftItem'
    $body = @{
        AlternateId = @{
            Type  = 'FriendlyId'
            Value = $FriendlyId
        }
    }
    $response = Invoke-PlayFabRest -Route $route -Headers $EntityHeaders -Body $body
    if (-not [bool]$response.Ok) {
        $errorName = [string](Get-ObjectProperty -Object $response.Error -Name 'Error')
        if ($errorName -in @('CatalogItemNotFound', 'ItemNotFound')) {
            return $null
        }
        [void](Assert-PlayFabRestResponse -Response $response -Route $route)
    }
    return $response.Data
}

function Ensure-CatalogDraftItem {
    param(
        [Parameter(Mandatory = $true)][hashtable]$EntityHeaders,
        [Parameter(Mandatory = $true)][string]$FriendlyId
    )

    Ensure-CatalogConfigForDraftItem -EntityHeaders $EntityHeaders

    $draftItem = Get-CatalogDraftItem -EntityHeaders $EntityHeaders -FriendlyId $FriendlyId
    if ($null -ne $draftItem) {
        Write-Host "OK: catalog draft item with FriendlyId '$FriendlyId' already exists."
        return $draftItem
    }

    $route = 'Catalog/CreateDraftItem'
    $body = @{
        Item       = @{
            Type              = 'catalogItem'
            AlternateIds      = @(
                @{
                    Type  = 'FriendlyId'
                    Value = $FriendlyId
                }
            )
            Title             = @{
                NEUTRAL = 'Godot API service smoke item'
            }
            Description       = @{
                NEUTRAL = 'Fixture item for godot-public-gdk-ext PlayFab service live tests.'
            }
            DisplayProperties = @{
                source  = 'godot-public-gdk-ext-live-tests'
                fixture = 'api-services'
            }
        }
        Publish    = $false
        CustomTags = @{
            source = 'godot-public-gdk-ext-live-tests'
        }
    }

    $response = Invoke-PlayFabRest -Route $route -Headers $EntityHeaders -Body $body
    if (-not [bool]$response.Ok) {
        $errorName = [string](Get-ObjectProperty -Object $response.Error -Name 'Error')
        if ($errorName -notin @('CatalogItemIdConflict', 'ItemAlreadyExists')) {
            [void](Assert-PlayFabRestResponse -Response $response -Route $route)
        }
    } else {
        Write-Host "OK: created catalog draft item with FriendlyId '$FriendlyId'."
    }

    $draftItem = Get-CatalogDraftItem -EntityHeaders $EntityHeaders -FriendlyId $FriendlyId
    if ($null -eq $draftItem) {
        throw "Catalog draft item with FriendlyId '$FriendlyId' was not found after creation."
    }
    if (-not [bool]$response.Ok) {
        Write-Host "OK: catalog draft item with FriendlyId '$FriendlyId' already exists."
    }
    return $draftItem
}

function Ensure-ApiServiceFixtures {
    param(
        [Parameter(Mandatory = $true)][hashtable]$SecretHeaders,
        [Parameter(Mandatory = $true)][hashtable]$EntityHeaders,
        [AllowNull()]$PrimaryAccount,
        [Parameter(Mandatory = $true)][string]$FixturePrefix,
        [Parameter(Mandatory = $true)][string]$TitleDataKey,
        [Parameter(Mandatory = $true)][string]$PublisherDataKey,
        [Parameter(Mandatory = $true)][string]$PlayerDataKey,
        [Parameter(Mandatory = $true)][string]$StatisticName,
        [Parameter(Mandatory = $true)][string]$CatalogItemId
    )

    $fixtures = [ordered]@{}
    $accounts = [System.Collections.Generic.List[object]]::new()
    if ($null -ne $PrimaryAccount -and -not [string]::IsNullOrWhiteSpace([string]$PrimaryAccount.PlayFabId)) {
        $accounts.Add($PrimaryAccount)
    }

    if (-not $SkipServiceAccounts) {
        $friendAccount = Ensure-CustomIdAccount -SecretHeaders $SecretHeaders -Id "$FixturePrefix-friend"
        $peerAccount = Ensure-CustomIdAccount -SecretHeaders $SecretHeaders -Id "$FixturePrefix-peer"
        if (-not [string]::IsNullOrWhiteSpace([string]$friendAccount.PlayFabId)) {
            $accounts.Add($friendAccount)
        }
        if (-not [string]::IsNullOrWhiteSpace([string]$peerAccount.PlayFabId)) {
            $accounts.Add($peerAccount)
        }
        $fixtures['service_custom_ids'] = @(
            "$FixturePrefix-friend",
            "$FixturePrefix-peer"
        )
    }
    $primaryCustomId = if ($null -ne $PrimaryAccount) { [string]$PrimaryAccount.CustomId } else { '' }
    $fixtures['accounts'] = @{
        primary_custom_id = $primaryCustomId
        friend_custom_id  = if ($SkipServiceAccounts) { '' } else { "$FixturePrefix-friend" }
        peer_custom_id    = if ($SkipServiceAccounts) { '' } else { "$FixturePrefix-peer" }
    }

    if (-not $SkipServicePlayerData) {
        foreach ($account in $accounts) {
            $playFabId = [string]$account.PlayFabId
            if ([string]::IsNullOrWhiteSpace($playFabId)) {
                continue
            }
            $value = "configured:$($account.CustomId):$($script:TitleIdForRequests)"
            Set-UserDataValue -SecretHeaders $SecretHeaders -PlayFabId $playFabId -Key $PlayerDataKey -Value $value -Route 'Server/UpdateUserData' -Permission 'Public'
            Set-UserDataValue -SecretHeaders $SecretHeaders -PlayFabId $playFabId -Key "$PlayerDataKey`_readonly" -Value $value -Route 'Server/UpdateUserReadOnlyData'
            Set-UserPublisherDataValue -SecretHeaders $SecretHeaders -PlayFabId $playFabId -Key "$PlayerDataKey`_publisher" -Value $value
            Write-Host "OK: wrote player data fixtures for custom-ID account '$($account.CustomId)'."
        }
        $fixtures['player_data'] = @{
            key                = $PlayerDataKey
            read_only_key      = "$PlayerDataKey`_readonly"
            publisher_data_key = "$PlayerDataKey`_publisher"
        }
    }

    if (-not $SkipServiceTitleData) {
        $titleDataValue = "configured:$($script:TitleIdForRequests):$TitleDataKey"
        $publisherDataValue = "configured:$($script:TitleIdForRequests):$PublisherDataKey"
        Set-TitleDataValue -SecretHeaders $SecretHeaders -Key $TitleDataKey -Value $titleDataValue
        Set-PublisherDataValue -SecretHeaders $SecretHeaders -Key $PublisherDataKey -Value $publisherDataValue
        $fixtures['title_data'] = @{
            key   = $TitleDataKey
            value = $titleDataValue
        }
        $fixtures['publisher_data'] = @{
            key   = $PublisherDataKey
            value = $publisherDataValue
        }
    }

    if (-not $SkipServiceStatistic) {
        Ensure-StatisticDefinition -EntityHeaders $EntityHeaders -Name $StatisticName
        $fixtures['statistic'] = @{
            name        = $StatisticName
            entity_type = 'title_player_account'
            columns     = @(@{ name = 'value'; aggregation_method = 'Last' })
        }
    }

    if (-not $SkipServiceCatalogDraftItem) {
        $catalogDraft = Ensure-CatalogDraftItem -EntityHeaders $EntityHeaders -FriendlyId $CatalogItemId
        $catalogItem = Get-ObjectProperty -Object $catalogDraft -Name 'Item'
        $resolvedCatalogItemId = [string](Get-ObjectProperty -Object $catalogItem -Name 'Id')
        if ([string]::IsNullOrWhiteSpace($resolvedCatalogItemId)) {
            $resolvedCatalogItemId = $CatalogItemId
        }
        $fixtures['catalog'] = @{
            draft_item_id = $resolvedCatalogItemId
            friendly_id   = $CatalogItemId
            type          = 'catalogItem'
        }
        $fixtures['inventory'] = @{
            collection_id   = 'default'
            catalog_item_id = $resolvedCatalogItemId
            friendly_id     = $CatalogItemId
        }
    }

    $fixtures['entity_data'] = @{
        object_name = 'godot_services_smoke_object'
        file_name   = 'godot_services_smoke_file.json'
    }
    $fixtures['groups'] = @{
        group_name_prefix = "$FixturePrefix-group"
        role_id           = 'members'
    }
    $fixtures['cloud_script'] = @{
        function_name        = 'godot_services_smoke'
        configured_by_script = $false
        note                 = 'Classic CloudScript upload/publish is not changed by this script; add a function with this name before enabling execute_cloud_script live assertions.'
    }
    $fixtures['experimentation'] = @{
        note = 'get_treatment_assignment can be smoke-tested without script-created experiments; configure title experiments separately for assignment-specific assertions.'
    }
    $fixtures['localization'] = @{
        note = 'get_language_list uses the title language list and does not require script-created data.'
    }

    return [pscustomobject]$fixtures
}

function Set-LiveTestTitleDataMarker {
    param(
        [Parameter(Mandatory = $true)][hashtable]$SecretHeaders,
        [Parameter(Mandatory = $true)][string]$MarkerCustomId,
        [Parameter(Mandatory = $true)][string]$MarkerMultiplayerCustomIdPrefix,
        [Parameter(Mandatory = $true)][string]$MarkerLeaderboardName,
        [AllowNull()]$MatchmakingQueue = $null,
        [AllowNull()]$ApiServiceFixtures = $null
    )

    $route = 'Admin/SetTitleData'
    $marker = [ordered]@{
        repository              = 'gaming-microsoft/godot-public-gdk-ext'
        title_id                = $script:TitleIdForRequests
        custom_id               = $MarkerCustomId
        multiplayer_custom_ids  = @(
            "$MarkerMultiplayerCustomIdPrefix-host",
            "$MarkerMultiplayerCustomIdPrefix-client",
            "$MarkerMultiplayerCustomIdPrefix-observer"
        )
        leaderboard             = @{
            name        = $MarkerLeaderboardName
            entity_type = 'title_player_account'
            columns     = @(@{ name = 'score'; sort_direction = 'Descending' })
        }
        lobby_search_properties = @('string_key1', 'number_key1')
        note                    = 'Lobby search properties use reserved PlayFab keys and do not require title setup.'
        updated_utc             = (Get-Date).ToUniversalTime().ToString('o')
    }
    if ($null -ne $MatchmakingQueue) {
        $marker['matchmaking_queue'] = $MatchmakingQueue
    }
    if ($null -ne $ApiServiceFixtures) {
        $marker['api_services'] = $ApiServiceFixtures
    }
    $body = @{
        Key   = 'godot_public_gdk_ext_live_tests'
        Value = ($marker | ConvertTo-Json -Depth 10 -Compress)
    }

    $response = Invoke-PlayFabRest -Route $route -Headers $SecretHeaders -Body $body
    [void](Assert-PlayFabRestResponse -Response $response -Route $route)
    Write-Host "OK: wrote title data marker 'godot_public_gdk_ext_live_tests'."
}

Write-Host "Configuring PlayFab title '$TitleId' for Godot live tests."
Write-Host "Using PlayFab developer secret from environment variable '$SecretKeyEnvVar'."

$secretHeaders = @{ 'X-SecretKey' = $SecretKey }
$primaryAccount = $null
if (-not $SkipCustomIdAccount) {
    $primaryAccount = Ensure-CustomIdAccount -SecretHeaders $secretHeaders -Id $CustomId
}
if (-not $SkipMultiplayerWorkerAccounts) {
    Ensure-MultiplayerWorkerAccounts -SecretHeaders $secretHeaders -CustomIdPrefix $MultiplayerCustomIdPrefix
}

$entityToken = Get-TitleEntityToken -SecretHeaders $secretHeaders
$entityHeaders = @{ 'X-EntityToken' = $entityToken }
$matchmakingQueue = $null
if (-not $SkipMultiplayerMatchmakingQueue) {
    $matchmakingQueue = Ensure-MatchmakingQueue -EntityHeaders $entityHeaders -Name $MatchmakingQueueName
}
Ensure-LeaderboardDefinition -EntityHeaders $entityHeaders -Name $LeaderboardName -SizeLimit $LeaderboardSizeLimit
$APIServiceFixtures = $null
if (-not $SkipApiServiceFixtures) {
    $APIServiceFixtures = Ensure-ApiServiceFixtures `
        -SecretHeaders $secretHeaders `
        -EntityHeaders $entityHeaders `
        -PrimaryAccount $primaryAccount `
        -FixturePrefix $ServiceFixturePrefix `
        -TitleDataKey $ServiceTitleDataKey `
        -PublisherDataKey $ServicePublisherDataKey `
        -PlayerDataKey $ServicePlayerDataKey `
        -StatisticName $ServiceStatisticName `
        -CatalogItemId $ServiceCatalogItemId
}

if (-not $SkipTitleDataMarker) {
    Set-LiveTestTitleDataMarker -SecretHeaders $secretHeaders -MarkerCustomId $CustomId -MarkerMultiplayerCustomIdPrefix $MultiplayerCustomIdPrefix -MarkerLeaderboardName $LeaderboardName -MatchmakingQueue $matchmakingQueue -ApiServiceFixtures $APIServiceFixtures
}

Write-Host ''
Write-Host 'Live test command:'
$liveTestCommand = if ($SkipCustomIdAccount) {
    "pwsh -NoLogo -NoProfile -ExecutionPolicy Bypass -File .\tools\run_all_tests.ps1 -Hosts tests\godot\playfab -Live -PlayFabTitleId `"$TitleId`""
} else {
    "pwsh -NoLogo -NoProfile -ExecutionPolicy Bypass -File .\tools\run_all_tests.ps1 -Hosts tests\godot\playfab -Live -PlayFabTitleId `"$TitleId`" -PlayFabCustomId `"$CustomId`""
}
if (-not $SkipMultiplayerMatchmakingQueue) {
    $liveTestCommand = "$liveTestCommand -PlayFabMatchmakingQueue `"$MatchmakingQueueName`""
}
Write-Host $liveTestCommand
if ($SkipCustomIdAccount) {
    Write-Host 'Custom-ID live service tests remain pending unless PLAYFAB_CUSTOM_ID is set by the caller.'
} else {
    Write-Host "The Multiplayer runner will derive worker custom IDs from PLAYFAB_CUSTOM_ID as '$MultiplayerCustomIdPrefix-<role>'."
}
if (-not $SkipMultiplayerMatchmakingQueue) {
    Write-Host "The Multiplayer runner will use matchmaking queue '$MatchmakingQueueName'."
} else {
    Write-Host 'Set PLAYFAB_MULTIPLAYER_MATCH_QUEUE separately only when optional matchmaking smoke should run.'
}
if (-not $SkipApiServiceFixtures) {
    Write-Host "API-service fixtures use custom IDs '$ServiceFixturePrefix-friend' and '$ServiceFixturePrefix-peer'."
    Write-Host "API-service fixtures use title data '$ServiceTitleDataKey', publisher data '$ServicePublisherDataKey', statistic '$ServiceStatisticName', and catalog draft item '$ServiceCatalogItemId'."
}
