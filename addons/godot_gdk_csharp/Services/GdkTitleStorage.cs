using System.Threading.Tasks;
using Godot;
using GodotGdk.Internal;
using GodotGdk.Types;

namespace GodotGdk.Services;

/// <summary><c>GDK.title_storage</c> — title-managed blob storage.</summary>
public sealed class GdkTitleStorage : GdkServiceBase
{
    internal GdkTitleStorage(GodotObject o) : base(o) { }

    public Task<GdkResult> GetQuotaAsync(GdkUser user, string storageType) =>
        CallResultAsync("get_quota_async", user?.Raw, storageType);

    public Task<GdkResult> ListBlobMetadataAsync(GdkUser user, string storageType, string blobPath, int skipItems, int maxItems) =>
        CallResultAsync("list_blob_metadata_async", user?.Raw, storageType, blobPath, skipItems, maxItems);

    public Task<GdkResult> GetNextBlobMetadataAsync(GdkTitleStorageBlobMetadataResult result) =>
        CallResultAsync("get_next_blob_metadata_async", result?.Raw);

    public Task<GdkResult> DownloadBlobAsync(GdkUser user, string storageType, string blobPath) =>
        CallResultAsync("download_blob_async", user?.Raw, storageType, blobPath);

    public Task<GdkResult> UploadBlobAsync(
        GdkUser user, string storageType, string blobPath, byte[] data,
        string displayName = "", string eTag = "", string matchCondition = "") =>
        CallResultAsync("upload_blob_async", user?.Raw, storageType, blobPath, data, displayName, eTag, matchCondition);

    public Task<GdkResult> DeleteBlobAsync(
        GdkUser user, string storageType, string blobPath, string eTag = "", string matchCondition = "") =>
        CallResultAsync("delete_blob_async", user?.Raw, storageType, blobPath, eTag, matchCondition);
}
