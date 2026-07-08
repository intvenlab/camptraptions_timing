using System.IO.Ports;

namespace FirmwareUf2Flasher.Services;

internal sealed class BootloaderTriggerResult
{
    public required string TriggerPort { get; init; }
    public required IReadOnlyList<string> PortsBefore { get; init; }
    public required IReadOnlyList<string> PortsAfter { get; init; }
}

internal static class BootloaderService
{
    public static IReadOnlyList<string> GetPorts()
    {
        return SerialPort.GetPortNames()
            .OrderBy(static p => p, StringComparer.OrdinalIgnoreCase)
            .ToArray();
    }

    public static async Task<BootloaderTriggerResult> Trigger1200BaudAsync(
        string portName,
        Action<string> log,
        CancellationToken cancellationToken)
    {
        var portsBefore = GetPorts();
        if (!portsBefore.Contains(portName, StringComparer.OrdinalIgnoreCase))
        {
            throw new InvalidOperationException($"Selected port '{portName}' is not present.");
        }

        log($"Tickle bootloader on {portName} (1200 baud open/close).");
        using (var serial = new SerialPort(portName, 1200))
        {
            serial.ReadTimeout = 250;
            serial.WriteTimeout = 250;
            serial.DtrEnable = true;
            serial.RtsEnable = true;
            serial.Open();
            await Task.Delay(200, cancellationToken).ConfigureAwait(false);
            serial.Close();
        }

        await Task.Delay(250, cancellationToken).ConfigureAwait(false);
        var portsAfter = await WaitForPortChangeAsync(portsBefore, TimeSpan.FromSeconds(8), log, cancellationToken)
            .ConfigureAwait(false);

        return new BootloaderTriggerResult
        {
            TriggerPort = portName,
            PortsBefore = portsBefore,
            PortsAfter = portsAfter
        };
    }

    private static async Task<IReadOnlyList<string>> WaitForPortChangeAsync(
        IReadOnlyList<string> baseline,
        TimeSpan timeout,
        Action<string> log,
        CancellationToken cancellationToken)
    {
        var deadline = DateTime.UtcNow + timeout;
        while (DateTime.UtcNow < deadline)
        {
            cancellationToken.ThrowIfCancellationRequested();
            var current = GetPorts();
            if (!AreEqualSet(baseline, current))
            {
                log($"Serial port change detected: {string.Join(", ", current)}");
                return current;
            }

            await Task.Delay(200, cancellationToken).ConfigureAwait(false);
        }

        log("No serial-port change detected within timeout; continuing.");
        return GetPorts();
    }

    private static bool AreEqualSet(IReadOnlyList<string> left, IReadOnlyList<string> right)
    {
        if (left.Count != right.Count)
        {
            return false;
        }

        for (var i = 0; i < left.Count; i++)
        {
            if (!left[i].Equals(right[i], StringComparison.OrdinalIgnoreCase))
            {
                return false;
            }
        }

        return true;
    }
}
