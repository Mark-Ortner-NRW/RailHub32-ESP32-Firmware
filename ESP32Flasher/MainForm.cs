using System;
using System.Diagnostics;
using System.Drawing;
using System.Drawing.Drawing2D;
using System.IO;
using System.IO.Ports;
using System.Linq;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace ESP32Flasher
{
    public partial class MainForm : Form
    {
        // Volvo-inspired color palette
        private static readonly Color VolvoBlack = Color.FromArgb(20, 20, 20);
        private static readonly Color VolvoGold = Color.FromArgb(204, 166, 102);
        private static readonly Color VolvoLightGray = Color.FromArgb(240, 240, 240);
        private static readonly Color VolvoMediumGray = Color.FromArgb(100, 100, 100);
        private static readonly Color VolvoDarkGray = Color.FromArgb(45, 45, 45);
        private static readonly Color VolvoAccent = Color.FromArgb(0, 120, 215);

        private Label titleLabel;
        private Label statusLabel;
        private Panel progressPanel;
        private Panel progressBarFill;
        private Label progressLabel;
        private Label detailsLabel;
        private Button flashButton;
        private PictureBox iconBox;

        private string? detectedPort;
        private bool isFlashing = false;
        private int progressValue = 0;

        public MainForm()
        {
            InitializeComponent();
            SetupVolvoUI();
            _ = DetectESP32Async();
        }

        private void InitializeComponent()
        {
            this.Text = "RailHub32 Flasher";
            this.Size = new Size(600, 450);
            this.BackColor = VolvoBlack;
            this.FormBorderStyle = FormBorderStyle.FixedDialog;
            this.MaximizeBox = false;
            this.StartPosition = FormStartPosition.CenterScreen;
            this.Font = new Font("Segoe UI", 10F, FontStyle.Regular);
        }

        private void SetupVolvoUI()
        {
            // Icon/Logo area
            iconBox = new PictureBox
            {
                Size = new Size(64, 64),
                Location = new Point(268, 30),
                BackColor = Color.Transparent,
                SizeMode = PictureBoxSizeMode.CenterImage
            };
            DrawVolvoIcon(iconBox);
            this.Controls.Add(iconBox);

            // Title
            titleLabel = new Label
            {
                Text = "RailHub32",
                Font = new Font("Segoe UI Light", 28F, FontStyle.Regular),
                ForeColor = VolvoLightGray,
                AutoSize = false,
                Size = new Size(600, 50),
                Location = new Point(0, 105),
                TextAlign = ContentAlignment.MiddleCenter,
                BackColor = Color.Transparent
            };
            this.Controls.Add(titleLabel);

            // Subtitle
            var subtitleLabel = new Label
            {
                Text = "ESP32 FIRMWARE FLASHER",
                Font = new Font("Segoe UI", 9F, FontStyle.Regular),
                ForeColor = VolvoMediumGray,
                AutoSize = false,
                Size = new Size(600, 25),
                Location = new Point(0, 155),
                TextAlign = ContentAlignment.MiddleCenter,
                BackColor = Color.Transparent
            };
            this.Controls.Add(subtitleLabel);

            // Status Label
            statusLabel = new Label
            {
                Text = "Initializing...",
                Font = new Font("Segoe UI", 11F, FontStyle.Regular),
                ForeColor = VolvoGold,
                AutoSize = false,
                Size = new Size(500, 30),
                Location = new Point(50, 200),
                TextAlign = ContentAlignment.MiddleLeft,
                BackColor = Color.Transparent
            };
            this.Controls.Add(statusLabel);

            // Progress Panel Container
            progressPanel = new Panel
            {
                Size = new Size(500, 8),
                Location = new Point(50, 240),
                BackColor = VolvoDarkGray,
                Visible = false
            };

            // Progress Bar Fill
            progressBarFill = new Panel
            {
                Size = new Size(0, 8),
                Location = new Point(0, 0),
                BackColor = VolvoGold
            };
            progressPanel.Controls.Add(progressBarFill);
            this.Controls.Add(progressPanel);

            // Progress Percentage Label
            progressLabel = new Label
            {
                Text = "0%",
                Font = new Font("Segoe UI", 9F, FontStyle.Regular),
                ForeColor = VolvoMediumGray,
                AutoSize = false,
                Size = new Size(500, 25),
                Location = new Point(50, 255),
                TextAlign = ContentAlignment.MiddleLeft,
                BackColor = Color.Transparent,
                Visible = false
            };
            this.Controls.Add(progressLabel);

            // Details Label
            detailsLabel = new Label
            {
                Text = "",
                Font = new Font("Consolas", 8.5F, FontStyle.Regular),
                ForeColor = VolvoMediumGray,
                AutoSize = false,
                Size = new Size(500, 60),
                Location = new Point(50, 285),
                BackColor = Color.Transparent
            };
            this.Controls.Add(detailsLabel);

            // Flash Button
            flashButton = new Button
            {
                Text = "FLASH FIRMWARE",
                Font = new Font("Segoe UI", 11F, FontStyle.Regular),
                Size = new Size(200, 45),
                Location = new Point(200, 360),
                FlatStyle = FlatStyle.Flat,
                BackColor = VolvoDarkGray,
                ForeColor = VolvoLightGray,
                Cursor = Cursors.Hand,
                Enabled = false
            };
            flashButton.FlatAppearance.BorderSize = 0;
            flashButton.FlatAppearance.MouseOverBackColor = VolvoAccent;
            flashButton.FlatAppearance.MouseDownBackColor = Color.FromArgb(0, 100, 180);
            flashButton.Click += FlashButton_Click;
            this.Controls.Add(flashButton);
        }

        private void DrawVolvoIcon(PictureBox pictureBox)
        {
            var bmp = new Bitmap(64, 64);
            using (var g = Graphics.FromImage(bmp))
            {
                g.SmoothingMode = SmoothingMode.AntiAlias;
                g.Clear(Color.Transparent);

                // Draw Volvo-inspired circular logo with diagonal line
                using (var pen = new Pen(VolvoGold, 3))
                {
                    // Circle
                    g.DrawEllipse(pen, 8, 8, 48, 48);
                    
                    // Diagonal arrow (representing upload/flash)
                    g.DrawLine(pen, 24, 40, 40, 24);
                    
                    // Arrow head
                    g.DrawLine(pen, 40, 24, 35, 26);
                    g.DrawLine(pen, 40, 24, 38, 29);
                }
            }
            pictureBox.Image = bmp;
        }

        private async Task DetectESP32Async()
        {
            await Task.Delay(500);

            UpdateStatus("Scanning for ESP32 device...", VolvoGold);

            var ports = SerialPort.GetPortNames();
            
            foreach (var port in ports)
            {
                try
                {
                    using (var serialPort = new SerialPort(port, 115200))
                    {
                        serialPort.Open();
                        await Task.Delay(100);
                        serialPort.Close();
                        
                        // Found a valid port
                        detectedPort = port;
                        UpdateStatus($"ESP32 detected on {port}", VolvoGold);
                        UpdateDetails($"Port: {port}\nBaud Rate: 115200\nReady to flash");
                        flashButton.Invoke(() => flashButton.Enabled = true);
                        return;
                    }
                }
                catch
                {
                    // Port not accessible or no device
                    continue;
                }
            }

            UpdateStatus("No ESP32 found - Connect device and restart", Color.FromArgb(180, 0, 0));
            UpdateDetails("Please connect your ESP32 board via USB\nand restart the application.");
        }

        private async void FlashButton_Click(object? sender, EventArgs e)
        {
            if (isFlashing) return;

            isFlashing = true;
            flashButton.Enabled = false;
            progressPanel.Visible = true;
            progressLabel.Visible = true;

            await FlashFirmwareAsync();

            isFlashing = false;
            await Task.Delay(3000);
            
            // Reset UI
            progressPanel.Visible = false;
            progressLabel.Visible = false;
            flashButton.Enabled = true;
        }

        private async Task FlashFirmwareAsync()
        {
            try
            {
                UpdateStatus("Preparing firmware...", VolvoGold);
                UpdateProgress(5);

                // Get firmware file from esp32-controller directory
                var firmwarePath = Path.Combine(
                    Path.GetDirectoryName(Application.ExecutablePath) ?? "",
                    "..", "..", "..", "..", "esp32-controller", ".pio", "build", "esp32dev", "firmware.bin"
                );

                var bootloaderPath = Path.Combine(
                    Path.GetDirectoryName(Application.ExecutablePath) ?? "",
                    "..", "..", "..", "..", "esp32-controller", ".pio", "build", "esp32dev", "bootloader.bin"
                );

                var partitionsPath = Path.Combine(
                    Path.GetDirectoryName(Application.ExecutablePath) ?? "",
                    "..", "..", "..", "..", "esp32-controller", ".pio", "build", "esp32dev", "partitions.bin"
                );

                firmwarePath = Path.GetFullPath(firmwarePath);
                bootloaderPath = Path.GetFullPath(bootloaderPath);
                partitionsPath = Path.GetFullPath(partitionsPath);

                if (!File.Exists(firmwarePath))
                {
                    UpdateStatus("Error: Firmware file not found", Color.FromArgb(180, 0, 0));
                    UpdateDetails($"Please build the firmware first.\nExpected: {firmwarePath}");
                    return;
                }

                UpdateProgress(10);
                UpdateStatus("Connecting to ESP32...", VolvoGold);
                UpdateDetails($"Port: {detectedPort}\nErasing flash...");

                // Find esptool.py
                var platformioCore = Path.Combine(
                    Environment.GetFolderPath(Environment.SpecialFolder.UserProfile),
                    ".platformio", "packages", "tool-esptoolpy", "esptool.py"
                );

                if (!File.Exists(platformioCore))
                {
                    UpdateStatus("Error: esptool.py not found", Color.FromArgb(180, 0, 0));
                    UpdateDetails("Please ensure PlatformIO is installed.");
                    return;
                }

                UpdateProgress(20);

                // Build esptool command
                var arguments = $"\"{platformioCore}\" " +
                    $"--chip esp32 " +
                    $"--port {detectedPort} " +
                    $"--baud 921600 " +
                    $"--before default_reset " +
                    $"--after hard_reset " +
                    $"write_flash -z " +
                    $"--flash_mode dio " +
                    $"--flash_freq 40m " +
                    $"--flash_size detect ";

                if (File.Exists(bootloaderPath))
                    arguments += $"0x1000 \"{bootloaderPath}\" ";
                
                if (File.Exists(partitionsPath))
                    arguments += $"0x8000 \"{partitionsPath}\" ";

                arguments += $"0x10000 \"{firmwarePath}\"";

                UpdateProgress(30);
                UpdateStatus("Flashing firmware...", VolvoGold);

                var process = new Process
                {
                    StartInfo = new ProcessStartInfo
                    {
                        FileName = "python",
                        Arguments = arguments,
                        UseShellExecute = false,
                        RedirectStandardOutput = true,
                        RedirectStandardError = true,
                        CreateNoWindow = true
                    }
                };

                var output = "";
                process.OutputDataReceived += (s, e) => {
                    if (!string.IsNullOrEmpty(e.Data))
                    {
                        output += e.Data + "\n";
                        
                        // Parse progress from esptool output
                        if (e.Data.Contains("Writing at"))
                        {
                            var percent = 30 + (int)((output.Split("Writing at").Length - 1) * 1.5);
                            if (percent > 95) percent = 95;
                            UpdateProgress(percent);
                        }
                    }
                };

                process.ErrorDataReceived += (s, e) => {
                    if (!string.IsNullOrEmpty(e.Data))
                    {
                        output += e.Data + "\n";
                    }
                };

                process.Start();
                process.BeginOutputReadLine();
                process.BeginErrorReadLine();

                await process.WaitForExitAsync();

                if (process.ExitCode == 0)
                {
                    UpdateProgress(100);
                    UpdateStatus("Firmware flashed successfully!", VolvoGold);
                    UpdateDetails("ESP32 is rebooting...\nDevice ready for use.");
                }
                else
                {
                    UpdateStatus("Flashing failed", Color.FromArgb(180, 0, 0));
                    UpdateDetails($"Error code: {process.ExitCode}\n" + 
                        $"{output.Split('\n').TakeLast(3).Aggregate((a, b) => a + "\n" + b)}");
                }
            }
            catch (Exception ex)
            {
                UpdateStatus("Error during flash process", Color.FromArgb(180, 0, 0));
                UpdateDetails($"Exception: {ex.Message}");
            }
        }

        private void UpdateStatus(string message, Color color)
        {
            if (statusLabel.InvokeRequired)
            {
                statusLabel.Invoke(() => {
                    statusLabel.Text = message;
                    statusLabel.ForeColor = color;
                });
            }
            else
            {
                statusLabel.Text = message;
                statusLabel.ForeColor = color;
            }
        }

        private void UpdateDetails(string message)
        {
            if (detailsLabel.InvokeRequired)
            {
                detailsLabel.Invoke(() => detailsLabel.Text = message);
            }
            else
            {
                detailsLabel.Text = message;
            }
        }

        private void UpdateProgress(int percent)
        {
            if (percent < 0) percent = 0;
            if (percent > 100) percent = 100;

            progressValue = percent;

            if (progressBarFill.InvokeRequired)
            {
                progressBarFill.Invoke(() => {
                    progressBarFill.Width = (int)(progressPanel.Width * (percent / 100.0));
                    progressLabel.Text = $"{percent}%";
                });
            }
            else
            {
                progressBarFill.Width = (int)(progressPanel.Width * (percent / 100.0));
                progressLabel.Text = $"{percent}%";
            }
        }
    }
}
