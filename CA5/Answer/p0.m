clear all;

clc;

t_start = 0;
t_end = 1;
f_s = 20;
ts = 1/f_s;
N = f_s*(t_end - t_start);
t = t_start:ts:t_end - ts;
f = (-f_s/2):(f_s/N):(f_s/2 - f_s/N);

x1 = exp(1i*2*pi*5*t) + exp(1i*2*pi*8*t);
figure;
subplot(1,4,1);
plot(t, abs(x1));
y1 = fftshift(fft(x1));
subplot(1,4,2);
plot(f, abs(y1));

x2 = exp(1i*2*pi*5*t) + exp(1i*2*pi*5.1*t);
subplot(1,4,3);
plot(t, abs(x2));
y2 = fftshift(fft(x2));
subplot(1,4,4);
plot(f, abs(y2));
