namespace FirmwareUf2Flasher.Services;

internal static class Uf2DriveService
{
    public static async Task<DriveInfo> WaitForUf2DriveAsync(
        TimeSpan timeout,
        Action<string> log,
        CancellationToken cancellationToken)
    {
        log("Waiting for UF2 drive...");
        var deadline = DateTime.UtcNow + timeout;
        while (DateTime.UtcNow < deadline)
        {
            cancellationToken.ThrowIfCancellationRequested();
            foreach (var drive in DriveInfo.GetDrives())
            {
                if (!IsUf2Drive(drive))
                {
                    continue;
                }

                log($"UF2 drive detected: {drive.Name} ({SafeVolumeLabel(drive)})");
                return drive;
            }

            await Task.Delay(250, cancellationToken).ConfigureAwait(false);
        }

        throw new TimeoutException("Timed out waiting for UF2 drive.");
    }

    public static async Task<string> CopyUf2Async(
        string uf2SourcePath,
        DriveInfo uf2Drive,
        Action<string> log,
        CancellationToken cancellationToken)
    {
        if (!File.Exists(uf2SourcePath))
        {
            throw new FileNotFoundException("UF2 file not found.", uf2SourcePath);
        }

        var destinationPath = Path.Combine(uf2Drive.RootDirectory.FullName, Path.GetFileName(uf2SourcePath));
        const int maxAttempts = 5;

        for (var attempt = 1; attempt <= maxAttempts; attempt++)
        {
            cancellationToken.ThrowIfCancellationRequested();
            try
            {
                log($"Copying UF2 to {destinationPath} (attempt {attempt}/{maxAttempts})...");
                File.Copy(uf2SourcePath, destinationPath, true);
                log("UF2 copy complete.");
                return destinationPath;
            }
            catch (IOException ex) when (attempt < maxAttempts)
            {
                log($"Copy attempt failed: {ex.Message}");
                await Task.Delay(300, cancellationToken).ConfigureAwait(false);
            }
            catch (UnauthorizedAccessException ex) when (attempt < maxAttempts)
            {
                log($"Copy attempt failed: {ex.Message}");
                await Task.Delay(300, cancellationToken).ConfigureAwait(false);
            }
        }

        throw new IOException("Failed to copy UF2 file to drive after multiple retries.");
    }

    public static async Task WaitForDriveRemovalAsync(
        string driveRoot,
        TimeSpan timeout,
        Action<string> log,
        CancellationToken cancellationToken)
    {
        log("Waiting for UF2 drive to disconnect (board reboot)...");
        var deadline = DateTime.UtcNow + timeout;
        while (DateTime.UtcNow < deadline)
        {
            cancellationToken.ThrowIfCancellationRequested();
            var stillPresent = DriveInfo.GetDrives()
                .Any(d => d.Name.Equals(driveRoot, StringComparison.OrdinalIgnoreCase));
            if (!stillPresent)
            {
                log("UF2 drive disconnected.");
                return;
            }

            await Task.Delay(200, cancellationToken).ConfigureAwait(false);
        }

        log("UF2 drive still present after timeout; continuing to serial verification.");
    }

    private static bool IsUf2Drive(DriveInfo drive)
    {
        try
        {
            if (!drive.IsReady)
            {
                return false;
            }

            if (drive.DriveType is not (DriveType.Removable or DriveType.Fixed))
            {
                return false;
            }

            var infoUf2 = Path.Combine(drive.RootDirectory.FullName, "INFO_UF2.TXT");
            if (File.Exists(infoUf2))
            {
                return true;
            }

            var label = SafeVolumeLabel(drive);
            return label.Contains("UF2", StringComparison.OrdinalIgnoreCase)
                || label.Contains("BOOT", StringComparison.OrdinalIgnoreCase);
        }
        catch
        {
            return false;
        }
    }

    private static string SafeVolumeLabel(DriveInfo drive)
    {
        try
        {
            return drive.VolumeLabel;
        }
        catch
        {
            return "unknown";
        }
    }
}
