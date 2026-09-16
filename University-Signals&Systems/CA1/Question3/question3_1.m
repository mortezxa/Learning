ts = 1e-9;
T = 1e-5;
tau = 1e-6;

t = 0:ts:T;
s = double(t <= tau);
plot(t, s)
xlabel('Time (s)')
ylabel('Amplitude')
title('Radar Transmitted Signal s(t)')
grid on