using Godot;
using GodotGdk.Internal;

namespace GodotGdk.Types;

/// <summary>Metadata for a single title-storage blob.</summary>
public sealed class GdkTitleStorageBlobMetadata : GdkObject
{
    internal GdkTitleStorageBlobMetadata(GodotObject o) : base(o) { }
    public static GdkTitleStorageBlobMetadata From(GodotObject o) => o == null ? null : new GdkTitleStorageBlobMetadata(o);

    public string BlobPath => GetString("blob_path");
    public string BlobType => GetString("blob_type");
    public string StorageType => GetString("storage_type");
    public string DisplayName => GetString("display_name");
    public string ETag => GetString("e_tag");
    public long ClientTimestamp => GetInt("client_timestamp");
    public long Length => GetInt("length");
    public string ServiceConfigurationId => GetString("service_configuration_id");
    public string Xuid => GetString("xuid");
}

/// <summary>A page of blob metadata, with paging state.</summary>
public sealed class GdkTitleStorageBlobMetadataResult : GdkObject
{
    internal GdkTitleStorageBlobMetadataResult(GodotObject o) : base(o) { }
    public static GdkTitleStorageBlobMetadataResult From(GodotObject o) => o == null ? null : new GdkTitleStorageBlobMetadataResult(o);

    public Godot.Collections.Array Items => GetArray("items");
    public bool HasNext => GetBool("has_next");
    public string StorageType => GetString("storage_type");
    public string BlobPath => GetString("blob_path");
}
