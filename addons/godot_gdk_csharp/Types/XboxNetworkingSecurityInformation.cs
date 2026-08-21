using Godot;
using GodotXbox.Internal;

namespace GodotXbox.Types;

/// <summary>
/// Network Security Allow List (NSAL) information for a title endpoint, returned
/// by <c>GDK.networking.query_security_information_for_url_async()</c>.
/// </summary>
public sealed class XboxNetworkingSecurityInformation : XboxObject
{
    internal XboxNetworkingSecurityInformation(GodotObject o) : base(o) { }

    public static XboxNetworkingSecurityInformation From(GodotObject o) =>
        o == null ? null : new XboxNetworkingSecurityInformation(o);

    /// <summary>Bitmask of HTTP security protocols the NSAL entry enables for the URL.</summary>
    public long EnabledHttpSecurityProtocolFlags => Call("get_enabled_http_security_protocol_flags").AsInt64();

    /// <summary>
    /// One dictionary per certificate thumbprint, each with <c>type</c>,
    /// <c>type_name</c>, <c>bytes</c>, and <c>hex</c> entries.
    /// </summary>
    public Godot.Collections.Array Thumbprints => Call("get_thumbprints").AsGodotArray();

    /// <summary>The whole record as a dictionary.</summary>
    public Godot.Collections.Dictionary ToDictionary() => Call("to_dictionary").AsGodotDictionary();
}
