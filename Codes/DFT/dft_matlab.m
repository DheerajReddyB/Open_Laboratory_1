% Parameters
Fs = 1000;              % Sampling frequency (Hz)
T = 1/Fs;               % Sampling period
L = 1000;               % Length of signal
t = (0:L-1)*T;          % Time vector

% Sine Wave: f = 50 Hz
f_sine = 10                      0;            % Frequency of sine wave
A = 1;                  % Amplitude
y = A * sin(2*pi*f_sine*t);

% Compute DFT using FFT
Y = fft(y);

% Frequency vector
f = Fs*(0:(L/2))/L;

% Compute amplitude spectrum (magnitude)
P2 = abs(Y/L);          % Two-sided spectrum
P1 = P2(1:L/2+1);       % Single-sided spectrum
P1(2:end-1) = 2*P1(2:end-1);

% Compute phase spectrum
phase = angle(Y);
phase = phase(1:L/2+1);

% Plot time-domain sine wave
figure;
subplot(3,1,1);
plot(t, y);
title('Time-Domain Sine Wave');
xlabel('Time (s)');
ylabel('Amplitude');
grid on;

% Plot amplitude spectrum
subplot(3,1,2);
stem(f, P1);
title('Amplitude Spectrum');
xlabel('Frequency (Hz)');
ylabel('|Y(f)|');
grid on;

% Plot phase spectrum
subplot(3,1,3);
stem(f, phase);
title('Phase Spectrum');
xlabel('Frequency (Hz)');
ylabel('Phase (radians)');
grid on;
