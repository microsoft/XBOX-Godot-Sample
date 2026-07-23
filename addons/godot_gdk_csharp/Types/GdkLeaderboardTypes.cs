using Godot;
using GodotGdk.Internal;

namespace GodotGdk.Types;

/// <summary>A leaderboard page: its stat, columns, and the current rows.</summary>
public sealed class GdkLeaderboard : GdkObject
{
    internal GdkLeaderboard(GodotObject o) : base(o) { }
    public static GdkLeaderboard From(GodotObject o) => o == null ? null : new GdkLeaderboard(o);

    public string StatName => GetString("stat_name");
    public string QueryType => GetString("query_type");
    public int TotalRowCount => GetInt32("total_row_count");
    public bool HasNext => GetBool("has_next");
    public Godot.Collections.Array Columns => GetArray("columns");
    public Godot.Collections.Array Rows => GetArray("rows");
}

/// <summary>A leaderboard column descriptor (stat name + type).</summary>
public sealed class GdkLeaderboardColumn : GdkObject
{
    internal GdkLeaderboardColumn(GodotObject o) : base(o) { }
    public static GdkLeaderboardColumn From(GodotObject o) => o == null ? null : new GdkLeaderboardColumn(o);

    public string StatName => GetString("stat_name");
    public string StatType => GetString("stat_type");
}

/// <summary>A single leaderboard row (a ranked player and their values).</summary>
public sealed class GdkLeaderboardRow : GdkObject
{
    internal GdkLeaderboardRow(GodotObject o) : base(o) { }
    public static GdkLeaderboardRow From(GodotObject o) => o == null ? null : new GdkLeaderboardRow(o);

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
