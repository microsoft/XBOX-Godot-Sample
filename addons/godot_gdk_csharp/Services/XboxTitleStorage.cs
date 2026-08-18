using System.Threading.Tasks;
using Godot;
using GodotXbox.Internal;
using GodotXbox.Types;

namespace GodotXbox.Services;

/// <summary><c>GDK.title_storage</c> — title-managed blob storage.</summary>
public sealed class XboxTitleStorage : XboxServiceBase
{
    internal XboxTitleStorage(GodotObject o) : base(o) { }

    public Task<XboxResult> GetQuotaAsync(XboxUser user, string storageType) =>
        CallResultAsync("get_quota_async", user?.Raw, storageType);

    public Task<XboxResult> ListBlobMetadataAsync(XboxUser user, string storageType, string blobPath, int skipItems, int maxItems) =>
        CallResultAsync("list_blob_metadata_async", user?.Raw, storageType, blobPath, skipItems, maxItems);

    public Task<XboxResult> GetNextBlobMetadataAsync(XboxTitleStorageBlobMetadataResult result) =>
        CallResultAsync("get_next_blob_metadata_async", result?.Raw);

    public Task<XboxResult> DownloadBlobAsync(XboxUser user, string storageType, string blobPath) =>
        CallResultAsync("download_blob_async", user?.Raw, storageType, blobPath);

    public Task<XboxResult> UploadBlobAsync(
        XboxUser user, string storageType, string blobPath, byte[] data,
        string displayName = "", string eTag = "", string matchCondition = "") =>
        CallResultAsync("upload_blob_async", user?.Raw, storageType, blobPath, data, displayName, eTag, matchCondition);

    public Task<XboxResult> DeleteBlobAsync(
        XboxUser user, string storageType, string blobPath, string eTag = "", string matchCondition = "") =>
        CallResultAsync("delete_blob_async", user?.Raw, storageType, blobPath, eTag, matchCondition);
}
