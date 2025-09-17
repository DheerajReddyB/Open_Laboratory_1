% Parameters
Fs = 2048;          % Sampling frequency (Hz)
f = 5000;            % Sine wave frequency (Hz)
A = 1.0;            % Amplitude
duration = 1.0;     % Duration in seconds
N = Fs * duration;  % Total samples

% Time vector and sine wave
t = (0:N-1)/Fs;
x = A * sin(2*pi*f*t);

% FFT
X = fft(x);
Mag = abs(X) * 2 / N;           % Normalize for amplitude
Phase = angle(X);               % Phase spectrum
f_axis = (0:N-1) * Fs / N;      % Frequency axis

% Plot Time-Domain Signal
figure;
plot(t, x);
xlabel('Time (s)');
ylabel('Amplitude');
title('Sine Wave');
grid on;

% Plot Magnitude Spectrum
figure;
stem(f_axis(1:N/2), Mag(1:N/2), 'filled');
xlabel('Frequency (Hz)');
ylabel('Magnitude');
title('FFT Magnitude Spectrum');
grid on;

% Plot Phase Spectrum
figure;
stem(f_axis(1:N/2), Phase(1:N/2), 'filled');
xlabel('Frequency (Hz)');
ylabel('Phase (rad)');
title('FFT Phase Spectrum');
grid on;

% Display key FFT bin (should show spike near bin 100)
[~, peak_bin] = max(Mag);
fprintf('Peak Bin: %d\n', peak_bin);
fprintf('Frequency: %.2f Hz\n', f_axis(peak_bin));
fprintf('Magnitude: %.4f\n', Mag(peak_bin));
fprintf('Phase: %.4f radians\n', Phase(peak_bin));

% Save CSVs (optional)
%writematrix([t(:), x(:)], 'sine_wave_matlab.csv');   % Time-domain
%fft_out = [f_axis(:), Mag(:), Phase(:)];
%writematrix(fft_out, 'fft_output_matlab.csv');       % Frequency-domain
