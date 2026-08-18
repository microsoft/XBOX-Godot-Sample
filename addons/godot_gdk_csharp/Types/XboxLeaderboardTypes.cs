using Godot;
using GodotXbox.Internal;

namespace GodotXbox.Types;

/// <summary>A leaderboard page: its stat, columns, and the current rows.</summary>
public sealed class XboxLeaderboard : XboxObject
{
    internal XboxLeaderboard(GodotObject o) : base(o) { }
    public static XboxLeaderboard From(GodotObject o) => o == null ? null : new XboxLeaderboard(o);

    public string StatName => GetString("stat_name");
    public string QueryType => GetString("query_type");
    public int TotalRowCount => GetInt32("total_row_count");
    public bool HasNext => GetBool("has_next");
    public Godot.Collections.Array Columns => GetArray("columns");
    public Godot.Collections.Array Rows => GetArray("rows");
}

/// <summary>A leaderboard column descriptor (stat name + type).</summary>
public sealed class XboxLeaderboardColumn : XboxObject
{
    internal XboxLeaderboardColumn(GodotObject o) : base(o) { }
    public static XboxLeaderboardColumn From(GodotObject o) => o == null ? null : new XboxLeaderboardColumn(o);

    public string StatName => GetString("stat_name");
    public string StatType => GetString("stat_type");
}

/// <summary>A single leaderboard row (a ranked player and their values).</summary>
public sealed class XboxLeaderboardRow : XboxObject
{
    internal XboxLeaderboardRow(GodotObject o) : base(o) { }
    public static XboxLeaderboardRow From(GodotObject o) => o == null ? null : new XboxLeaderboardRow(o);

    public string Gamertag => GetString("gamertag");
    public string ModernGamertag => GetString("modern_gamertag");
    public string ModernGamertagSuffix => GetString("modern_gamertag_suffix");
    public string UniqueModernGamertag => GetString("unique_modern_gamertag");
    public string Xuid => GetString("xuid");
    public double Percentile => GetDouble("percentile");
    public int Rank => GetInt32("rank");
    public int GlobalRank => GetInt32("global_rank");
    public string[] ColumnValues => Get("column_values").AsStringArray();
}
