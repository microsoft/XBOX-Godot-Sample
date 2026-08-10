using Godot;
using GodotXbox.Internal;

namespace GodotXbox.Types;

/// <summary>Metadata for a single title-storage blob.</summary>
public sealed class XboxTitleStorageBlobMetadata : XboxObject
{
    internal XboxTitleStorageBlobMetadata(GodotObject o) : base(o) { }
    public static XboxTitleStorageBlobMetadata From(GodotObject o) => o == null ? null : new XboxTitleStorageBlobMetadata(o);

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
public sealed class XboxTitleStorageBlobMetadataResult : XboxObject
{
    internal XboxTitleStorageBlobMetadataResult(GodotObject o) : base(o) { }
    public static XboxTitleStorageBlobMetadataResult From(GodotObject o) => o == null ? null : new XboxTitleStorageBlobMetadataResult(o);

    public Godot.Collections.Array Items => GetArray("items");
    public bool HasNext => GetBool("has_next");
    public string StorageType => GetString("storage_type");
    public string BlobPath => GetString("blob_path");
}
