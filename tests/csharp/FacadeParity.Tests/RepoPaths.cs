using System;
using System.IO;

namespace FacadeParity.Tests;

/// <summary>Locates repo-relative paths from the test bin directory.</summary>
internal static class RepoPaths
{
    private static string _root;

    public static string Root
    {
        get
        {
            if (_root != null)
            {
                return _root;
            }

            var dir = new DirectoryInfo(AppContext.BaseDirectory);
            while (dir != null)
            {
                if (Directory.Exists(Path.Combine(dir.FullName, "addons")) &&
                    Directory.Exists(Path.Combine(dir.FullName, "spec")))
                {
                    _root = dir.FullName;
                    return _root;
                }

                dir = dir.Parent;
            }

            throw new DirectoryNotFoundException("Could not locate repo root from " + AppContext.BaseDirectory);
        }
    }

    public static string DocClasses(string addon) =>
        Path.Combine(Root, "addons", addon, "doc_classes");
}
