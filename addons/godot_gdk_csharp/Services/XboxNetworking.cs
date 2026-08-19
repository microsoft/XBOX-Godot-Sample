using System;
using System.Threading.Tasks;
using Godot;
using GodotXbox.Internal;
using GodotXbox.Types;

namespace GodotXbox.Services;

/// <summary>
/// <c>GDK.networking</c> — preferred local UDP multiplayer port, connectivity
/// hints, and NSAL security information for title endpoints.
/// </summary>
/// <remarks>
/// <see cref="QueryConfigurationSetting"/>, <see cref="SetConfigurationSetting"/>
/// and <see cref="QueryStatistics"/> are documented no-ops on Windows: the
/// setter reports <c>E_NOTIMPL</c> and the queries report zeros.
/// </remarks>
public sealed class XboxNetworking : XboxServiceBase
{
    internal XboxNetworking(GodotObject o) : base(o)
    {
        _o.Connect("preferred_local_udp_multiplayer_port_changed",
            Callable.From((Variant a0) => PreferredLocalUdpMultiplayerPortChanged?.Invoke(a0.AsInt32())));
        _o.Connect("connectivity_hint_changed",
            Callable.From((Variant a0) => ConnectivityHintChanged?.Invoke(a0.AsGodotDictionary())));
    }

    /// <summary>Connectivity level hint. Mirrors <c>XNetworkingConnectivityLevelHint</c>.</summary>
    public enum ConnectivityLevelHint
    {
        ConnectivityLevelUnknown = 0,
        ConnectivityLevelNone = 1,
        ConnectivityLevelLocalAccess = 2,
        ConnectivityLevelInternetAccess = 3,
        ConnectivityLevelConstrainedInternetAccess = 4,
    }

    /// <summary>Connectivity cost hint. Mirrors <c>XNetworkingConnectivityCostHint</c>.</summary>
    public enum ConnectivityCostHint
    {
        ConnectivityCostUnknown = 0,
        ConnectivityCostUnrestricted = 1,
        ConnectivityCostFixed = 2,
        ConnectivityCostVariable = 3,
    }

    /// <summary>Certificate thumbprint type. Mirrors <c>XNetworkingThumbprintType</c>.</summary>
    public enum ThumbprintType
    {
        ThumbprintTypeLeaf = 0,
        ThumbprintTypeIssuer = 1,
        ThumbprintTypeRoot = 2,
    }

    /// <summary>TCP queued-receive-buffer setting. Mirrors <c>XNetworkingConfigurationSetting</c>.</summary>
    public enum ConfigurationSetting
    {
        ConfigurationSettingMaxTitleTcpQueuedReceiveBufferSize = 0,
        ConfigurationSettingMaxSystemTcpQueuedReceiveBufferSize = 1,
        ConfigurationSettingMaxToolsTcpQueuedReceiveBufferSize = 2,
    }

    /// <summary>TCP queued-receive-buffer statistics type. Mirrors <c>XNetworkingStatisticsType</c>.</summary>
    public enum StatisticsType
    {
        StatisticsTypeTitleTcpQueuedReceivedBufferUsage = 0,
        StatisticsTypeSystemTcpQueuedReceivedBufferUsage = 1,
        StatisticsTypeToolsTcpQueuedReceivedBufferUsage = 2,
    }

    /// <summary>
    /// Raised when the preferred local UDP multiplayer port changes, and once
    /// after initialization with the current value.
    /// </summary>
    public event Action<int> PreferredLocalUdpMultiplayerPortChanged;

    /// <summary>
    /// Raised when the device connectivity hint changes, and once after
    /// initialization with the current value.
    /// </summary>
    public event Action<Godot.Collections.Dictionary> ConnectivityHintChanged;

    /// <summary>
    /// Synchronous preferred-port query. The GDK documents this as unsafe on
    /// time-sensitive threads; prefer
    /// <see cref="QueryPreferredLocalUdpMultiplayerPortAsync"/> on the main thread.
    /// </summary>
    public XboxResult QueryPreferredLocalUdpMultiplayerPort() =>
        XboxResult.From(Call("query_preferred_local_udp_multiplayer_port").AsGodotObject());

    public Task<XboxResult> QueryPreferredLocalUdpMultiplayerPortAsync() =>
        CallResultAsync("query_preferred_local_udp_multiplayer_port_async");

    public XboxResult GetConnectivityHint() =>
        XboxResult.From(Call("get_connectivity_hint").AsGodotObject());

    public Task<XboxResult> QuerySecurityInformationForUrlAsync(string url) =>
        CallResultAsync("query_security_information_for_url_async", url);

    public XboxResult QueryConfigurationSetting(ConfigurationSetting setting) =>
        XboxResult.From(Call("query_configuration_setting", (int)setting).AsGodotObject());

    public XboxResult SetConfigurationSetting(ConfigurationSetting setting, long value) =>
        XboxResult.From(Call("set_configuration_setting", (int)setting, value).AsGodotObject());

    public XboxResult QueryStatistics(StatisticsType statisticsType) =>
        XboxResult.From(Call("query_statistics", (int)statisticsType).AsGodotObject());
}
