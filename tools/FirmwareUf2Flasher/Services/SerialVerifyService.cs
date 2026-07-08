using System.IO.Ports;

namespace FirmwareUf2Flasher.Services;

internal sealed class BootVerificationResult
{
    public required bool Success { get; init; }
    public required string PortName { get; init; }
    public required string? MatchedBootLine { get; init; }
    public required string? FailureReason { get; init; }
}

internal static class SerialVerifyService
{
    public static async Task<BootVerificationResult> VerifyBootLineAsync(
        IReadOnlyList<string> preferredPorts,
        string expectedBuildToken,
        TimeSpan timeout,
        Action<string> log,
        CancellationToken cancellationToken)
    {
        var deadline = DateTime.UtcNow + timeout;
        while (DateTime.UtcNow < deadline)
        {
            cancellationToken.ThrowIfCancellationRequested();
            var candidates = GetCandidatePorts(preferredPorts);
            foreach (var port in candidates)
            {
                var result = await TryReadBootLineAsync(port, expectedBuildToken, log, cancellationToken)
                    .ConfigureAwait(false);
                if (result is not null)
                {
                    return result;
                }
            }

            await Task.Delay(250, cancellationToken).ConfigureAwait(false);
        }

        return new BootVerificationResult
        {
            Success = false,
            PortName = "(none)",
            MatchedBootLine = null,
            FailureReason = "Timeout waiting for matching [BOOT] line."
        };
    }

    private static IReadOnlyList<string> GetCandidatePorts(IReadOnlyList<string> preferredPorts)
    {
        var live = SerialPort.GetPortNames()
            .Where(static p => !string.IsNullOrWhiteSpace(p))
            .Select(static p => p.Trim())
            .OrderBy(static p => p, StringComparer.OrdinalIgnoreCase)
            .ToList();

        if (preferredPorts.Count == 0)
        {
            return live;
        }

        var ranked = preferredPorts
            .Where(p => live.Contains(p, StringComparer.OrdinalIgnoreCase))
            .ToList();

        foreach (var port in live)
        {
            if (!ranked.Contains(port, StringComparer.OrdinalIgnoreCase))
            {
                ranked.Add(port);
            }
        }

        return ranked;
    }

    private static async Task<BootVerificationResult?> TryReadBootLineAsync(
        string portName,
        string expectedBuildToken,
        Action<string> log,
        CancellationToken cancellationToken)
    {
        try
        {
            using var serial = new SerialPort(portName, 115200);
            serial.ReadTimeout = 400;
            serial.WriteTimeout = 400;
            serial.NewLine = "\n";
            serial.Open();
            log($"Reading boot output from {portName}...");

            var started = DateTime.UtcNow;
            var localWindow = TimeSpan.FromSeconds(5);
            while (DateTime.UtcNow - started < localWindow)
            {
                cancellationToken.ThrowIfCancellationRequested();
                string? line;
                try
                {
                    line = await Task.Run(serial.ReadLine, cancellationToken).ConfigureAwait(false);
                }
                catch (TimeoutException)
                {
                    continue;
                }

                if (string.IsNullOrWhiteSpace(line))
                {
                    continue;
                }

                line = line.Trim();
                log($"[{portName}] {line}");
                if (!line.StartsWith("[BOOT]", StringComparison.Ordinal))
                {
                    continue;
                }

                var expectedToken = $"build={expectedBuildToken}";
                if (line.Contains(expectedToken, StringComparison.Ordinal))
                {
                    return new BootVerificationResult
                    {
                        Success = true,
                        PortName = portName,
                        MatchedBootLine = line,
                        FailureReason = null
                    };
                }

                return new BootVerificationResult
                {
                    Success = false,
                    PortName = portName,
                    MatchedBootLine = line,
                    FailureReason = $"Boot line seen but expected token '{expectedToken}' not found."
                };
            }
        }
        catch (UnauthorizedAccessException)
        {
            return null;
        }
        catch (IOException)
        {
            return null;
        }
        catch (InvalidOperationException)
        {
            return null;
        }

        return null;
    }
}
