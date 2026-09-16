ts = 1e-9;
T = 1e-5;
tau = 1e-6;
R = 450;
C = 3e8;
alpha = 0.5;

t = 0:ts:T;
td = 2*R / C;

r = alpha * double(t >= td & t <= td + tau);

figure;
plot(t, r, 'r', 'LineWidth', 1.5);
xlabel('Time (s)');
ylabel('Amplitude');
title('Radar Received Signal r(t)');
grid on;
xlim([0 6e-6]);
