using FirmwareUf2Flasher.Services;

namespace FirmwareUf2Flasher;

public sealed class MainForm : Form
{
    private readonly TextBox _uf2PathText = new() { Dock = DockStyle.Fill };
    private readonly Button _browseButton = new() { Text = "Browse..." };
    private readonly ComboBox _portCombo = new() { Dock = DockStyle.Fill, DropDownStyle = ComboBoxStyle.DropDownList };
    private readonly Button _refreshPortsButton = new() { Text = "Refresh" };
    private readonly TextBox _expectedBuildText = new() { Dock = DockStyle.Fill, PlaceholderText = "YYYY-MM-DD HH:MM:SS" };
    private readonly Button _uploadButton = new() { Text = "Upload", AutoSize = true };
    private readonly Button _cancelButton = new() { Text = "Cancel", AutoSize = true, Enabled = false };
    private readonly Label _statusLabel = new() { Text = "Idle", AutoSize = true };
    private readonly TextBox _logText = new()
    {
        Dock = DockStyle.Fill,
        Multiline = true,
        ScrollBars = ScrollBars.Vertical,
        ReadOnly = true
    };

    private CancellationTokenSource? _uploadCts;
    private bool _busy;

    public MainForm()
    {
        Text = "Camptraptions UF2 Flasher (Prototype)";
        Width = 900;
        Height = 620;
        StartPosition = FormStartPosition.CenterScreen;

        BuildLayout();
        HookEvents();
        RefreshPortList();
    }

    private void BuildLayout()
    {
        var root = new TableLayoutPanel
        {
            Dock = DockStyle.Fill,
            ColumnCount = 1,
            RowCount = 3,
            Padding = new Padding(10)
        };
        root.RowStyles.Add(new RowStyle(SizeType.AutoSize));
        root.RowStyles.Add(new RowStyle(SizeType.AutoSize));
        root.RowStyles.Add(new RowStyle(SizeType.Percent, 100));

        var inputs = new TableLayoutPanel { Dock = DockStyle.Top, ColumnCount = 4, AutoSize = true };
        inputs.ColumnStyles.Add(new ColumnStyle(SizeType.AutoSize));
        inputs.ColumnStyles.Add(new ColumnStyle(SizeType.Percent, 100));
        inputs.ColumnStyles.Add(new ColumnStyle(SizeType.AutoSize));
        inputs.ColumnStyles.Add(new ColumnStyle(SizeType.AutoSize));

        inputs.Controls.Add(new Label { Text = "UF2 file:", Anchor = AnchorStyles.Left, AutoSize = true }, 0, 0);
        inputs.Controls.Add(_uf2PathText, 1, 0);
        inputs.Controls.Add(_browseButton, 2, 0);
        inputs.SetColumnSpan(_browseButton, 2);

        inputs.Controls.Add(new Label { Text = "Device COM:", Anchor = AnchorStyles.Left, AutoSize = true }, 0, 1);
        inputs.Controls.Add(_portCombo, 1, 1);
        inputs.Controls.Add(_refreshPortsButton, 2, 1);

        inputs.Controls.Add(new Label { Text = "Expected build:", Anchor = AnchorStyles.Left, AutoSize = true }, 0, 2);
        inputs.Controls.Add(_expectedBuildText, 1, 2);
        inputs.SetColumnSpan(_expectedBuildText, 3);

        var actions = new FlowLayoutPanel { Dock = DockStyle.Top, AutoSize = true, FlowDirection = FlowDirection.LeftToRight };
        actions.Controls.Add(_uploadButton);
        actions.Controls.Add(_cancelButton);
        actions.Controls.Add(new Label { Text = "Status:", AutoSize = true, Padding = new Padding(8, 6, 0, 0) });
        actions.Controls.Add(_statusLabel);

        root.Controls.Add(inputs, 0, 0);
        root.Controls.Add(actions, 0, 1);
        root.Controls.Add(_logText, 0, 2);
        Controls.Add(root);
    }

    private void HookEvents()
    {
        _browseButton.Click += (_, _) =>
        {
            using var ofd = new OpenFileDialog
            {
                Filter = "UF2 files (*.uf2)|*.uf2|All files (*.*)|*.*",
                CheckFileExists = true
            };
            if (ofd.ShowDialog(this) == DialogResult.OK)
            {
                _uf2PathText.Text = ofd.FileName;
            }
        };

        _refreshPortsButton.Click += (_, _) => RefreshPortList();
        _uploadButton.Click += async (_, _) => await RunUploadAsync();
        _cancelButton.Click += (_, _) => _uploadCts?.Cancel();
    }

    private void RefreshPortList()
    {
        var ports = BootloaderService.GetPorts();
        _portCombo.Items.Clear();
        foreach (var port in ports)
        {
            _portCombo.Items.Add(port);
        }

        if (_portCombo.Items.Count > 0)
        {
            _portCombo.SelectedIndex = 0;
        }
        else
        {
            _portCombo.Text = string.Empty;
        }
    }

    private async Task RunUploadAsync()
    {
        if (_busy)
        {
            return;
        }

        var uf2Path = _uf2PathText.Text.Trim();
        var expectedBuild = _expectedBuildText.Text.Trim();
        var selectedPort = _portCombo.SelectedItem?.ToString();

        if (string.IsNullOrWhiteSpace(uf2Path) || !File.Exists(uf2Path))
        {
            MessageBox.Show(this, "Choose a valid UF2 file first.", "UF2 Flasher", MessageBoxButtons.OK, MessageBoxIcon.Warning);
            return;
        }

        if (string.IsNullOrWhiteSpace(selectedPort))
        {
            MessageBox.Show(this, "Select a COM port first.", "UF2 Flasher", MessageBoxButtons.OK, MessageBoxIcon.Warning);
            return;
        }

        if (string.IsNullOrWhiteSpace(expectedBuild))
        {
            MessageBox.Show(this, "Expected build token is required for strict verification.", "UF2 Flasher", MessageBoxButtons.OK, MessageBoxIcon.Warning);
            return;
        }

        _uploadCts = new CancellationTokenSource();
        SetBusy(true);
        SetStatus("Running...");
        AppendLog("=== UF2 upload run started ===");

        try
        {
            var trigger = await BootloaderService.Trigger1200BaudAsync(selectedPort, AppendLog, _uploadCts.Token);

            var uf2Drive = await Uf2DriveService.WaitForUf2DriveAsync(
                TimeSpan.FromSeconds(25),
                AppendLog,
                _uploadCts.Token);

            await Uf2DriveService.CopyUf2Async(uf2Path, uf2Drive, AppendLog, _uploadCts.Token);
            await Uf2DriveService.WaitForDriveRemovalAsync(
                uf2Drive.Name,
                TimeSpan.FromSeconds(15),
                AppendLog,
                _uploadCts.Token);

            var candidatePorts = trigger.PortsAfter.Concat(trigger.PortsBefore).Distinct(StringComparer.OrdinalIgnoreCase).ToArray();
            var verification = await SerialVerifyService.VerifyBootLineAsync(
                candidatePorts,
                expectedBuild,
                TimeSpan.FromSeconds(30),
                AppendLog,
                _uploadCts.Token);

            if (!verification.Success)
            {
                SetStatus("FAILED");
                AppendLog($"Verification failed: {verification.FailureReason}");
                MessageBox.Show(this, verification.FailureReason ?? "Verification failed.", "UF2 Flasher", MessageBoxButtons.OK, MessageBoxIcon.Error);
                return;
            }

            SetStatus("PASS");
            AppendLog($"Verified on {verification.PortName}: {verification.MatchedBootLine}");
            MessageBox.Show(this, "Upload + strict verification passed.", "UF2 Flasher", MessageBoxButtons.OK, MessageBoxIcon.Information);
        }
        catch (OperationCanceledException)
        {
            SetStatus("Canceled");
            AppendLog("Run canceled by user.");
        }
        catch (Exception ex)
        {
            SetStatus("FAILED");
            AppendLog($"ERROR: {ex.Message}");
            MessageBox.Show(this, ex.Message, "UF2 Flasher", MessageBoxButtons.OK, MessageBoxIcon.Error);
        }
        finally
        {
            SetBusy(false);
            _uploadCts?.Dispose();
            _uploadCts = null;
            AppendLog("=== Run complete ===");
        }
    }

    private void SetBusy(bool busy)
    {
        _busy = busy;
        _uploadButton.Enabled = !busy;
        _browseButton.Enabled = !busy;
        _refreshPortsButton.Enabled = !busy;
        _portCombo.Enabled = !busy;
        _expectedBuildText.Enabled = !busy;
        _uf2PathText.Enabled = !busy;
        _cancelButton.Enabled = busy;
    }

    private void SetStatus(string value)
    {
        _statusLabel.Text = value;
    }

    private void AppendLog(string message)
    {
        var line = $"{DateTime.Now:HH:mm:ss.fff} {message}";
        if (InvokeRequired)
        {
            BeginInvoke(new Action(() => AppendLog(message)));
            return;
        }

        _logText.AppendText(line + Environment.NewLine);
    }
}
